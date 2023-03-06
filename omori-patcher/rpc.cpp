#include "rpc.h"
#include "utils.h"
#include "js.h"
#include <json/json.h>
#include <string>

using std::string;

namespace rpc
{
    void hookState(js::JSHookType type, const string& name, const string& hookName)
    {
        if (!js::chowFuncs.contains(name))
        {
            Utils::Warnf("Function: %s not found, not hooking", name.c_str());
            return;
        }
        if (!js::chowHooks.contains(name))
        {
            js::chowHooks[name] = {};
        }
        js::chowHooks[name].push_back(js::JSHook{
                type,
                NULL
        });
        JSAtom hookNameAtom = js::JS_NewAtom(js::JSContextInst, hookName.c_str(), hookName.length());
        Utils::Infof("atom=%d", hookNameAtom);
        // JSValue hookFunc = js::JS_GetGlobalVar(js::JSContextInst, hookNameAtom, true);
        // Utils::Infof("hookFunc*=%p tag=0x%llx int32=%d float64=%f ptr=%p", hookFunc.tag, hookFunc.u.int32, hookFunc.u.float64, hookFunc.u.ptr);
    }

    void hookCommit(const string& name)
    {
        for (const auto &item: js::chowHooks[name])
        {
            Utils::Infof("%s %d %p", name.c_str(), item.type, item.hook);
        }
    }

    void processMessage(int funcId, const Json::Value& value)
    {
        switch (funcId) {
            case 1:
                hookState(js::JSHookType::PRE, value["name"].asString(), value["callback"].asString());
                break;
            case 2:
                hookState(js::JSHookType::REPLACE, value["name"].asString(), value["callback"].asString());
                break;
            case 3:
                hookState(js::JSHookType::POST, value["name"].asString(), value["callback"].asString());
                break;
            case 4:
                hookCommit(value["name"].asString());
                break;
            default:
                Utils::Warnf("Unknown function id: %d, ignoring", funcId);
                break;
        }
    }

    void ParseMessage(const char* msg)
    {
        auto str = Utils::ParseJson(msg);
        auto func = str.get("func", Json::Value(0)).asInt();

        if (func != 0)
        {
            processMessage(func, str["data"]);
        }
    }
}