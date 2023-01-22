#pragma once
#include "pch.h"

typedef unsigned int natural;

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
}