
#ifndef JMICRO_JM_MSG_MSG_H_
#define JMICRO_JM_MSG_MSG_H_

#include "jm_cons.h"
#include "c_types.h"
#include "jm_buffer.h"

#define HEADER_LEN  13 // 2(flag)+2(data len with short)+1(type)

#define EXT_HEADER_LEN  2

//public static final int SEC_LEN  128

#define  PROTOCOL_BIN  0
#define  PROTOCOL_JSON  1
#define  PROTOCOL_EXTRA  2
#define  PROTOCOL_RAW 3

#define  PRIORITY_0  0
#define  PRIORITY_1  1
#define  PRIORITY_2  2
#define  PRIORITY_3  3
#define  PRIORITY_4  4
#define  PRIORITY_5  5
#define  PRIORITY_6  6
#define  PRIORITY_7  7

#define  PRIORITY_MIN  PRIORITY_0
#define  PRIORITY_NORMAL  PRIORITY_3
#define  PRIORITY_MAX  PRIORITY_7

//public static final long MAX_LONG_VALUE  Long.MAX_VALUE*2

//public static final byte MSG_VERSION  (byte)1

#define FLAG_LENGTH_INT  (1 << 0)

#define FLAG_UP_PROTOCOL  1
#define FLAG_DOWN_PROTOCOL  8
#define FLAG_UP_PROTOCOL_MASK  0xFFF9
#define FLAG_DOWN_PROTOCOL_MASK  0xFCFF

#define FLAG_MONITORABLE  (1 << 3)

#define FLAG_EXTRA  (1 << 4)

#define FLAG_OUT_MESSAGE  (1 << 5)

#define FLAG_ERROR  (1 << 6)

#define FLAG_FORCE_RESP_JSON  (1 << 7)

#define FLAG_DEV  (1 << 10)

#define FLAG_RESP_TYPE  11
#define FLAG_RESP_TYPE_MASK  0xE7FF //1110 0111 1111 1111

#define FLAG_LOG_LEVEL  13
#define FLAG_LOG_LEVEL_MASK  0x1FFF //0001 1111 1111 1111

/****************  extra constants flag   *********************/

#define EXTRA_FLAG_DEBUG_MODE  (1 << 0)

#define EXTRA_FLAG_PRIORITY  1

#define EXTRA_FLAG_DUMP_UP  (1 << 3)

#define EXTRA_FLAG_DUMP_DOWN  (1 << 4)

#define EXTRA_FLAG_UP_SSL  (1 << 5)

#define EXTRA_FLAG_DOWN_SSL  (1 << 6)

#define EXTRA_FLAG_IS_SEC  (1 <<8)

#define EXTRA_FLAG_IS_SIGN  (1 << 9)

#define EXTRA_FLAG_ENC_TYPE  (1 << 10)

#define EXTRA_FLAG_RPC_MCODE  (1 << 11)

#define EXTRA_FLAG_SECTET_VERSION  (1 << 12)

#define EXTRA_FLAG_INS_ID  (1 << 13)

#define EXTRA_FLAG_FROM_APIGATEWAY  (1 << 14)

#define EXTRA_FLAG_UDP  (1 << 15)

#define EXTRA_KEY_LINKID  -127
#define EXTRA_KEY_INSID  -126
#define EXTRA_KEY_TIME  -125
#define EXTRA_KEY_SM_CODE  -124
#define EXTRA_KEY_SM_NAME  -123
#define EXTRA_KEY_SIGN  -122
#define EXTRA_KEY_SALT  -121
#define EXTRA_KEY_SEC  -120
#define EXTRA_KEY_LOGIN_KEY  -119

//public static final Byte EXTRA_KEY_ARRAY  -116
#define EXTRA_KEY_FLAG  -118

#define EXTRA_KEY_MSG_ID  -117

#define EXTRA_KEY_LOGIN_SYS  -116
#define EXTRA_KEY_ARG_HASH -115

