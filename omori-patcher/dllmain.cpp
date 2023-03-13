// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <cstring>
#include "js.h"
#include "pch.h"
#include "mem.h"
#include "utils.h"
#include "consts.h"
#include "modloader.h"
#include "rpc.h"
#include "detours.h"
#include "fs_overlay.h"

void JS_NewCFunctionHook(JSContext* ctx, void* function, char* name, int length)
{
    if (name != nullptr && *name != 0)
    {
        if (!js::chowFuncs.contains(string(name)))
        {
            js::chowFuncs.insert(make_pair(string(name), js::ChowJSFunction {
                    function,
                    length
            }));
        }
        // Utils::Infof("[NewCFunction] JSContext* ctx = 0x%p, function*=%p, name*=%p, name=%s, length=%d", ctx, function, name, name, length);
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
    // TODO(nemtudom345): This is really hacky, but I can not for the life of me get a native c function to register
    if (strncmp("<omori-patcher>: ", msg, strlen("<omori-patcher>: ")) == 0)
    {
        rpc::ParseMessage(msg + strlen("<omori-patcher>: "));
        return;
    }
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
    js::JSRuntimeInst = (JSRuntime*) (*((JSRuntime**)Consts::JSContextPtr));
    js::JSContextInst = (JSContext*) (*((JSContext**)Consts::JSRuntimePtr));

    Utils::Infof("JSRuntime* rt = %p", js::JSRuntimeInst);
    Utils::Infof("JSContext* ctx = %p", js::JSContextInst);

    Utils::Info("Initializing omori-patcher stdlib");
    js::JS_Eval(Utils::ReadFileStr("stdlib.js"), "stdlib.js");

    Utils::Info("Running mods...");
    ModLoader::RunMods();
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

    Utils::Info("Patching win32 functions...");
    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    FS_RegisterDetours();
    if (DetourTransactionCommit() != NO_ERROR)
    {
        Utils::Error("Failed to patch win32 functions");
        return;
    }
    Utils::Success("Patching complete");

    Utils::Info("Parsing mods...");
    ModLoader::mods = ModLoader::ParseMods();
    Utils::Successf("Parsed %d %s", ModLoader::mods.size(), ModLoader::mods.size() == 1 ? "mod" : "mods");
    Utils::Info("Registering files for fs overlay");
    for (const Mod& mod : ModLoader::mods)
    {
        FS_RegisterOverlay(mod);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

