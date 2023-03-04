#ifndef OMORI_PATCHER_JS_H
#define OMORI_PATCHER_JS_H

#include "Chowdren.exe.h"

namespace js
{
    extern JSContext* JSContextInst;
    extern JSRuntime* JSRuntimeInst;

    void JS_NewCFunction(JSContext* ctx, void* function, const char* name, int length);
    void JS_Eval(const char* code, const char *filename);
    void JS_EvalMod(const char* code, const char *filename);
}

#endif //OMORI_PATCHER_JS_H
