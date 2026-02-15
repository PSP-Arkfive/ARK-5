function(add_compat_prx PRX_NAME REBOOT_BUF_SUFFIX)
   set(options "")
   set(oneValueArgs "")
   set(multiValueArgs SRC_FILES BTCNF_FILES LINK_LIBS)

   cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

   set(REBOOTEX_NAME ${PRX_NAME}_reboot)

   add_prx_module(${PRX_NAME} exports.exp)

   target_sources(${PRX_NAME} PRIVATE ${ARG_SRC_FILES})
   target_compile_options(${PRX_NAME} PRIVATE ${COMPILE_OPS})
   target_include_directories(${PRX_NAME} PRIVATE include ${CFWSDK}/include ${CMAKE_CURRENT_BINARY_DIR})
   target_link_directories(${PRX_NAME} PRIVATE ${CFWSDK}/libs)
   target_link_libraries(${PRX_NAME} PRIVATE
      -nostartfiles
      arkctrl
      pspsystemctrl_kernel
      ${ARG_LINK_LIBS}
      pspsdk
      pspmodinfo
      pspkernel
   )

   add_custom_command(OUTPUT payload.h
      DEPENDS ${REBOOTEX_NAME}
      COMMAND bin2c "$<TARGET_FILE:${REBOOTEX_NAME}>.bin.gz" payload.h rebootbuffer_${REBOOT_BUF_SUFFIX}
   )

   foreach(BTCNF_BIN_FILE IN LISTS ARG_BTCNF_FILES)
      string(REPLACE ".bin" ".txt" BTCNF_TXT_FILE "${BTCNF_BIN_FILE}")
      add_custom_command(
         OUTPUT ${BTCNF_BIN_FILE}
         DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
         COMMAND python3 $<TARGET_PROPERTY:tool_btcnf,EXEC> build "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_TXT_FILE}"
         COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_SOURCE_DIR}/btcnf/${BTCNF_BIN_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${BTCNF_BIN_FILE}"
      )
   endforeach()

   add_custom_target(${PRX_NAME}_genbtcnf ALL
      DEPENDS ${ARG_BTCNF_FILES}
   )

   add_executable(${REBOOTEX_NAME})
   target_sources(${REBOOTEX_NAME} PRIVATE rebootex/main.c)
   target_compile_options(${REBOOTEX_NAME} PRIVATE ${COMPILE_OPS})
   target_link_libraries(${REBOOTEX_NAME} PRIVATE bootloadex ansic)

   if(DEBUG)
      target_compile_definitions(${REBOOTEX_NAME} PRIVATE -DDEBUG=${DEBUG})
      target_link_libraries(${REBOOTEX_NAME} PRIVATE colordebugger)
   endif()

   add_custom_command(
      TARGET ${REBOOTEX_NAME}
      POST_BUILD
      COMMAND psp-strip -s ${REBOOTEX_NAME}
      COMMAND psp-objcopy -O binary ${REBOOTEX_NAME} ${REBOOTEX_NAME}.bin
      COMMAND python3 $<TARGET_PROPERTY:tool_gz,EXEC> ${REBOOTEX_NAME}.bin ${REBOOTEX_NAME}.bin.gz
   )
endfunction()


add_subdirectory(ePSP)
add_subdirectory(ePSX)
add_subdirectory(PSP)
add_subdirectory(vPSP)
