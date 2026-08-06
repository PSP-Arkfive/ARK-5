add_prx_module(stargate exports.exp)

target_sources(stargate PRIVATE 
   main.c
   imports.S
   src/loadmodule_patch.c
   src/nodrm_patch.c
   src/io_patch.c
   src/key_decrypt.c
   src/pspcipher.c
   src/gamefix.c
   src/hide.c
   src/chn_iso.c
)

target_compile_options(stargate PRIVATE ${COMPILE_OPS})
target_include_directories(stargate PRIVATE include)
target_link_libraries(stargate PRIVATE
   -nostartfiles
   pspsystemctrl_kernel
   pspsysc_user
   pspsemaphore_660
   pspsdk
   pspmodinfo
   pspkernel
)
