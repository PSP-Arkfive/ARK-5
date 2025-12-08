add_executable(vitapopscompat_reboot)
target_sources(vitapopscompat_reboot PRIVATE patches.c)
target_link_libraries(vitapopscompat_reboot PRIVATE bootloadex_vita)
target_compile_definitions(vitapopscompat_reboot PRIVATE REBOOTEX REDIRECT_FLASHFS)

if(DEBUG)
    target_compile_definitions(vitapopscompat_reboot PRIVATE -DDEBUG=${DEBUG})
    target_link_libraries(vitapopscompat_reboot PRIVATE colordebugger)
endif()

add_custom_command(
    TARGET vitapopscompat_reboot
    POST_BUILD
    COMMAND psp-strip -s vitapopscompat_reboot
    COMMAND psp-objcopy -O binary vitapopscompat_reboot vitapopscompat_reboot.bin
    COMMAND python3 $<TARGET_PROPERTY:tool_gz,EXEC> vitapopscompat_reboot.bin vitapopscompat_reboot.bin.gz
)