#define EXTRA_KEY_PS_OP_CODE -114//
#define EXTRA_KEY_PS_ARGS -113 //

#define EXTRA_KEY_UDP_PORT -111//UDP远程端口
#define EXTRA_KEY_UDP_HOST -110//UDP远程主机地址
#define EXTRA_KEY_UDP_ACK -109//UDP是否需要应答，true需要应答，false不需要应答
#define EXTRA_KEY_CHANNEL_NO -108//渠道号

#define EXTRA_KEY_GROUP_ID -107//设备所属组，同一个组的设备才能相互通信


#define EXTRA_SKEY_UDP_PORT "-111"//UDP远程端口
#define EXTRA_SKEY_UDP_HOST "-110"//UDP远程主机地址
#define EXTRA_SKEY_UDP_ACK "-109"//UDP是否需要应答，true需要应答，false不需要应答

#define EXTRA_KEY_SMSG_ID -112

//rpc method name
#define EXTRA_KEY_METHOD  127
#define EXTRA_KEY_EXT0  126
#define EXTRA_KEY_EXT1  125
#define EXTRA_KEY_EXT2  124
#define EXTRA_KEY_EXT3  123

#define EXTRA_KEY_CLIENT_ID  122
#define EXTRA_KEY_EXT4  121
#define EXTRA_KEY_EXT5  120
#define MSG_TYPE_PINGPONG  0

#define MSG_TYPE_NO_RESP  1

#define MSG_TYPE_MANY_RESP  2


#define  PREFIX_TYPE_ID -128

#define  GET_PREFIX(n) (PREFIX_TYPE_ID+n)

#define  PREFIX_TYPE_NULL GET_PREFIX(0) //-128

//FINAL
#define  PREFIX_TYPE_FINAL GET_PREFIX(1) //-127

#define  PREFIX_TYPE_SHORT (GET_PREFIX(2))//-126
#define  PREFIX_TYPE_STRING (GET_PREFIX(3))//-125

#define  PREFIX_TYPE_LIST GET_PREFIX(4)//-124
#define  PREFIX_TYPE_SET GET_PREFIX(5)//-123
#define  PREFIX_TYPE_MAP GET_PREFIX(6)//-122

#define  PREFIX_TYPE_BYTE GET_PREFIX(7)//-121
#define  PREFIX_TYPE_SHORTT GET_PREFIX(8)//-120
#define  PREFIX_TYPE_INT GET_PREFIX(9)//-119
#define  PREFIX_TYPE_LONG GET_PREFIX(10)//-118
#define  PREFIX_TYPE_FLOAT GET_PREFIX(11)//-117
#define  PREFIX_TYPE_DOUBLE GET_PREFIX(12)//-116
#define  PREFIX_TYPE_CHAR GET_PREFIX(13)//-115
#define  PREFIX_TYPE_BOOLEAN GET_PREFIX(14)//-114
#define  PREFIX_TYPE_STRINGG GET_PREFIX(15)//-113
#define  PREFIX_TYPE_DATE GET_PREFIX(16)//-112
#define  PREFIX_TYPE_BYTEBUFFER GET_PREFIX(17)//-111
#define  PREFIX_TYPE_REQUEST GET_PREFIX(18)//-110
#define  PREFIX_TYPE_RESPONSE GET_PREFIX(19)//-109
#define  PREFIX_TYPE_PROXY GET_PREFIX(20)//-108

#define  PREFIX_TYPE_HASHMAP 127 // jm_hashmap_t not emap
#define  PREFIX_TYPE_WAIT_SLAVE_RESP 126 // wait_slave_resp_t
#define  PREFIX_TYPE_LOG_SERVER 125 // log_server_t

