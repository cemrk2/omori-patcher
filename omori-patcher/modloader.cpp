#include "js.h"
#include "utils.h"
#include "consts.h"
#include "modloader.h"

namespace ModLoader
{
    Mod ParseMod(const char* modId)
    {
        string infopath = string("mods\\") + modId + "\\mod.json";
        Json::Value root;
        if (!Utils::PathExists(infopath.c_str()))
        {
            Utils::Warnf("Mod: %s doesn't have a mod.json, skipping", modId);
            return {root, string(modId)};
        }
        root = Utils::ParseJson(Utils::ReadFileStr(infopath.c_str()));

        return {
                root,
                string(modId),
                root["id"].asString(),
                root["name"].asString(),
                root["description"].asString(),
                root["version"].asString(),
                root["main"].asString(),
        };
    }

    std::vector<Mod> ParseMods()
    {
        std::vector<Mod> mods;

        HANDLE handle;
        WIN32_FIND_DATAA finfo;

        if((handle = FindFirstFileA("mods/*", &finfo)) != INVALID_HANDLE_VALUE){
            do{
                auto name = finfo.cFileName;
                if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
                {
                    continue;
                }
                auto mod = ParseMod(name);
                if (!mod.id.empty()) mods.push_back(mod);
            }while(FindNextFileA(handle, &finfo));
            FindClose(handle);
        }

        return mods;
    }

    void LoadMods()
    {
        for (const auto& mod : ParseMods())
        {
            js::JS_EvalMod(Utils::ReadFileStr(("mods\\" + mod.modDir + "\\" + mod.main).c_str()), (mod.modDir + "/" + mod.main).c_str());
        }
    }

    void processMessage(int funcId, const Json::Value& value)
    {
        switch (funcId) {
            case 1:
                Utils::Infof("mp_pre: name: %s callback: %s", value["name"].asCString(), value["callback"].asCString());
                break;
            case 2:
                Utils::Infof("mp_replace: name: %s callback: %s", value["name"].asCString(), value["callback"].asCString());
                break;
            case 3:
                Utils::Infof("mp_post: name: %s callback: %s", value["name"].asCString(), value["callback"].asCString());
                break;
            case 4:
                Utils::Infof("mp_commit: name: %s", value["name"].asCString());
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