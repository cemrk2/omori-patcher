// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <cstring>
#include "js.h"
#include "pch.h"
#include "mem.h"
#include "utils.h"
#include "consts.h"
#include "modloader.h"

void JS_NewCFunctionHook(void* ctx, void* function, char* name, int length, int magic, short data_len, uint64_t unk)
{
    if (name != nullptr && *name != 0)
    {
        if (js::JSContextInst == nullptr)
        {
            js::JSRuntimeInst = (JSRuntime * )(*((JSRuntime **) Consts::JSContextPtr));
            js::JSContextInst = (JSContext * )(*((JSContext **) Consts::JSRuntimePtr));
        }

        if (!js::chowNativeFunctions.contains(string(name)))
        {
            js::chowNativeFunctions.insert(make_pair(string(name), ChowJSFunction {
                function,
                length
            }));
        }
        Utils::Infof("[NewCFunction] JSContext* ctx = 0x%p, function*=%p, name*=%p, name=%s, length=%d magic=%d data_len=%d unk=%d", ctx, function, name, name, length, magic, data_len, unk);
    }
}

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

    printf("%s\n", msg);
}

void PostEvalBinHook()
{
    Utils::Infof("JSRuntime* rt = %p", js::JSRuntimeInst);
    Utils::Infof("JSContext* ctx = %p", js::JSContextInst);

    Utils::Info("Register native functions...");
    ModLoader::Register();

    ModLoader::LoadMods();
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Utils::Success("DLL Successfully loaded!");

    Mem::Hook(Consts::JS_NewCFunction3, (DWORD_PTR) &JS_NewCFunctionHook, true);
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

