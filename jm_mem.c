#include "jm_cons.h"
#include "jm_mem.h"
#include "debug.h"
#include "jm_buffer.h"
#include "jm_stdcimpl.h"

#if JM_STM32==1
#include "stdlib.h"
#endif

#if ESP8266==1
#include <osapi.h>
#include "mem.h"
#endif

#ifdef WIN32
#include "stdio.h"
#include "string.h"
#include "test.h"
#endif

#include <stddef.h>

#define SIZE 11

extern jm_mem_op *jmm;

//锟斤拷锟斤拷Map
static jm_hashmap_t* cacheMap;

/***************************内存监控开始*********************************/

static uint32_t totalAlloc = 0;

//static uint32_t totalFree = 0;

#if DEBUG_MEMORY == 1

static uint32_t debugHashmapItemSize = 0;

static jm_hashmap_t *mallocMem;

static jm_hashmap_t *mmmMap;

typedef struct jm_mmm{
    sint8_t dataType;//数据类型
    uint32_t totalAlloc;//占据总内存
} jm_mmm_t;


ICACHE_FLASH_ATTR static char* _printMemData(jm_hashmap_t *params);
ICACHE_FLASH_ATTR static void _printStaticData(sint8_t dt0);
ICACHE_FLASH_ATTR static void _printPdData();
ICACHE_FLASH_ATTR void mem_printMemInfo();

ICACHE_FLASH_ATTR void mem_free(void *p, unsigned n) {

    if(!p) return;

#if ESP8266==1
        p = p - 4;
#else
        p = (uint8_t*)p-1;
#endif

    if(jm_hashmap_exist(mallocMem, p)) {
        totalAlloc -= n;

        sint8_t dt = ((sint8_t*)p)[0];
        jm_mmm_t * m = NULL;
        if(dt != 0) {
            m = jm_hashmap_get(mmmMap,(void*)dt);
            if(m) {
                m->totalAlloc -= n;
            } else {
                JM_MEM_DEBUG("Nm=%u dt=%d\n", n,dt);
            }
        }
        //JM_MEM_DEBUG("MEM _os_free free=%u, totalAlloc=%u\n",n, totalAlloc);
		mem_free(p,0);
        jm_hashmap_remove(mallocMem,p);
        if(m) {
            JM_MEM_DEBUG("mf=%lx totalAlloc=%u dt=%d, dtsize=%u\n", p, totalAlloc, m->dataType, m->totalAlloc);
        }else {
            JM_MEM_DEBUG("nf=%lx totalAlloc=%u not type data dt=%d\n", p, totalAlloc,dt);
        }

    } else {
        //检查重复FREE内存错误
        JM_MEM_DEBUG("rf=%lx, totalAlloc=%u\n",p,totalAlloc);
    }

}

ICACHE_FLASH_ATTR void* mem_zalloc(unsigned n, sint8_t dataType) {

    //JM_MEM_DEBUG("MEM _os_zalloc req=%u, totalAlloc=%u\n",n, totalAlloc);
#if ESP8266==1
	//JM_MEM_DEBUG("mem_zalloc invoke os_zalloc %u, totalAlloc=%u\n",(n+4),totalAlloc);
    void *p = os_zalloc(n+4);//8266结构体内存4字节对齐，为了兼容，统一增加4字节前缀
 #else
    void *p = malloc(n+1);
#endif

    if(p) {
        totalAlloc += n;

        ((sint8_t*)p)[0] = dataType;

        if(dataType != 0 && mmmMap) {
        	//JM_MEM_DEBUG("mem_zalloc datatype map %d\n",dataType);
            jm_mmm_t * m = jm_hashmap_get(mmmMap, (void*)dataType);
            if(!m && dataType != PREFIX_TYPE_HASHMAP) {
#if ESP8266==1
                m = os_zalloc(sizeof(jm_mmm_t));
#else
                m = malloc(sizeof(jm_mmm_t));
#endif
                jm_hashmap_put(mmmMap, (void*)dataType, m);
				
                m->dataType = dataType;
                m->totalAlloc = 0;
            }
			
			if(m)  m->totalAlloc += n;
            //JM_MEM_DEBUG("_os_zalloc put memory size: %u, pointer: %lx, total: %u, dt:%d, dtsize: %u\n",n, p, totalAlloc, dataType, m->totalAlloc);
        }

        //JM_MEM_DEBUG("_os_zalloc: %lx, totalAlloc: %u, dt:%d\n", p, totalAlloc,dataType);

       // JM_MEM_DEBUG("mem_zalloc save mem pointer: %lu\n",p);
		if(mallocMem != NULL)
			jm_hashmap_put(mallocMem,p,(void*)n);

        //JM_MEM_DEBUG("mem_zalloc success mac\n");

#if ESP8266==1
        return (void*)(4+p);
#else
        return ((uint8_t*)p+1);
#endif

    } else {
    	mem_printMemInfo();
    	return NULL;
    }
}

