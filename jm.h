#ifndef JMICRO_JM_H_
#define JMICRO_JM_H_

#include "jm_cons.h"

#if JM_BUF_ENABLE==1
#include "jm_buffer.h"
#endif

#if JM_MSG_ENABLE==1 || JM_EMAP_ENABLE==1 ||JM_ELIST_ENABLE==1
#define JM_MSG_EXTRA_ENABLE 1
#include "jm_msg.h"
#endif

#include "jm_client.h"
#include "debug.h"
#include "jm_cfg_def.h"
#include "jm_constants.h"
#include "jm_mem.h"
#include "jm_stdcimpl.h"
#include "jm_mem.h"
//#include "jm_options.h"
//#include "jm_printf.h"
#include "c_types.h"
#include "jm_mem.h"

#endif //JMICRO_JM_H_
