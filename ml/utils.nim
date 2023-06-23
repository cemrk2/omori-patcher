proc Info(message : cstring) : void {.importc: "?Info@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}
proc Warn(message : cstring) : void {.importc: "?Warn@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}
proc Success(message : cstring) : void {.importc: "?Success@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}
proc Error(message : cstring) : void {.importc: "?Error@@YAXPEBD@Z", dynlib: "omori-patcher.dll".}

export Info