ICACHE_FLASH_ATTR void _mem_printVal(sint8_t needDataType, sint8_t type, uint16_t size,  uint64_t val) {

	if(needDataType != 0 && needDataType != type) {
		return;
	}

    switch((uint8_t)type) {
        case (uint8_t)PREFIX_TYPE_BYTE:
			JM_MEM_DEBUG("s=%u byte=%d dt=%d\n", size, val, type);
			break;
        case (uint8_t)PREFIX_TYPE_SHORTT:
        	JM_MEM_DEBUG("s=%u, short=%d dt=%d\n", size, val,type);
		    break;
        case (uint8_t)PREFIX_TYPE_INT:
        	 JM_MEM_DEBUG("s=%u, int=%d dt=%d\n", size, val,type);
		     break;
        case (uint8_t)PREFIX_TYPE_LONG:
        	JM_MEM_DEBUG("s=%u, long=%d dt=%d\n", size, val,type);
        	break;
        case (uint8_t)PREFIX_TYPE_STRINGG:
        	{

			#if ESP8266==1
        		char *p = ((char*)val) + 4;
			#else
        		char *p = ((char*)val) + 1;
			#endif

        		if(size > 0) {
					if(p[size] != '\0') {
						//保证字符串正常结束
						char tc = p[size];
						p[size] = '\0';
						JM_MEM_DEBUG("s=%u str=%s dt=%d\n", size, p, type);
						p[size] = tc;
					} else {
						JM_MEM_DEBUG("s=%u str=%s dt=%d\n", size, p, type);
					}
				} else {
					JM_MEM_DEBUG("s=%u str=%s dt=%d\n", size, p, type);
				}
        	}
        	break;
        case (uint8_t)PREFIX_TYPE_BOOLEAN:
        	JM_MEM_DEBUG("s=%u BOOL=%d dt=%d\n", size, val,type);
        	break;
        case (uint8_t)PREFIX_TYPE_CHAR:
        	JM_MEM_DEBUG("s=%u str=%c dt=%d\n", size, val,type);
        	break;
        case (uint8_t)PREFIX_TYPE_LIST:
        case (uint8_t)PREFIX_TYPE_SET:
			JM_MEM_DEBUG("s=%u list dt=%d\n", size,type);
			break;
        case (uint8_t)PREFIX_TYPE_MAP:
        case (uint8_t)PREFIX_TYPE_PROXY:
			JM_MEM_DEBUG("s=%u map dt=%d\n", size,type);
			break;
        case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
        	JM_MEM_DEBUG("s=%u buffer dt=%d\n", size,type);
        	break;
        case (uint8_t)PREFIX_TYPE_SHORT:
			JM_MEM_DEBUG("s=%u anytype dt=%d\n", size,type);
			break;
        default:
        	JM_MEM_DEBUG("s=%u other dt=%d\n", size, type);
    }
}

ICACHE_FLASH_ATTR static void _printStaticData(sint8_t dt0) {

    //打印当前未释放的字符串值
    jm_hash_map_iterator_t ite = {mallocMem, NULL, -1,mallocMem->ver};

    uint32_t total = 0;

    void* k = 0;

    while((k = jm_hashmap_iteNext(&ite))) {
        unsigned m = (unsigned)jm_hashmap_get(mallocMem, k);
        if(m) {
            total += m;
            sint8_t dt = ((sint8_t*)k)[0];
            _mem_printVal(dt0, dt, m, (unsigned)k);
        }
    }

    JM_MEM_DEBUG("Mem totalAlloc: %u,  total: %u\n", totalAlloc, total);
}

