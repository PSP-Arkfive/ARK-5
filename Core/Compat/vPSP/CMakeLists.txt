cmake_minimum_required(VERSION 3.28)

project(pentazemin VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

set(ARK_REPOS ark-dev-sdk BootLoadEx)

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

set(ARKSDK ${ark-dev-sdk_SOURCE_DIR})

add_prx_module(pentazemin exports.exp)

target_sources(pentazemin PRIVATE 
   main.c
   src/funcs.c
   src/usbcam.c
   src/vitamem.c
   src/syspatch.c
   src/flashfs.c
   src/io_patch.c
   src/loadexec.c
   src/adrenaline.c
   src/init_patch.c
   src/power_patch.c
   src/LflashFatfmt.c
   src/impose_patch.c
   src/utility_patch.c
   src/vlf.c
   imports.S
   payload.h
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(pentazemin PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)
target_include_directories(pentazemin PRIVATE include ${ARKSDK}/include ${CMAKE_CURRENT_BINARY_DIR})
target_link_directories(pentazemin PRIVATE ${ARKSDK}/libs)
target_link_libraries(pentazemin PRIVATE
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
  DEPENDS pentazemin_reboot
  COMMAND bin2c "$<TARGET_FILE:pentazemin_reboot>.bin.gz" payload.h rebootbuffer_pentazemin
)

set(BTCNF_FILES psvbtjnf.bin psvbtknf.bin)

foreach(BTCNF_BIN_FILE IN LISTS BTCNF_FILES)
  string(REPLACE ".bin" ".txt" BTCNF_TXT_FILE "${BTCNF_BIN_FILE}")
  add_custom_command(
    OUTPUT ${BTCNF_BIN_FILE}
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
    COMMAND python3 $<TARGET_PROPERTY:tool_btcnf,EXEC> build "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
    COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_BIN_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${BTCNF_BIN_FILE}"
  )
endforeach()

add_custom_target(pentazemin_genbtcnf ALL
    DEPENDS ${BTCNF_FILES}
)

add_subdirectory(rebootex)