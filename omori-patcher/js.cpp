#include <string>
#include <cstring>
#include "js.h"
#include "consts.h"

using std::string;

namespace js
{
    typedef void (*JS_EvalFunc)(JSContext* ctx, const char* buf, size_t buf_len, const char* filename, int eval_flags);

    JSContext* JSContextInst;
    JSRuntime* JSRuntimeInst;

    void JS_Eval(const char *code, const char *filename = "<omori-patcher>") {
        auto JS_Eval = (JS_EvalFunc) Consts::JS_Eval;
        JS_Eval(JSContextInst, code, strlen(code), filename, 0);
    }

    void JS_EvalMod(const char *code, const char *filename = "<omori-patcher>") {
        char *filenameJS = (char *) malloc(
                strlen(filename) + 1); // this part of the code is going to come back to haunt me one day
        memcpy(filenameJS, filename, strlen(filename) + 1);
        for (size_t i = 0; i < strlen(filenameJS); i++) {
            if (filenameJS[i] == '\\') filenameJS[i] = '/';
            if (filenameJS[i] == '\'') filenameJS[i] = '"';
        }
        JS_Eval((string("try { (()=>{ ") + code + " })(); } catch(ex){ print('Failed to run script: " + filenameJS +
                 "'); console.error(ex); }").c_str(), filename);
        free(filenameJS);
    }
}