ICACHE_FLASH_ATTR static void _printPdData() {
    //打印当前数据类型对应的内存使用情况
    jm_hash_map_iterator_t ite = {mmmMap, NULL, -1,mmmMap->ver};
    uint32_t total = 0;
    void* k = 0;
    while((k = jm_hashmap_iteNext(&ite))) {
        jm_mmm_t *m = (jm_mmm_t *)jm_hashmap_get(mmmMap, k);
        if(m) {
        total += m->totalAlloc;
        JM_MEM_DEBUG("Mem dt:%d, dtsize: %u\n", m->dataType, m->totalAlloc);
        }
    }
    JM_MEM_DEBUG("Mem totalAlloc: %u,  total: %u\n", totalAlloc, total);

}

#else // DEBUG_MEMORY==1

ICACHE_FLASH_ATTR void mem_free(void *p, unsigned n) {
	totalAlloc -= n;
	#if ESP8266==1
    os_free(p);
	#else
	free(p);
	#endif
	//JM_MEM_DEBUG("free mat=%u n=%d\n",totalAlloc,n);
}

ICACHE_FLASH_ATTR void* mem_zalloc(unsigned n, sint8_t dataType) {

#if ESP8266==1
	//JM_MEM_DEBUG("mem B n=%d\n",n);
	void *p = os_malloc(n);
	//JM_MEM_DEBUG("mem 1\n");
	if(!p) {
		JM_MEM_ERROR("MO heap size=%u\n", system_get_free_heap_size());
		//system_print_meminfo();
	} else {
		totalAlloc += n;
		//JM_MEM_DEBUG("mem mat=%d\n",totalAlloc);
	}
#else
	//JM_MEM_DEBUG("mem2\n");
    void *p = malloc(n);
	if(p) {
		totalAlloc += n;
		//JM_MEM_DEBUG("mem mat=%d\n",totalAlloc);
	} else {
		JM_MEM_ERROR("mem MO totalAlloc=%d\n",totalAlloc);
	}
#endif
	return p;
}

#endif // DEBUG_MEMORY==0

ICACHE_FLASH_ATTR static char* _printMemData(jm_hashmap_t *params) {
    char *pt = jm_hashmap_get(params, "type");
    if(!pt) return "Invalid type value\n";
	
#if DEBUG_MEMORY == 1
    if(jm_strcasecmp("pd",pt) == 0) {
    	_printPdData();
    } else  if(jm_strcasecmp("str",pt) == 0) {
    	_printStaticData(PREFIX_TYPE_STRINGG);
    }
#else
    if(jm_strcasecmp("mem",pt) == 0) {
    	mem_printMemInfo();
	}
#endif

    return "success";
}

ICACHE_FLASH_ATTR void mem_printMemInfo() {
	JM_MEM_DEBUG("mem totalAlloc=%d\n",totalAlloc);
#if ESP8266==1
	JM_MEM_DEBUG("Mem free heap size: %u\n",system_get_free_heap_size());
	JM_MEM_DEBUG("Mem system_show_malloc\n");
	///system_show_malloc();

	JM_MEM_DEBUG("Mem system_print_meminfo\n");
	system_print_meminfo();

#endif

#if DEBUG_MEMORY == 1
	JM_MEM_DEBUG("mem debugHashmapSize=%u\n", debugHashmapItemSize);
	_printStaticData(0);
	_printPdData();
#endif
}

ICACHE_FLASH_ATTR void jm_mem_init() {
#if DEBUG_MEMORY==1
    if(mallocMem == NULL) {
        mallocMem = jm_hashmap_create(20, PREFIX_TYPE_INT);
    }

    if(mmmMap==NULL) {
        mmmMap = jm_hashmap_create(10, PREFIX_TYPE_BYTE);
    }
#endif

#if JM_STM32==1 && JM_PS_ENABLE==1
    cmd_registFun("printMemDataa",_printMemData);
#endif
	
}

/***************************内存监控结束*********************************/

/*******************************Hash Map 锟斤拷锟斤拷锟斤拷锟�***************************************/

ICACHE_FLASH_ATTR char* jm_utils_copyStr(char *srcStr){
    if(!srcStr) return NULL;
    uint16_t len = jm_strlen(srcStr);
    char *m = jm_utils_mallocStr(len, "jm_utils_copyStr");
    if(m) {
         jmm->jm_memcpy(m, srcStr, len);
    }
    return m;
}

