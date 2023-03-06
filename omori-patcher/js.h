#ifndef OMORI_PATCHER_JS_H
#define OMORI_PATCHER_JS_H

#include <map>
#include <string>
#include <vector>
#include "quickjs.h"

#define JS_VALUE_GET_TAG(v) (int)((uintptr_t)(v) & 0xf)
#define JS_VALUE_GET_PTR(v) (void *)((intptr_t)(v) & ~0xf)
#define JS_VALUE_GET_OBJ(v) ((JSObject *)JS_VALUE_GET_PTR(v))

namespace js
{
    struct ChowJSFunction
    {
        void* function;
        int length;
    };

    struct FunctionBackup
    {
        void* addr;
        size_t size;
        void* backup;
    };

    enum JSHookType
    {
        PRE,
        REPLACE,
        POST
    };

    struct JSHook
    {
        JSHookType type;
        void* hook;
    };

    extern JSContext* JSContextInst;
    extern JSRuntime* JSRuntimeInst;
    extern std::map<std::string, ChowJSFunction> chowFuncs;
    extern std::map<std::string, FunctionBackup> chowBackups;
    extern std::map<std::string, std::vector<JSHook>> chowHooks;

    void JS_NewCFunction(JSContext* ctx, void* function, const char* name, int length);
    JSValue JS_GetGlobalVar(JSContext* ctx, JSAtom prop, bool throw_ref_error);
    JSAtom JS_NewAtom(JSContext* ctx, const char* str, size_t len);
    void JS_Eval(const char* code, const char *filename);
    void JS_EvalMod(const char* code, const char *filename);
}

#endif //OMORI_PATCHER_JS_H
