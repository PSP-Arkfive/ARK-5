cmake_minimum_required(VERSION 3.28)

project(ark5 VERSION 1.0 LANGUAGES C ASM)
set(PSPCFWSDK_PATH "" CACHE PATH "Path to local psp-cfw-sdk (for local development)")
set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

set(DIST_DIR ${CMAKE_CURRENT_LIST_DIR}/dist)
set(FLASH0_DIR ${DIST_DIR}/flash0)

include(FetchContent)
if(PSPCFWSDK_PATH)
message(STATUS "Using local pspcfw repo: ${PSPCFWSDK_PATH}")
FetchContent_Declare(
   pspcfwsdk
   SOURCE_DIR ${PSPCFWSDK_PATH}
   EXCLUDE_FROM_ALL
)
else()
FetchContent_Declare(
   pspcfwsdk
   GIT_REPOSITORY https://github.com/pspdev/psp-cfw-sdk.git
   GIT_TAG main
   EXCLUDE_FROM_ALL
)
endif()
FetchContent_MakeAvailable(pspcfwsdk)

set(CFWSDK ${pspcfwsdk_SOURCE_DIR})
set(BUILDTOOLS ${CFWSDK}/build-tools)
set(COMPILE_OPS -std=c99 -Os -G0 -Wall -fno-pic)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

add_subdirectory(Core)

add_custom_target(dist ALL
   DEPENDS 
      inferno   
      popcorn
      stargate
      systemctrl
      vshcontrol
      xmbcontrol
      pspcompat
      pspcompat_genbtcnf
      vitacompat
      vitacompat_genbtcnf
      vitapops
      vitapops_genbtcnf
      vitaplus
      vitaplus_genbtcnf

   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   
   COMMAND mkdir -p ${FLASH0_DIR}

   #Create compressed prx modules
   COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_systemctrl.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:systemctrl>" SystemControl 0x3007
   COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_vshctrl.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:vshcontrol>" VshControl 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_xmbctrl.prx ${BUILDTOOLS}/gz/UserModule.hdr "$<TARGET_FILE:xmbcontrol>" XmbControl 0x0000
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_inferno.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:inferno>" PRO_Inferno_Driver 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_popcorn.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:popcorn>" PROPopcornManager 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_stargate.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:stargate>" Stargate 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_pspcompat.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:pspcompat>" PSPCompat 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_vitacompat.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:vitacompat>" VitaCompat 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_vitapops.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:vitapops>" VitaPopsCompat 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_vitaplus.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:vitaplus>" VitaPlusCompat 0x3007
	
   #Copy PSP Compat btcnf files
   #COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:pspcompat>/pstbtcnf_dt.bin ${FLASH0_DIR}/
   #COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:pspcompat>/pstbtcnf_tt.bin ${FLASH0_DIR}/

   #Copy Vita Compat btcnf files
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitacompat>/psvbtcnf.bin ${FLASH0_DIR}/
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitacompat>/psvbtinf.bin ${FLASH0_DIR}/

   #Copy Vita Pops Compat btcnf files
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitapops>/psxbtcnf.bin ${FLASH0_DIR}/

   #Copy Vita Adrenaline btcnf files
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitaplus>/psvbtjnf.bin ${FLASH0_DIR}/
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitaplus>/psvbtknf.bin ${FLASH0_DIR}/

   #Pack FLASH0.ARK
   COMMAND python3 $<TARGET_PROPERTY:tool_pack,EXEC> -p ${DIST_DIR}/FLASH0.ARK ${PROJECT_SOURCE_DIR}/flash0.txt -s
)
