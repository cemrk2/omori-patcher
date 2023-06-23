import winim/lean
import utils

proc MlMain() {.stdcall, exportc, dynlib.} =
    Info("Hello, Nim!")

proc NimMain {.cdecl, importc.}

proc DllMain(hinstDLL: HINSTANCE, fdwReason: DWORD, lpvReserved: LPVOID) : BOOL {.stdcall, exportc, dynlib.} =
  NimMain()

  return true