ICACHE_FLASH_ATTR void* jm_utils_malloc(uint16_t size, sint8_t dt){
    void *m = jmm->jm_zalloc_fn(size, dt);
    if(m) {
    	//JM_MEM_DEBUG("jm_utils_malloc jm_memset begin\n");
        jmm->jm_memset(m, 0, size);
       // JM_MEM_DEBUG("jm_utils_malloc jm_memset end\n");
    }else {
		JM_MEM_ERROR("malloc fail s=%d dt=%d\n",size,dt);
	}
    return m;
}

/**
*
*   size:字符串本身大小，不包括结尾字符
*/
ICACHE_FLASH_ATTR char* jm_utils_mallocStr(uint16_t size, char *msg){
    char *m = jm_utils_mallocWithError(size+1, PREFIX_TYPE_STRINGG, msg);
    return m;
}

/**
*
* size:字符串本身大小，不包括结尾字符
*/
ICACHE_FLASH_ATTR void jm_utils_releaseStr(char *p, uint16_t size){
    if(size == 0) {
        size = jm_strlen(p);
        if(size > 64) {
            JM_MEM_DEBUG("ur: %s",p);
        }
    }    
    jmm->jm_free_fn(p, size+1);
}

ICACHE_FLASH_ATTR void* jm_utils_mallocWithError(uint16_t size, sint8_t dt, char *msg){
    void *m = jm_utils_malloc(size,dt);
    if(!m) {
        JM_MEM_ERROR("malloc fail %s,alloc:%u\n",msg, size);
		jmm->jm_resetSys(msg);
    }else {
		//JM_MEM_DEBUG("malloc suc %s,alloc:%u\n",msg?msg:"", size);
	}
    //JM_MEM_DEBUG("jm_utils_mallocWithError sucee return\n");
    return m;
}

ICACHE_FLASH_ATTR static BOOL _hashmap_isStrKey(jm_hashmap_t *map){
    if(map == NULL) return false;
    return map->keyType == PREFIX_TYPE_STRINGG;
}

ICACHE_FLASH_ATTR static jm_hashmap_item_t* _hashmap_getItem(jm_hashmap_t *map, void *key, uint32_t idx) {
    if(map == NULL) return NULL;
	if(map->arr[idx] != NULL) {
        jm_hashmap_item_t* it = map->arr[idx];
		while(it != NULL) {
            if(_hashmap_isStrKey(map)) {
                if(jm_strcmp(key,it->key) == 0)
                    return it;
            } else {
                if(it->key == key)
                    return it;
            }
			it = it->next;
		}
	}
	return NULL;
}

ICACHE_FLASH_ATTR static jm_hashmap_item_t* _hashmap_newItem(jm_hashmap_t *map, void *cacheName, uint32_t idx) {
    if(map == NULL) return NULL;
    jm_hashmap_item_t* it = _hashmap_getItem(map,cacheName,idx);
	if(it != NULL) return it;

    uint16_t es = sizeof(jm_hashmap_item_t);
    //uint16_t ess = sizeof(struct _hashmap_item);
    //JM_MEM_DEBUG("_hni=%u idx=%d\n",es, idx);

#if DEBUG_MEMORY == 1
    jm_hashmap_item_t* nit = os_zalloc(es);
    debugHashmapItemSize += es;
#else
    jm_hashmap_item_t* nit = jm_utils_mallocWithError(es,PREFIX_TYPE_HASHMAP_ITEM,"_hashmap_newItem");
#endif

    if(_hashmap_isStrKey(map)) {
        nit->key = jm_utils_copyStr(cacheName);
    } else {
        nit->key = cacheName;
    }

    nit->next = NULL;

   // JM_MEM_DEBUG("newitem arr=%p idx=%d\n",map->arr, idx);
    if(map->arr[idx] == NULL) {
        map->arr[idx] = nit;
    } else {
        jm_hashmap_item_t* tail =  map->arr[idx];
        while(tail->next != NULL) tail = tail->next;
        tail->next = nit;
    }

	map->size++;
	//JM_MEM_DEBUG("newitem size=%d E\n",map->size);
	return nit;
}

