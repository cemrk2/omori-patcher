// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <string>
#include <utility>
#include <cstring>
#include "js.h"
#include "pch.h"
#include "mem.h"
#include "utils.h"
#include "consts.h"
#include "rpc.h"
#include "detours.h"
#include "fs_overlay.h"

static HMODULE modHandle;

void JS_NewCFunctionHook(JSContext* ctx, void* function, char* name, int length)
{
    if (name != nullptr && *name != 0)
    {
        if (!js::chowFuncs.contains(std::string(name)))
        {
            js::chowFuncs.insert(std::make_pair(std::string(name), js::ChowJSFunction {
                    function,
                    length
            }));
        }
        // Infof("[NewCFunction] JSContext* ctx = 0x%p, function*=%p, name*=%p, name=%s, length=%d", ctx, function, name, name, length);
    }
}

void JS_EvalBinHook(JSContext* ctx, char* filename)
{
    Infof("[evalbin] JSContext* ctx = %p, filename = %s", ctx, filename);
}

void PrintHook(char* msg)
{
    if (strncmp("console.log: getImage", msg, strlen("console.log: getImage")) == 0) return;
    const char* log = "console.log: ";
    const char* warn = "console.warn: ";
    const char* err = "console.error: ";
    // TODO(nemtudom345): This is really hacky, but I can not for the life of me get a native c function to register
    if (strncmp("<omori-patcher>: ", msg, strlen("<omori-patcher>: ")) == 0)
    {
        rpc::ParseMessage(msg + strlen("<omori-patcher>: "));
        return;
    }
    if (strncmp(log, msg, strlen(log)) == 0)
    {
        Infof("[console.log] %s", msg+strlen(log));
        return;
    }
    if (strncmp(warn, msg, strlen(warn)) == 0)
    {
        Warnf("[console.warn] %s", msg+strlen(warn));
        return;
    }
    if (strncmp(err, msg, strlen(err)) == 0)
    {
        Errorf("[console.error] %s", msg+strlen(err));
        return;
    }

    printf("%s\n", msg);
}

void PostEvalBinHook()
{
    js::JSRuntimeInst = (JSRuntime*) (*((JSRuntime**)Consts::JSContextPtr));
    js::JSContextInst = (JSContext*) (*((JSContext**)Consts::JSRuntimePtr));

    Infof("JSRuntime* rt = %p", js::JSRuntimeInst);
    Infof("JSContext* ctx = %p", js::JSContextInst);

    Info("Initializing omori-patcher stdlib");
    js::JS_Eval(Utils::ReadFileStr("stdlib.js"), "stdlib.js");
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Success("DLL Successfully loaded!");

    Mem::Hook(Consts::JS_NewCFunction3, (DWORD_PTR) &JS_NewCFunctionHook, true);
    Mem::Hook(Consts::JS_EvalBin, (DWORD_PTR) &JS_EvalBinHook, true);
    Mem::Hook(Consts::JSImpl_print_i, (DWORD_PTR) &PrintHook, true);
    Mem::Hook(Consts::JSInit_PostEvalBin, (DWORD_PTR) &PostEvalBinHook, false);

    Info("Patching win32 functions...");
    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    FS_RegisterDetours();
    if (DetourTransactionCommit() != NO_ERROR)
    {
        Error("Failed to patch win32 functions");
        return;
    }

    Success("Patching complete");

    Info("Initializing nim module");
    modHandle = LoadLibraryA("ml.dll");
    Infof("modHandle: %p", modHandle);
    if (modHandle == 0 || modHandle == INVALID_HANDLE_VALUE) {
        Error("Failed to initialize nim module!");
        return;
    }
    auto MlMain = GetProcAddress(modHandle, "MlMain");
    Infof("MlMain: %p", MlMain);
    if (MlMain == 0 || MlMain == INVALID_HANDLE_VALUE) {
        Error("Failed to find MlMain!");
        return;
    }
    MlMain();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

