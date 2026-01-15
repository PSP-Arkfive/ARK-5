cmake_minimum_required(VERSION 3.28)

project(inferno VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

include(FetchContent)

FetchContent_Declare(
   psp-cfw-sdk
   GIT_REPOSITORY https://github.com/PSP-Arkfive/psp-cfw-sdk.git
   GIT_TAG main
   EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(psp-cfw-sdk)

set(CFWSDK ${psp-cfw-sdk_SOURCE_DIR})

add_prx_module(inferno exports.exp)

target_sources(inferno PRIVATE 
   main.c
   src/iodrv_funcs.c
   src/umd.c
   src/isoread.c
   src/isocache.c
   ${CFWSDK}/src/ansi-c/lowerstring.c
   ${CFWSDK}/src/ansi-c/strcasecmp.c
   imports.S
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(inferno PRIVATE -std=c99 -Os -G0 -Wall)
target_include_directories(inferno PRIVATE include ${CFWSDK}/include)
target_link_directories(inferno PRIVATE ${CFWSDK}/libs)
target_link_libraries(inferno PRIVATE
   -nostartfiles
   -nostdlib
   pspsystemctrl_kernel
   pspsdk
   pspmodinfo
   pspkernel
)

if(DEBUG)
   target_compile_definitions(inferno PRIVATE -DDEBUG=${DEBUG})
endif()