#define  PREFIX_TYPE_MASTERSLAVE_DEV 124 // jm_masterslave_dev_t
#define  PREFIX_TYPE_CLIENT_MSG_RESULT 123 // jm_cli_msg_result_t
#define  PREFIX_TYPE_CHRI 122 // CHRI
#define  PREFIX_TYPE_PUBSUB_LISTENER_MAP 121 // _pubsub_listener_map

#define  PREFIX_TYPE_CACHE_HEADER 120 // jm_cache_header_t
#define  PREFIX_TYPE_CACHE 119 // jm_cache_t
#define  PREFIX_TYPE_TCP_CLIENT_SEND_ITEM 118 // _jm_tcpclient_send_item

#define  PREFIX_TYPE_PUBSUB_LISTENER_ITEM 117 // _pubsub_listener_map
#define  PREFIX_TYPE_MESSAGE 116 // jm_msg_t
#define  PREFIX_TYPE_MSG_EXTRA_DATA 115 // _msg_extra_data
#define  PREFIX_TYPE_PUBSUB_ITEM 114 // _c_pubsub_item
#define  PREFIX_TYPE_PUBSUB_ITEM 114 // _c_pubsub_item
#define  PREFIX_TYPE_HASHMAP_ITEM 113 // jm_hashmap_item_t
#define  PREFIX_TYPE_JM_BUFFER 112 // jm_buffer
#define  PREFIX_TYPE_JM_BUFFER_ARRAY 111 // jm_buffer中的Byte Array
#define  PREFIX_TYPE_JM_BYTES 110 // jm_buffer
#define  PREFIX_TYPE_CTRL_ITEM 109 // ctrl_item

#if JM_EMAP_ENABLE==1
struct _emap_t;
#endif//JM_EMAP_ENABLE==1

#if JM_ELIST_ENABLE==1
struct _elist_t;
#endif//JM_ELIST_ENABLE==1

#if JM_MSG_EXTRA_ENABLE==1
typedef union _msg_extra_data_val {
    struct _elist_t *list;
    struct _emap_t *map;
    char *strVal;
	uint8_t *bytesVal;
	char charVal;
	sint8_t s8Val;
	sint16_t s16Val;
	sint32_t s32Val;
	sint64_t s64Val;
	BOOL boolVal;
    void *anyVal;
} jm_msg_extra_data_val;

typedef struct _msg_extra_data {
	char* strKey;
	sint8_t key;
	jm_msg_extra_data_val value;
#if DEBUG_CACHE_ENABLE == 1
    uint8_t model;
#endif
	struct _msg_extra_data *next;
	uint16_t len;
	BOOL neddFreeBytes;
	BOOL copyKey;
	sint8_t type;
} jm_msg_extra_data_t;

typedef struct _msg_extra_data_iterator {
	jm_msg_extra_data_t *header;
	jm_msg_extra_data_t *cur;
} jm_msg_extra_data_iterator_t;
#endif //JM_MSG_EXTRA_ENABLE==1


#if JM_EMAP_ENABLE==1
typedef struct _emap_t {
	jm_msg_extra_data_t *_hd;
	uint16_t size;
    sint8_t keyType;
} jm_emap_t;

typedef struct _emap_iterator_t{
    jm_msg_extra_data_t *cur;
} jm_emap_iterator_t;

#endif// JM_EMAP_ENABLE==1


#if JM_ELIST_ENABLE==1
typedef struct _elist_t {
	jm_msg_extra_data_t *_hd;//private field
	uint16_t size;
} jm_elist_t;

typedef  struct _elist_iterator_t{
    jm_msg_extra_data_t *cur;
}jm_elist_iterator_t;
#endif //JM_ELIST_ENABLE==1

#if JM_MSG_ENABLE==1
/**
 * Messge format:
 */