ICACHE_FLASH_ATTR static uint32_t _hashmap_idx(jm_hashmap_t *map, void *cacheName) {
    uint32_t h = (uint32_t)cacheName;
    if(_hashmap_isStrKey(map)) {
        h = jm_hash32(cacheName, jm_strlen(cacheName));
    }
	return h % map->cap;
}

ICACHE_FLASH_ATTR static void _hashmap_release_item(jm_hashmap_item_t* it, BOOL isStrKey,BOOL needFreeVal) {

     if(it->key && isStrKey) {
        jm_utils_releaseStr(it->key, 0);
		it->key = NULL;
	}

    if(needFreeVal && it->val) {
        jmm->jm_free_fn(it->val, sizeof(it->val));
        it->val = NULL;
    }

#if DEBUG_MEMORY==1
    mem_free(it,0);
    debugHashmapItemSize += sizeof(jm_hashmap_item_t);
#else
    jmm->jm_free_fn(it, sizeof(jm_hashmap_item_t));
#endif
}

ICACHE_FLASH_ATTR BOOL jm_hashmap_put(jm_hashmap_t *map, void *cacheName, void *val){
    if(map == NULL) {
    	JM_MEM_ERROR("hm NULL\n");
    	return false;
    }
	uint32_t idx = _hashmap_idx(map,cacheName);
    jm_hashmap_item_t* it = _hashmap_getItem(map,cacheName, idx);
	if(it == NULL) {
		it = _hashmap_newItem(map, cacheName, idx);
		if(it == NULL) {
			//锟节达拷锟斤拷锟�
			JM_MEM_DEBUG("hput: %s", cacheName);
			map->ver++;
			return false;
		}
	}
	//锟斤拷锟斤拷值
	it->val = val;

	return true;
}

ICACHE_FLASH_ATTR BOOL jm_hashmap_remove(jm_hashmap_t *map, void *cacheName){
    if(map == NULL) return true;
	uint32_t idx = _hashmap_idx(map, cacheName);
	if(map->arr[idx] == NULL) return true;//删除不存在的元素
	if(map->arr[idx]->next == NULL) {
        //没有hash冲突，只有一个元素
        BOOL f = false;
        if(_hashmap_isStrKey(map)) {
            f = jm_strcmp(cacheName, map->arr[idx]->key) == 0;
        } else {
            f = cacheName == map->arr[idx]->key;
        }

        if(f) {
            _hashmap_release_item(map->arr[idx], _hashmap_isStrKey(map), map->needFreeVal);
            map->arr[idx] = NULL;
            map->size--;
            map->ver++;
            return true;
        }

		return false;
	}

	//hash冲突
    jm_hashmap_item_t* it = map->arr[idx];
    jm_hashmap_item_t* pre = NULL;

    BOOL strKey = _hashmap_isStrKey(map);

	while(it != NULL) {
        BOOL f = false;
        if(strKey) {
            f = jm_strcmp(cacheName, it->key) == 0;
        } else {
            f = cacheName == it->key;
        }

        if(f) {
            if(pre == NULL) {
                //第一个元素即是要删除的元素
                map->arr[idx] = it->next;
            } else {
                pre->next = it->next;
            }

            it->next = NULL;
            _hashmap_release_item(it,strKey,map->needFreeVal);
            map->size--;
            map->ver++;
            return true;
        }else{
            pre = it;
            it =  it->next;
        }
	}

	return false;
}

//锟接伙拷锟斤拷锟斤拷取
ICACHE_FLASH_ATTR void* jm_hashmap_get(jm_hashmap_t *map, void *cacheName){
    if(map == NULL) return NULL;
	uint32_t idx = _hashmap_idx(map,cacheName);
    jm_hashmap_item_t* it = _hashmap_getItem(map, cacheName, idx);
	if(it != NULL) {
		return it->val;
	}
	return NULL;
}

//元锟斤拷KEY锟角凤拷锟斤拷锟�
ICACHE_FLASH_ATTR BOOL jm_hashmap_exist(jm_hashmap_t *map, void *cacheName){
    if(map == NULL) return false;
	uint32_t idx = _hashmap_idx(map, cacheName);
	return _hashmap_getItem(map, cacheName, idx) != NULL;
}

