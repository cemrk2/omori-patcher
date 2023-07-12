import std/strformat
import std/strutils
import std/tables
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
        olid*: Table[string, string]

proc parseMod*(m: var Mod, jsonData : string) =
    m.raw = json.parseJson(jsonData)
    m.meta = to(m.raw, ModMeta)

    if m.raw.hasKey("image_deltas"):
        for delta in m.raw["image_deltas"]:
            var src = delta["patch"].getStr()
            var patch = delta["with"].getStr()
            var dir = delta["dir"].getBool()

            if dir:
                Warn(fmt"Directory deltas are not supported, skipping {src}")
            else:
                m.olid[src] = patch


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