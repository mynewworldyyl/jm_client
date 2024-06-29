#include "jm_cons.h"
#include "c_types.h"
#include "stdarg.h"
#include "jm_stdcimpl.h"
#include "jm_mem.h"
#include "debug.h"
#include "jm_buffer.h"

#define FNV_64_INIT 0xcbf29ce484222325L
#define FNV_64_PRIME 0x100000001b3L

#define FNV_32_INIT 0x811c9dc5
#define FNV_32_PRIME 0x01000193

#if ESP8266==1
#include "user_interface.h"
#include "osapi.h"
#include "ctype.h"
#include "mem.h"
#endif

#ifdef WIN32
#include "stdio.h"
#include "string.h"
#include "test.h"
#include "mem.h"
#endif

extern jm_mem_op *jmm;

static char HEX[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

ICACHE_FLASH_ATTR char jm_getHex(uint8_t hexNum, BOOL low) {
	if(low) {
		return HEX[hexNum & 0x0F];
	}else {
		return HEX[(hexNum>>4) & 0x0F];
	}
}

/**
 * 16进制数转换成10进制数
 * 如：0xE4=14*16+4=228
 */
ICACHE_FLASH_ATTR int jm_hexToInt(char *s) {
    int value;
    int c;

    c = ((unsigned char *)s)[0];
    if (isupper(c))
        c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *)s)[1];
    if (isupper(c))
        c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}


//6个字节MAC地址转字符串, toAddr 18个字符长度
ICACHE_FLASH_ATTR char* jm_toMacAddrStr(uint8_t* addr) {
    JM_STD_DEBUG("Alloc jm_toMacAddrStr %u\n",17);
	char *ptr = (char *)jm_utils_mallocStr(17,"jm_toMacAddrStr");
	for(int i= 0; i < 6; i++) {
		ptr[i*3] = jm_getHex(addr[i], false);
		ptr[i*3+1] = jm_getHex(addr[i], true );
		ptr[i*3+2] = ':';
	}
	ptr[17] = '\0';
	return ptr;
}


ICACHE_FLASH_ATTR uint32_t jm_hash32(char* data, uint16_t len) {
	uint32_t rv = FNV_32_INIT;
	for (int i = 0; i < len; i++) {
		rv = rv ^ data[i];
		rv = rv * FNV_32_PRIME;
	}
	return rv;
}

ICACHE_FLASH_ATTR uint64_t jm_hash64(char* k, uint16_t len) {
	uint64_t rv = FNV_64_INIT;
	for (int i = 0; i < len; i++) {
		rv ^= k[i];
		rv *= FNV_64_PRIME;
	}
	return rv;
}

ICACHE_FLASH_ATTR uint8_t jm_itoa(unsigned int n, char *buf) {
	int i;

	if (n < 10) {
		buf[0] = n + '0';
		buf[1] = '\0';
		return 1;
	}

	jm_itoa(n / 10, buf);

	for (i = 0; buf[i] != '\0'; i++)
		;

	buf[i] = (n % 10) + '0';

	buf[i + 1] = '\0';

	return i+1;
}

ICACHE_FLASH_ATTR long jm_atoi(char* pstr) {
	long int_ret = 0;
	long int_sign = 1;

	if (pstr == NULL)
	{
		return -1;
	}
	while (((*pstr) == ' ') || ((*pstr) == '\n') || ((*pstr) == '\t')
			|| ((*pstr) == '\b')) {
		pstr++;
	}

	if (*pstr == '-') {
		int_sign = -1;
	}
	if (*pstr == '-' || *pstr == '+') {
		pstr++;
	}

	while (*pstr >= '0' && *pstr <= '9')
	{
		int_ret = int_ret * 10 + *pstr - '0';
		pstr++;
	}
	int_ret = int_sign * int_ret;

	return int_ret;
}

ICACHE_FLASH_ATTR void jm_xtoa(unsigned int n, char * buf) {
	int i;

	if (n < 16) {
		if (n < 10) {
			buf[0] = n + '0';
		} else {
			buf[0] = n - 10 + 'a';
		}
		buf[1] = '\0';
		return;
	}
	jm_xtoa(n / 16, buf);

	for (i = 0; buf[i] != '\0'; i++)
		;

	if ((n % 16) < 10) {
		buf[i] = (n % 16) + '0';
	} else {
		buf[i] = (n % 16) - 10 + 'a';
	}
	buf[i + 1] = '\0';
}

