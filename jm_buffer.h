/*
 * jm_buffer.h
 *
 *  Created on: 2023锟斤拷4锟斤拷9锟斤拷
 *      Author: yeyulei
 */


#ifndef JMICRO_JM_BUFFER_H_
#define JMICRO_JM_BUFFER_H_

#include "jm_cons.h"
#include "c_types.h"

#define WRITE_BUF_SIZE 512

//#define READ_BUF_SIZE 512

#define MEM_REALLOC_BLOCK_SIZE  128

#define MEM_REALLOC_ADD_SIZE  16

#define MAX_SHORT_VALUE  32767

#define MAX_BYTE_VALUE  127

#define MAX_INT_VALUE  (1024*1024*10)  //10M

//#define EXTRA_KEY_TYPE_BYTE 0
//#define EXTRA_KEY_TYPE_STRING 1

#define BB_EMPTY 0x01
#define BB_FULL 0x02

#ifdef NET_DATA_LITTLE_END
#define NET_DATA_BIG_END false
#else
#define NET_DATA_BIG_END true
#endif

typedef struct {
    void *data;
    uint8_t type;
    uint16_t subType;
    uint8_t flag;
}jm_event_t;

typedef void (*jm_evnet_listener)(jm_event_t *event);

typedef struct _jm_mem_op {
	//void (*jm_postEvent)(uint32_t evt);
    void* (*jm_zalloc_fn)(unsigned sz,sint8_t dataType);
    void (*jm_free_fn)(void *ptr,unsigned n);
    void* (*jm_realloc_fn)(void *, unsigned);
    void* (*jm_memcpy)(void *dest, const void *src,unsigned count);
    void* (*jm_memset)(void *s, int c, unsigned count);
    void(*jm_resetSys)(char *cause);
    void(*jm_delay)(uint32_t delauInMs);

	void (*jm_postEvent)(uint8_t evt, uint16_t subType, void *data, uint8_t flag);
	BOOL (*jm_regEventListener)(uint8_t key, jm_evnet_listener lis);
	BOOL (*jm_unregEventListener)(uint8_t key, jm_evnet_listener lis);
} jm_mem_op;

typedef enum _client_send_msg_result{
	JM_SUCCESS=-100,
	SOCKET_SENDER_NULL=-99,//底层SOCKET没建立
	ENCODE_MSG_FAIL=-98,//消息编码失败
	HANDLE_MSG_FAIL=-97,//消息处理器未找到
	MSG_CREATE_FAIL=-96,//消息创建失败
	MEMORY_OUTOF_RANGE=-95,//内存申请失败，也就是内存溢出
	MSG_WAIT_NOT_FOUND=-94,//没找到等待响应的消息
	SEND_DATA_ERROR=-93,//发送数据错误
	NO_DATA_TO_SEND=-92,//无数据可发送,
	INVALID_PS_DATA=-91, //PUBSUB数据无效
	INVALID_RESP_DATA=-90,//RPC接收到无效数据
	SEND_QUEQUE_EXCEED=-89, //发送队列已满
	SEND_INVALID_ACCOUNT=-88, //无效账号ID
	SEND_INVALID_DEVICE_ID=-87,//无效设备ID
	BUSSY=-86,//忙

} jm_cli_send_msg_result_t;


typedef struct _jm_buffer
{
	uint8_t *data;
	uint16_t capacity;
	//锟斤拷一锟斤拷锟街节存储锟斤拷前锟斤拷锟斤拷状态
	uint8_t status;
	uint16_t rpos ;

	uint16_t wpos;

	struct _jm_buffer *wrap_buf;
	BOOL rw_flag;// true:只读；false:只写

	BOOL needReleaseData;

	int16_t rmark ;//锟斤拷录锟斤拷前锟斤拷rpos位锟矫ｏ拷锟斤拷锟斤拷rmark_reset锟斤拷原
	uint8_t rmark_status;//锟斤拷录锟斤拷前锟斤拷status锟斤拷锟斤拷锟斤拷rmark_reset锟斤拷原

} jm_buf_t;

/*
 * jm_buffer.c
 *
 *  Created on: 2023锟斤拷4锟斤拷9锟斤拷
 *      Author: yeyulei
 */



