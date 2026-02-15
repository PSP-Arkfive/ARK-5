add_prx_module(vshcontrol exports.exp)

target_sources(vshcontrol PRIVATE 
   main.c
   imports.S
   src/vshpatch.c
   src/xmbiso.c
   src/isoreader.c
   src/virtual_pbp.c
   src/virtual_mp4.c
   src/dirent_track.c
   src/disctype.c
   src/vshmenu.c
   src/usbdevice.c
   src/custom_update.c
   src/hibernation.c
   src/registry.c
)

target_compile_options(vshcontrol PRIVATE ${COMPILE_OPS})
target_include_directories(vshcontrol PRIVATE include)
target_link_directories(vshcontrol PRIVATE libs)
target_link_libraries(vshcontrol PRIVATE
   -nostartfiles
   cisoread
   arkctrl
   pspsystemctrl_kernel
   pspusb
   pspusbdevice_driver
   pspreg
   pspsdk
   pspmodinfo
   pspkernel
)
