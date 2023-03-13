#ifndef ModLoader_H
#define ModLoader_H
#include <json/json.h>
#include <cstring>

using std::string;

struct Mod
{
    Json::Value rawConfig;
    string modDir;
    string id;
    string name;
    string description;
    string version;
    string main;
    Json::Value files;
};

namespace ModLoader {
    extern std::vector<Mod> mods;

    Mod ParseMod(const char *modId);
    std::vector<Mod> ParseMods();
    void RunMods();
}
#endif