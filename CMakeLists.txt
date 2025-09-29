cmake_minimum_required(VERSION 3.28)

project(ark5 VERSION 1.0 LANGUAGES C ASM)
set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

set(DIST_DIR ${CMAKE_CURRENT_LIST_DIR}/dist)
set(FLASH0_DIR ${DIST_DIR}/flash0)

set(ARK_REPOS
   ark-dev-sdk
   SystemControl
   Inferno2
   PopCorn
   Stargate
   VSHControl
   PSPCompat
   VitaCompat
   VitaPopsCompat
   Pentazemin
   XMBControl
)

include(FetchContent)
foreach(ARK_REPO ${ARK_REPOS})
   string(REPLACE "-" "" ARK_REPO_NAME "${ARK_REPO}")
   FetchContent_Declare(
      ${ARK_REPO_NAME}
      GIT_REPOSITORY https://github.com/PSP-Arkfive/${ARK_REPO}.git
      GIT_TAG main
      EXCLUDE_FROM_ALL
   )
   FetchContent_MakeAvailable(${ARK_REPO_NAME})
endforeach()

set(ARKSDK ${arkdevsdk_SOURCE_DIR})
set(BUILDTOOLS ${ARKSDK}/build-tools)

add_custom_target(BuildARK5 ALL
   DEPENDS 
      systemctrl
      vshcontrol
      xmbcontrol
      inferno
      popcorn
      stargate
      pspcompat
      pspcompat_genbtcnf
      vitacompat
      vitacompat_genbtcnf
      vitapopscompat
      vitapopscompat_genbtcnf
      pentazemin
      pentazemin_genbtcnf

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
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_vitapops.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:vitapopscompat>" VitaPopsCompat 0x3007
	COMMAND python3 $<TARGET_PROPERTY:tool_pspgz,EXEC> ${FLASH0_DIR}/ark_vitaplus.prx ${BUILDTOOLS}/gz/SystemControl.hdr "$<TARGET_FILE:pentazemin>" Pentazemin 0x3007
	
   #Copy PSP Compat btcnf files
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:pspcompat>/pstbtcnf_dt.bin ${FLASH0_DIR}/
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:pspcompat>/pstbtcnf_tt.bin ${FLASH0_DIR}/

   #Copy Vita Compat btcnf files
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitacompat>/psvbtcnf.bin ${FLASH0_DIR}/
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitacompat>/psvbtinf.bin ${FLASH0_DIR}/

   #Copy Vita Pops Compat btcnf files
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:vitapopscompat>/psxbtcnf.bin ${FLASH0_DIR}/

   #Copy Vita Adrenaline btcnf files
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:pentazemin>/psvbtjnf.bin ${FLASH0_DIR}/
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:pentazemin>/psvbtknf.bin ${FLASH0_DIR}/

   #Pack FLASH0.ARK
   COMMAND python3 $<TARGET_PROPERTY:tool_pack,EXEC> -p ${DIST_DIR}/FLASH0.ARK ${BUILDTOOLS}/pack/flash0.txt -s
)
