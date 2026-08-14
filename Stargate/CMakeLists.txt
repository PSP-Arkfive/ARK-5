add_prx_module(stargate exports.exp)

target_sources(stargate PRIVATE 
   main.c
   imports.S
   src/chn_iso.c
   src/gamefix.c
   src/hide.c
   src/io_patch.c
   src/key_decrypt.c
   src/loadmodule_patch.c
   src/nodrm_patch.c
   src/pspcipher.c
)

target_compile_options(stargate PRIVATE ${COMPILE_OPS})
target_include_directories(stargate PRIVATE include)
target_link_libraries(stargate PRIVATE
   -nostartfiles
   pspsystemctrl_kernel
   pspsysc_user
   pspsemaphore_660
   pspnodrm_helper
   pspsdk
   pspmodinfo
   pspkernel
)
