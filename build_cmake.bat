@echo off
if not exist build\CMakeCache.txt cmake -B%cd%/build -G "Visual Studio 17 2022" -T host=x64 -A x64
cd build
cmake --build .
cd ..
