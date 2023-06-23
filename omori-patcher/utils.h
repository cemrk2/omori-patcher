#pragma once
#include "pch.h"
#include <cstring>

typedef unsigned int natural;

struct FileData {
    BYTE* data;
    size_t size;
};

namespace Utils
{
	void BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr);
	void Info(const char* msg);
	void Infof(const char* msg, ...);
	void Success(const char* msg);
	void Successf(const char* msg, ...);
	void Warn(const char* msg);
	void Warnf(const char* msg, ...);
	void Error(const char* msg);
	void Errorf(const char* msg, ...);
    bool PathExists(const char* filename);
    FileData ReadFileData(const char* filename);
    char* ReadFileStr(const char* filename);
    bool WriteFileData(const char* filename, void* data, size_t dataLen, bool replaceExisting);
    const char* GetAbsolutePath(const char* p1);
    const wchar_t* GetAbsolutePathW(const wchar_t* p1);
}