typedef struct _jm_msg {
	//0B00111000 5---3
	//public static final short FLAG_LEVEL = 0X38;

	//�Ƿ����÷���log
	//public static final short FLAG_LOGGABLE = 1 << 3;
	//uint64_t startTime;

	//uint32_t len = -1;

	//1 byte length
	//private byte version;

	//payload length with byte,4 byte length
	//private int len;

	//2 byte length
	//private byte ext;

	//normal message ID	or JRPC request ID
	uint32_t msgId;

	jm_buf_t *payload;

	//private Map<Byte,Object> extraMap;
	jm_emap_t *extraMap;

	/**
	 * 0        dm:       is development mode EXTRA_FLAG_DEBUG_MODE = 1 << 1;
	 * 1,2      PP:       Message priority   EXTRA_FLAG_PRIORITY
	 * 3        up:       dump up stream data
	 * 4        do:       dump down stream data
	 * 5 	    US        ����SSL  0:no encrypt 1:encrypt
	 * 6        DS        ����SSL  0:no encrypt 1:encrypt
	 * 7
	 * 8        MK        RPC��������
	 * 9        SV        �Գ���Կ�汾
	 * 10       SE        ����
	 * 11       SI        �Ƿ���ǩ��ֵ 0���ޣ�1����
	 * 12       ENT       encrypt type 0:�ԳƼ��ܣ�1��RSA �ǶԳƼ���
	 * 13       ERROR     0:�������� 1��������Ӧ��
	 * 14       GW        API gateway forward message   EXTRA_FLAG_FROM_APIGATEWAY
	 * 15       UDP       通过UDP传输报文，1: UDP, 0:TCP
	 UDP  GW  E  ENT SI  SE  WE MK SV  DS   US   DO   UP  P    P   dm
	 |    |   |   |  |   |   |  |  |   |    |    |    |   |    |   |
     15  14  13  12  11  10  9  8  7   6    5    4    3   2    1   0

	 |    |   |   |  |   |   |    |   |   |    |    |    |   |    |   |
     31  30  29  28  27  26  25   24  23  22   21   20   19  18   17  16

	 *
	 * @return
	 */
	sint32_t extrFlag ;

	//uint8_t *extra;

	//jm_req_t *req;

	//struct _jm_msg *cacheNext;
	
		/**
	 * 0        S:       data length type 0:short 1:int
	 * 1        UPR:     up protocol  0: bin,  1: json
	 * 2        DPR:     down protocol 0: bin, 1: json
	 * 3        M:       Monitorable
	 * 4        Extra    Contain extra data
	 * 5        Innet    message from outer network
	 * 6
	 * 7
	 * 8
	 * 9
	 * 10
	 * 11��12   Resp type  MSG_TYPE_PINGPONG��MSG_TYPE_NO_RESP��MSG_TYPE_MANY_RESP
	 * 13 14 15 LLL      Log level
	 * @return
	 */
	sint16_t flag;

	// 1 byte
	uint8_t type;
} jm_msg_t;

#endif // JM_MSG_ENABLE==1

#if JM_MSG_EXTRA_ENABLE==1
typedef void (*jm_extra_iterator_fn)(sint8_t key, void *val, sint8_t type);

typedef void (*jm_extra_siterator_fn)(char *key, void *val, sint8_t type);
#endif //JM_MSG_EXTRA_ENABLE

