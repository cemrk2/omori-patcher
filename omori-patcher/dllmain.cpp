// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <cstring>
#include "pch.h"
#include "utils.h"
#include "mem.h"
#include "consts.h"
#include "Chowdren.exe.h"

using std::string;

typedef void (*JS_EvalFunc)(JSContext* ctx, const char* buf, size_t buf_len, const char* filename, int eval_flags);

JSContext* JSContextInst;
JSRuntime* JSRuntimeInst;

void JS_Eval(const char* code, const char* filename = "<omori-patcher>")
{
    auto JS_Eval = (JS_EvalFunc) Consts::JS_Eval;
    JS_Eval(JSContextInst, code, strlen(code), filename, 0);
}

void JS_EvalMod(const char* code, const char* filename = "<omori-patcher>")
{
    JS_Eval((string("try { (()=>{ ") + code + " })(); } catch(ex){ console.error(ex); }").c_str(), filename);
}

void JS_NewCFunctionHook(void* ctx, void* function, char* name, int length)
{
    if (name != nullptr && *name != 0)
    {
        // Utils::Infof("[NewCFunction] JSContext* ctx = 0x%p, function*=%p, name*=%p, name=%s, length=%d", ctx, function, name, name, length);
    }
}

void JS_DumpMemoryUsage(FILE* fp,JSMemoryUsage* s,JSRuntime* rt);

void JS_EvalBinHook(JSContext* ctx, char* filename)
{
    Utils::Infof("[evalbin] JSContext* ctx = %p, filename = %s", ctx, filename);
}

void PrintHook(char* msg)
{
    if (strncmp("console.log: getImage", msg, strlen("console.log: getImage")) == 0) return;
    const char* log = "console.log: ";
    const char* warn = "console.warn: ";
    const char* err = "console.error: ";
    if (strncmp(log, msg, strlen(log)) == 0)
    {
        Utils::Infof("[console.log] %s", msg+strlen(log));
        return;
    }
    if (strncmp(warn, msg, strlen(warn)) == 0)
    {
        Utils::Warnf("[console.warn] %s", msg+strlen(warn));
        return;
    }
    if (strncmp(err, msg, strlen(err)) == 0)
    {
        Utils::Errorf("[console.error] %s", msg+strlen(err));
        return;
    }

    Utils::Infof("[print] %s", msg);
}

void PostEvalBinHook()
{
    JSRuntimeInst = (JSRuntime*) (*((JSRuntime**)Consts::JSContextPtr));
    JSContextInst = (JSContext*) (*((JSContext**)Consts::JSRuntimePtr));
    Utils::Info("PostEvalBinHook");
    Utils::Infof("JSRuntime* rt = %p", JSRuntimeInst);
    Utils::Infof("JSContext* ctx = %p", JSContextInst);

    for (auto mod : Utils::ParseMods())
    {
        JS_EvalMod(Utils::ReadFileStr(("mods\\" + mod.modDir + "\\" + mod.main).c_str()), mod.main.c_str());
    }
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Utils::Success("DLL Successfully loaded!");

    Mem::Hook(Consts::JS_NewCFunction, (DWORD_PTR) &JS_NewCFunctionHook, true);
    Mem::Hook(Consts::JS_EvalBin, (DWORD_PTR) &JS_EvalBinHook, true);
    Mem::Hook(Consts::JSImpl_print_i, (DWORD_PTR) &PrintHook, true);
    Mem::Hook(Consts::JSInit_PostEvalBin, (DWORD_PTR) &PostEvalBinHook, false);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

