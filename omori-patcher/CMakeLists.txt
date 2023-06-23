add_library (omori-patcher SHARED framework.h pch.h pch.cpp dllmain.cpp utils.cpp utils.h mem.cpp mem.h consts.h js.cpp js.h quickjs.h rpc.cpp rpc.h fs_overlay.h fs_overlay.cpp)
# get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
# include_directories(${JSON_INC_PATH})
target_link_libraries("omori-patcher" PRIVATE "Zydis" "zasm" "lib_detours")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET omori-patcher PROPERTY CXX_STANDARD 20)
endif()
