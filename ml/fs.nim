import std/strformat
import modloader

proc AddFileMap(src : cstring, dst : cstring) : void {.importc: "?AddFileMap@@YAXPEBD0@Z", dynlib: "omori-patcher.dll".}

proc RegisterOverlayedFiles(mods : seq[Mod]) =
    for m in mods:
        for file in m.files:
            AddFileMap(cstring(file), cstring(fmt"{m.path}/{file}"))

export RegisterOverlayedFiles