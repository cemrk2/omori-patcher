add_library (omori-patcher SHARED framework.h pch.h pch.cpp dllmain.cpp utils.cpp utils.h mem.cpp mem.h consts.h)
target_link_libraries("omori-patcher" PRIVATE "Zydis")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET omori-patcher PROPERTY CXX_STANDARD 20)
endif()