ICACHE_FLASH_ATTR void* jm_hashmap_iteNext(jm_hash_map_iterator_t *ite){
	if(ite == NULL) {
		JM_MEM_ERROR("hin N!\n");
		return NULL;
	}

	if(ite->ver != ite->map->ver) {
		JM_MEM_ERROR("Ver Updated ended Ite\n");
		return NULL;
	}

	if(ite->cur == NULL || ite->cur->next == NULL) {
		for(ite->idx += 1; ite->idx < ite->map->cap; ite->idx++) {
            jm_hashmap_item_t* it = ite->map->arr[ite->idx];
			if(it != NULL) {
				ite->cur = it;//从链表第一个元素开始遍历
				return ite->cur->key;
			}
		}
		return NULL;
	} else {
		ite->cur = ite->cur->next;
		return ite->cur->key;
	}
}

ICACHE_FLASH_ATTR BOOL jm_hashmap_iteRemove(jm_hash_map_iterator_t *ite){
    if(ite == NULL || ite->map == NULL || ite->map->size == 0 || ite->cur == NULL || ite->idx <= -1) {
        JM_MEM_ERROR("hir N!\n");
        return true;
    }

    jm_hashmap_item_t* pre = ite->map->arr[ite->idx];
    if(pre == ite->cur) {
        //删除元素是链头第一个元素
        ite->map->arr[ite->idx] = pre->next;
        ite->map->size--;
        pre->next = NULL;
        _hashmap_release_item(pre, _hashmap_isStrKey(ite->map), ite->map->needFreeVal);

		ite->cur = NULL;
        return true;
    } else {
        while(pre != NULL && pre->next != ite->cur) {
            pre = pre->next;
        }

        if(pre == NULL) {
            //不存在此元素
            return false;
        }

        pre->next = ite->cur->next;
        ite->cur->next = NULL;
        _hashmap_release_item(ite->cur, _hashmap_isStrKey(ite->map),ite->map->needFreeVal);
        ite->map->size--;

        ite->cur = pre;//将迭代器当前元素指向删除元素的前一个元素
        return true;
    }

}

ICACHE_FLASH_ATTR jm_hashmap_t* jm_hashmap_create(uint16_t cap, sint8_t keyType){
    //JM_MEM_DEBUG("hcr %u\n",sizeof(jm_hashmap_t));

    jm_hashmap_t *map = jm_utils_mallocWithError(sizeof(jm_hashmap_t), PREFIX_TYPE_HASHMAP, "jm_hashmap_create");

	if(cap <= 0) {
		JM_MEM_ERROR("hcr %d, use default: %d\n",cap,SIZE);
		cap = SIZE;
	}

    int size = cap * sizeof(jm_hashmap_item_t*);
   // JM_MEM_DEBUG("Alloc jm_hashmap_create item arr %u\n",size);

   // JM_MEM_DEBUG("jm_hashmap_create p=%lu\n",p);

	map->arr = jm_utils_mallocWithError(size, PREFIX_TYPE_HASHMAP_ITEM, "hashmap arr");

	//JM_MEM_DEBUG("jm_hashmap_create arr succss\n");

	map->cap = cap;
	map->size = 0;
    map->keyType = keyType;
    map->needFreeVal = false;
    map->ver = 0;
	return map;
}

ICACHE_FLASH_ATTR void jm_hashmap_release(jm_hashmap_t *map){
    if(map == NULL) return;
    BOOL isStrKey = _hashmap_isStrKey(map);
	for(int idx = 0; idx < map->cap; idx++) {
        jm_hashmap_item_t *it = map->arr[idx], *next = NULL;
		while(it != NULL) {
			next = it->next;
			it->next = NULL;
			_hashmap_release_item(it,isStrKey,map->needFreeVal);
			it = next;
		}
        map->arr[idx] = NULL;
	}

    jmm->jm_free_fn(map->arr, map->cap * sizeof(jm_hashmap_item_t*));
	map->arr = NULL;
    jmm->jm_free_fn(map, sizeof(jm_hashmap_t));
}

/*******************************Hash Map 锟斤拷锟斤拷锟斤拷锟�***************************************/

