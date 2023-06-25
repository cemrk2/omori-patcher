import std/strformat
import std/strutils
import binstreams
import zippy
import pixie
import utils

# https://github.com/rphsoftware/OneLoaderImageDelta
proc applyOLID(olidName : string, imgPath : string, olidStream : FileStream) =
    # Magic bytes
    const spaceship = [0xFE'u8, 0xFF'u8, 0xD8'u8, 0x08'u8, 0xDD'u8, 0x21'u8]
    for b in spaceship:
        if not b == olidStream.read(uint8):
            Error(fmt"Invalid header for olid {olidName}, skipping!")
            return

    let w = olidStream.read(uint32)
    let h = olidStream.read(uint32)
    discard olidStream.read(uint64) # unique identifier
    let olidLen = olidStream.read(uint32)
    let img = readImage(imgPath)

    if not img.width == int(w) or not img.height == int(h):
        Error(fmt"({olidName}) Image source dimensions ({img.width}x{img.height}) doesn't match olid dimensions ({w}x{h})")
        return

    Info(fmt"OLID zipped len: {olidLen}")
    Info(fmt"Decompressing {olidName}")
    let olidCompressed = olidStream.readStr(olidLen)
    var decompressed : string
    try:
        decompressed = uncompress(olidCompressed, dfZlib)
    except CatchableError:
        Error(fmt"Failed to decompress {olidName}")
        Error(getCurrentExceptionMsg())
        return
    Success("decompressed successfully!")
    var cstr = decompressed.cstring


export applyOLID