// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "utils.h"
#include "mem.h"
#include "consts.h"

void JS_NewCFunctionHook(void* ctx, void* function, char* name, int length)
{
    if (name != nullptr && *name != 0)
    {
        Utils::Infof("JSContext* ctx = 0x%p, function*=%p, name*=%p, name=%s, length=%d", ctx, function, name, name, length);
    }
}

void JS_EvalHook(void* ctx,char* buf,size_t buf_len,char* filename,int eval_flags)
{
    Utils::Infof("JSContext* ctx = %p,char* buf = %s,size_t buf_len = %d,char* filename = %s,int eval_flags = %d", ctx, buf, buf_len, filename, eval_flags);
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Utils::Success("DLL Successfully loaded!");

    Mem::Hook(Consts::JS_NewCFunction, Consts::JS_NewCFunctionOffset, (DWORD_PTR) &JS_NewCFunctionHook);
    Mem::Hook(Consts::JS_Eval, Consts::JS_EvalOffset, (DWORD_PTR) &JS_EvalHook);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

