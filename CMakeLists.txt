cmake_minimum_required(VERSION 3.0.0)

#set(CMAKE_VERBOSE_MAKEFILE on )

set(CMAKE_CXX_STANDARD 17)

project(DebugLoop VERSION 0.1.0)

file(GLOB ALL_SRC "*.cpp")

add_library(DebugLoop SHARED ${ALL_SRC})
add_definitions(-DUNICODE)


if (NOT MSVC)
    target_link_libraries(DebugLoop -static-libgcc -static-libstdc++)
    add_custom_command(TARGET DebugLoop POST_BUILD 
                       COMMAND ${CMAKE_STRIP} libDebugLoop.dll)
endif(NOT MSVC)