/*******************************Cache 锟斤拷锟藉开始************************************/
#if DEBUG_CACHE_ENABLE==1
ICACHE_FLASH_ATTR BOOL cache_init(char *cacheName, uint16_t itemSize, sint8_t dataType){

	if(cacheMap == NULL) {
		cacheMap = jm_hashmap_create(SIZE, PREFIX_TYPE_STRINGG);//锟斤拷一锟斤拷Map
		if(cacheMap == NULL) {
			JM_MEM_ERROR("cini F1: %s, %d\n",cacheName);
			return false;
		}
	}

	if(jm_hashmap_exist(cacheMap, cacheName)) {
		JM_MEM_ERROR("cini e: %s\n",cacheName);
		return false;//锟斤拷锟斤拷锟窖撅拷锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷
	}

    JM_MEM_DEBUG("cini a %u\n",sizeof(struct _jm_cache_header));
	jm_cache_header_t* ch = jm_utils_mallocWithError(sizeof(struct _jm_cache_header),
        PREFIX_TYPE_CACHE_HEADER, "create _jm_cache_header");

	ch->cache = NULL;
	ch->memSize = itemSize;
	ch->eleNum = 0;
    ch->dataType = dataType;

	if(!jm_hashmap_put(cacheMap, cacheName,ch)) {
		JM_MEM_ERROR("cini F: %s\n",cacheName);
        jmm->jm_free_fn(ch,sizeof(struct _jm_cache_header));
		return false;//锟斤拷锟斤拷锟窖撅拷锟斤拷锟斤拷
	}

	return true;
}

ICACHE_FLASH_ATTR static jm_cache_t* _cache_createCacheItem(jm_cache_header_t *ch, sint8_t dt, char *cn){

    JM_MEM_DEBUG("_ccci %u\n",sizeof(struct _jm_cache));
	jm_cache_t* c = jm_utils_mallocWithError(sizeof(struct _jm_cache),PREFIX_TYPE_CACHE,"create jm_cache_t");

    JM_MEM_DEBUG("_ccci a %u\n",ch->memSize);
	void* it = jm_utils_mallocWithError(ch->memSize,dt,"cache data");

	c->item = it;

	c->next = ch->cache;
	ch->cache = c;
	return c;
}



static jm_hashmap_t* modelStatis = NULL;

typedef struct _jm_cache_debug {
    uint8_t model;
    char *cacheName;
    uint32_t takeMemSize;//占据内存大上
    uint32_t takeNum;//当前正在使用的个数
    uint32_t createNum;//创建个数
    uint32_t getNum;//取得个数
    uint32_t backNum;//反还个数
} jm_cache_debug_t;

ICACHE_FLASH_ATTR static jm_cache_debug_t* _cache_debug_getStatic(uint8_t model, char *cn){
    if(modelStatis == NULL) {
        modelStatis = jm_hashmap_create(10, PREFIX_TYPE_BYTE);
    }

    jm_cache_debug_t *cd = jm_hashmap_get(modelStatis,model);
    if(cd == NULL) {
        cd= jm_cli_getJmm()->jm_zalloc_fn(sizeof(jm_cache_debug_t),0);
        jm_cli_getJmm()->jm_memset(cd, 0, sizeof(jm_cache_debug_t));

        //cd = ( jm_cache_debug_t *)jm_utils_malloc(sizeof(jm_cache_debug_t));
        cd->model = model;
        cd->cacheName = cn;
        jm_hashmap_put(modelStatis, model, cd);
    }

    return cd;
}

ICACHE_FLASH_ATTR static jm_cache_t* _cache_debug_print(uint8_t model){
    jm_cache_debug_t* cd = _cache_debug_getStatic(model,NULL);

    //JM_MEM_DEBUG("name=%s, model=%u, createNum=%u, getNum=%u, ",cd->cacheName, cd->model, cd->createNum, cd->getNum);
   // JM_MEM_DEBUG("backNum=%u, takeNum=%u, takeMemSize=%u\n",cd->backNum, cd->takeNum, cd->takeMemSize);
}

ICACHE_FLASH_ATTR static jm_cache_t* _cache_get(jm_cache_header_t *ch, jm_cache_t *c, char *cn, uint8_t model){
    jm_cache_debug_t* cd = _cache_debug_getStatic(model,cn);
    if(cd) {
        cd->getNum++;
        cd->takeNum++;
        cd->takeMemSize = cd->takeNum * ch->memSize;

        if(jm_strcasecmp(cd->cacheName,"C_PS_ITEM_EXTRA_") != 0){
           // JM_MEM_DEBUG("get,");
            _cache_debug_print(model);
        }
       
    }
}

