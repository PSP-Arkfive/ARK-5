cmake_minimum_required(VERSION 3.28)

project(xmbcontrol VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

add_prx_module(xmbcontrol exports.exp)

target_sources(xmbcontrol PRIVATE 
   main.c
   stub.S
   src/xmbpatch.c
   src/list.c
   src/settings.c
   src/config.c
   src/plugins.c
   src/battery.c
   src/vshmenu.c
   src/utils.c
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(xmbcontrol PRIVATE -std=c99 -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division)
target_include_directories(xmbcontrol PRIVATE include)
target_link_libraries(xmbcontrol PRIVATE
   -nostartfiles
   pspsystemctrl_user
   pspkubridge
   pspvshctrl
   pspreg
   pspsysc_user
   psppower
   pspdebug
   pspdisplay
   pspge
   pspctrl
   pspnet
   pspnet_apctl
)

if(DEBUG)
   target_compile_definitions(xmbcontrol PRIVATE -DDEBUG=${DEBUG})
endif()
