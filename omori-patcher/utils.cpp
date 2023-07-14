#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include <codecvt>
#include "utils.h"
#include "pch.h"
#include "io.h"
#include "consts.h"

using std::string;

#ifndef _UTILS_H
#define _UTILS_H

__declspec(dllexport) void Info(const char* msg)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::INFO);
    std::cout << "[INFO] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);
    std::cout << msg << std::endl;
}

__declspec(dllexport) void Infof(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::INFO);
    std::cout << "[INFO] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);

    vprintf(format, argptr);
    va_end(argptr);
    std::cout << std::endl;
}

__declspec(dllexport) void Success(const char* msg)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::SUCCESS);
    std::cout << "[SUCCESS] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);
    std::cout << msg << std::endl;
}

__declspec(dllexport) void Successf(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::SUCCESS);
    std::cout << "[SUCCESS] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);

    vprintf(format, argptr);
    va_end(argptr);
    std::cout << std::endl;
}

__declspec(dllexport) void Warn(const char* msg)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::WARN);
    std::cout << "[WARN] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);
    std::cout << msg << std::endl;
}

__declspec(dllexport) void Warnf(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::WARN);
    std::cout << "[WARN] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);

    vprintf(format, argptr);
    va_end(argptr);
    std::cout << std::endl;
}

__declspec(dllexport) void Error(const char* msg)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::ERR);
    std::cout << "[ERROR] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);
    std::cout << msg << std::endl;
}

__declspec(dllexport) void Errorf(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, Consts::ERR);
    std::cout << "[ERROR] ";
    SetConsoleTextAttribute(hConsole, Consts::RESET);

    vprintf(format, argptr);
    va_end(argptr);
    std::cout << std::endl;
}

#endif

namespace Utils
{
    void rebindHandle(DWORD targetStream)
    {
        HANDLE stdHandle = GetStdHandle(targetStream);
        if (stdHandle != INVALID_HANDLE_VALUE) {
            int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
            if (fileDescriptor != -1) {
                const char* mode = targetStream == STD_INPUT_HANDLE ? "r" : "w";
                FILE* file = _fdopen(fileDescriptor, mode);
                if (file != nullptr) {
                    int targetNo;
                    switch (targetStream) {
                        case STD_INPUT_HANDLE:
                            targetNo = _fileno(stdin);
                            break;
                        
                        case STD_OUTPUT_HANDLE:
                            targetNo = _fileno(stdout);
                            break;

                        case STD_ERROR_HANDLE:
                            targetNo = _fileno(stderr);
                            break;
                        
                        default:
                            break;
                    }
                    int dup2Result = _dup2(_fileno(file), targetNo);
                    if (dup2Result == 0) {
                        setvbuf(stdin, nullptr, _IONBF, 0);
                    }
                }
            }
        }
    }

    // https://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows
    void BindCrtHandlesToStdHandles()
    {
        FILE* dummyFile;
        freopen_s(&dummyFile, "nul", "r", stdin);
        freopen_s(&dummyFile, "nul", "w", stdout);
        freopen_s(&dummyFile, "nul", "w", stderr);

        rebindHandle(STD_INPUT_HANDLE);
        rebindHandle(STD_OUTPUT_HANDLE);
        rebindHandle(STD_ERROR_HANDLE);

        std::wcin.clear();
        std::cin.clear();

        std::wcout.clear();
        std::cout.clear();

        std::wcerr.clear();
        std::cerr.clear();
    }
    
    bool PathExists(const char* path)
    {
        return GetFileAttributesA(path) != 0;
    }

    FileData ReadFileData(const char* filename)
    {
        OFSTRUCT finfo;
        auto handle = (HANDLE) OpenFile(filename, &finfo, OF_READ);
        if (handle == nullptr) {
            Errorf("Failed to open file for reading: %s", filename);
            return {
                nullptr,
                0
            };
        }

        DWORD size = GetFileSize(handle, nullptr);
        void* buffer = malloc(size);

        if (!ReadFile(handle, buffer, size, nullptr, nullptr)) {
            Errorf("Failed to read file: %s", filename);
            return {
                    nullptr,
                    size
            };
        }
        CloseHandle(handle);

        return {
            (BYTE*)buffer,
            size
        };
    }

    char* ReadFileStr(const char* filename)
    {
        auto file = ReadFileData(filename);
        char* newBuffer = (char*) malloc(file.size + 1);
        memset(newBuffer, 0, file.size+1);
        memcpy(newBuffer, file.data, file.size);
        free(file.data);
        return newBuffer;
    }

    bool WriteFileData(const char* filename, void* data, size_t dataLen, bool replaceExisting)
    {
        if (Utils::PathExists(filename)) {
            if (replaceExisting) {
                DeleteFileA(filename);
            } else {
                return true;
            }
        }
        auto handle = (HANDLE) CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (handle == nullptr) {
            Errorf("Failed to open file for writing: %s", filename);
            return false;
        }

        if (!WriteFile(handle, data, dataLen, NULL, NULL)) {
            Errorf("Failed to write data to file: %s", filename);
        }

        CloseHandle(handle);

        return true;
    }

    const char* GetAbsolutePath(const char* p1)
    {
        TCHAR newPath[MAX_PATH] = TEXT("");
        TCHAR** lppPart = {NULL};
        GetFullPathNameA(p1, MAX_PATH, (LPSTR)newPath, (LPSTR*)lppPart);
        char* buff = (char*) malloc(MAX_PATH+1);
        memcpy(buff, newPath, MAX_PATH+1);
        return buff;
    }

    const wchar_t* GetAbsolutePathW(const wchar_t* p1)
    {
        TCHAR newPath[4096] = TEXT("");
        TCHAR** lppPart = {NULL};
        GetFullPathNameW(p1, 4096, (LPWSTR)newPath, (LPWSTR*)lppPart);
        wchar_t* buff = (wchar_t*) malloc(4096+1);
        memcpy(buff, newPath, 4096+1);
        return buff;
    }
}