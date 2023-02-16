#pragma once
#include "pch.h"

namespace Consts {
    const int RESET = 7;
    const int SUCCESS = 10;
    const int INFO = 11;
    const int ERR = 12;
    const int WARN = 14;

    const DWORD_PTR OMORI_BASE = 0x0000000140000000;
    const DWORD_PTR codecave = 0x0000000142BEC625;
    const DWORD_PTR codecaveEnd = 0x0000000142BED625;

    const DWORD_PTR JS_Eval = 0x0000000142777C70;
    const int JS_EvalOffset = 1;
    const DWORD_PTR JS_NewCFunction = 0x00000001426B1A54;
    const int JS_NewCFunctionOffset = 1;
}