#ifdef __cplusplus
extern "C"
{
#endif

void ICACHE_FLASH_ATTR jm_buf_print(jm_buf_t *buf, uint32_t startPos, uint32_t len);

void ICACHE_FLASH_ATTR jm_buf_clear(jm_buf_t *buf);

void ICACHE_FLASH_ATTR jm_buf_rmark(jm_buf_t *buf);
BOOL ICACHE_FLASH_ATTR jm_buf_rmark_reset(jm_buf_t *buf);

BOOL ICACHE_FLASH_ATTR jm_buf_reset(jm_buf_t *buf);

uint16_t ICACHE_FLASH_ATTR jm_buf_get_rpos(jm_buf_t *buf);

uint16_t ICACHE_FLASH_ATTR jm_buf_get_wpos(jm_buf_t *buf);

ICACHE_FLASH_ATTR char* jm_buf_readString(jm_buf_t *buf,int8_t *flag);

BOOL ICACHE_FLASH_ATTR jm_buf_set_rpos(jm_buf_t *buf, uint16_t rpos);
BOOL ICACHE_FLASH_ATTR jm_buf_set_wpos(jm_buf_t *buf, uint16_t wpos);
BOOL ICACHE_FLASH_ATTR jm_buf_move_forward(jm_buf_t * buf, uint16_t forwarnCnt);

jm_buf_t * ICACHE_FLASH_ATTR jm_buf_buffer_wrap(jm_buf_t *src,  uint16_t cap, BOOL rw_flag);
jm_buf_t* ICACHE_FLASH_ATTR jm_buf_create(int capacity);
ICACHE_FLASH_ATTR void ICACHE_FLASH_ATTR jm_buf_release(jm_buf_t * buf);

ICACHE_FLASH_ATTR jm_buf_t* jm_buf_wrapArrayReadBuf(uint8_t *data, int dataLenInBytes);

BOOL ICACHE_FLASH_ATTR jm_buf_is_full(jm_buf_t *buf);
BOOL ICACHE_FLASH_ATTR jm_buf_is_empty(jm_buf_t *buf);

//锟斤拷锟皆讹拷锟街斤拷锟斤拷
uint16_t ICACHE_FLASH_ATTR jm_buf_readable_len(jm_buf_t *buf);
//锟斤拷锟斤拷写锟街斤拷锟斤拷
uint16_t ICACHE_FLASH_ATTR jm_buf_writeable_len(jm_buf_t *buf);

BOOL ICACHE_FLASH_ATTR jm_buf_get_u8(jm_buf_t *buf,uint8_t *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_s8(jm_buf_t *buf, int8_t *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_bool(jm_buf_t *buf, BOOL *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_char(jm_buf_t *buf,char *rst);

//锟斤拷指锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
jm_buf_t* ICACHE_FLASH_ATTR jm_buf_read_buf(jm_buf_t *buf);
char* ICACHE_FLASH_ATTR jm_buf_read_chars(jm_buf_t *buf);
BOOL ICACHE_FLASH_ATTR jm_buf_get_bytes(jm_buf_t *buf, uint8_t *bytes, uint16_t len);
BOOL ICACHE_FLASH_ATTR jm_buf_get_chars(jm_buf_t *buf, char *chars, uint16_t len);
BOOL ICACHE_FLASH_ATTR jm_buf_get_buf(jm_buf_t *buf, jm_buf_t *dest, uint16_t len);
ICACHE_FLASH_ATTR BOOL jm_buf_writeStringLen(jm_buf_t *buf, uint16_t len);
ICACHE_FLASH_ATTR BOOL jm_buf_writeString(jm_buf_t *buf, char *str, uint16_t len);

//取锟斤拷锟斤拷锟斤拷指锟斤拷位锟矫碉拷一锟斤拷锟街节ｏ拷锟剿凤拷锟斤拷锟斤拷锟侥憋拷锟街革拷锟斤拷锟�
char ICACHE_FLASH_ATTR jm_buf_get_by_index(jm_buf_t *buf,  uint16_t index);
BOOL ICACHE_FLASH_ATTR jm_buf_get_u16(jm_buf_t *buf, uint16_t *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_s16(jm_buf_t *buf, int16_t *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_s32(jm_buf_t *buf, int32_t *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_u32(jm_buf_t *buf,uint32_t *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_u64(jm_buf_t *buf, uint64_t *rst);
BOOL ICACHE_FLASH_ATTR jm_buf_get_s64(jm_buf_t *buf, sint64_t *rst);
/*******************************Write method begin********************************************/

BOOL ICACHE_FLASH_ATTR check_write_len(jm_buf_t *buf, uint16_t len);
BOOL ICACHE_FLASH_ATTR jm_buf_put_u8(jm_buf_t *buf, uint8_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_s8(jm_buf_t *buf, int8_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_bytes(jm_buf_t *buf, uint8_t *bytes, int16_t len);
BOOL ICACHE_FLASH_ATTR jm_buf_put_chars(jm_buf_t *buf, char *bytes, int16_t len);

BOOL ICACHE_FLASH_ATTR jm_buf_put_char(jm_buf_t *buf, char x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_bool(jm_buf_t *buf, BOOL x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_u16(jm_buf_t *buf, uint16_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_s16(jm_buf_t *buf, int16_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_u32(jm_buf_t *buf, uint32_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_s32(jm_buf_t *buf, int32_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_u64(jm_buf_t *buf, uint64_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_s64(jm_buf_t *buf, sint64_t x);
BOOL ICACHE_FLASH_ATTR jm_buf_put_buf(jm_buf_t *buf, jm_buf_t *src);

#ifdef __cplusplus
}
#endif

#endif /* JMICRO_JM_BUFFER_H_ */
