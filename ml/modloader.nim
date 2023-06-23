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

proc parseMod*(m: var Mod, jsonData : string) =
    m.raw = json.parseJson(jsonData)
    m.meta = to(m.raw, ModMeta)

export ModMeta, Mod