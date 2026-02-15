add_prx_module(inferno exports.exp)

target_sources(inferno PRIVATE 
   main.c
   src/iodrv_funcs.c
   src/umd.c
   src/isoread.c
   src/isocache.c
   imports.S
)

target_compile_options(inferno PRIVATE ${COMPILE_OPS})
target_include_directories(inferno PRIVATE include ${CFWSDK}/include)
target_link_directories(inferno PRIVATE ${CFWSDK}/libs)
target_link_libraries(inferno PRIVATE
   -nostartfiles
   cisoread
   pspsystemctrl_kernel
   pspsdk
   pspmodinfo
   pspkernel
)
