#ifndef _JM_PORT_H_
#define _JM_PORT_H_

#include "jm_cons.h"
#include "c_types.h"
#include "debug.h"
#include "jm_buffer.h"

#define JM_MAIN_DEBUG_ENABLE 0
#define  JM_MAIN_ERROR_ENABLE 1

#if JM_MAIN_DEBUG_ENABLE==1
#define JM_MAIN_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_MAIN_DEBUG(format, ...)
#endif

#if  JM_MAIN_ERROR_ENABLE==1
#define JM_MAIN_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_MAIN_ERROR(format, ...)
#endif

#define TASK_QUEUE_SIZE 5

typedef void (*jm_eventHandler_fn)(jm_event_t *jevent);

#ifdef __cplusplus
extern "C"
{
#endif

//公开方法
ICACHE_FLASH_ATTR void jm_setup();
ICACHE_FLASH_ATTR void jm_loop();

//内部方法
ICACHE_FLASH_ATTR void jm_runEvent();
ICACHE_FLASH_ATTR void jm_ElseMs(uint32_t mseq);

#ifdef __cplusplus
}
#endif

#endif /* _JM_PORT_H_ */