ICACHE_FLASH_ATTR int jm_isDigit(unsigned char c) {
	if (c >= '0' && c <= '9')
		return 1;
	else
		return 0;
}

ICACHE_FLASH_ATTR BOOL jm_isBlank(unsigned char c) {
	return (c == ' ') || (c == '\n') || (c == '\t') || (c == '\b')|| (c == '\r');
}

ICACHE_FLASH_ATTR int jm_isDigitStr(unsigned char *pstr) {
	if(pstr == NULL || jm_strlen(pstr) == 0) return false;

	while ( (jm_isBlank(*pstr) || ((*pstr) == '+')  || ((*pstr) == '-'))) {
		pstr++;
	}

	if(!jm_isDigit(*pstr)) {
		//第一个就不是数字,直接返回
		return false;
	}

	//第一个是数字
	pstr++;
	while((*pstr) != '\0') {
		if(jm_isBlank(*pstr)) return true; //遇到空格结束
		if (!jm_isDigit(*pstr)) return false;
		pstr++;
	}
	return true;
}

ICACHE_FLASH_ATTR int jm_isLetter(unsigned char c) {
	if (c >= 'a' && c <= 'z')
		return 1;
	else if (c >= 'A' && c <= 'Z')
		return 1;
	else
		return 0;
}

/**
 * memset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
ICACHE_FLASH_ATTR void * jm_memset(void * s, int c, unsigned int count) {
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

/**
 * strcpy - Copy a %NUL terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 */
