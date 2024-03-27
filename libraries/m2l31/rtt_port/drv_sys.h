#ifndef __DRV_SYS_H__
#define __DRV_SYS_H__

#include <rtthread.h>
#include "NuMicro.h"

struct nu_module
{
    char      *name;
    void      *m_pvBase;
    uint32_t   u32RstId;
    IRQn_Type  eIRQn;
} ;
typedef struct nu_module *nu_module_t;

#endif
