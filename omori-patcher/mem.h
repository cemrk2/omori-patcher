#pragma once
#include "pch.h"
#include "consts.h"

typedef unsigned int natural;

struct HookResult
{
    void* trampolinePtr;
    void* backupPtr;
};

static DWORD_PTR mallocI = Consts::codecave;

namespace Mem
{
    void Write(DWORD_PTR addr, void* input, size_t len);
    void* CreateCall(DWORD_PTR addr);
    void Hook(DWORD_PTR insn, DWORD_PTR targetFn);
}