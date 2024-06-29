ICACHE_FLASH_ATTR void jm_extra_release(jm_msg_extra_data_t *em) {
	if(em == NULL) return;
	while(em) {
		//SINFO("r0 ");
        if(em->neddFreeBytes) {
            switch ((uint8_t)em->type) {
                case (uint8_t)PREFIX_TYPE_STRINGG:
                    jm_utils_releaseStr(em->value.strVal, 0);
                    em->value.strVal = NULL;
                    break;
                case (uint8_t)PREFIX_TYPE_MAP:
                case (uint8_t)PREFIX_TYPE_PROXY:
                    jm_emap_release(em->value.map);
                    em->value.map = NULL;
                    break;
                case (uint8_t)PREFIX_TYPE_LIST:
                case (uint8_t)PREFIX_TYPE_SET:
                    jm_elist_release(em->value.list);
                    em->value.list = NULL;
                    break;
                case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
                    jm_buf_release((jm_buf_t*)em->value.bytesVal);
                    em->value.bytesVal = NULL;
                    break;
                case (uint8_t)PREFIX_TYPE_SHORT:
                    jmm->jm_free_fn(em->value.anyVal,0);
                    em->value.anyVal = NULL;
                    break;
            }
        }
        //SINFO("r1 ");
		if(em->strKey && em->copyKey) {
			// SINFO("r11 ");
			jm_utils_releaseStr(em->strKey, 0);
			// SINFO("r12 ");
			em->strKey = NULL;
			// SINFO("r13 ");
		}
		//SINFO("r2 ");
		jm_msg_extra_data_t *n = em->next;
		em->next = NULL;

/*
#if DEBUG_CACHE_ENABLE == 1
        cache_back(CACHE_PUBSUB_ITEM_EXTRA, em, em->model);
#else
        cache_back(CACHE_PUBSUB_ITEM_EXTRA, em, 0);
#endif
*/
		//SINFO("r3 ");
		jmm->jm_free_fn(em, sizeof(jm_msg_extra_data_t));
		//SINFO("r4 ");
		em = n;
	}
	//SINFO("r5 ");
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t *jm_extra_create(uint8_t model) {
   // jm_msg_extra_data_t *ex = cache_get(CACHE_PUBSUB_ITEM_EXTRA, true, model);
	jm_msg_extra_data_t *ex = (jm_msg_extra_data_t *)jm_utils_mallocWithError(sizeof(jm_msg_extra_data_t),PREFIX_TYPE_MSG_EXTRA_DATA,"jm_extra_create");

#if DEBUG_CACHE_ENABLE == 1
    if(ex) ex->model = model;
#endif
    return ex;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_get(jm_msg_extra_data_t *header, sint8_t key) {
	jm_msg_extra_data_t *em = header;
	while(em != NULL) {
		if(em->key == key) {
			return em;
		}
		em = em->next;
	}
	return NULL;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* _extra_setVal(jm_msg_extra_data_t *eem, sint8_t type, char *val) {
    switch((uint8_t)type) {
        case (uint8_t)PREFIX_TYPE_BYTE:
            eem->value.s8Val = (sint8_t)val;
            return eem;
        case (uint8_t)PREFIX_TYPE_SHORTT:
            eem->value.s16Val = (sint16_t)val;
            return eem;
        case (uint8_t)PREFIX_TYPE_INT:
            eem->value.s32Val = (sint32_t)val;
            return eem;
        case (uint8_t)PREFIX_TYPE_LONG:
            eem->value.s64Val = (sint64_t)val;
            return eem;
        case (uint8_t)PREFIX_TYPE_STRINGG:
            if(val) {
                eem->len = jm_strlen((char*)val);
            }

            eem->value.strVal = val;
            eem->neddFreeBytes = true;
            return eem;
        case (uint8_t)PREFIX_TYPE_BOOLEAN:
            eem->value.boolVal = (BOOL)val;
            return eem;
        case (uint8_t)PREFIX_TYPE_CHAR:
            eem->value.charVal = (char)val;
            return eem;
        case (uint8_t)PREFIX_TYPE_LIST:
        case (uint8_t)PREFIX_TYPE_SET:
            eem->value.list = (jm_elist_t*)val;
            eem->neddFreeBytes = true;
            return eem;
        case (uint8_t)PREFIX_TYPE_MAP:
        case (uint8_t)PREFIX_TYPE_PROXY:
            //msgExtra->len = len < 0 ? jm_strlen(val) : len;
            eem->value.map = (jm_emap_t*)val;
            eem->neddFreeBytes = true;
            return eem;
        case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
            eem->value.bytesVal = (uint8_t *)val;
            eem->neddFreeBytes = true;
            eem->len = sizeof(val);
            return eem;
        case (uint8_t)PREFIX_TYPE_SHORT:
            eem->value.anyVal = val;
            eem->neddFreeBytes = false;
        default:
            return eem;
    }
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putByType(jm_msg_extra_data_t *header, sint8_t key, void *val, sint8_t type) {
    jm_msg_extra_data_t *eem = jm_extra_put(header, key, type);
    if(eem == NULL) {
        return NULL;
    }
    return _extra_setVal(eem,type,val);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_put(jm_msg_extra_data_t *header, sint8_t key, sint8_t type) {
	jm_msg_extra_data_t *eem = jm_extra_get(header, key);
	if(eem != NULL) {
		eem->key = key;
		eem->type = type;
		return eem;//已经存在节点返回原节点头
	}

	jm_msg_extra_data_t *em = jm_extra_create(MODEL_JMSG+3);
	if(em==NULL) return NULL;

	em->key = key;
	//em->value = val;
	em->type = type;
	em->len = 0;

	if(header == NULL) {
		em->next = NULL;
	} else {

		//头锟斤拷锟斤拷锟斤拷
		em->next = header->next;
        header->next = em;

		//msg->extraMap = em;
		//msg->extraMap->next = NULL;
	}
	return em;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_iteNext(jm_msg_extra_data_iterator_t *ite) {
	if(ite == NULL || ite->cur == NULL || ite->cur->next == NULL) return NULL;
	ite->cur = ite->cur->next;
	return ite->cur;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sget(jm_msg_extra_data_t *header, char *key) {
	jm_msg_extra_data_t *em = header;
	while(em != NULL) {
		if(em->strKey == key) {
			//常量池字符串内存地址肯定相等
			return em;
		}else if(jm_strcasecmp(key, em->strKey)==0) {
			//非常量，如malloc申请的内存字符串
			return em;
		}
		em = em->next;
	}
	return NULL;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputBool(jm_msg_extra_data_t *header, char *strKey, BOOL val) {
	return jm_extra_sputByType(header, strKey,(void*)val, PREFIX_TYPE_BOOLEAN);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputS8(jm_msg_extra_data_t *header, char *strKey, sint8_t val) {
	return jm_extra_sputByType(header, strKey,(void*)val, PREFIX_TYPE_BYTE);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputS16(jm_msg_extra_data_t *header, char *strKey, sint16_t val) {
	return jm_extra_sputByType(header, strKey,(void*)val, PREFIX_TYPE_SHORTT);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputS32(jm_msg_extra_data_t *header, char *strKey, sint32_t val) {
	return jm_extra_sputByType(header, strKey,(void*)val, PREFIX_TYPE_INT);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputS64(jm_msg_extra_data_t *header, char *strKey, sint64_t val) {
	jm_msg_extra_data_t *msgExtra = jm_extra_sput(header, strKey, PREFIX_TYPE_LONG,false);
	msgExtra->value.s64Val = val;
	return msgExtra;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputStr(jm_msg_extra_data_t *e, char *strKey, char* val, uint16_t len) {
	jm_msg_extra_data_t *msgExtra = jm_extra_sputByType(e, strKey, val, PREFIX_TYPE_STRINGG);
	if(len > 0) msgExtra->len = len;
	return msgExtra;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputMap(jm_msg_extra_data_t *header, char *strKey, jm_emap_t *val,sint8_t type) {
	return jm_extra_sputByType(header, strKey, val, type);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputColl(jm_msg_extra_data_t *header, char *strKey, jm_elist_t *val, sint8_t type) {
	return jm_extra_sputByType(header, strKey, val, type);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sput(jm_msg_extra_data_t *header, char *strKey, sint8_t type, BOOL copyKey) {

	if(header != NULL && strKey != NULL && jm_strlen(strKey) > 0) {
		//列表元素的KEY为空
		jm_msg_extra_data_t *eem = jm_extra_sget(header, strKey);
		if(eem != NULL) {
			eem->type = type;
			return eem;
		}
	}

	jm_msg_extra_data_t *em = jm_extra_create(MODEL_JMSG+4);
	if(em==NULL) return NULL;

	if(copyKey) {
		em->strKey = jm_utils_copyStr(strKey);
	} else {
		em->strKey = strKey; //jm_utils_copyStr(strKey);
	}

	//em->value = val;
	em->copyKey = copyKey;
	em->type = type;
	em->len = 0;
	em->next = NULL;
	em->neddFreeBytes = true;

	if(header == NULL) return em;

	jm_msg_extra_data_t *tail = header;
	while(tail->next != NULL) tail = tail->next; //找到最后一个元素
	tail->next = em; //加到最后一个元素
   /* em->next = header->next; //头部插入法
    header->next = em;*/

	return em;  //永远返回新增加的元素
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputByType(jm_msg_extra_data_t *header, char *strKey, void* val, sint8_t type) {
	jm_msg_extra_data_t *eem = jm_extra_sput(header, strKey, type,false);
	if(eem == NULL) {
		return NULL;
	}
    return _extra_setVal(eem,type,val);
}

ICACHE_FLASH_ATTR void* jm_extra_sgetVal(jm_msg_extra_data_t *ex) {
	if(ex == NULL) return NULL;
	switch((uint8_t)ex->type) {
	case (uint8_t)PREFIX_TYPE_BYTE:
		return (void*)ex->value.s8Val;
	case (uint8_t)PREFIX_TYPE_SHORTT:
		return (void*)ex->value.s16Val;
	case (uint8_t)PREFIX_TYPE_INT:
		return (void*)ex->value.s32Val;
	case (uint8_t)PREFIX_TYPE_LONG:
		return (void*)ex->value.s64Val;
	case (uint8_t)PREFIX_TYPE_STRINGG:
        return (void*)ex->value.strVal;
	case (uint8_t)PREFIX_TYPE_LIST:
	case (uint8_t)PREFIX_TYPE_SET:
        return ex->value.list;
	case (uint8_t)PREFIX_TYPE_MAP:
	case (uint8_t)PREFIX_TYPE_PROXY:
		return ex->value.map;
	case (uint8_t)PREFIX_TYPE_BOOLEAN:
		return (void*) ex->value.boolVal;
	case (uint8_t)PREFIX_TYPE_CHAR:
		return (void*) ex->value.charVal;
    case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
        return (void*) ex->value.bytesVal;
    case (uint8_t)PREFIX_TYPE_SHORT:
        return (void*) ex->value.anyVal;
	default:
		return NULL;
	}
}

ICACHE_FLASH_ATTR void* jm_extra_sgetValByKey(jm_msg_extra_data_t *ex, char *strKey) {
	return jm_extra_sgetVal(jm_extra_sget(ex, strKey));
}

ICACHE_FLASH_ATTR  sint16_t jm_extra_sgetS16(jm_msg_extra_data_t *e, char *strKey){
	return (sint16_t)jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  sint8_t jm_extra_sgetS8(jm_msg_extra_data_t *e, char *strKey){
	return (sint8_t)jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  sint64_t jm_extra_sgetS64(jm_msg_extra_data_t *e, char *strKey){
	jm_msg_extra_data_t* ed = jm_extra_sget(e, strKey);
	if(ed) return ed->value.s64Val;
	else return 0;//无效值
}

ICACHE_FLASH_ATTR  sint32_t jm_extra_sgetS32(jm_msg_extra_data_t *e, char *strKey){
	return (sint32_t)jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  char jm_extra_sgetChar(jm_msg_extra_data_t *e, char *strKey){
	return (char)jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  BOOL jm_extra_sgetBool(jm_msg_extra_data_t *e, char *strKey){
	return (BOOL)jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  char* jm_extra_sgetStr(jm_msg_extra_data_t *e, char *strKey){
	return (char*)jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  jm_emap_t* jm_extra_sgetMap(jm_msg_extra_data_t *e, char *strKey){
	return jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  jm_elist_t* jm_extra_sgetColl(jm_msg_extra_data_t *e, char *strKey){
	return jm_extra_sgetValByKey(e, strKey);
}

ICACHE_FLASH_ATTR  char* jm_extra_sgetStrCpy(jm_msg_extra_data_t *e, char *strKey){
	jm_msg_extra_data_t* msgEx = jm_extra_sget(e,strKey);
	if(msgEx && msgEx->type == PREFIX_TYPE_STRINGG) {
        return jm_utils_copyStr(msgEx->value.strVal);
	}
	return NULL;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputByte(jm_msg_extra_data_t *e, char *strKey, sint8_t val){
	return jm_extra_sputByType(e,strKey,(void*)val,PREFIX_TYPE_BYTE);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputShort(jm_msg_extra_data_t *e, char *strKey, sint16_t val){
	return jm_extra_sputByType(e, strKey, (void*)val, PREFIX_TYPE_SHORTT);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputInt(jm_msg_extra_data_t *e, char *strKey, sint32_t val){
	return jm_extra_sputByType(e, strKey, (void*)val, PREFIX_TYPE_INT);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputLong(jm_msg_extra_data_t *e, char *strKey, sint64_t val){
	jm_msg_extra_data_t *eem = jm_extra_sput(e, strKey,PREFIX_TYPE_LONG,false);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s64Val = val;
	return eem;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_sputChar(jm_msg_extra_data_t *e, char *strKey, char val){
	return jm_extra_sputByType(e, strKey, (void*)val, PREFIX_TYPE_CHAR);
}

ICACHE_FLASH_ATTR static jm_msg_extra_data_t * jm_extra_decodeVal(jm_buf_t *b) {

	sint8_t type;
	if(!jm_buf_get_s8(b,&type)){
		goto jmerror;
	}

	jm_msg_extra_data_t *rst = jm_extra_create(MODEL_JMSG+2);
	//void *val = NULL;
	sint16_t len = 0;

	if(type == PREFIX_TYPE_NULL) {
		rst->value.bytesVal = NULL;
	}else if(PREFIX_TYPE_BYTEBUFFER == type){
		if(!jm_buf_get_s16(b,&len)) {
			goto jmerror;
		}

		rst->len = len;

		if(len == 0) {
			rst->value.bytesVal = NULL;
		} else {
            JM_MSG_DEBUG("msg jm_extra_decodeVal %u\n",len);
			uint8_t *arr = jm_utils_mallocWithError(len, PREFIX_TYPE_BYTEBUFFER,"decode array buffer");
			if(!jm_buf_get_bytes(b,arr,len)) {
				if(arr) jmm->jm_free_fn(arr,len);
				goto jmerror;
			}
			rst->neddFreeBytes = true;
			rst->value.bytesVal = arr;
		}
	}else if(type == PREFIX_TYPE_INT){
		int32_t v;
		if(!jm_buf_get_s32(b,&v)) {
			goto jmerror;
		} else {
			rst->value.s32Val = v;
		}
	}else if(PREFIX_TYPE_BYTE == type){
		sint8_t v;
		if(!jm_buf_get_s8(b,&v)) {
			goto jmerror;
		} else {
			rst->value.s8Val = v;
		}
	}else if(PREFIX_TYPE_SHORTT == type){
		sint16_t v;
		if(!jm_buf_get_s16(b,&v)) {
			goto jmerror;
		} else {
			rst->value.s16Val = v;
		}
	}else if(PREFIX_TYPE_LONG == type){
		sint64_t v=0;
		if(!jm_buf_get_s64(b,&v)) {
			goto jmerror;
		} else {
			rst->value.s64Val = v;
		}
	}else if(PREFIX_TYPE_FLOAT == type){
		
	}else if(PREFIX_TYPE_DOUBLE == type){
		
	}else if(PREFIX_TYPE_BOOLEAN == type){
		BOOL v;
		if(!jm_buf_get_bool(b,&v)) {
			goto jmerror;
		} else {
			rst->value.boolVal = v;
		}
	}else if(PREFIX_TYPE_CHAR == type){
		char v;
		if(!jm_buf_get_char(b,&v)) {
			goto jmerror;
		} else {
			rst->value.charVal = v;
		}
	}else if(PREFIX_TYPE_STRINGG == type){

		sint8_t slen;
		if(!jm_buf_get_s8(b,&slen)) {
			goto jmerror;
		}
		len = slen;
		rst->len = 0;
		rst->neddFreeBytes = true;

		if(len == -1) {
			rst->value.strVal = NULL;
		}else if(len == 0) {
            JM_MSG_DEBUG("msg jm_extra_decodeVal2 %u\n",sizeof(char));
			char* vptr = jm_utils_mallocStr(sizeof(char),"jm_extra_decodeVal2");
			rst->value.strVal = vptr;
		}else {

			sint32_t ilen = len;

			if(len == 127) {
				sint16_t slen;
				if(!jm_buf_get_s16(b,&slen)) {
					goto jmerror;
				}
				ilen = slen;

				if(slen == 32767) {
					if(!jm_buf_get_s32(b,&ilen)) {
						goto jmerror;
					}
				}
			}

			rst->len = ilen;

            JM_MSG_DEBUG("msg jm_extra_decodeVal3 %u\n",ilen + 1);
			char* vptr = jm_utils_mallocStr(ilen,"jm_extra_decodeVal3");

			if(!jm_buf_get_bytes(b,vptr,(uint16_t)ilen)) {
                JM_MSG_ERROR("msg get str jmerror\n");
				goto jmerror;
			}

			vptr[ilen] = '\0';
			rst->value.strVal = vptr;

		}
	}else if(PREFIX_TYPE_MAP == type){
		jm_emap_t *map = jm_extra_decode(b);
		rst->value.map = map; //是的，确实有点坑，但是这样用完全没问题，但是要记住，根据type的类型去使用值
		rst->neddFreeBytes = true;//记录要释放内存
	}else {
        JM_MSG_ERROR("msg invalid type: %d",type);
        goto jmerror;
    }

	//rst->key = k;
	rst->type = type;
	//rst->value = val;
	rst->next = NULL;
	rst->len = len;

	return rst;

	jmerror:
        JM_MSG_ERROR("msg F");
		if(rst) jm_extra_release(rst);
		return NULL;

}

ICACHE_FLASH_ATTR static BOOL jm_extra_encodeVal(jm_msg_extra_data_t *extras, jm_buf_t *b) {
	sint8_t type = extras->type;

	if (PREFIX_TYPE_BYTEBUFFER == type) {
		JM_MSG_DEBUG("msg List\n");
		uint8_t *ptr = extras->value.bytesVal;
		if(extras->len <= 0) {
			jm_buf_put_u16(b, 0);
			return false ;
		}

		if (!jm_buf_put_u16(b, extras->len)) {
			return false ;
		}
		if (!jm_buf_put_bytes(b, ptr, extras->len)) {
			return false;
		}
		return true;
	} else if(type == PREFIX_TYPE_INT) {
		JM_MSG_DEBUG("msg i: %d\n", extras->value.s32Val);
		if (!jm_buf_put_s32(b, extras->value.s32Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_BYTE == type) {
		JM_MSG_DEBUG("msg b: %d\n", extras->value.s8Val);
		if (!jm_buf_put_s8(b, extras->value.s8Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_SHORTT == type) {
		JM_MSG_DEBUG("msg s: %d\n", extras->value.s16Val);
		if (!jm_buf_put_s16(b, extras->value.s16Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_LONG == type) {
		JM_MSG_DEBUG("msg l: %d\n", extras->value.s64Val);
		if (!jm_buf_put_s64(b, extras->value.s64Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_FLOAT == type) {
		JM_MSG_DEBUG("msg f: %d\n", extras->value.s64Val);

	} else if (PREFIX_TYPE_DOUBLE == type) {
		JM_MSG_DEBUG("msg d: %d\n", extras->value.s64Val);

	} else if (PREFIX_TYPE_BOOLEAN == type) {
		JM_MSG_DEBUG("msg b: %d\n", extras->value.boolVal);
		if (!jm_buf_put_bool(b, extras->value.boolVal)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_CHAR == type) {
		JM_MSG_DEBUG("msg c: %d\n", extras->value.charVal);
		if (!jm_buf_put_char(b, extras->value.charVal)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_STRINGG == type) {
		JM_MSG_DEBUG("msg s len:%d, value: %s\n", extras->len, extras->value.strVal);
		sint8_t len = extras->len;
		if(len < MAX_BYTE_VALUE) {
			jm_buf_put_s8(b,len);
		}else if(len < MAX_SHORT_VALUE) {
			//0X7F=01111111=127 byte
			//0X0100=00000001 00000000=128 short
			jm_buf_put_s8(b,MAX_BYTE_VALUE);
			jm_buf_put_s16(b,len);
		}else if(len < MAX_INT_VALUE) {
			jm_buf_put_s8(b,MAX_BYTE_VALUE);
			jm_buf_put_s16(b,MAX_SHORT_VALUE);
			jm_buf_put_s32(b,len);
		} else {
			JM_MSG_ERROR("msg l exceed %d\n", len);
			return false;
		}

		if(len > 0) {
			if(!jm_buf_put_chars(b,extras->value.strVal,len)) {
				JM_MSG_ERROR("msg F1\n");
				return false;
			}
			//JM_MSG_DEBUG("jm_extra_encode write string value success %s\n",extras->value.strVal);
		}else {
			JM_MSG_ERROR("msg len less than zero %d\n",len);
		}
	}else if (PREFIX_TYPE_MAP == type) {
		JM_MSG_DEBUG("msg m\n");
		//请求参数是Map
		if(!jm_extra_encode(extras->value.map->_hd, b, NULL, extras->value.map->keyType)) {
			//编码Map失败
			return false;
		}
	}

	return true;
}

/**
 * len  extra
 */
ICACHE_FLASH_ATTR jm_emap_t* jm_extra_decode(jm_buf_t *b){

	if(b == NULL || jm_buf_is_empty(b)){
        JM_MSG_ERROR("msg N1\n");
        return NULL;
    }

	//int elen = b.readUnsignedShort();
	uint8_t eleLen; //元素的个数,最多可以存放255个元素
	if(!jm_buf_get_u8(b, &eleLen)) {
		JM_MSG_ERROR("msg: F2\r\n");
		return NULL;
	}

	if(eleLen == 0) {
        JM_MSG_ERROR("msg F3\n");
        return NULL;
    }

	sint8_t keyType;
	if(!jm_buf_get_s8(b, &keyType)) {
		JM_MSG_ERROR("msg F4\n");
		return NULL;
	}

    jm_emap_t *em = jm_emap_create(keyType);

	while(eleLen > 0) {
		eleLen--;

		char *p = NULL;
		sint8_t k = 0;

		if(keyType == PREFIX_TYPE_STRINGG) {//字符串
			sint8_t flag;
			p = jm_buf_readString(b,&flag);
			if(flag != JM_SUCCESS){
				JM_MSG_ERROR("msg F keyType\n");
				return NULL;
			}
			//v->strKey = k;
		} else {
			if(!jm_buf_get_s8(b,&k)) {
				JM_MSG_ERROR("msg F err\n");
				return NULL;
			}
			//v->key = k;
		}

        jm_msg_extra_data_t *ed  = jm_extra_decodeVal(b);
		if(ed == NULL) {
			continue;
		}

		if(keyType == PREFIX_TYPE_STRINGG) {//字符串
			ed->strKey = p;
			ed->copyKey = true;
		} else {
			ed->key = k;
		}

        jm_emap_putExtra(em,ed);
	}

	//jm_buf_release(wrapBuf);
	return em;

}

/**
 *wl锟芥储锟斤拷锟斤拷锟絙yte锟斤拷锟街斤拷锟斤拷
 *锟斤拷锟斤拷晒锟斤拷锟斤拷锟絫rue,失锟杰凤拷锟斤拷false
 */
ICACHE_FLASH_ATTR BOOL jm_extra_encode(jm_msg_extra_data_t *extras, jm_buf_t *b, uint16_t *wl, sint8_t keyType){

	//JM_MSG_DEBUG("jm_extra_encode begin\n");

	if(extras == NULL) {
		JM_MSG_ERROR("msg extras is NULL\n");
		return false;
	}

	//JM_MSG_DEBUG("jm_extra_encode wl\n");
	if(wl) *wl = 0;

	//锟斤拷锟街碉拷前写写位锟矫ｏ拷
	uint16_t wpos = b->wpos;

	//JM_MSG_DEBUG("jm_extra_encode count len, extras: %u\n",extras);
	int eleCnt = 0;
	jm_msg_extra_data_t *te = extras;
	while(te != NULL) {
		eleCnt++;
		te = te->next;
		//JM_MSG_DEBUG("jm_extra_encode count len, te: %u\n",te);
	}

	//JM_MSG_DEBUG("jm_extra_encode write len\n");
	jm_buf_put_u8(b,eleCnt);//写入元素个数

	if(eleCnt == 0) {
		if(wl) *wl = 1;
		JM_MSG_ERROR("msg F0\n");
		return true;//锟睫革拷锟斤拷锟斤拷锟斤拷
	}

	//JM_MSG_DEBUG("jm_extra_encode put keytype\n");
	if(!jm_buf_put_s8(b, keyType)) {
		JM_MSG_ERROR("msg F1 keyType: %d\n", keyType);
		return false;
	}

	//JM_MSG_DEBUG("jm_extra_encode encode loop begin\n");

	BOOL strKey = keyType == PREFIX_TYPE_STRINGG;
	while(extras != NULL) {

		if(strKey) {
			JM_MSG_DEBUG("msg skey: %s \n",extras->strKey);
			if(!jm_buf_writeString(b, extras->strKey,jm_strlen(extras->strKey))) {
				JM_MSG_ERROR("msg F1: %s", extras->strKey);
				return false;
			}
		}else {
			JM_MSG_DEBUG("msg byte key: %d \n",extras->key);
			if(!jm_buf_put_s8(b, extras->key)) {
				JM_MSG_ERROR("msg F3: %d", extras->key);
				return false;
			}
		}

		if((PREFIX_TYPE_STRINGG == extras->type || PREFIX_TYPE_BYTEBUFFER == extras->type) &&
				extras->value.strVal == NULL) {
			JM_MSG_DEBUG("msg PREFIX_TYPE_NULL: %d", PREFIX_TYPE_NULL);
			if(!jm_buf_put_s8(b, PREFIX_TYPE_NULL)) {
				JM_MSG_ERROR("msg F4: %d", PREFIX_TYPE_NULL);
				return false;
			} else {
				extras = extras->next;
				continue;
			}
		}

		//JM_MSG_DEBUG("jm_extra_encode type: %d \n",extras->type);
		if(!jm_buf_put_s8(b, extras->type)) {
			JM_MSG_ERROR("msg F5: %d", extras->type);
			return false;
		}

		if(!jm_extra_encodeVal(extras,b)) {
			return false;
		}
		//JM_MSG_DEBUG("jm_extra_encode value success\n");

		extras = extras->next;
	}

	//锟杰癸拷写锟斤拷锟街斤拷锟斤拷锟斤拷也锟斤拷锟角革拷锟斤拷锟斤拷息锟侥筹拷锟斤拷
	uint16_t wlen;
	if(b->wpos >= wpos) {
		wlen = b->wpos - wpos;
	} else {
		wlen = b->capacity- (wpos - b->wpos);
	}

	if(wl) *wl = wlen;

	JM_MSG_DEBUG("msg F6: %d\n",wlen);

	return true;
}

//锟较诧拷锟斤拷锟斤拷from 锟节碉拷全锟斤拷锟斤拷锟捷合碉拷 to锟斤拷
ICACHE_FLASH_ATTR jm_msg_extra_data_t * jm_extra_pullAll(jm_msg_extra_data_t *from, jm_msg_extra_data_t *to){
	if(from == NULL) {
		JM_MSG_ERROR("from is NULL\n");
		return to;
	}

	jm_msg_extra_data_t *f = from;
	while(f->next != NULL) f = f->next;//查找最后一个元素
	f->next = to;//插入链表头部
	return from;
}

ICACHE_FLASH_ATTR  sint16_t jm_extra_getS16(jm_msg_extra_data_t *e, sint8_t key){
	jm_msg_extra_data_t* ed = jm_extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s16Val;
}

ICACHE_FLASH_ATTR  sint8_t jm_extra_getS8(jm_msg_extra_data_t *e, sint8_t key){
	jm_msg_extra_data_t* ed = jm_extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s8Val;
}

ICACHE_FLASH_ATTR  sint64_t jm_extra_getS64(jm_msg_extra_data_t *e, sint8_t key){
	jm_msg_extra_data_t* ed = jm_extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s64Val;
}

ICACHE_FLASH_ATTR  sint32_t jm_extra_getS32(jm_msg_extra_data_t *e, sint8_t key){
	jm_msg_extra_data_t* ed = jm_extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s32Val;
}

ICACHE_FLASH_ATTR  char jm_extra_getChar(jm_msg_extra_data_t *e, sint8_t key){
	jm_msg_extra_data_t* ed = jm_extra_get(e,key);
	return ed == NULL ? 0 :ed->value.charVal;
}

ICACHE_FLASH_ATTR  BOOL jm_extra_getBool(jm_msg_extra_data_t *e, sint8_t key){
	jm_msg_extra_data_t* ed = jm_extra_get(e,key);
	return ed == NULL ? false : ed->value.boolVal;
}

ICACHE_FLASH_ATTR  char* jm_extra_getStr(jm_msg_extra_data_t *e, sint8_t key){
	jm_msg_extra_data_t* ed = jm_extra_get(e,key);
	return ed == NULL ? NULL : ed->value.strVal;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putMap(jm_msg_extra_data_t *header, sint8_t key, jm_emap_t *val,sint8_t type) {
    return jm_extra_putByType(header, key, val, type);
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putColl(jm_msg_extra_data_t *header, sint8_t key, jm_elist_t *val, sint8_t type) {
    return jm_extra_putByType(header, key, val, type);
}

ICACHE_FLASH_ATTR  jm_emap_t* jm_extra_getMap(jm_msg_extra_data_t *e, sint8_t key){
    jm_msg_extra_data_t* ed = jm_extra_get(e,key);
    return ed == NULL ? NULL : ed->value.map;
}

ICACHE_FLASH_ATTR  jm_elist_t* jm_extra_getColl(jm_msg_extra_data_t *e, sint8_t key){
    jm_msg_extra_data_t* ed = jm_extra_get(e,key);
    return ed == NULL ? NULL : ed->value.list;
}

ICACHE_FLASH_ATTR  void* jm_extra_getAnyVal(jm_msg_extra_data_t *e, sint8_t key){
    jm_msg_extra_data_t* ed = jm_extra_get(e,key);
    return ed == NULL ? NULL : ed->value.anyVal;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putByte(jm_msg_extra_data_t *e, sint8_t key, sint8_t val){
	jm_msg_extra_data_t *eem = jm_extra_put(e, key, PREFIX_TYPE_BYTE);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s8Val = val;
	return eem;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putShort(jm_msg_extra_data_t *e, sint8_t key, sint16_t val){
	jm_msg_extra_data_t *eem = jm_extra_put(e, key,PREFIX_TYPE_SHORTT);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s16Val = val;
	return eem;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putInt(jm_msg_extra_data_t *e, sint8_t key, sint32_t val){
	jm_msg_extra_data_t *eem = jm_extra_put(e, key,PREFIX_TYPE_INT);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s32Val = val;
	return eem;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putLong(jm_msg_extra_data_t *e, sint8_t key, sint64_t val){
	jm_msg_extra_data_t *eem = jm_extra_put(e, key,PREFIX_TYPE_LONG);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s64Val = val;
	return eem;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putChar(jm_msg_extra_data_t *e, sint8_t key, char val){
	jm_msg_extra_data_t *eem = jm_extra_put(e, key, PREFIX_TYPE_CHAR);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.charVal = val;
	return eem;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putBool(jm_msg_extra_data_t *e, sint8_t key, BOOL val){
	jm_msg_extra_data_t *eem = jm_extra_put(e, key,PREFIX_TYPE_BOOLEAN);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.boolVal = val;
	return eem;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* jm_extra_putStr(jm_msg_extra_data_t *e, sint8_t key, char* val, uint16_t len){
	jm_msg_extra_data_t *eem = jm_extra_put(e, key,PREFIX_TYPE_STRINGG);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.strVal = val;
	eem->len = len;
	return eem;
}
