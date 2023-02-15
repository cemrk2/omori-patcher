// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "utils.h"
#include "mem.h"
#include "consts.h"

void JS_NewCFunctionHook(void* ctx, void* function, char* name, int length)
{
    Utils::Infof("JSContext* ctx = 0x%p, function*=%p, name=%s, length=%d", ctx, function, name, length);
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Utils::Success("DLL Successfully loaded!");

    Mem::Hook(Consts::JS_NewCFunction, Consts::JS_NewCFunctionOffset, (DWORD_PTR) &JS_NewCFunctionHook);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

