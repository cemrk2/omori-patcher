#pragma once
#include "pch.h"
#include <cstring>

typedef unsigned int natural;

struct FileData {
    BYTE* data;
    size_t size;
};

__declspec(dllexport) void Info(const char* msg);
__declspec(dllexport) void Infof(const char* msg, ...);
__declspec(dllexport) void Success(const char* msg);
__declspec(dllexport) void Successf(const char* msg, ...);
__declspec(dllexport) void Warn(const char* msg);
__declspec(dllexport) void Warnf(const char* msg, ...);
__declspec(dllexport) void Error(const char* msg);
__declspec(dllexport) void Errorf(const char* msg, ...);

namespace Utils
{
	void BindCrtHandlesToStdHandles();
    bool PathExists(const char* filename);
    FileData ReadFileData(const char* filename);
    char* ReadFileStr(const char* filename);
    bool WriteFileData(const char* filename, void* data, size_t dataLen, bool replaceExisting);
    const char* GetAbsolutePath(const char* p1);
    const wchar_t* GetAbsolutePathW(const wchar_t* p1);
}