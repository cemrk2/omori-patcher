import std/strformat
import std/jsonutils
import modloader
import jsonpatch
import utils
import json

proc AddFileMap(src : cstring, dst : cstring) : void {.importc: "?AddFileMap@@YAXPEBD0@Z", dynlib: "omori-patcher.dll".}
proc AddBinFile(path : cstring, size : csize_t, data : pointer) {.importc: "?AddBinFile@@YAXPEBD_KPEAX@Z", dynlib: "omori-patcher.dll".}

proc applyJsond(src : string, jsond : string) : string =
    let srcDoc = json.parseJson(src)
    let jsondDoc = json.parseJson(jsond)
    var deltas : seq[Operation]
    var newDoc = srcDoc
    for deltaPatch in jsondDoc:
        let op = deltaPatch["op"].getStr()

        case op:
            of "add":
                deltas.add(newAddOperation(deltaPatch["path"].getStr().toJsonPointer, deltaPatch["value"]))
            of "remove":
                deltas.add(newRemoveOperation(deltaPatch["path"].getStr().toJsonPointer))
            of "replace":
                deltas.add(newReplaceOperation(deltaPatch["path"].getStr().toJsonPointer, deltaPatch["value"]))
            of "move":
                deltas.add(newMoveOperation(deltaPatch["path"].getStr().toJsonPointer, deltaPatch["from"].getStr().toJsonPointer))
            of "test":
                deltas.add(newTestOperation(deltaPatch["path"].getStr().toJsonPointer, deltaPatch["value"]))
            of "copy":
                deltas.add(newCopyOperation(deltaPatch["path"].getStr().toJsonPointer, deltaPatch["from"].getStr().toJsonPointer))
    for delta in deltas:
        newDoc = patch(newDoc, delta)
    return $newDoc.toJson

proc RegisterOverlayedFiles(mods : seq[Mod]) =
    for m in mods:
        for file in m.files:
            AddFileMap(cstring(file), cstring(fmt"{m.path}/{file}"))
        for file in m.jsond:
            let jsondFile = open(fmt"{m.path}/{file}")
            let srcFilePath = file[0 .. len(file)-2]
            let srcFile = open(fmt"{srcFilePath}")
            var src = readAll(srcFile)
            var jsond = readAll(jsondFile)

            Info(fmt"{file} -> {srcFilePath}")
            let newSrc = cstring(applyJsond(src, jsond))
            AddBinFile(cstring(file[0 .. len(file)-2]), csize_t(len(newSrc)), cast[pointer](newSrc))


export RegisterOverlayedFiles