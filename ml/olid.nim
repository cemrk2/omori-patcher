import std/strformat
import std/strutils
import binstreams
import zippy
import utils

# https://github.com/rphsoftware/OneLoaderImageDelta
proc applyOLID(olidName : string, imgBuffer : pointer, olidStream : FileStream, imgSize : BiggestInt) : void =
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

    Info(fmt"{w} {h}")

export applyOLID