ICACHE_FLASH_ATTR BOOL jm_emap_put(jm_emap_t *map, void *key, void *val, sint8_t type, BOOL needFreeMem, BOOL copyKey){
	jm_msg_extra_data_t *ex = NULL;
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        ex = jm_extra_sputByType(map->_hd, key, val, type);
    }else {
		//ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putByType(jm_msg_extra_data_t *header, int8_t key, void *val, sint8_t type)
		//jm_msg_extra_data_t *_hd;
		ex = (jm_msg_extra_data_t *)jm_extra_putByType((jm_msg_extra_data_t *)(map->_hd), (int8_t)key, val, type);
    }

	if(ex) {
		ex->copyKey = copyKey;
        ex->neddFreeBytes = needFreeMem;
		if(map->_hd == NULL) {
            map->_hd = ex;
        }
		map->size++;
		return true;
	} else {
		return false;
	}
}

ICACHE_FLASH_ATTR BOOL jm_emap_putByte(jm_emap_t *map, void *key, sint8_t val, BOOL copyKey){
    return jm_emap_put(map, key, (void*)val, PREFIX_TYPE_BYTE,false,copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putShort(jm_emap_t *map, void *key, sint16_t val, BOOL copyKey){
    return jm_emap_put(map, key, (void*)val,PREFIX_TYPE_SHORTT,false,copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putInt(jm_emap_t *map, void *key, sint32_t val, BOOL copyKey){
    return jm_emap_put(map, key, (void*)val,PREFIX_TYPE_INT,false,copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putBool(jm_emap_t *map, void *key, BOOL val, BOOL copyKey){
    return jm_emap_put(map, key, (void*)val,PREFIX_TYPE_BOOLEAN,false,copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putLong(jm_emap_t *map, void *key, sint64_t val, BOOL copyKey){
    return jm_emap_put(map, key, (void*)val,PREFIX_TYPE_LONG,false, copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putStr(jm_emap_t *map, void *key, char *val,BOOL needFreeMem, BOOL copyKey){
    return jm_emap_put(map, key, (void*)val, PREFIX_TYPE_STRINGG, needFreeMem, copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putAll(jm_emap_t *map, jm_emap_t *src){
    jm_emap_iterator_t ite = {src->_hd};
    void *k  = NULL;
    while((k= jm_emap_next(&ite)) != NULL) {
        jm_msg_extra_data_t* ex = jm_emap_get(src,k);
        if(ex != NULL) {
            void* v = jm_extra_sgetVal(ex);
            if(ex->strKey != NULL) {
                jm_emap_put(map, ex->strKey, v, ex->type, false,true);
            }else {
                jm_emap_put(map, (void*)ex->key, v, ex->type, false,false);
            }
        }
    }
}

ICACHE_FLASH_ATTR BOOL jm_emap_putMap(jm_emap_t *map, void *key, jm_emap_t *val, BOOL needFreeMem,BOOL copyKey){
    return jm_emap_put(map, key, val,PREFIX_TYPE_MAP, needFreeMem,copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putList(jm_emap_t *map, void *key, jm_elist_t *val, BOOL needFreeMem,BOOL copyKey){
    return jm_emap_put(map, key, val,PREFIX_TYPE_LIST, needFreeMem,copyKey);
}

ICACHE_FLASH_ATTR BOOL jm_emap_putExtra(jm_emap_t *map, jm_msg_extra_data_t *ex){
	if(ex) {
		if(map->_hd == NULL) map->_hd = ex;
		else {
			jm_msg_extra_data_t *tail = map->_hd;
			while(tail->next != NULL) tail = tail->next; //找到最后一个元素
			tail->next = ex; //加到最后一个元素
		}
		map->size++;
		return true;
	} else {
		return false;
	}
}


//复制map到一个新的map，注意释放原始map前，要确保_hd不被释放，将_hd设备为NULL即可
ICACHE_FLASH_ATTR jm_emap_t* jm_emap_copy(jm_emap_t *map){
	jm_emap_t *ps = jm_emap_create(map->keyType);
	ps->size = map->size;
	ps->_hd = map->_hd;
	return ps;
}

ICACHE_FLASH_ATTR BOOL jm_emap_remove(jm_emap_t *map, void *key, BOOL release){
    jm_msg_extra_data_t *ex = NULL, *pre = map->_hd;

    if(map->keyType == PREFIX_TYPE_STRINGG) {
        if(jm_strcasecmp(key, pre->strKey) == 0) {
           //第一个元素即为要删除的元素
           ex = pre;
        } else {
            while(pre->next != NULL) {
                if(jm_strcasecmp(key,pre->next->strKey) == 0) {
                    ex = pre->next;
                    break;
                } else {
                    pre = pre->next;
                }
            }
        }
    } else {
    	//SINFO("1");
        if((void*)(pre->key) == key) {
            //第一个元素即为要删除的元素
            ex = pre;
        } else {
        	//SINFO("2");
            while(pre->next != NULL) {
                if((void*)(pre->next->key) == key) {
                    ex = pre->next;
                    break;
                } else {
                    pre = pre->next;
                }
            }
        }
        //SINFO("3");
    }

    if(ex != NULL) {
    	//SINFO("4");
    	if(ex == map->_hd) {
			//删除首元素
    		//SINFO("5");
			map->_hd = ex->next;
			ex->next = NULL;
			if(release)
				jm_extra_release(ex);
			else
				jmm->jm_free_fn(ex, sizeof(jm_msg_extra_data_t));
			//SINFO("6");
		} else {
			//SINFO("7 ");
			pre->next = ex->next;
			//SINFO("71 ");
			ex->next = NULL;
			if(release)
			{
				//SINFO("72 ");
				jm_extra_release(ex);
				//SINFO("73 ");
			}
			else
			{
				//SINFO("81 ");
				jmm->jm_free_fn(ex, sizeof(jm_msg_extra_data_t));
				//SINFO("82 ");
			}
		}

        map->size--;
        //SINFO("9 ");
    }
   // SINFO("10");
    return true;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_emap_get(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sget(map->_hd, key);
    }else {
        return jm_extra_get(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR char* jm_emap_getStr(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetStr(map->_hd,key);
    } else {
        return jm_extra_getStr(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR sint8_t jm_emap_getByte(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetS8(map->_hd,key);
    } else {
        return jm_extra_getS8(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR BOOL jm_emap_getBool(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetBool(map->_hd,key);
    } else {
        return jm_extra_getBool(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR sint16_t jm_emap_getShort(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetS16(map->_hd,key);
    } else {
        return jm_extra_getS16(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR sint32_t jm_emap_getInt(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetS32(map->_hd,key);
    } else {
        return jm_extra_getS32(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR sint64_t jm_emap_getLong(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetS64(map->_hd,key);
    } else {
        return jm_extra_getS64(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR jm_emap_t* jm_emap_getMap(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetMap(map->_hd,key);
    } else {
        return jm_extra_getMap(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR jm_elist_t* jm_emap_getList(jm_emap_t *map, void *key){
    if(map->keyType == PREFIX_TYPE_STRINGG) {
        return jm_extra_sgetColl(map->_hd,key);
    } else {
        return jm_extra_getColl(map->_hd, (sint8_t)key);
    }
}

ICACHE_FLASH_ATTR BOOL jm_emap_exist(jm_emap_t *map, void *key){
	return jm_emap_get(map,key) != NULL;
}

ICACHE_FLASH_ATTR BOOL jm_emap_size(jm_emap_t *map){
	return map->size;
}

ICACHE_FLASH_ATTR void* jm_emap_next(jm_emap_iterator_t *ite){
    if(ite == NULL || ite->cur == NULL) return NULL;
    jm_msg_extra_data_t *pre = ite->cur;
    ite->cur = ite->cur->next;

    if(pre->strKey != NULL) {
        return pre->strKey;
    } else {
        return (void*)pre->key;
    }
}

#if DEBUG_CACHE_ENABLE == 1
static uint32_t emapCnt = 0;
#endif

ICACHE_FLASH_ATTR jm_emap_t* jm_emap_create(sint8_t keyType){

    JM_MSG_DEBUG("jm_emap_create %u\n",sizeof(jm_emap_t));
	jm_emap_t *m = (jm_emap_t *)jm_utils_mallocWithError(sizeof(jm_emap_t),PREFIX_TYPE_MAP,"jm_emap_create");

    m->keyType = keyType;
	m->_hd = NULL;
	m->size = 0;

#if DEBUG_CACHE_ENABLE == 1
    emapCnt++;
    JM_MSG_DEBUG("msg emap create %u\n",emapCnt);
#endif

	return m;
}

ICACHE_FLASH_ATTR void jm_emap_release(jm_emap_t *map){
	if(map == NULL) {
		return;
	}

	if(map->_hd) {
        jm_extra_release(map->_hd);
        map->_hd = NULL;
    }
    jmm->jm_free_fn(map, sizeof(jm_emap_t));

#if DEBUG_CACHE_ENABLE == 1
    emapCnt--;
    JM_MSG_DEBUG("emap release %u\n",emapCnt);
#endif

}
