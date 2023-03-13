#include <mutex>
#include <iostream>
#include "fs_overlay.h"
#include "utils.h"
#include "detours.h"

static BOOL (WINAPI* trueSetFilePointerEx)(HANDLE hFile, _LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod) = SetFilePointerEx;
static HANDLE (WINAPI* trueCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;
static BOOL (WINAPI* trueReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = ReadFile;

std::map<std::wstring, std::wstring> overlay;
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
        wchar_t* s_wcstring = new wchar_t[s_newsize];
        wchar_t* newS_wcstring = new wchar_t[newS_newsize];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, s_wcstring, s_newsize, s, _TRUNCATE);
        mbstowcs_s(&convertedChars, newS_wcstring, newS_newsize, newS, _TRUNCATE);

        std::wcout << s_wcstring << " -> " << newS_wcstring << L"\n";

        overlay[std::wstring(s_wcstring)] = newS_wcstring;
        free((void*) s);
        free((void*) s_wcstring);
    }
    Utils::Infof("%d", overlay.size());
}

HANDLE WINAPI hookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    if (overlay.contains(Utils::GetAbsolutePathW(lpFileName)))
    {
        return trueCreateFileW(overlay[Utils::GetAbsolutePathW(lpFileName)].c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    return trueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

void FS_RegisterDetours()
{
    DetourAttach(&(PVOID &) trueCreateFileW, (PVOID) hookedCreateFileW);
}