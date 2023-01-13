// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

void PatcherMain()
{
    MessageBox(NULL, L"Hello, World!", L"OMORI Patcher", 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
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

