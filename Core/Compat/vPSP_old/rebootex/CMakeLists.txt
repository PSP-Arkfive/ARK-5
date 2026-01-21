add_executable(pentazemin_reboot)
target_sources(pentazemin_reboot PRIVATE patches.c)
target_link_libraries(pentazemin_reboot PRIVATE bootloadex_vita)
target_compile_definitions(pentazemin_reboot PRIVATE REBOOTEX)

if(DEBUG)
    target_compile_definitions(pentazemin_reboot PRIVATE -DDEBUG=${DEBUG})
    target_link_libraries(pentazemin_reboot PRIVATE colordebugger)
endif()

add_custom_command(
    TARGET pentazemin_reboot
    POST_BUILD
    COMMAND psp-strip -s pentazemin_reboot
    COMMAND psp-objcopy -O binary pentazemin_reboot pentazemin_reboot.bin
    COMMAND python3 $<TARGET_PROPERTY:tool_gz,EXEC> pentazemin_reboot.bin pentazemin_reboot.bin.gz
)
