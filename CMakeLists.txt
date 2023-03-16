# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("omori-patcher")

# Register Zydis dependency.
# Disable build of tools and examples.
option(ZYDIS_BUILD_DOXYGEN "" OFF)
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)

find_package(SailC++ CONFIG REQUIRED)
find_package(PNG REQUIRED)

add_subdirectory("libs/lib_detours")
add_subdirectory("libs/jsoncpp")
add_subdirectory("libs/zasm")
add_subdirectory("omori-patcher")