#ifndef USER_DEBUG_H_
#define USER_DEBUG_H_

#include "jm_cons.h"

#ifndef NULL
#define NULL 0
#endif

#include "jm_cfg_def.h"
#include "c_types.h"
#if ESP8266==1
#include "osapi.h"
#endif

#ifdef JM_STM32
#include "stm32_adapter.h"
#endif

#if JMICRO_MEM_DEBUG==1  && JM_ENV == 0
#define MINFO(format, ...)  os_printf(format,## __VA_ARGS__)
#else
#define MINFO(format, ...)
#endif

/*
#define SINFO(format, ...) do { \
      extern sys_config_t sysCfg; \
      if(sysCfg.slogEnable) \
            os_printf(format,## __VA_ARGS__);\
}while(0)
*/


#if SLOG_ENABLE==1 && JM_ENV == 0
#define SINFO(format, ...) do { \
      extern sys_config_t sysCfg; \
      if(sysCfg.slogEnable) \
            os_printf(format,## __VA_ARGS__);\
}while(0)
// #define SINFO(format, ...)  os_printf(format,## __VA_ARGS__)
#else
#define SINFO(format, ...)
#endif


#if JLOG_ENABLE==1 && JM_ENV == 0
#define JINFO(format, ...) do { \
      extern sys_config_t sysCfg; \
      if(sysCfg.jlogEnable==1) \
            SINFO(format, ## __VA_ARGS__);\
      else \
		    SINFO(format,## __VA_ARGS__); \
}while(0)

//#define JINFO(format, ...)  jlog_printf( format, ## __VA_ARGS__)
#else
#define JINFO(format, ...)
#endif

#endif //USER_DEBUG_H_
