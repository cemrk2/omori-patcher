#pragma once
#include "pch.h"

namespace Consts {
    const int RESET = 7;
    const int SUCCESS = 10;
    const int INFO = 11;
    const int ERR = 12;
    const int WARN = 14;

    const DWORD_PTR OMORI_BASE = 0x0000000140000000;
    const DWORD_PTR codecave = 0x00007FF76D60C621;
    const DWORD_PTR codecaveEnd = 0x00007FF76D60D625;


    const DWORD_PTR JS_NewCFunction = 0x00007FF76D0D1A54;
    const DWORD_PTR JS_NewCFunctionOffset = 1;
}