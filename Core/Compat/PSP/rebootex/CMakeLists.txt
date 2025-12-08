add_executable(pspcompat_reboot)
target_link_libraries(pspcompat_reboot PRIVATE bootloadex_psp)
target_compile_definitions(pspcompat_reboot PRIVATE REBOOTEX)

if(DEBUG)
    target_compile_definitions(pspcompat_reboot PRIVATE -DDEBUG=${DEBUG})
    target_link_libraries(pspcompat_reboot PRIVATE colordebugger)
endif()

add_custom_command(
    TARGET pspcompat_reboot
    POST_BUILD
    COMMAND psp-strip -s pspcompat_reboot
    COMMAND psp-objcopy -O binary pspcompat_reboot pspcompat_reboot.bin
    COMMAND python3 $<TARGET_PROPERTY:tool_gz,EXEC> pspcompat_reboot.bin pspcompat_reboot.bin.gz
)
