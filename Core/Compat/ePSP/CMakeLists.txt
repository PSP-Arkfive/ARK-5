cmake_minimum_required(VERSION 3.28)

project(vitacompat VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

include(FetchContent)
FetchContent_Declare(
    ark-dev-sdk
    GIT_REPOSITORY https://github.com/PSP-Arkfive/ark-dev-sdk.git
    GIT_TAG main
    EXCLUDE_FROM_ALL
)
FetchContent_Declare(
    BootLoadEx
    GIT_REPOSITORY https://github.com/PSP-Arkfive/BootLoadEx.git
    GIT_TAG main
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ark-dev-sdk BootLoadEx)

set(ARKSDK ${ark-dev-sdk_SOURCE_DIR})

add_prx_module(vitacompat exports.exp)

target_sources(vitacompat PRIVATE 
   main.c
   src/vlf.c
   src/syspatch.c
   src/vitamem.c
   src/vitaflash.c 
   src/popspatch.c
   src/filesystem.c
   src/fatef.c
   src/flashfs.c
   imports.S
   payload.h
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(vitacompat PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)
target_include_directories(vitacompat PRIVATE include ${ARKSDK}/include ${CMAKE_CURRENT_BINARY_DIR})
target_link_directories(vitacompat PRIVATE ${ARKSDK}/libs)
target_link_libraries(vitacompat PRIVATE
   -nostartfiles
   -nostdlib
   pspsystemctrl_kernel
   pspkermit_driver
   pspkermitperipheral_driver
   pspsdk
   pspmodinfo
   pspkernel
)

add_custom_command(OUTPUT payload.h
  DEPENDS vitacompat_reboot
  COMMAND bin2c "$<TARGET_FILE:vitacompat_reboot>.bin.gz" payload.h rebootbuffer_vita
)

set(BTCNF_FILES psvbtcnf.bin psvbtinf.bin)

foreach(BTCNF_BIN_FILE IN LISTS BTCNF_FILES)
  string(REPLACE ".bin" ".txt" BTCNF_TXT_FILE "${BTCNF_BIN_FILE}")
  add_custom_command(
    OUTPUT ${BTCNF_BIN_FILE}
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
    COMMAND python3 $<TARGET_PROPERTY:tool_btcnf,EXEC> build "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
    COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_BIN_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${BTCNF_BIN_FILE}"
  )
endforeach()

add_custom_target(vitacompat_genbtcnf ALL
    DEPENDS ${BTCNF_FILES}
)

add_subdirectory(rebootex)