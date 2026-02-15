add_prx_module(xmbcontrol exports.exp)

target_sources(xmbcontrol PRIVATE 
   main.c
   stub.S
   src/xmbpatch.c
   src/list.c
   src/settings.c
   src/config.c
   src/plugins.c
   src/battery.c
   src/vshmenu.c
   src/utils.c
   src/vshcube.c
)

target_compile_options(xmbcontrol PRIVATE ${COMPILE_OPS} -O2 -fshort-wchar -mno-check-zero-division)
target_include_directories(xmbcontrol PRIVATE include)
target_link_libraries(xmbcontrol PRIVATE
   -nostartfiles
   pspgum
   pspgu
   arkctrl
   pspsystemctrl_user
   pspkubridge
   pspvshctrl
   pspreg
   pspsysc_user
   psppower
   pspdebug
   pspdisplay
   pspge
   pspctrl
   pspnet
   pspnet_apctl
)
