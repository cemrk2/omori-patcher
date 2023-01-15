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
    Utils::Debug(Consts::initialEval, 20);
    HookResult* resPtr = (HookResult*) malloc(sizeof(HookResult{0,0}));
    if (resPtr == NULL)
    {
        Utils::Error("Failed to allocate HookResult, aborting...");
        return;
    }
    HookResult res = Mem::Hook(Consts::initialEval, (DWORD_PTR) &test, false);
    // TODO: Actually fix this
    /*
    BYTE* backup = new BYTE[12]
    {
        0xE8, 0xDB, 0xB5, 0xB8, 0xFF,
        0x48, 0x8B, 0x0D, 0x48, 0x32, 0x58, 0x00
    };
    memcpy((void*)((DWORD_PTR)res.trampolinePtr + 12), backup, 12); */
    Utils::Infof("backupPtr: 0x%p trampolinePtr: 0x%p", res.backupPtr, res.trampolinePtr);
    
    Utils::Debug(Consts::initialEval, 20);
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

