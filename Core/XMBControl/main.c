#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <psputility_sysparam.h>

#include <ark.h>
#include <cfwmacros.h>
#include <kubridge.h>
#include <vshctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <rebootexconfig.h>

#include "main.h"
#include "utils.h"
#include "list.h"
#include "settings.h"
#include "plugins.h"

PSP_MODULE_INFO("XmbControl", 0x0007, 2, 1);

int module_start(SceSize args, void *argp)
{

    RebootexConfigARK* (*getRebootexConfig)(RebootexConfigARK*) = 
        (void*)sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", 0x18B687A6);
    struct KernelCallArg kargs;
    kargs.arg1 = (u32)&rebootex_config;
    kuKernelCall(getRebootexConfig, &kargs);

    psp_model = kuKernelGetModel();

    sctrlSEGetConfig((SEConfig*)&se_config);

    sctrlArkGetConfig(&ark_config);

    findAllTranslatableStrings();
    
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);

    vshcube_init();

    return 0;
}
