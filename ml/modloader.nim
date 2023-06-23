import std/strformat
import std/strutils
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
        jsond*: seq[string]

proc parseMod*(m: var Mod, jsonData : string) =
    m.raw = json.parseJson(jsonData)
    m.meta = to(m.raw, ModMeta)

    if m.raw.hasKey("files"):
        var filesObj = m.raw["files"]
        for type in filesObj.keys:
            var files = filesObj[type]
            for fileNode in files:
                let file = fileNode.getStr()
                let exts = file.split('.')
                var ext = ""
                if exts.len() > 1:
                    ext = exts[len(exts)-1]

                case ext:
                    of "jsond":
                        Info(fmt"[{m.meta.id}] (jsond) path: {file}")
                        m.jsond.add(file)
                    else:
                        Info(fmt"[{m.meta.id}] (static) path: {file}")
                        m.files.add(file)


export ModMeta, Mod