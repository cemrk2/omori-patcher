// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "utils.h"
#include "mem.h"
#include "consts.h"

void JS_EvalHook(void* ctx, char* input, int input_len)
{
    Utils::Infof("JSContext* ctx = 0x%p, input=%s, input_len=%d", ctx, input, input_len);
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Utils::Success("DLL Successfully loaded!");

    Mem::HookOnce(Consts::JS_Eval, (DWORD_PTR) &JS_EvalHook);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

