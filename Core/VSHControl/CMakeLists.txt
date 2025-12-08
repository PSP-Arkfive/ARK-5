cmake_minimum_required(VERSION 3.28)

project(vshcontrol VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

add_prx_module(vshcontrol exports.exp)

target_sources(vshcontrol PRIVATE 
   main.c
   imports.S
   src/vshpatch.c
   src/xmbiso.c
   src/isoreader.c
   src/virtual_pbp.c
   src/virtual_mp4.c
   src/dirent_track.c
   src/disctype.c
   src/vshmenu.c
   src/usbdevice.c
   src/custom_update.c
   src/hibernation.c
   src/registry.c
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(vshcontrol PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)
target_include_directories(vshcontrol PRIVATE include)
target_link_directories(vshcontrol PRIVATE libs)
target_link_libraries(vshcontrol PRIVATE
   -nostartfiles
   -nostdlib
   pspsystemctrl_kernel
   pspusb
   pspusbdevice_driver
   pspreg
   pspsdk
   pspmodinfo
   pspkernel
)

if(DEBUG)
   target_compile_definitions(vshcontrol PRIVATE -DDEBUG=${DEBUG})
endif()
