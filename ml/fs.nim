import std/strformat
import std/jsonutils
import std/strutils
import std/tables
import binstreams
import modloader
import jsonpatch
import utils
import olid
import json
import os

type
    FileType = enum
        ASSET, JSOND, OLID

proc AddFileMap(src : cstring, dst : cstring) : void {.importc: "?AddFileMap@@YAXPEBD0@Z", dynlib: "omori-patcher.dll".}
proc AddBinFile(path : cstring, size : csize_t, data : pointer) {.importc: "?AddBinFile@@YAXPEBD_KPEAX@Z", dynlib: "omori-patcher.dll".}

proc applyJsond(src : string, jsond : string) : string =
    let srcNode = json.parseJson(src)
    let deltas = json.parseJson(jsond).to(JsonPatch)
    var newDoc = patch(srcNode, deltas)
    return $newDoc.toJson

proc registerFile(m : Mod, file : string, file2 : string, fileType : FileType) =
    case (fileType)
        of ASSET:
            AddFileMap(cstring(file), cstring(fmt"{m.path}/{file}"))
        of JSOND:
            let jsondFile = open(fmt"{m.path}/{file}")
            let srcFilePath = file[0 .. len(file)-2]
            let srcFile = open(fmt"{srcFilePath}")
            var src = readAll(srcFile)
            var jsond = readAll(jsondFile)

            Info(fmt"{file} -> {srcFilePath}")
            let newSrc = cstring(applyJsond(src, jsond))
            AddBinFile(cstring(file[0 .. len(file)-2]), csize_t(len(newSrc)), cast[pointer](newSrc))
        of OLID:
            var olidH = open(fmt"{m.path}/{file2}")
            Info(fmt"olid: {file} {file2}")
            var applied = applyOLID(file2, file, binstreams.newFileStream(olidH, bigEndian))
            AddBinFile(cstring(file[0 .. len(file)-1]), csize_t(len(applied)), cast[pointer](applied.cstring))


proc RegisterOverlayedFiles(mods : seq[Mod]) =
    for m in mods:
        for file in m.files:
            if file.endsWith("/"):
                for f in walkDirRec(fmt"{m.path}/{file}", {pcFile}):
                    registerFile(m, f[len(m.path) + 1 .. len(f) - 1], "", ASSET)
            else:
                registerFile(m, file, "", ASSET)
        for file in m.jsond:
            if file.endsWith("/"):
                for f in walkDirRec(fmt"{m.path}/{file}", {pcFile}):
                    registerFile(m, f[len(m.path) + 1 .. len(f) - 1], "", JSOND)
            else:
                registerFile(m, file, "", JSOND)
        for file, patch in m.olid.pairs:
            registerFile(m, file, patch, OLID)


export RegisterOverlayedFiles