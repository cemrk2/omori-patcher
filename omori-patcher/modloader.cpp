#include "js.h"
#include "utils.h"
#include "consts.h"
#include "modloader.h"

namespace ModLoader
{

    std::vector<Mod> mods;

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
                root.get("main", "").asString(),
                root.get("files", {})
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

    void RunMods()
    {
        for (const auto& mod : mods)
        {
            if (!mod.main.empty()) js::JS_EvalMod(Utils::ReadFileStr(("mods\\" + mod.modDir + "\\" + mod.main).c_str()), (mod.modDir + "/" + mod.main).c_str());
        }
    }
}