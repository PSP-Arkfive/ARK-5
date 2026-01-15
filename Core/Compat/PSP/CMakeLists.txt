cmake_minimum_required(VERSION 3.28)

project(pspcompat VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

set(ARK_REPOS psp-cfw-sdk BootLoadEx)

include(FetchContent)
foreach(ARK_REPO ${ARK_REPOS})
   FetchContent_Declare(
      ${ARK_REPO}
      GIT_REPOSITORY https://github.com/PSP-Arkfive/${ARK_REPO}.git
      GIT_TAG main
      EXCLUDE_FROM_ALL
   )
endforeach()
FetchContent_MakeAvailable(${ARK_REPOS})

set(CFWSDK ${psp-cfw-sdk_SOURCE_DIR})

add_prx_module(pspcompat exports.exp)

target_sources(pspcompat PRIVATE 
   main.c
   src/syspatch.c
   src/wpa2.c
   src/cwpatch.c
   src/high_mem.c
   src/libertas.c
   src/usb_charge.c
   src/region_free.c
   src/pops4tool.c
   imports.S
   payload.h
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(pspcompat PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)
target_include_directories(pspcompat PRIVATE include ${CFWSDK}/include ${CMAKE_CURRENT_BINARY_DIR})
target_link_directories(pspcompat PRIVATE ${CFWSDK}/libs)
target_link_libraries(pspcompat PRIVATE
   -nostartfiles
   -nostdlib
   pspsystemctrl_kernel
   pspsdk
   pspmodinfo
   pspkernel
)

add_custom_command(OUTPUT payload.h
  DEPENDS pspcompat_reboot
  COMMAND bin2c "$<TARGET_FILE:pspcompat_reboot>.bin.gz" payload.h rebootbuffer_psp
)

set(BTCNF_FILES pstbtcnf_dt.bin pstbtcnf_tt.bin)

foreach(BTCNF_BIN_FILE IN LISTS BTCNF_FILES)
  string(REPLACE ".bin" ".txt" BTCNF_TXT_FILE "${BTCNF_BIN_FILE}")
  add_custom_command(
    OUTPUT ${BTCNF_BIN_FILE}
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
    COMMAND python3 $<TARGET_PROPERTY:tool_btcnf,EXEC> build "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
    COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_BIN_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${BTCNF_BIN_FILE}"
  )
endforeach()

add_custom_target(pspcompat_genbtcnf ALL
    DEPENDS ${BTCNF_FILES}
)

add_subdirectory(rebootex)