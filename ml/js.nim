import std/strformat
import std/jsonutils
import std/strutils
import winim/lean
import winim/shell
import modloader
import utils
import json
import os

type
    RPC_ID = enum
        readFileEx = 0, writeFileEx = 1, mkdirEx = 2, getAsyncKeyState = 3, messageBox = 4, getClipboard = 5

proc c_JS_EvalMod(code : cstring, filename : cstring) : void {.importc: "?JS_EvalMod@@YAXPEBD0@Z", dynlib: "omori-patcher.dll".}

proc JS_EvalMod(code : string, filename : string) =
    c_JS_EvalMod(cstring(code), cstring(filename))

proc JSInit_cb(mods : seq[Mod]) =
    for m in mods:
      if m.hasMain:
        try:
            JS_EvalMod(readFile(m.main), m.main)
        except CatchableError:
            Error(getCurrentExceptionMsg())

proc JSEscape(txt: string): string =
    return txt
        .replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\'", "\\\'")
        .replace("\r", "\\r")
        .replace("\n", "\\n")


proc RPC_cb(data: cstring) =
    let node = json.parseJson($(data))
    let f = cast[RPC_ID](node["func"].getInt())
    let data = node["data"]

    case (f)
        of RPC_ID.readFileEx:
            let fn = data["function"].getStr()
            let filename = data["filename"].getStr()
            let data = readFile(filename)
            JS_EvalMod(fmt"window.{fn}('{JSEscape(filename)}', '{JSEscape(data)}');", "js.nim:RPC_cb")
        of RPC_ID.writeFileEx:
            if not data["replace"].getBool() and fileExists(data["filename"].getStr()):
                return
            writeFile(data["filename"].getStr(), data["data"].getStr())
        of RPC_ID.mkdirEx:
            let dirname = data["dirname"].getStr()
            if not dirExists(dirname):
                createDir(dirname)
        of RPC_ID.getAsyncKeyState:
            let fn = data["function"].getStr()
            let keys = data["keys"]
            var states: seq[bool]
            for key in keys:
                states.add(GetAsyncKeyState(int32(key.getInt())) != 0)
            JS_EvalMod(fmt"window.{fn}({$states.toJson()});", "js.nim:RPC_cb")

        of RPC_ID.messageBox:
            MessageBoxA(0, data["body"].getStr(), data["title"].getStr(), 0)

        of RPC_ID.getClipboard:
            let fn = data["function"].getStr()
            var clipboardType = "unknown"
            OpenClipboard(0)
            var hClipboardData = GetClipboardData(CF_HDROP)
            var clipboardData = ""
            if hClipboardData != 0:
                let hDrop = cast[HDROP](GlobalLock(hClipboardData))
                clipboardType = "file"
                let fileCount = DragQueryFileA(hDrop, cast[UINT](0xFFFFFFFF'u32), nil, 0)
                for i in 0..<fileCount:
                    let filenameLength = DragQueryFileA(hDrop, i, nil, 0);
                    var filename = newString(filenameLength)
                    DragQueryFileA(hDrop, i, filename.cstring, filenameLength+1);
                    clipboardData = clipboardData & filename & ";"
                if clipboardData.len() > 0:
                    clipboardData = clipboardData.substr(0, clipboardData.len()-2)
            else:
                hClipboardData = GetClipboardData(CF_UNICODETEXT)
                if hClipboardData != 0:
                    clipboardData = $cast[WideCString](GlobalLock(hClipboardData))
                    clipboardType = "text"
                    GlobalUnlock(hClipboardData)
            CloseClipboard()
            JS_EvalMod(fmt"window.{fn}('{JSEscape(clipboardType)}', '{JSEscape(clipboardData)}');", "js.nim:RPC_cb")

export JS_EvalMod, RPC_cb, JSInit_cb