#ifdef __cplusplus
extern "C" {
#endif
/***************************************EXTRA DATA OPERATION BEGIN***************************************/


/****************************************Extra Map Method Begin*********************************************/

#if JM_ELIST_ENABLE==1	
ICACHE_FLASH_ATTR BOOL jm_emap_putExtra(jm_emap_t *map, jm_msg_extra_data_t *ex);
ICACHE_FLASH_ATTR BOOL jm_emap_put(jm_emap_t *map, void *key, void *val, sint8_t type, BOOL needFreeMem,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putByte(jm_emap_t *map, void *key, sint8_t val,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putShort(jm_emap_t *map, void *key, sint16_t val,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putInt(jm_emap_t *map, void *key, sint32_t val,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putLong(jm_emap_t *map, void *key, sint64_t val,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putBool(jm_emap_t *map, void *key, BOOL val,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putStr(jm_emap_t *map, void *key, char *val,BOOL needFreeMem,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putMap(jm_emap_t *map, void *key, jm_emap_t *val, BOOL needFreeMem,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putList(jm_emap_t *map, void *key, jm_elist_t *val, BOOL needFreeMem,BOOL copyKey);
ICACHE_FLASH_ATTR BOOL jm_emap_putAll(jm_emap_t *map, jm_emap_t *src);

ICACHE_FLASH_ATTR BOOL jm_emap_remove(jm_emap_t *map, void *key, BOOL release);
ICACHE_FLASH_ATTR BOOL jm_emap_exist(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR BOOL jm_emap_size(jm_emap_t *map);
ICACHE_FLASH_ATTR jm_emap_t* jm_emap_create(sint8_t keyType);
//复制map到一个新的map，注意释放原始map前，要确保_hd不被释放，将_hd设备为NULL即可
ICACHE_FLASH_ATTR jm_emap_t* jm_emap_copy(jm_emap_t *map);

ICACHE_FLASH_ATTR void jm_emap_release(jm_emap_t *map);
ICACHE_FLASH_ATTR void* jm_emap_next(jm_emap_iterator_t *ite);

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_emap_get(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR char* jm_emap_getStr(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR sint8_t jm_emap_getByte(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR sint16_t jm_emap_getShort(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR sint32_t jm_emap_getInt(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR sint64_t jm_emap_getLong(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR jm_emap_t* jm_emap_getMap(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR jm_elist_t* jm_emap_getList(jm_emap_t *map, void *key);
ICACHE_FLASH_ATTR BOOL jm_emap_getBool(jm_emap_t *map, void *key);

//ICACHE_FLASH_ATTR jm_hash_map_iterator_t* jm_hashmap_iteCreate(map_iterator_fn ite);
//ICACHE_FLASH_ATTR void* jm_hashmap_iteNext(jm_hash_map_iterator_t *ite);
#endif //JM_ELIST_ENABLE

/****************************************Extra List Method Begin*********************************************/
#if JM_ELIST_ENABLE==1
ICACHE_FLASH_ATTR BOOL jm_elist_addExtra(jm_elist_t *list, jm_msg_extra_data_t *ex);
ICACHE_FLASH_ATTR BOOL jm_elist_add(jm_elist_t *list, void*  val, sint8_t type, BOOL needFreeMem);
ICACHE_FLASH_ATTR BOOL jm_elist_removeByIdx(jm_elist_t *list, sint16_t idx);
ICACHE_FLASH_ATTR BOOL jm_elist_remove(jm_elist_t *list, void *val, sint8_t type);
ICACHE_FLASH_ATTR uint16_t jm_elist_size(jm_elist_t *list);
ICACHE_FLASH_ATTR void* jm_elist_get(jm_elist_t *list, sint16_t idx);
ICACHE_FLASH_ATTR BOOL jm_elist_exist(jm_elist_t *list, void *val, sint8_t type);
ICACHE_FLASH_ATTR jm_elist_t* jm_elist_create();
ICACHE_FLASH_ATTR void jm_elist_release(jm_elist_t *list);
ICACHE_FLASH_ATTR void* jm_elist_next(jm_elist_iterator_t *ite);

//ICACHE_FLASH_ATTR jm_hash_map_iterator_t* jm_hashmap_iteCreate(map_iterator_fn ite);
//ICACHE_FLASH_ATTR void* jm_hashmap_iteNext(jm_hash_map_iterator_t *ite);
#endif //JM_ELIST_ENABLE

/************************************String KEY Extra begin***********************************************/
#if JM_MSG_EXTRA_ENABLE==1

ICACHE_FLASH_ATTR jm_emap_t* jm_extra_decode(jm_buf_t *b);

ICACHE_FLASH_ATTR BOOL jm_extra_encode(jm_msg_extra_data_t *extras, jm_buf_t *b, uint16_t *wl,sint8_t keyType);

ICACHE_FLASH_ATTR void jm_extra_release(jm_msg_extra_data_t *extra);

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_create(uint8_t model);

ICACHE_FLASH_ATTR jm_msg_extra_data_t * jm_extra_pullAll(jm_msg_extra_data_t *from, jm_msg_extra_data_t *to);

ICACHE_FLASH_ATTR jm_msg_extra_data_t * jm_extra_iteNext();

ICACHE_FLASH_ATTR void* jm_extra_sgetVal(jm_msg_extra_data_t *ex);
ICACHE_FLASH_ATTR void* jm_extra_sgetValByKey(jm_msg_extra_data_t *ex, char *strKey);

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sget(jm_msg_extra_data_t *header, char *key);
ICACHE_FLASH_ATTR sint16_t jm_extra_sgetS16(jm_msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR sint8_t jm_extra_sgetS8(jm_msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR sint64_t jm_extra_sgetS64(jm_msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR sint32_t jm_extra_sgetS32(jm_msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR char jm_extra_sgetChar(jm_msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR BOOL jm_extra_sgetBool(jm_msg_extra_data_t *e, char *strKey);

ICACHE_FLASH_ATTR char* jm_extra_sgetStr(jm_msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR  char* jm_extra_sgetStrCpy(jm_msg_extra_data_t *e, char *strKey);

ICACHE_FLASH_ATTR jm_emap_t* jm_extra_sgetMap(jm_msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR  jm_elist_t* jm_extra_sgetColl(jm_msg_extra_data_t *e, char *strKey);

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputByType(jm_msg_extra_data_t *header, char *strKey, void*  val, sint8_t type);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sput(jm_msg_extra_data_t *header, char *strKey, sint8_t type,BOOL copyKey);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputByte(jm_msg_extra_data_t *e, char *strKey, sint8_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputShort(jm_msg_extra_data_t *e, char *strKey, sint16_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputInt(jm_msg_extra_data_t *e, char *strKey, sint32_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputLong(jm_msg_extra_data_t *e, char *strKey, sint64_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputChar(jm_msg_extra_data_t *e, char *strKey, char val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputBool(jm_msg_extra_data_t *e, char *strKey, BOOL val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputStr(jm_msg_extra_data_t *e, char *strKey, char* val, uint16_t len);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputMap(jm_msg_extra_data_t *header, char *strKey, jm_emap_t *val,sint8_t type);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputColl(jm_msg_extra_data_t *header, char *strKey, jm_elist_t *val, sint8_t type);
/************************************String KEY Extra end***********************************************/


/**************************************BYTE KEY EXTRA***************************************************/

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_get(jm_msg_extra_data_t *header, sint8_t key);
ICACHE_FLASH_ATTR  sint16_t jm_extra_getS16(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  sint8_t jm_extra_getS8(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  sint64_t jm_extra_getS64(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  sint32_t jm_extra_getS32(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  char jm_extra_getChar(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  BOOL jm_extra_getBool(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  char* jm_extra_getStr(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR jm_emap_t* jm_extra_getMap(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  jm_elist_t* jm_extra_getColl(jm_msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  void* jm_extra_getAnyVal(jm_msg_extra_data_t *e, sint8_t key);

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_put(jm_msg_extra_data_t *header, sint8_t key, sint8_t type);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putByte(jm_msg_extra_data_t *e, sint8_t key, sint8_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putShort(jm_msg_extra_data_t *e, sint8_t key, sint16_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putInt(jm_msg_extra_data_t *e, sint8_t key, sint32_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putLong(jm_msg_extra_data_t *e, sint8_t key, sint64_t val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putChar(jm_msg_extra_data_t *e, sint8_t key, char val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putBool(jm_msg_extra_data_t *e, sint8_t key, BOOL val);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putStr(jm_msg_extra_data_t *e, sint8_t key, char* val, uint16_t len);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putMap(jm_msg_extra_data_t *header, sint8_t key, jm_emap_t *val, sint8_t type);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putColl(jm_msg_extra_data_t *header, sint8_t key, jm_elist_t *val, sint8_t type);
ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putByType(jm_msg_extra_data_t *header, sint8_t key, void *val, sint8_t type);

#endif //JM_MSG_EXTRA_ENABLE

/**************************************EXTRA DATA OPERATION END**********************************/

#if JM_MSG_ENABLE==1
ICACHE_FLASH_ATTR BOOL jm_msg_isUpSsl(jm_msg_t *msg);
ICACHE_FLASH_ATTR void jm_msg_setUpSsl(jm_msg_t *msg, BOOL f);

ICACHE_FLASH_ATTR BOOL jm_msg_isUdp(jm_msg_t *msg);
ICACHE_FLASH_ATTR void jm_msg_setUdp(jm_msg_t *msg, BOOL f);

ICACHE_FLASH_ATTR BOOL jm_msg_isDownSsl(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setDownSsl(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isFromApiGateway(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setFromApiGateway(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isRsaEnc(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setEncType(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isSecretVersion(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setSecretVersion(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isSign(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setSign(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isSec(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setSec(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isRpcMk(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setRpcMk(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isDumpUpStream(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setDumpUpStream(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isDumpDownStream(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setDev(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isDev(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setDumpDownStream(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isLoggable(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL jm_msg_isDebugMode(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setDebugMode(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isMonitorable(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setMonitorable(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isError(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setError(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isOuterMessage(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setOuterMessage(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isForce2Json(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setForce2Json(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL jm_msg_isNeedResponse(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL jm_msg_isPubsubMessage(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL jm_msg_isPingPong(jm_msg_t *msg);

/**
 */
ICACHE_FLASH_ATTR  void jm_msg_setLengthType(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR BOOL jm_msg_isLengthInt(jm_msg_t *msg);
ICACHE_FLASH_ATTR  sint8_t jm_msg_getPriority(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL jm_msg_setPriority(jm_msg_t *msg, sint32_t l);
ICACHE_FLASH_ATTR  sint8_t jm_msg_getLogLevel(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL jm_msg_setLogLevel(jm_msg_t *msg, sint16_t v);
ICACHE_FLASH_ATTR sint8_t jm_msg_getRespType(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL jm_msg_setRespType(jm_msg_t *msg, sint16_t v);
ICACHE_FLASH_ATTR  sint8_t jm_msg_getUpProtocol(jm_msg_t *msg );
ICACHE_FLASH_ATTR  void jm_msg_setUpProtocol(jm_msg_t *msg, sint16_t protocol);
ICACHE_FLASH_ATTR  sint8_t jm_msg_getDownProtocol(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void jm_msg_setDownProtocol(jm_msg_t *msg, sint16_t protocol);

ICACHE_FLASH_ATTR jm_msg_t *jm_msg_decode(jm_buf_t *b);
ICACHE_FLASH_ATTR BOOL jm_msg_encode(jm_msg_t *msg, jm_buf_t *buf);
ICACHE_FLASH_ATTR jm_msg_t *jm_msg_readMessage(jm_buf_t *buf);

ICACHE_FLASH_ATTR jm_msg_t* jm_msg_create_rpc_msg(sint32_t mcode, jm_buf_t *payload);
ICACHE_FLASH_ATTR jm_msg_t* jm_msg_create_ps_msg(jm_buf_t *payload);
ICACHE_FLASH_ATTR jm_msg_t* jm_msg_create_msg(sint8_t tyep, jm_buf_t *payload);
ICACHE_FLASH_ATTR void jm_msg_release(jm_msg_t *msg);

#endif //JM_MSG_ENABLE==1

#ifdef __cplusplus
}
#endif



#endif /* JMICRO_JM_MSG_MSG_H_ */
