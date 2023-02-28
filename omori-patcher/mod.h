#ifndef OMORI_PATCHER_MOD_H
#define OMORI_PATCHER_MOD_H

#include <json/json.h>
#include <cstring>

using std::string;

struct Mod
{
    Json::Value rawConfig;
    string id;
    string name;
    string description;
    string version;
    string main;
};

#endif //OMORI_PATCHER_MOD_H
