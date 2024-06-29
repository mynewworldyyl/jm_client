#ifndef _JM_CMD_H_
#define _JM_CMD_H_

#include "jm_cons.h"
#include "c_types.h"
#include "jm_mem.h"

typedef char* (*cmd_fn)(jm_hashmap_t *ps);

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR char* cmd_exe(jm_hashmap_t *params);
ICACHE_FLASH_ATTR BOOL cmd_registFun(char *name, cmd_fn f);

#ifdef __cplusplus
}
#endif

#endif /* _JM_CMD_H_ */