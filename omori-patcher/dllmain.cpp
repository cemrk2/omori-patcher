// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include "pch.h"
#include "utils.h"
#include "mem.h"
#include "consts.h"
#include "Chowdren.exe.h"

JSContext* JSContextInst;
JSRuntime* JSRuntimeInst;

/*
void JS_NewCFunctionHook(void* ctx, void* function, char* name, int length)
{
    if (name != nullptr && *name != 0)
    {
        Utils::Infof("[NewCFunction] JSContext* ctx = 0x%p, function*=%p, name*=%p, name=%s, length=%d", ctx, function, name, name, length);
    }
}
*/

void JS_DumpMemoryUsage(FILE* fp,JSMemoryUsage* s,JSRuntime* rt);

void JS_EvalHook(void* ctx,char* buf,size_t buf_len,char* filename,int eval_flags)
{
    Utils::Infof("[eval] JSContext* ctx = %p,char* buf = %s,size_t buf_len = %d,char* filename = %s,int eval_flags = %d", ctx, buf, buf_len, filename, eval_flags);
}

void JS_EvalBinHook(void* ctx, char* filename)
{
    Utils::Infof("[evalbin] JSContext* ctx = %p, filename = %s", ctx, filename);
}

void PrintHook(char* msg)
{
    Utils::Infof("[print] %s", msg);
}

void PostEvalBinHook()
{
    Utils::Info("PostEvalBinHook");
    Utils::Infof("JSRuntime* rt = %p", JSRuntimeInst);
    Utils::Infof("JSContext* ctx = %p", JSContextInst);
}

void PostNewRuntime2(JSRuntime* rt)
{
    JSRuntimeInst = rt;
}

void PostNewContextRaw(JSContext* ctx)
{
    JSContextInst = ctx;
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Utils::Success("DLL Successfully loaded!");

    // Mem::Hook(Consts::JS_NewCFunction, Consts::JS_NewCFunctionOffset, (DWORD_PTR) &JS_NewCFunctionHook);
    Mem::Hook(Consts::JS_Eval, (DWORD_PTR) &JS_EvalHook, true);
    Mem::Hook(Consts::JS_EvalBin, (DWORD_PTR) &JS_EvalBinHook, true);
    Mem::Hook(Consts::JSImpl_print_i, (DWORD_PTR) &PrintHook, true);
    Mem::Hook(Consts::JSInit_PostEvalBin, (DWORD_PTR) &PostEvalBinHook, false);

    Mem::HookAssembly(Consts::JS_NewRuntime2, (DWORD_PTR) &PostNewRuntime2, false, [](zasm::x86::Assembler a) {
        a.mov(zasm::x86::rcx, zasm::x86::rax);
    });
    Mem::HookAssembly(Consts::JS_NewContextRaw, (DWORD_PTR) &PostNewContextRaw, false, [](zasm::x86::Assembler a) {
        a.mov(zasm::x86::rcx, zasm::x86::rax);
    });
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