ICACHE_FLASH_ATTR char * jm_strcpy(char * dest, const char *src) {
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

/**
 * jm_strlen - Find the length of a string
 * @s: The string to be sized
 */
ICACHE_FLASH_ATTR unsigned int jm_strlen(const char * s) {
    if(s == NULL) return 0;

	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

/**
 * strcat - Append one %NUL-terminated string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 */
ICACHE_FLASH_ATTR char * jm_strcat(char * dest, const char * src) {
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}

/*
 * Private functions.
 */

ICACHE_FLASH_ATTR static int is_space(char c) {
	return (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r'
			|| c == '\n');
}

ICACHE_FLASH_ATTR static char* skip_spaces(const char *str) {
	while (is_space(*str)) {
		++str;
	}
	return (char*) str;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* dec_to_signed(const char *str, long *out) {
	const char * cur = skip_spaces(str);
	long value = 0;
	int isneg = 0, isempty = 1;
	if (cur[0] == '+') {
		cur += 1;
	} else if (cur[0] == '-') {
		cur += 1;
		isneg = 1;
	}
	while (*cur != '\0' && *cur >= '0' && *cur <= '9') {
		value = (value * 10) + (*cur - '0');
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	if (isneg) {
		*out = -value;
	} else {
		*out = value;
	}
	return (char*) cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* dec_to_unsigned(const char *str,
		unsigned long *out) {
	const char * cur = skip_spaces(str);
	unsigned long value = 0;
	int isempty = 1;
	while (*cur != '\0' && *cur >= '0' && *cur <= '9') {
		value = (value * 10) + (*cur - '0');
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	*out = value;
	return (char*) cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* hex_to_signed(const char *str, long *out) {
	const char * cur = skip_spaces(str);
	long value = 0;
	int isneg = 0, isempty = 1;
	if (cur[0] == '+') {
		cur += 1;
	} else if (cur[0] == '-') {
		cur += 1;
		isneg = 1;
	}
	if (cur[0] == '0' && cur[1] == 'x') {
		cur += 2;
	}
	while (*cur != '\0') {
		if (*cur >= '0' && *cur <= '9') {
			value = (value * 16) + (*cur - '0');
		} else if (*cur >= 'a' && *cur <= 'f') {
			value = (value * 16) + 10 + (*cur - 'a');
		} else if (*cur >= 'A' && *cur <= 'F') {
			value = (value * 16) + 10 + (*cur - 'A');
		} else {
			break;
		}
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	if (isneg) {
		*out = -value;
	} else {
		*out = value;
	}
	return (char*) cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* hex_to_unsigned(const char *str,
		unsigned long *out) {
	const char * cur = skip_spaces(str);
	unsigned long value = 0;
	int isempty = 1;
	if (cur[0] == '0' && cur[1] == 'x') {
		cur += 2;
	}
	while (*cur != '\0') {
		if (*cur >= '0' && *cur <= '9') {
			value = (value * 16) + (*cur - '0');
		} else if (*cur >= 'a' && *cur <= 'f') {
			value = (value * 16) + 10 + (*cur - 'a');
		} else if (*cur >= 'A' && *cur <= 'F') {
			value = (value * 16) + 10 + (*cur - 'A');
		} else {
			break;
		}
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	*out = value;
	return (char*) cur;
}

#define MFMT_DEC_TO_SIGNED(TYPE, NAME)                          \
ICACHE_FLASH_ATTR static char*                                                    \
dec_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        long v;                                                 \
        char *cur = dec_to_signed(str, &v);                     \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

#define MFMT_DEC_TO_UNSIGNED(TYPE, NAME)                        \
ICACHE_FLASH_ATTR static char*                                                    \
dec_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        unsigned long v;                                        \
        char *cur = dec_to_unsigned(str, &v);                   \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

#define MFMT_HEX_TO_SIGNED(TYPE, NAME)                                  \
ICACHE_FLASH_ATTR static char*                                                            \
hex_to_##NAME(const char *str, TYPE *out)                               \
{                                                                       \
        long v;                                                         \
        char *cur = hex_to_signed(str, &v);                             \
        if (cur != str){                                                \
                *out = (TYPE)v;                                         \
        }                                                               \
        return cur;                                                     \
}

#define MFMT_HEX_TO_UNSIGNED(TYPE, NAME)                        \
ICACHE_FLASH_ATTR static char*                                                    \
hex_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        unsigned long v;                                        \
        char *cur = hex_to_unsigned(str, &v);                   \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_SIGNED_TO_HEX(TYPE, NAME)                                  \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_hex(TYPE val, int uppercase, char padchar, uint16_t padlen,     \
              uint16_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        uint16_t isneg = 0, cnt = 0;                                      \
        if (uppercase){                                                 \
                uppercase = 'A';                                        \
        }else{                                                          \
                uppercase = 'a';                                        \
        }                                                               \
        if (val < 0){                                                   \
                isneg = 1;                                              \
                val = -val;                                             \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 16;                                  \
                val = val / 16;                                         \
        }while (val != 0);                                              \
        if (padlen > isneg + cnt){                                      \
                padlen -= isneg + cnt;                                  \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        if (isneg && len > 0){                                          \
                str[0] = '-';                                           \
                str += 1;                                               \
                len -= 1;                                               \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                if (buf[cnt] < 10){                                     \
                        *str = buf[cnt] + '0';                          \
                }else{                                                  \
                        *str = (buf[cnt] - 10) + uppercase;             \
                }                                                       \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_SIGNED_TO_DEC(TYPE, NAME)                                  \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_dec(TYPE val, char padchar, uint16_t padlen,                    \
              uint16_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        uint16_t isneg = 0, cnt = 0;                                      \
        if (val < 0){                                                   \
                isneg = 1;                                              \
                val = -val;                                             \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 10;                                  \
                val = val / 10;                                         \
        }while (val != 0);                                              \
        if (padlen > isneg + cnt){                                      \
                padlen -= isneg + cnt;                                  \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        if (isneg && len > 0){                                          \
                str[0] = '-';                                           \
                str += 1;                                               \
                len -= 1;                                               \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                *str = buf[cnt] + '0';                                  \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_UNSIGNED_TO_HEX(TYPE, NAME)                                \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_hex(TYPE val, int uppercase, char padchar, uint16_t padlen,     \
              uint16_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        uint16_t cnt = 0;                                                 \
        if (uppercase){                                                 \
                uppercase = 'A';                                        \
        }else{                                                          \
                uppercase = 'a';                                        \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 16;                                  \
                val = val / 16;                                         \
        }while (val != 0);                                              \
        if (padlen > cnt){                                              \
                padlen -= cnt;                                          \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                if (buf[cnt] < 10){                                     \
                        *str = buf[cnt] + '0';                          \
                }else{                                                  \
                        *str = (buf[cnt] - 10) + uppercase;             \
                }                                                       \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_UNSIGNED_TO_DEC(TYPE, NAME)                                \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_dec(TYPE val, char padchar, uint16_t padlen,                    \
              uint16_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        uint16_t cnt = 0;                                                 \
        do{                                                             \
                buf[cnt++] = val % 10;                                  \
                val = val / 10;                                         \
        }while (val != 0);                                              \
        if (padlen > cnt){                                              \
                padlen -= cnt;                                          \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                *str = buf[cnt] + '0';                                  \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

MFMT_DEC_TO_SIGNED(int, int)
MFMT_HEX_TO_SIGNED(int, int)
MFMT_SIGNED_TO_DEC(int, int)
MFMT_SIGNED_TO_HEX(int, int)

MFMT_DEC_TO_UNSIGNED(unsigned int, uint)
MFMT_HEX_TO_UNSIGNED(unsigned int, uint)
MFMT_UNSIGNED_TO_DEC(unsigned int, uint)
MFMT_UNSIGNED_TO_HEX(unsigned int, uint)
MFMT_UNSIGNED_TO_HEX(uint16_t, siz)

ICACHE_FLASH_ATTR static const char* jm_parse_arg(const char *fmt,
		const char *str, va_list args) {
	int *intp, intv = 0;
	unsigned int *uintp, uintv = 0, width = 0;
	char *charp;
	const char *cur = str;
	fmt = dec_to_uint(fmt, &width);
	if (*fmt == 'd') {
		cur = dec_to_int(str, &intv);
		if (cur != str) {
			intp = va_arg(args, int*);
			*intp = intv;
		}
	} else if (*fmt == 'u') {
		cur = dec_to_uint(str, &uintv);
		if (cur != str) {
			uintp = va_arg(args, unsigned int*);
			*uintp = uintv;
		}
	} else if (*fmt == 'x' || *fmt == 'X') {
		cur = hex_to_uint(str, &uintv);
		if (cur != str) {
			uintp = va_arg(args, unsigned int*);
			*uintp = uintv;
		}
	} else if (*fmt == 'c') {
		charp = va_arg(args, char*);
		if (width == 0) {
			width = 1;
		}
		while (cur[0] != '\0' && uintv < width) {
			charp[uintv] = cur[0];
			++cur;
			++uintv;
		}
	} else if (*fmt == 's') {
		charp = va_arg(args, char*);
		while (cur[0] != '\0' && !is_space(cur[0])
				&& (width == 0 || uintv < width)) {
			charp[uintv] = cur[0];
			++cur;
			++uintv;
		}
		charp[uintv] = '\0';
	} else if (*fmt == '%' && str[0] == '%') {
		++cur;
	}
	return cur;
}

ICACHE_FLASH_ATTR static char* jm_print_arg(const char *fmt, char *str,
                                            uint16_t len, va_list args) {
	unsigned int uintv, width = 0;
    uint16_t charplen = 0, padlen = 0;
	int intv;
	char *charp, padchar = (*fmt == '0' ? '0' : ' ');
	fmt = dec_to_uint(fmt, &width);
	if (*fmt == 'd' || *fmt == 'i') {
		intv = va_arg(args, int);
		str = int_to_dec(intv, padchar, width, len, str);
	} else if (*fmt == 'u') {
		uintv = va_arg(args, unsigned int);
		str = uint_to_dec(uintv, padchar, width, len, str);
	} else if (*fmt == 'x' || *fmt == 'X') {
		uintv = va_arg(args, unsigned int);
		str = uint_to_hex(uintv, (*fmt == 'X'), padchar, width, len, str);
	} else if (*fmt == 'p') {
		charp = (char*) va_arg(args, void*);
		str = siz_to_hex((uint16_t) charp, 0, padchar, width, len, str);
	} else if (*fmt == 'c') {
		intv = va_arg(args, int);
		if (width > 1) {
			padlen = (uint16_t) width - 1;
			padlen = (padlen < len ? padlen : len);
			memset(str, ' ', padlen);
			str += padlen;
			len -= padlen;
		}
		if (len > 0) {
			str[0] = (char) intv;
			str += 1;
			len -= 1;
		}
	} else if (*fmt == 's') {
		charp = va_arg(args, char*);
		charplen = strlen(charp);
		if (width > 0 && (uint16_t) width > charplen) {
			padlen = (uint16_t) width - charplen;
			padlen = (padlen < len ? padlen : len);
			memset(str, ' ', padlen);
			str += padlen;
			len -= padlen;
		}
		charplen = (charplen < len ? charplen : len);
		memcpy(str, charp, charplen);
		str += charplen;
		len -= charplen;
	} else if (*fmt == '%') {
		str[0] = '%';
		++str;
	}
	return str;
}

/*
 * Public functions.
 */

ICACHE_FLASH_ATTR int jm_print(char *str, uint16_t len, const char *fmt, ...) {
	va_list args;
	int cnt = 0;
	char *tmp, *cur = str;
	if (len == 0) {
		return cnt;
	}
	--len;
	va_start(args, fmt);
	while (fmt[0] != '\0' && len > 0) {
		if (fmt[0] == '%') {
			tmp = jm_print_arg(&fmt[1], cur, len, args);
			if (tmp == cur) {
				break;
			}
			len -= (tmp - cur);
			cur = tmp;
			++fmt;
			while (fmt[0] >= '0' && fmt[0] <= '9') {
				++fmt;
			}
			++fmt;
		} else {
			cur[0] = fmt[0];
			--len;
			++cur;
			++fmt;
		}
	} va_end(args);
	cnt = (int) (cur - str);
	str[cnt] = '\0';
	return cnt;
}

ICACHE_FLASH_ATTR int jm_scan(const char *str, const char *fmt, ...) {
	int ret = 0;
	va_list args;
	va_start(args, fmt);
	while (fmt[0] != '\0' && str[0] != '\0') {
		if (fmt[0] == '%') {
			const char * tmp = jm_parse_arg(&fmt[1], str, args);
			if (tmp == str) {
				break;
			}
			if (fmt[1] != '%') {
				++ret;
			}
			++fmt;
			while (fmt[0] >= '0' && fmt[0] <= '9') {
				++fmt;
			}
			++fmt;
			str = tmp;
		} else if (is_space(fmt[0])) {
			++fmt;
			str = skip_spaces(str);
		} else if (fmt[0] == str[0]) {
			++fmt;
			++str;
		} else {
			break;
		}
	}

	va_end(args);
	return ret;
}

static int maxExponent = 511;	/* Largest possible base 10 exponent.  Any
				 * exponent larger than this will already
				 * produce underflow or overflow, so there's
				 * no need to worry about additional digits.
				 */
static double powersOf10[] = {	/* Table giving binary powers of 10.  Entry */
    10.,			/* is 10^2^i.  Used to convert decimal */
    100.,			/* exponents into floating-point numbers. */
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};

/*
 *----------------------------------------------------------------------
 *
 * strtod --
 *
 *	This procedure converts a floating-point number from an ASCII
 *	decimal representation to internal double-precision format.
 *
 * Results:
 *	The return value is the double-precision floating-point
 *	representation of the characters in string.  If endPtr isn't
 *	NULL, then *endPtr is filled in with the address of the
 *	next character after the last one that was part of the
 *	floating-point number.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

ICACHE_FLASH_ATTR double jm_strtod(const char * string, char **endPtr)
    /* const char *string;	A decimal ASCII floating-point number,
				 * optionally preceded by white space.
				 * Must have form "-I.FE-X", where I is the
				 * integer part of the mantissa, F is the
				 * fractional part of the mantissa, and X
				 * is the exponent.  Either of the signs
				 * may be "+", "-", or omitted.  Either I
				 * or F may be omitted, or both.  The decimal
				 * point isn't necessary unless F is present.
				 * The "E" may actually be an "e".  E and X
				 * may both be omitted (but not just one).
				 */
    /* char **endPtr;		If non-NULL, store terminating character's
				 * address here. */
{
	int sign, expSign = FALSE;
	double fraction, dblExp, *d;
	register const char *p;
	register int c;
	int exp = 0; /* Exponent read from "EX" field. */
	int fracExp = 0; /* Exponent that derives from the fractional
	 * part.  Under normal circumstatnces, it is
	 * the negative of the number of digits in F.
	 * However, if I is very long, the last digits
	 * of I get dropped (otherwise a long I with a
	 * large negative exponent could cause an
	 * unnecessary overflow on I alone).  In this
	 * case, fracExp is incremented one for each
	 * dropped digit. */
	int mantSize; /* Number of digits in mantissa. */
	int decPt; /* Number of mantissa digits BEFORE decimal
	 * point. */
	const char *pExp; /* Temporarily holds location of exponent
	 * in string. */

	/*
	 * Strip off leading blanks and check for a sign.
	 */

	p = string;
	while (isspace(*p)) {
		p += 1;
	}
	if (*p == '-') {
		sign = TRUE;
		p += 1;
	} else {
		if (*p == '+') {
			p += 1;
		}
		sign = FALSE;
	}

	/*
	 * Count the number of digits in the mantissa (including the decimal
	 * point), and also locate the decimal point.
	 */

	decPt = -1;
	for (mantSize = 0;; mantSize += 1) {
		c = *p;
		if (!isdigit(c)) {
			if ((c != '.') || (decPt >= 0)) {
				break;
			}
			decPt = mantSize;
		}
		p += 1;
	}

	/*
	 * Now suck up the digits in the mantissa.  Use two integers to
	 * collect 9 digits each (this is faster than using floating-point).
	 * If the mantissa has more than 18 digits, ignore the extras, since
	 * they can't affect the value anyway.
	 */

	pExp = p;
	p -= mantSize;
	if (decPt < 0) {
		decPt = mantSize;
	} else {
		mantSize -= 1; /* One of the digits was the point. */
	}
	if (mantSize > 18) {
		fracExp = decPt - 18;
		mantSize = 18;
	} else {
		fracExp = decPt - mantSize;
	}
	if (mantSize == 0) {
		fraction = 0.0;
		p = string;
		goto done;
	} else {
		int frac1, frac2;
		frac1 = 0;
		for (; mantSize > 9; mantSize -= 1) {
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac1 = 10 * frac1 + (c - '0');
		}
		frac2 = 0;
		for (; mantSize > 0; mantSize -= 1) {
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac2 = 10 * frac2 + (c - '0');
		}
		fraction = (1.0e9 * frac1) + frac2;
	}

	/*
	 * Skim off the exponent.
	 */

	p = pExp;
	if ((*p == 'E') || (*p == 'e')) {
		p += 1;
		if (*p == '-') {
			expSign = TRUE;
			p += 1;
		} else {
			if (*p == '+') {
				p += 1;
			}
			expSign = FALSE;
		}
		while (isdigit(*p)) {
			exp = exp * 10 + (*p - '0');
			p += 1;
		}
	}
	if (expSign) {
		exp = fracExp - exp;
	} else {
		exp = fracExp + exp;
	}

	/*
	 * Generate a floating-point number that represents the exponent.
	 * Do this by processing the exponent one bit at a time to combine
	 * many powers of 2 of 10. Then combine the exponent with the
	 * fraction.
	 */

	if (exp < 0) {
		expSign = TRUE;
		exp = -exp;
	} else {
		expSign = FALSE;
	}
	if (exp > maxExponent) {
		exp = maxExponent;
		//errno = 34;
	}
	dblExp = 1.0;
	for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
		if (exp & 01) {
			dblExp *= *d;
		}
	}
	if (expSign) {
		fraction /= dblExp;
	} else {
		fraction *= dblExp;
	}

	done: if (endPtr != NULL) {
		*endPtr = (char *) p;
	}

	if (sign) {
		return -fraction;
	}
	return fraction;
}

ICACHE_FLASH_ATTR int jm_strcmp(const char *s1, const char *s2) {
	return jm_strcasecmp(s1,s2);
}

ICACHE_FLASH_ATTR int jm_strcasecmp(const char *s1, const char *s2) {
    int offset,ch;
    unsigned char a,b;

    int s1len = jm_strlen(s1);
    int s2len = jm_strlen(s2);

    if(s1len == 0 && s2len == 0) return 0;
    if(s1len == 0 && s2len > 0) return -1;
    if(s1len > 0 && s2len == 0) return 1;

    offset = 0;
    ch = 1;
    while( *(s1+offset) != '\0' )
    {
        /* check for end of s2 */
        if( *(s2+offset)=='\0')
            return( *(s1+offset) );

        a = (unsigned)*(s1+offset);
        b = (unsigned)*(s2+offset);
        ch = toupper(a) - toupper(b);
        if( ch<0 || ch>0 )
            return(ch);
        offset++;
    }

    return(ch);
}

ICACHE_FLASH_ATTR  int jm_strncasecmp(const char *s1, const char *s2, int n) {

	int s1len = jm_strlen(s1);
	int s2len = jm_strlen(s2);

	if(s1len == 0 && s2len == 0) return 0;
	if(s1len == 0 && s2len > 0) return -1;
	if(s1len > 0 && s2len == 0) return 1;

    if (n && s1 != s2) {
        do {
            int d = tolower(*s1) - tolower(*s2);
            if (d || *s1 == '\0' || *s2 == '\0') return d;
            s1++;
            s2++;
        } while (--n);
    }
    return 0;
}

ICACHE_FLASH_ATTR char* jm_strrchr(char* range, char elmnt){
    char* temp=0;
    while(*range!='\0'){
        if(*range== elmnt){
           temp=range;
        }
        range++;
    }
    return temp;
}

/***********************************************URL Parse begin*******************************************/

ICACHE_FLASH_ATTR sint32_t _findNextAtCharBeforeEqualChar(char *qryStr, uint32_t totalLen , uint32_t fromIdx){
    sint32_t ret = -1;
    while(fromIdx < totalLen) {
        if(qryStr[fromIdx] == '=') {
            return ret;//在=号前没有&符号了
        }else if(qryStr[fromIdx] == '&'){
            ret = fromIdx;
        }
        ++fromIdx;
    }
    return ret;
}


ICACHE_FLASH_ATTR jm_hashmap_t* jm_parseQryParams(char *qryStr ) {

    if(qryStr == NULL) return NULL;

    jm_hashmap_t *ps = jm_hashmap_create(10, PREFIX_TYPE_STRINGG);
    ps->needFreeVal = true;

    if(ps == NULL) {
        JM_STD_ERROR("jmq memout\n");
        return NULL;
    }
   // char *qryStr = "a=b&b=c";
    uint32_t totalLen = jm_strlen(qryStr);

    uint8_t len = 100;
	char key[len];
    char val[len];
    //char val[100];
	memset(key,0,len);
	memset(val,0,len);

    uint8_t kidx = 0, vidx = 0;

    //char *ks = NULL;

    uint8_t i=0;
    uint8_t findKey = true;
    char c;
    BOOL set = true;

    while((c=qryStr[i]) != '\0') {

        char c = qryStr[i++];
        if(findKey) {
            //当前查找KEY
            if(c == '=') {
                findKey = false; //进入查找值模式
            } else {
                key[kidx++] = c;
            }
        } else {

            if(c == '&') {
                int nextAtCharBeforeEqualChar = _findNextAtCharBeforeEqualChar(qryStr,totalLen,i);
                if(nextAtCharBeforeEqualChar > 0) {
                    memcpy(val + vidx,qryStr+i-1,nextAtCharBeforeEqualChar - i+1);
                    vidx += (nextAtCharBeforeEqualChar - i + 1);
                    i = nextAtCharBeforeEqualChar + 1;
                }
                char *vs = jm_utils_mallocStr(vidx, "hashmap create key");
                if(vidx > 0){
                    memcpy(vs,val,vidx);
                    memset(val,0,vidx);
                }
                vs[vidx] = '\0';
                jm_hashmap_put(ps,key,vs);

				memset(key,0,kidx);

                vidx = kidx = 0;
                findKey = true; //进入查找值模式
                set = true;
            } else {
                set = false;
                val[vidx++] = c;
            }
        }
    }

    //最后一个值
    if(!set) {
        char *vs = jm_utils_mallocStr(vidx, "hashmap create val");
        if(vs == NULL) {
            JM_STD_DEBUG("jmq memout alloc size: %d\n",(vidx+1));
            return ps;//少一条数据，但是总比返回NULL具有更好的用户体验
        }

        if(vidx > 0){
            memcpy(vs,val,vidx);
        }
        vs[vidx] = '\0';
        jm_hashmap_put(ps,key,vs);

    }

    return ps;

}

/***********************************************URL Parse end*******************************************/


