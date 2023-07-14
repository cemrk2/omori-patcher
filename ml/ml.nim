import winim/lean
import std/strformat
import modloader
import utils
import os
import fs
import js

var mods : seq[Mod]

proc mlMain() =
  Success("Nim module initialzied!")

  Info("Searching for mods")
  if not dirExists("mods"):
    Info("No mods directory found, creating one")
    createDir("mods")
  else:
    for file in walkDir("mods", true, true):
      var m = Mod()
      m.path = fmt"mods/{file.path}"
      m.parseMod(readFile(fmt"{m.path}/mod.json"))
      mods.add(m)

  var txt = "mod"
  if len(mods) > 1:
    txt = "mods"
  Info(fmt"Parsed {len(mods)} {txt}")
  for m in mods:
    Info(fmt"- {m.meta.name} ({m.meta.id}) v{m.meta.version}")

  Info("Registering overlay")
  RegisterOverlayedFiles(mods)

proc MlMain() {.stdcall, exportc, dynlib.} =
    try:
      mlMain()
    except CatchableError:
      Error(getCurrentExceptionMsg())

proc JSInit() {.stdcall, exportc, dynlib.} =
  JSInit_cb(mods)

proc NimMain {.cdecl, importc.}

proc DllMain(hinstDLL: HINSTANCE, fdwReason: DWORD, lpvReserved: LPVOID) : BOOL {.stdcall, exportc, dynlib.} =
  NimMain()

  return true