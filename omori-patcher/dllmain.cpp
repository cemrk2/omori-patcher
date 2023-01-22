// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "utils.h"
#include "mem.h"
#include "consts.h"

void test()
{
    Utils::Info("hi");
}

void PatcherMain()
{
    AllocConsole();
    SetConsoleTitle(L"OMORI Patcher Logs");

    // Restore stdout, stderr and stdin
    Utils::BindCrtHandlesToStdHandles(true, true, true);

    Utils::Success("DLL Successfully loaded!");
    Utils::Debug(Consts::JS_Eval, 20);
    
    Mem::Hook(Consts::JS_Eval, (DWORD_PTR) &test);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        PatcherMain();
    }
    return TRUE;
}

