add_executable(vitacompat_reboot)
target_sources(vitacompat_reboot PRIVATE patches.c)
target_link_libraries(vitacompat_reboot PRIVATE bootloadex_vita)
target_compile_definitions(vitacompat_reboot PRIVATE REBOOTEX REDIRECT_FLASHFS)

if(DEBUG)
    target_compile_definitions(vitacompat_reboot PRIVATE -DDEBUG=${DEBUG})
    target_link_libraries(vitacompat_reboot PRIVATE colordebugger)
endif()

add_custom_command(
    TARGET vitacompat_reboot
    POST_BUILD
    COMMAND psp-strip -s vitacompat_reboot
    COMMAND psp-objcopy -O binary vitacompat_reboot vitacompat_reboot.bin
    COMMAND python3 $<TARGET_PROPERTY:tool_gz,EXEC> vitacompat_reboot.bin vitacompat_reboot.bin.gz
)
