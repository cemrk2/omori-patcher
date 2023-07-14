# import std/strformat
import modloader
import utils

proc c_JS_EvalMod(code : cstring, filename : cstring) : void {.importc: "?JS_EvalMod@@YAXPEBD0@Z", dynlib: "omori-patcher.dll".}

proc JS_EvalMod(code : string, filename : string) =
    c_JS_EvalMod(cstring(code), cstring(filename))

proc JSInit_cb(mods : seq[Mod]) =
    for m in mods:
      if m.hasMain:
        try:
            JS_EvalMod(m.mainScript, m.main)
        except CatchableError:
            Error(getCurrentExceptionMsg())

    

export JS_EvalMod, JSInit_cb