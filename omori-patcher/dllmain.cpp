// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <malloc.h>
#include "utils.h"

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
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        PatcherMain();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

