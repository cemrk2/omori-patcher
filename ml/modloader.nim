import std/strformat
import utils
import json

type
    ModMeta = object
        id*: string
        name*: string
        description*: string
        version*: string

type
    Mod* = object
        raw*: JsonNode
        meta*: ModMeta
        path*: string
        files*: seq[string]

proc parseMod*(m: var Mod, jsonData : string) =
    m.raw = json.parseJson(jsonData)
    m.meta = to(m.raw, ModMeta)

    if m.raw.hasKey("files"):
        var filesObj = m.raw["files"]
        for type in filesObj.keys:
            var files = filesObj[type]
            for fileNode in files:
                var file = fileNode.getStr()
                var typecpy {.inject.} = type
                Info(fmt"[{m.meta.id}] type: {typecpy} path: {file}")
                m.files.add(file)


export ModMeta, Mod