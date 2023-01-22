#pragma once
#include "pch.h"

typedef unsigned int natural;

typedef struct HookResult
{
	void* trampolinePtr;
	void* backupPtr;
};

namespace Consts {
	const int RESET = 7;
	const int SUCCESS = 10;
	const int INFO = 11;
	const int ERR = 12;
	const int WARN = 14;

	const DWORD_PTR codecave = 0x0000000142BEC100;
	const DWORD_PTR codecaveEnd = 0x0000000142BED0B5;
	const DWORD_PTR JS_Eval = 0x00000001427776EC+1;
}

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

namespace Mem
{
	void Write(DWORD_PTR addr, void* input, size_t len);
	void* CreateCall(DWORD_PTR addr);
	void Hook(DWORD_PTR insn, DWORD_PTR targetFn);
}

