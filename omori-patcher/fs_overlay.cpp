#include <map>
#include <locale>
#include <string>
#include <codecvt>
#include <mutex>
#include "fs_overlay.h"
#include "utils.h"
#include "detours.h"

static BOOL (WINAPI* trueSetFilePointerEx)(HANDLE hFile, _LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod) = SetFilePointerEx;
static HANDLE (WINAPI* trueCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;
static BOOL (WINAPI* trueReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = ReadFile;

std::map<std::wstring, std::wstring> overlay;
std::map<std::wstring, FileData> binOverlay;
std::map<HANDLE, std::wstring> fileMap;
std::map<HANDLE, _LARGE_INTEGER> filePtrMap;
std::mutex mapMutex;

// https://gist.github.com/rosasurfer/33f0beb4b10ff8a8c53d943116f8a872
std::wstring convert(std::string str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

__declspec(dllexport) void AddFileMap(const char* src, const char* dst)
{
    auto wide_src = std::wstring(Utils::GetAbsolutePathW(convert(std::string(src)).c_str()));
    auto wide_dst = std::wstring(Utils::GetAbsolutePathW(convert(std::string(dst)).c_str()));

    overlay.insert(std::pair(wide_src, wide_dst));
}

__declspec(dllexport) void AddBinFile(const char* path, size_t size, void* bin)
{
    auto wide_path = std::wstring(Utils::GetAbsolutePathW(convert(std::string(path)).c_str()));
    void* cpy = malloc(size);
    memcpy(cpy, bin, size); // This is done to make sure that the data doesn't get accidentalyl gc'd

    binOverlay.insert(std::pair(wide_path, FileData{
        (BYTE*) cpy,
        size
    }));
}

BOOL WINAPI hookedSetFilePointerEx(HANDLE hFile, _LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    auto filename = fileMap[hFile];
    if (binOverlay.contains(filename))
    {
        if (!filePtrMap.contains(hFile)) filePtrMap[hFile] = _LARGE_INTEGER{};
        switch (dwMoveMethod) {
            case FILE_BEGIN:
                filePtrMap[hFile].QuadPart = 0;
                *lpNewFilePointer = filePtrMap[hFile];
                break;
            case FILE_CURRENT:
                *lpNewFilePointer = filePtrMap[hFile];
                break;
            case FILE_END:
                filePtrMap[hFile].QuadPart = (long long) binOverlay[filename].size;
                *lpNewFilePointer = filePtrMap[hFile];
                break;
            default:
                Warnf("Unsupported setFilePointerEx move method: %d", dwMoveMethod);
        }
        return true;
    }
    return trueSetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
}

HANDLE WINAPI hookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    HANDLE handle;
    if (overlay.contains(Utils::GetAbsolutePathW(lpFileName)))
    {
        handle = trueCreateFileW(overlay[Utils::GetAbsolutePathW(lpFileName)].c_str(), dwDesiredAccess, dwShareMode,
                                 lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    else {
        handle = trueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                                 dwFlagsAndAttributes, hTemplateFile);
    }
    fileMap[handle] = Utils::GetAbsolutePathW(lpFileName);
    return handle;
}

BOOL WINAPI hookedReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    auto filename = fileMap[hFile];
    if (binOverlay.contains(filename))
    {
        if (lpOverlapped != nullptr)
        {
            Warn("lpOverlapped != nullptr on an overlayed file, thinks might break");
        }
        size_t len = nNumberOfBytesToRead;
        size_t offset = filePtrMap[hFile].QuadPart;
        if (len + offset > binOverlay[filename].size) len = len + offset - binOverlay[filename].size;
        filePtrMap[hFile].QuadPart += nNumberOfBytesToRead;
        if ((size_t)filePtrMap[hFile].QuadPart >= binOverlay[filename].size) filePtrMap[hFile].QuadPart = len;
        memcpy(lpBuffer, binOverlay[filename].data+offset, len);
        *lpNumberOfBytesRead = (DWORD)len;
        return true;
    }
    return trueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

void FS_RegisterDetours()
{
    DetourAttach(&(PVOID &) trueCreateFileW, (PVOID) hookedCreateFileW);
    DetourAttach(&(PVOID &) trueReadFile, (PVOID) hookedReadFile);
    DetourAttach(&(PVOID &) trueSetFilePointerEx, (PVOID) hookedSetFilePointerEx);
}