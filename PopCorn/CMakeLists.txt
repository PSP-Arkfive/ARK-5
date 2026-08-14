add_prx_module(popcorn exports.exp)

target_sources(popcorn PRIVATE 
   main.c
   src/icon.c
   src/libcrypt.c
   src/syspatch.c
)

target_compile_options(popcorn PRIVATE ${COMPILE_OPS})
target_include_directories(popcorn PRIVATE include ${CFWSDK}/include)
target_link_directories(popcorn PRIVATE ${CFWSDK}/libs)
target_link_libraries(popcorn PRIVATE
   -nostartfiles
   pspsystemctrl_kernel
   pspsemaphore_660
   pspnodrm_helper
   pspsdk
   pspmodinfo
   pspkernel
)
