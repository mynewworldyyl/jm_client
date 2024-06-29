#ifndef JM_CAHCE_H_
#define JM_CAHCE_H_
#include "jm_cons.h"
#include "c_types.h"

#define CACHE_PUBSUB_ITEM "C_PS_ITEM_"
#define CACHE_PUBSUB_ITEM_EXTRA "C_PS_ITEM_EXTRA_"

#define CACHE_MESSAGE "C_MESSAGE_"
#define CACHE_MESSAGE_EXTRA "C_MESSAGE_EXTRA_"

#define MODEL_OTHER 0
#define MODEL_JLOG 1
#define MODEL_JHOST 10
#define MODEL_JCLIENT 20
#define MODEL_JBUFFER 30
#define MODEL_JHTTP 40
#define MODEL_JSERIAL 50
#define MODEL_JNET 60
#define MODEL_JMSG 70

typedef struct _hashmap_item {
	void *key; //KEY
	void* val; //Value
	struct _hashmap_item* next; //hash
} jm_hashmap_item_t;

typedef struct _hashmap {
    int8_t keyType;
	uint32_t cap;
    jm_hashmap_item_t **arr;
	uint32_t size;
	BOOL needFreeVal;
	int8_t ver;
} jm_hashmap_t;

typedef struct _jm_hash_map_iterator {
    jm_hashmap_t *map;
    jm_hashmap_item_t *cur;
	int32_t idx;
	int8_t ver;
} jm_hash_map_iterator_t;

typedef struct _jm_cache {
	void* item;
	BOOL used;//
	struct _jm_cache* next;
} jm_cache_t;

typedef struct _jm_cache_header {
    uint16_t memSize;//每个元素所占内存大小
    uint16_t eleNum;//可用元素个数
    uint16_t totalNum;//总元素个数
	sint8_t dataType;//元素数据类型
	jm_cache_t* cache;//缓存链表
} jm_cache_header_t;

typedef void (*map_iterator_fn)(char *key, void *val, void *arg);

#ifdef __cplusplus
extern "C" {
#endif

//ICACHE_FLASH_ATTR static jm_hashmap_item_t* _hashmap_getItem(jm_hashmap_t *map, char *cacheName, uint32_t idx);

ICACHE_FLASH_ATTR void* jm_utils_malloc(uint16_t size,sint8_t dt);
ICACHE_FLASH_ATTR void* jm_utils_mallocWithError(uint16_t size,sint8_t dt, char *msg);
ICACHE_FLASH_ATTR char* jm_utils_copyStr(char *srcStr);

ICACHE_FLASH_ATTR BOOL jm_hashmap_put(jm_hashmap_t *map, void *cacheName, void *extra);
ICACHE_FLASH_ATTR BOOL jm_hashmap_remove(jm_hashmap_t *map, void *cacheName);

ICACHE_FLASH_ATTR void* jm_hashmap_get(jm_hashmap_t *map, void *cacheName);

ICACHE_FLASH_ATTR BOOL jm_hashmap_exist(jm_hashmap_t *map, void *cacheName);

ICACHE_FLASH_ATTR jm_hashmap_t* jm_hashmap_create(uint16_t cap, sint8_t keyType);
ICACHE_FLASH_ATTR void jm_hashmap_release(jm_hashmap_t *map);

//ICACHE_FLASH_ATTR jm_hash_map_iterator_t* jm_hashmap_iteCreate(map_iterator_fn ite);
ICACHE_FLASH_ATTR void* jm_hashmap_iteNext(jm_hash_map_iterator_t *ite);
//删除迭代器当前元素
ICACHE_FLASH_ATTR BOOL jm_hashmap_iteRemove(jm_hash_map_iterator_t *ite);

/*******************************Hash Map***************************************/



/*******************************Cache************************************/
//itemSize
ICACHE_FLASH_ATTR BOOL cache_init(char *cacheName, uint16_t itemSize, sint8_t dataType);

/**
 */
ICACHE_FLASH_ATTR void* cache_get(char *cacheName, BOOL created, uint8_t model);
//
ICACHE_FLASH_ATTR BOOL cache_back(char *cacheName, void *it, uint8_t model);


/*******************************Cache************************************/

ICACHE_FLASH_ATTR void mem_printMemInfo();
ICACHE_FLASH_ATTR void mem_free(void *p, unsigned n);
ICACHE_FLASH_ATTR void* mem_zalloc(unsigned n, sint8_t dataType);
ICACHE_FLASH_ATTR void jm_mem_init();

ICACHE_FLASH_ATTR char* jm_utils_mallocStr(uint16_t size, char *msg);
ICACHE_FLASH_ATTR void jm_utils_releaseStr(char *p, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif

