proc c_Info(message : cstring) : void {.importc: "?Info@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}
proc c_Warn(message : cstring) : void {.importc: "?Warn@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}
proc c_Success(message : cstring) : void {.importc: "?Success@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}
proc c_Error(message : cstring) : void {.importc: "?Error@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}

proc Info(message : string) =
    c_Info(cstring(message))

proc Warn(message : string) =
    c_Warn(cstring(message))

proc Success(message : string) =
    c_Success(cstring(message))

proc Error(message : string) =
    c_Error(cstring(message))


export Info, Warn, Success, Error