add_prx_module(systemctrl exports.exp)

target_sources(systemctrl PRIVATE 
   main.c
	src/modulemanager.c
	src/elf.c
	src/flushcache.c
	src/loadercore.c
	src/cryptography.c
	src/interruptman.c
	src/kubridge.c
	src/sctrl.c
	src/sctrl_se.c
	src/sctrl_hen.c
	src/oe_malloc.c
	src/syspatch.c
	src/mediasync.c
	src/hooknids.c
	src/nidresolver.c
	src/nid_660_data.c
	src/missingfunc.c
	src/rebootex.c
	src/sysmem.c
	src/loadexec.c
	src/exitgame.c
	src/gameinfo.c
	src/plugin.c
	src/leda.c
	src/psnfix.c
	src/cpuclock.c  
	src/msstor_cache.c
	src/threadman.c
	src/utility.c
	src/patches.c
	src/setlongjmp.S
)

target_compile_options(systemctrl PRIVATE ${COMPILE_OPS})
target_include_directories(systemctrl PRIVATE include)
target_link_libraries(systemctrl PRIVATE
   -nostartfiles
	microlz
   pspsemaphore_660
   pspinit_kernel_660
   pspsysmem_kernel_660
   pspsysclib_kernel_660
   psploadcore_kernel_660
   pspmodulemgr_kernel_660
   pspthreadman_kernel_660
   pspinterruptmanager_kernel_660
   pspsdk
   pspmodinfo
   pspkernel
)

if(DEBUG)
	target_sources(systemctrl PRIVATE debug/exception.c debug/exception_asm.S debug/printk.c debug/jumper.c)
   target_link_libraries(systemctrl PRIVATE colordebugger screenprinter)
else()
   target_sources(systemctrl PRIVATE debug/dummy.c)
endif()
