#ifndef JMICRO_JM_CONS_H_
#define JMICRO_JM_CONS_H_

#if ESP8266==1
#include "jm_esp8266.h"
#elif JM_STM32==1
#include "jm_stm32.h"
#elif JM_ESP32==1
#include "jm_esp32.h"
#endif

#ifndef JM_NETPROXY
#define JM_NETPROXY 1
#endif

#ifndef JM_LOGIN_ENABLE
#define JM_LOGIN_ENABLE 1
#endif

#ifndef JM_HB_ENABLE
#define JM_HB_ENABLE 1
#endif

#ifndef JM_ENV
#define JM_ENV 0
#endif








#ifndef SLOG_ENABLE
#define SLOG_ENABLE 1
#endif

#if JM_PS_ENABLE==1
#ifndef JLOG_ENABLE
#define JLOG_ENABLE 0
#endif
#else
#define JLOG_ENABLE 0
#endif //JM_PS_ENABLE==1

#ifndef DEBUG_MEMORY
#define DEBUG_MEMORY 0
#endif

#ifndef JM_RPC_ENABLE
#define JM_RPC_ENABLE 1
#endif

#ifndef JM_PS_ENABLE
#define JM_PS_ENABLE 0
#endif

#ifndef JM_KV_ENABLE
#define JM_KV_ENABLE 0
#endif

#ifndef JM_TIMER_ENABLE
#define JM_TIMER_ENABLE 1
#endif

#ifndef JM_ELIST_ENABLE
#define JM_ELIST_ENABLE 1
#endif

#ifndef JM_EMAP_ENABLE
#define JM_EMAP_ENABLE 1
#endif

#ifndef JM_BUF_ENABLE
#define JM_BUF_ENABLE 1
#endif

#ifndef JM_STD_TIME_ENABLE
#define JM_STD_TIME_ENABLE 0
#endif

#ifndef JM_HB_ENABLE
#if JM_RPC_ENABLE==1 && JM_NETPROXY==0 //网卡暂时不需要心跳
#define JM_HB_ENABLE 1
#else
#define JM_HB_ENABLE 0
#endif
#endif

#ifndef JM_MSG_ENABLE
#define JM_MSG_ENABLE 1
#endif

#ifndef JM_CLI_DEBUG_ENABLE
#define JM_CLI_DEBUG_ENABLE 0
#endif

#ifndef JM_CLI_ERROR_ENABLE
#define JM_CLI_ERROR_ENABLE 0
#endif

#ifndef JM_BUF_DEBUG_ENABLE
#define JM_BUF_DEBUG_ENABLE 0
#endif

#ifndef JM_BUF_ERROR_ENABLE
#define JM_BUF_ERROR_ENABLE 0
#endif

#ifndef JM_MSG_DEBUG_ENABLE
#define JM_MSG_DEBUG_ENABLE 0
#endif

#ifndef JM_MSG_ERROR_ENABLE
#define JM_MSG_ERROR_ENABLE 0
#endif

#ifndef JM_MEM_DEBUG_ENABLE
#define JM_MEM_DEBUG_ENABLE 0
#endif

#ifndef JM_MEM_ERROR_ENABLE
#define JM_MEM_ERROR_ENABLE 0
#endif

#ifndef JM_STD_DEBUG_ENABLE
#define JM_STD_DEBUG_ENABLE 0
#endif

#ifndef JM_STD_ERROR_ENABLE
#define JM_STD_ERROR_ENABLE 0
#endif

#if JM_CLI_ERROR_ENABLE==1
#define JM_CLI_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_CLI_ERROR(format, ...)
#endif

#if JM_CLI_DEBUG_ENABLE==1
#define JM_CLI_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_CLI_DEBUG(format, ...)
#endif

#if  JM_STD_ERROR_ENABLE==1
#define JM_STD_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_STD_ERROR(format, ...)
#endif



#if JM_BUF_DEBUG_ENABLE==1
#define JM_BUF_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_BUF_DEBUG(format, ...)
#endif

#if JM_BUF_ERROR_ENABLE==1
#define JM_BUF_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_BUF_ERROR(format, ...)
#endif

#if JM_MSG_DEBUG_ENABLE==1
#define JM_MSG_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_MSG_DEBUG(format, ...)
#endif

#if JM_MSG_ERROR_ENABLE==1
#define JM_MSG_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_MSG_ERROR(format, ...)
#endif

#if JM_MEM_DEBUG_ENABLE==1
#define JM_MEM_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_MEM_DEBUG(format, ...)
#endif

#if JM_MEM_ERROR_ENABLE==1
#define JM_MEM_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_MEM_ERROR(format, ...)
#endif

#if JM_STD_DEBUG_ENABLE==1
#define JM_STD_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_STD_DEBUG(format, ...)
#endif

#if JM_STD_ERROR_ENABLE==1
#define JM_STD_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_STD_ERROR(format, ...)
#endif

#endif //JMICRO_JM_CONS_H_
