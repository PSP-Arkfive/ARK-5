cmake_minimum_required(VERSION 3.28)

project(popcorn VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

include(FetchContent)

FetchContent_Declare(
   ark-dev-sdk
   GIT_REPOSITORY https://github.com/PSP-Arkfive/ark-dev-sdk.git
   GIT_TAG main
   EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(ark-dev-sdk)

set(ARKSDK ${ark-dev-sdk_SOURCE_DIR})

add_prx_module(popcorn exports.exp)

target_sources(popcorn PRIVATE 
   main.c
   src/icon.c
   src/syspatch.c
   src/libcrypt.c
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(popcorn PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)
target_include_directories(popcorn PRIVATE include ${ARKSDK}/include)
target_link_directories(popcorn PRIVATE ${ARKSDK}/libs)
target_link_libraries(popcorn PRIVATE
   -nostartfiles
   -nostdlib
   pspsystemctrl_kernel
   pspsdk
   pspmodinfo
   pspkernel
)

if(DEBUG)
   target_compile_definitions(popcorn PRIVATE -DDEBUG=${DEBUG})
endif()