ICACHE_FLASH_ATTR static jm_cache_t* _cache_back(jm_cache_header_t *ch, jm_cache_t *c, char *cn, uint8_t model){
    jm_cache_debug_t* cd = _cache_debug_getStatic(model,cn);
    if(cd) {
        cd->backNum++;
        cd->takeNum--;
        cd->takeMemSize = cd->takeNum * ch->memSize;

        if(jm_strcasecmp(cd->cacheName,"C_PS_ITEM_EXTRA_") != 0){
            //JM_MEM_DEBUG("back,");
            _cache_debug_print(model);
        }
        
    }
}

ICACHE_FLASH_ATTR static jm_cache_t* _cache_create(jm_cache_header_t *ch, jm_cache_t *c, char *cn, uint8_t model){
    jm_cache_debug_t* cd = _cache_debug_getStatic(model,cn);
    if(cd) {
        cd->createNum++;
        cd->getNum++;
        cd->takeNum++;
        cd->takeMemSize = cd->takeNum * ch->memSize;
        if(jm_strcasecmp(cd->cacheName,"C_PS_ITEM_EXTRA_") != 0){
           // JM_MEM_DEBUG("create,");
        _cache_debug_print(model);
        }
    }
}



/**
 */
ICACHE_FLASH_ATTR void* cache_get(char *cacheName, BOOL created, uint8_t model){
	if(cacheMap == NULL) {
		//JM_MEM_DEBUG("cg F: %s",cacheName);
		return NULL;
	}

	jm_cache_header_t *ch = jm_hashmap_get(cacheMap, cacheName);

	if(!ch) {
		//JM_MEM_DEBUG("cg cache not exist: %s\n",cacheName);
		return NULL;
	}

    return jm_utils_mallocWithError(ch->memSize,ch->dataType,"cache data");

	if(ch->cache && ch->eleNum > 0) {
		jm_cache_t *c = ch->cache, *pre = NULL;
		while(c) {
			if(c->used) {
				pre = c;
				c = c->next;
			} else {
				c->used = true;
				ch->eleNum--;
				os_memset(c->item,0,ch->memSize);

#if DEBUG_CACHE_ENABLE == 1
                _cache_get(ch,c,cacheName,model);
#endif

				return c->item;
			}
		}
	}

	//JM_MEM_DEBUG("cache_get cache element is NULL: %s\n",cacheName);
	if(!created) {
		//JM_MEM_DEBUG("cache_get no cache item to found: %s\n",cacheName);
		return NULL;
	}

	jm_cache_t* c = _cache_createCacheItem(ch,ch->dataType,cacheName);
	if(c) {
#if DEBUG_CACHE_ENABLE == 1
        _cache_create(ch,c,cacheName,model);
#else
        //JM_MEM_DEBUG("cache_get new: %s, useable num: %u, total:%d , take mem size:%u\n",cacheName, ch->eleNum, ch->totalNum,ch->totalNum * ch->memSize);
#endif

		c->used = true;
        ch->totalNum++; //总元素个数
		return c->item;
	} else {
		//JM_MEM_DEBUG("cache_get mem out fail: %s, useable num: %u, total:%d , take mem size:%u\n",cacheName, ch->eleNum, ch->totalNum, ch->totalNum * ch->memSize);
		return NULL;
	}
}

ICACHE_FLASH_ATTR BOOL cache_back(char *cacheName, void *it, uint8_t model){
	if(cacheMap == NULL) {
		//JM_MEM_ERROR("cache_back cache is NULL: %s", cacheName);
		return false;
	}

	jm_cache_header_t *ch = jm_hashmap_get(cacheMap, cacheName);

	if(!ch) {
		//JM_MEM_ERROR("cache_back cache not exist: %s",cacheName);
		return false;
	}

	jmm->jm_free_fn(it, ch->memSize);

	return true;
}

#endif //#if DEBUG_CACHE_ENABLE == 1

/*******************************Cache 锟斤拷锟斤拷锟斤拷锟�************************************/
