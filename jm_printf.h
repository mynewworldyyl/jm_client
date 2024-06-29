#ifndef __JM_PRINTF_H__
#define __JM_PRINTF_H__

#ifdef WIN32
#include "test.h"
#endif

#include "jm_options.h"

//-----------------------------------------------------------------
// Types:
//-----------------------------------------------------------------
typedef int (*FP_OUTBUF_PUTC)(void *arg, char c);

//-----------------------------------------------------------------
// Structures
//-----------------------------------------------------------------
struct vbuf
{
    FP_OUTBUF_PUTC  function;
    void *          function_arg;
    char *          buffer;
    int             offset;
    int             max_length;
};

//-----------------------------------------------------------------
// Prototypes:
//-----------------------------------------------------------------
ICACHE_FLASH_ATTR int     http_vsprintf(char *s, const char *format, va_list arg);
ICACHE_FLASH_ATTR int     http_vsnprintf(char *s, int maxlen, const char *format, va_list arg);
ICACHE_FLASH_ATTR int     http_sprintf(char *s, const char *format, ...);
ICACHE_FLASH_ATTR int     http_snprintf(char *s, int maxlen, const char *format, ...);
ICACHE_FLASH_ATTR int http_xsnprintf(FP_OUTBUF_PUTC outfunc, void *outfunc_arg, const char *format, va_list arg);

#endif //__JM_PRINTF_H__

