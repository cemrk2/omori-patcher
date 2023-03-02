#ifndef OMORI_PATCHER_JS_H
#define OMORI_PATCHER_JS_H

#include <map>
#include <string>
#include "consts.h"
#include "Chowdren.exe.h"

using std::map;
using std::string;

struct ChowJSFunction
{
    void* function;
    int length;
};

namespace js
{
    typedef void (*JS_NewCFunction2I)(JSContext *ctx, void *function, const char *name, int length, int cproto, int magic);
    const JS_NewCFunction2I JS_NewCFunction2 = (JS_NewCFunction2I)Consts::JS_NewCFunction2;

    extern JSContext* JSContextInst;
    extern JSRuntime* JSRuntimeInst;
    extern map<string, ChowJSFunction> chowNativeFunctions;

    void JS_Eval(const char *code, const char *filename);
    void JS_EvalMod(const char *code, const char *filename);
}

#endif //OMORI_PATCHER_JS_H
