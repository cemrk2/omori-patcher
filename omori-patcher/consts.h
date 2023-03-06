#pragma once
#include "pch.h"

namespace Consts {
    const int RESET = 7;
    const int SUCCESS = 10;
    const int INFO = 11;
    const int ERR = 12;
    const int WARN = 14;

    const DWORD_PTR JSContextPtr = 0x000000014316F3A8;
    const DWORD_PTR JSRuntimePtr = 0x000000014316F3B0;

    const DWORD_PTR OMORI_BASE = 0x0000000140000000;
    const DWORD_PTR codecave = 0x0000000142BEC625;
    const DWORD_PTR codecaveEnd = 0x0000000142BEE000;

    const DWORD_PTR JSInit_PostEvalBin = 0x0000000142825A2F;
    const DWORD_PTR JSImpl_print_i = 0x0000000142821ED1;
    const DWORD_PTR JS_Eval = 0x0000000142777C70;
    const DWORD_PTR JS_EvalBin = 0x0000000142777b34;
    const DWORD_PTR JS_NewCFunction2 = 0x00000001426B1A24;
    const DWORD_PTR JS_NewCFunction3 = 0x00000001426B1A54;
    const DWORD_PTR JS_NewAtom = 0x00000001426B1770;
    const DWORD_PTR JS_GetGlobalVar = 0x00000001426ACFAC;
}