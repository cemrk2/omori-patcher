import std/strformat
import bitops
import binstreams
import zippy
import pixie
import utils

# https://github.com/rphsoftware/OneLoaderImageDelta
proc applyOLID(olidName : string, imgPath : string, olidFileStream : FileStream) : string =
    # Magic bytes
    const spaceship = [0xFE'u8, 0xFF'u8, 0xD8'u8, 0x08'u8, 0xDD'u8, 0x21'u8]
    for b in spaceship:
        if not b == olidFileStream.read(uint8):
            Error(fmt"Invalid header for olid {olidName}, skipping!")
            return "\0"

    let w = int(olidFileStream.read(uint32))
    let h = int(olidFileStream.read(uint32))
    discard olidFileStream.read(uint64) # unique identifier
    let olidLen = olidFileStream.read(uint32)
    let oldImg = readImage(imgPath)
    let newImgCtx = newContext(newImage(w, h))

    newImgCtx.drawImage(oldImg, 0, 0)

    let olidCompressed = olidFileStream.readStr(olidLen)
    var decompressed : string
    try:
        decompressed = uncompress(olidCompressed, dfZlib)
    except CatchableError:
        Error(fmt"Failed to decompress {olidName}")
        Error(getCurrentExceptionMsg())
        return "\0"
    Success("Decompressed successfully!")

    # Decompressed OLID stream
    var olid = newMemStream(cast[seq[byte]](decompressed), bigEndian)
    var dptr = 0

    while dptr < len(decompressed):
        let tileX = int(olid.read(uint16))
        let tileY = int(olid.read(uint16))
        let tileBitstreamLen = olid.read(uint32)
        dptr += 8

        var tileStream = newSeq[uint8](tileBitstreamLen)
        for i in 0..<tileBitstreamLen:
            tileStream[i] = olid.read(uint8)
        dptr += int(tileBitstreamLen)

        var mask : array[32, uint8]
        var tileStreamPtr = 32
        for i in 0..<32:
            mask[i] = tileStream[i]

        try:
            for i in 0..<256:
                if bitand(mask[int(i / 8)] shr (i mod 8), 0x01) == 1:
                    let x = (tileX * 16) + (i mod 16)
                    let y = (tileY * 16) + int(floor(i / 16))

                    newImgCtx.image.unsafe[x, y] = rgba(tileStream[tileStreamPtr+3], tileStream[tileStreamPtr+2], tileStream[tileStreamPtr+1], tileStream[tileStreamPtr])
                    tileStreamPtr += 4

        except CatchableError:
            Error(getCurrentExceptionMsg())


    olid.close()

    return encodeImage(newImgCtx.image, PngFormat)


export applyOLID
