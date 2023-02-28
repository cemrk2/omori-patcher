#pragma once
#include "pch.h"
#include "mod.h"
#include <json/json.h>
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
	void Debug(DWORD_PTR addr, size_t len);
    bool PathExists(const char* filename);
    FileData ReadFileData(const char* filename);
    char* ReadFileStr(const char* filename);
    Json::Value ParseJson(const char* str);
    Mod ParseMod(const char* filename);
    std::vector<Mod> ParseMods();
}