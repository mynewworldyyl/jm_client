#ifndef JMICRO_JM_STDCIMPL_H_
#define JMICRO_JM_STDCIMPL_H_
#include "jm_cons.h"
#include "c_types.h"
#include "jm_msg.h"
#include "jm_mem.h"
#include <stdarg.h>

/*
typedef char *va_list;

//
//#include <string.h>

#define va_start(ap,p) (ap = (char *) (&(p)+1))
#define va_arg(ap, type) ((type *) (ap += sizeof(type)))[-1]
#define va_end(ap)
*/


#ifdef __cplusplus
extern "C"
{
#endif

//复制字符串
ICACHE_FLASH_ATTR char* jm_utils_copyStr(char *srcStr);

//判断字符串是否是数字串，忽略前后空格， 中间空格判断为结束
ICACHE_FLASH_ATTR int jm_isDigitStr(unsigned char *pstr);

//判断字符是否是空格
ICACHE_FLASH_ATTR BOOL jm_isBlank(unsigned char c);

//16进制数转换成10进制数
ICACHE_FLASH_ATTR int jm_hexToInt(char *s);

ICACHE_FLASH_ATTR jm_hashmap_t* jm_parseQryParams(char *qryStr);

ICACHE_FLASH_ATTR void jm_toHex(uint8_t* data, char *hex, uint16_t dataLen);

ICACHE_FLASH_ATTR char jm_getHex(uint8_t hexNum, BOOL low);
//6个字节MAC地址转字符串, toAddr 18个字符长度
ICACHE_FLASH_ATTR char* jm_toMacAddrStr(uint8_t* addr);

ICACHE_FLASH_ATTR double jm_strtod(const char * string, char **endPtr);

ICACHE_FLASH_ATTR int jm_strcasecmp(const char *s1, const char *s2);
ICACHE_FLASH_ATTR  int jm_strncasecmp(const char *s1, const char *s2, int n);
ICACHE_FLASH_ATTR char* jm_strrchr(char* range, char elmnt);
ICACHE_FLASH_ATTR char *jm_strcpy(char * dest,const char *src);
ICACHE_FLASH_ATTR char *jm_strcat(char * dest, const char * src);
ICACHE_FLASH_ATTR unsigned int jm_strlen(const char * s);
ICACHE_FLASH_ATTR void * jm_memset(void * s,int c,unsigned int count);
ICACHE_FLASH_ATTR void * jm_memcpy(void * dest,const void *src,unsigned int count);
//ICACHE_FLASH_ATTR int jm_sprintf(char * str, const char *fmt, ...);
ICACHE_FLASH_ATTR uint8_t jm_itoa(unsigned int n, char * buf);
ICACHE_FLASH_ATTR long jm_atoi(char* pstr);
ICACHE_FLASH_ATTR void jm_xtoa(unsigned int n, char * buf);
ICACHE_FLASH_ATTR int jm_isDigit(unsigned char c);
ICACHE_FLASH_ATTR int jm_isLetter(unsigned char c);

ICACHE_FLASH_ATTR int jm_scan(const char *str, const char *fmt, ...);
ICACHE_FLASH_ATTR int jm_print(char *str, uint16_t len, const char *fmt, ...);

ICACHE_FLASH_ATTR uint32_t jm_hash32(char* data, uint16_t len);
ICACHE_FLASH_ATTR uint64_t jm_hash64(char* k, uint16_t len);

#ifdef __cplusplus
}
#endif
#endif /* JMICRO_JM_STDCIMPL_H_ */
