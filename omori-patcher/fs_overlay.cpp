#include <mutex>
#include <iostream>
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

void FS_RegisterOverlay(const Mod& mod)
{
    auto assets = mod.files.get("assets", {});
    for (const auto& v : assets)
    {
        auto asset = v.asString();
        auto newAsset = "mods/" + mod.modDir + "/" + asset;
        const char* s = Utils::GetAbsolutePath(asset.c_str());
        const char* newS = Utils::GetAbsolutePath(newAsset.c_str());
        size_t s_newsize = strlen(s) + 1;
        size_t newS_newsize = strlen(newS) + 1;
        auto s_wcstring = new wchar_t[s_newsize];
        auto newS_wcstring = new wchar_t[newS_newsize];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, s_wcstring, s_newsize, s, _TRUNCATE);
        mbstowcs_s(&convertedChars, newS_wcstring, newS_newsize, newS, _TRUNCATE);

        std::wcout << s_wcstring << " -> " << newS_wcstring << L"\n";

        overlay[std::wstring(s_wcstring)] = newS_wcstring;
        free((void*) s);
        free((void*) s_wcstring);
    }
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
                filePtrMap[hFile].QuadPart = binOverlay[filename].size;
                *lpNewFilePointer = filePtrMap[hFile];
                break;
            default:
                Utils::Warnf("Unsupported setFilePointerEx move method: %d", dwMoveMethod);
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
            Utils::Warn("lpOverlapped != nullptr on an overlayed file, thinks might break");
        }
        size_t len = nNumberOfBytesToRead;
        size_t offset = filePtrMap[hFile].QuadPart;
        if (len + offset > binOverlay[filename].size) len = len + offset - binOverlay[filename].size;
        filePtrMap[hFile].QuadPart += nNumberOfBytesToRead;
        if (filePtrMap[hFile].QuadPart >= binOverlay[filename].size) filePtrMap[hFile].QuadPart = 0;
        memcpy(lpBuffer, binOverlay[filename].data+offset, len);
        *lpNumberOfBytesRead = len;
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