version = "1.0.0"
author = "cemrk"
description = "ModLoader for the Xbox PC version of OMORI"
license = "MIT"

requires "nim >= 1.6.12"
requires "winim >= 3.9.1"
requires "jsonpatch >= 0.1.0"
requires "zippy >= 0.10.8"
requires "pixie >= 5.0.6"
requires "binstreams >= 0.2.0"

task x64dbg, "Compiles omori-ml and launches omori with x64dbg":

  exec "cmd /c build_cmake.bat && copy build\\omori-patcher\\Debug\\omori-patcher.dll OMORI\\omori-patcher.dll /y"
  exec "nim c -d:debug --cc=vcc --app=lib --cpu=amd64 -o:ml.dll ml/ml.nim"
  exec "cmd /c del ml.lib"
  exec "cmd /c del ml.exp"
  exec "cmd /c copy ml.dll OMORI\\ml.dll /y && copy ml.pdb OMORI\\ml.pdb /y"
  exec "cmd /c cd OMORI && x64dbg %cd%\\OMORI\\OMORI.exe"

task debug, "Compiles omori-ml and launches omori":

  exec "cmd /c build_cmake.bat && copy build\\omori-patcher\\Debug\\omori-patcher.dll OMORI\\omori-patcher.dll /y"
  exec "nim c -d:debug --cc=vcc --app=lib --cpu=amd64 -o:ml.dll ml/ml.nim"
  exec "cmd /c del ml.lib"
  exec "cmd /c del ml.exp"
  exec "cmd /c copy ml.dll OMORI\\ml.dll /y && copy ml.pdb OMORI\\ml.pdb /y"
  exec "cmd /c cd OMORI && OMORI"

task release, "Compiles omori-patcher and ml in release mode":

  exec "cmd /c build_cmake.bat && copy build\\omori-patcher\\Debug\\omori-patcher.dll omori-patcher.dll"
  exec "nim c -d:release --cc=vcc --app=lib --cpu=amd64 -o:ml.dll ml/ml.nim"

