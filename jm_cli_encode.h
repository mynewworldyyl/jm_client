ICACHE_FLASH_ATTR static uint16_t _c_encodeWriteLen(jm_buf_t *b, jm_msg_extra_data_t *extras);

//decode
ICACHE_FLASH_ATTR static BOOL _c_decodeMap(jm_buf_t *b, jm_emap_t *m) {

	uint16_t eleLen;//元素的个数,最多可以存放255个元素
	if(!jm_buf_get_u16(b, &eleLen)) {
		JM_CLI_ERROR("cli F1\n");
		return false;
	}

	JM_CLI_DEBUG("_c_decodeMap eleLen=%d\n",eleLen);

	if(eleLen == 0) return true;//无元素

	while(eleLen-- > 0) {
		sint8_t flag;
		char *p =  jm_buf_readString(b,&flag);

       /* if(jm_strcasecmp(p, "deviceId")==0) {
            JM_CLI_DEBUG("_c_pubsubItemParseBin test deviceId\n");
        }*/

		JM_CLI_DEBUG("cli: idx:%d, key: %s\n",eleLen,p);

		sint8_t type;
		if(!jm_buf_get_s8(b,&type)){
			JM_CLI_ERROR("cli get type %d err\n",type);
			jm_utils_releaseStr(p,0);
			return false;
		}

		void *v = jm_cli_decodeExtra(b,type);
		if(v == NULL) {
			jm_cli_getJmm()->jm_free_fn(p,jm_strlen(p)+1);
			continue;
		}

		if(PREFIX_TYPE_MAP == type || PREFIX_TYPE_PROXY == type
				||PREFIX_TYPE_LIST == type || PREFIX_TYPE_SET == type) {
			jm_emap_put(m, p, v, type, true,false);
			//jm_utils_releaseStr(p, -1);
		} else {
			jm_msg_extra_data_t *ex = (jm_msg_extra_data_t *)v;
			ex->strKey = p;
			ex->copyKey = true;
			jm_emap_putExtra(m, ex);
		}
	}

	return true;
}

ICACHE_FLASH_ATTR static BOOL _c_decodeColl(jm_buf_t *b, jm_elist_t *s){
	uint16_t size;//元素的个数,最多可以存放255个元素
	if(!jm_buf_get_u16(b, &size)) {
		JM_CLI_ERROR("cli F\n");
		return false;
	}

	if(size == 0) return true;//无元素

	while(size-- > 0) {

		sint8_t type;
		if(!jm_buf_get_s8(b,&type)){
			JM_CLI_ERROR("cli get type %d err\n",type);
			return false;
		}

		void *v = jm_cli_decodeExtra(b,type);
		if(v != NULL) {
			if(PREFIX_TYPE_MAP == type || PREFIX_TYPE_PROXY == type
					||PREFIX_TYPE_LIST == type || PREFIX_TYPE_SET == type) {
				jm_elist_add(s,v,type,true);
			} else {
				jm_msg_extra_data_t *ex = (jm_msg_extra_data_t *)v;
				jm_elist_addExtra(s,v);
			}
		} else {
			JM_CLI_ERROR("cli N: %d\n",size);
		}
	}
	return true;
}

ICACHE_FLASH_ATTR void* jm_cli_decodeExtra(jm_buf_t *b, sint8_t type) {

	jm_msg_extra_data_t *rst = NULL;
	if(!(PREFIX_TYPE_MAP == type || PREFIX_TYPE_PROXY == type
			||PREFIX_TYPE_LIST == type || PREFIX_TYPE_SET == type)) {
		rst = jm_extra_create(MODEL_JMSG+1);
	}

	//void *val = NULL;
	uint16_t len = 0;

    switch((uint8_t)type){
    case (uint8_t)PREFIX_TYPE_NULL:
		rst->value.bytesVal = NULL;
        rst->value.strVal = NULL;
        rst->value.map = NULL;
        rst->value.list = NULL;
		rst->value.anyVal = NULL;
		JM_CLI_DEBUG("cli bf\n",type);
        break;
    case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
		//JM_CLI_DEBUG("jm_cli_decodeExtra bytebuffer type %d\n",type);
		if(!jm_buf_get_u16(b,&len)) {
			goto jmerror;
		}

		rst->len = len;

		if(len == 0) {
			rst->value.bytesVal = NULL;
		} else {
            JM_CLI_DEBUG("cli b %u\n",len);
			uint8_t *arr = (uint8_t*)jm_utils_mallocWithError(len,PREFIX_TYPE_BYTEBUFFER,"jm_cli_decodeExtra1");
			if(!jm_buf_get_bytes(b, arr, len)) {
				if(arr) jmm.jm_free_fn(arr,len);
				goto jmerror;
			}
			rst->neddFreeBytes = true;
			rst->value.bytesVal = arr;
		}
        break;
    case (uint8_t)PREFIX_TYPE_INT:
    	{
			//JM_CLI_DEBUG("jm_cli_decodeExtra INT value\n");
			sint32_t iv;
			if(!jm_buf_get_s32(b,&iv)) {
				goto jmerror;
			} else {
				rst->value.s32Val = iv;
			}
			JM_CLI_DEBUG("cli INT %d\n",rst->value.s32Val);
    	}
        break;
    case (uint8_t)PREFIX_TYPE_BYTE:
    	{
			//JM_CLI_ERROR("jm_cli_decodeExtra BYTE value\n");
			sint8_t v;
			if(!jm_buf_get_s8(b,&v)) {
				goto jmerror;
			} else {
				rst->value.s8Val = v;
			}
			JM_CLI_DEBUG("cli s8 %d\n",rst->value.s8Val);
    	}
        break;
    case (uint8_t)PREFIX_TYPE_SHORTT:
    	{
			sint16_t sv;
			if(!jm_buf_get_s16(b,&sv)) {
				goto jmerror;
			} else {
				rst->value.s16Val = sv;
			}
			JM_CLI_DEBUG("cli s16 %d\n",rst->value.s16Val);
    	}
        break;
    case (uint8_t)PREFIX_TYPE_LONG:
    	{
			sint64_t lv;
			if(!jm_buf_get_s64(b,&lv)) {
				JM_CLI_ERROR("cli s64 err\n");
				rst->value.s64Val = 0;
				goto jmerror;
			} else {
				rst->value.s64Val = lv;
			}
			JM_CLI_DEBUG("cli s64 %ld\n",rst->value.s64Val);
    	}
        break;
    case (uint8_t)PREFIX_TYPE_FLOAT:
		JM_CLI_DEBUG("cli FLOAT t: %d\n",type);
        break;
    case (uint8_t)PREFIX_TYPE_DOUBLE:
		JM_CLI_DEBUG("cli DOUBLE t: %d\n",type);
        break;
    case (uint8_t)PREFIX_TYPE_BOOLEAN:
    	{
			BOOL bv;
			if(!jm_buf_get_bool(b,&bv)) {
				goto jmerror;
			} else {
				rst->value.boolVal = bv;
			}
			JM_CLI_DEBUG("cli b t: %d\n",rst->value.boolVal);
    	}
        break;
    case (uint8_t)PREFIX_TYPE_CHAR:
    	{
			char cv;
			if(!jm_buf_get_char(b,&cv)) {
				goto jmerror;
			} else {
				rst->value.charVal = cv;
			}
			JM_CLI_DEBUG("cli c t: %d\n",rst->value.charVal);
    	}
        break;
    case (uint8_t)PREFIX_TYPE_STRINGG:
    	{
			//JM_CLI_DEBUG("jm_cli_decodeExtra string type: %d\n",type);
			sint8_t slen;
			if(!jm_buf_get_s8(b,&slen)) {
                JM_CLI_ERROR("cli s8 len F\n");
				goto jmerror;
			}
			len = slen;

			if(len == -1) {
				rst->value.strVal = NULL;
			}else if(len == 0) {
                JM_CLI_DEBUG("cli %u\n",sizeof(char));
				char* vptr = jm_utils_mallocStr(sizeof(char),"jm_cli_decodeExtra2");
                *vptr = '\0';
				rst->value.strVal = vptr;
			}else {
				sint32_t ilen = len;
				if(len == 127) {
					sint16_t slen;
					if(!jm_buf_get_s16(b,&slen)) {
                        JM_CLI_ERROR("cli s16 len F\n");
						goto jmerror;
					}
					ilen = slen;

					if(slen == 32767) {
						if(!jm_buf_get_s32(b,&ilen)) {
                            JM_CLI_ERROR("cli get s32 len F\n");
							goto jmerror;
						}
					}
				}

                rst->neddFreeBytes = true;
				rst->len = ilen;

                JM_CLI_DEBUG("cli %u\n",ilen + 1);
				char* vptr = (char*)jm_utils_mallocStr(ilen, "jm_cli_decodeExtra3");

				if(!jm_buf_get_bytes(b,vptr,(uint16_t)ilen)) {
                    jm_utils_releaseStr(vptr,0);
                    JM_CLI_ERROR("cli str error ilen: %d\n",ilen);
					goto jmerror;
				}

				vptr[ilen] = '\0';
				rst->value.strVal = vptr;
				JM_CLI_DEBUG("cli string val: %s\n",vptr);
			}
    	}
        break;
    case (uint8_t)PREFIX_TYPE_MAP:
    case (uint8_t)PREFIX_TYPE_PROXY:
		JM_CLI_DEBUG("cli decode map: %d\n",type);
		jm_emap_t *map = jm_emap_create(PREFIX_TYPE_STRINGG);
		if(!_c_decodeMap(b,map)) {
			JM_CLI_ERROR("cli F %d\n");
			if(map) jm_emap_release(map);
			return NULL;
		}
		//rst->neddFreeBytes = true;//记录要释放内存
		return map;
    case (uint8_t)PREFIX_TYPE_SET:
    case (uint8_t)PREFIX_TYPE_LIST:
		JM_CLI_ERROR("cli list type: %d\n",type);
		jm_elist_t *list = jm_elist_create();
		if(!_c_decodeColl(b,list)) {
			JM_CLI_ERROR("cli: decode map F %d\n");
			if(list) jm_elist_release(list);
		}
		//rst->neddFreeBytes = true;//记录要释放内存
		return list;
    default:
		JM_CLI_ERROR("cli unsupport type: %d\n",type);
		goto jmerror;
	}

	rst->type = type;
	rst->next = NULL;

	return rst;

	jmerror:
		JM_CLI_ERROR("cli EF: %d\n",type);
		if(rst) jm_extra_release(rst);
		return NULL;

}

ICACHE_FLASH_ATTR BOOL _c_encodeExtra(jm_buf_t *b, jm_msg_extra_data_t *extras);

ICACHE_FLASH_ATTR static uint16_t _c_encodeWriteLen(jm_buf_t *b, jm_msg_extra_data_t *extras)  {
	//JM_CLI_DEBUG("_c_encodeWriteLen count len, extras addr: %u\n",extras);
	int eleCnt = 0;
	jm_msg_extra_data_t *te = extras;
	while(te != NULL) {
		eleCnt++;
		te = te->next;
		//JM_CLI_DEBUG("_c_encodeWriteLen count len, te: %u\n",te);
	}

	//JM_CLI_DEBUG("_c_encodeWriteLen write len\n");
	if(!jm_buf_put_s16(b,eleCnt)){
		return 0;
	}
	return eleCnt;
}

//Endoce
ICACHE_FLASH_ATTR static BOOL _c_encodeColl(jm_buf_t *b, jm_msg_extra_data_t *extras)  {

	int eleCnt = _c_encodeWriteLen(b, extras);
	if(eleCnt == 0) {
		JM_CLI_ERROR("cli len zero\n");
		return true;//锟睫革拷锟斤拷锟斤拷锟斤拷
	}

	while(extras != NULL) {

        jm_msg_extra_data_t *ex = jm_cli_getExtraByType(extras,extras->type);

		if(!jm_cli_encodeExtra(b, ex, extras->type)) {
			JM_CLI_ERROR("cli F\n");
			return false;
		}
		extras = extras->next;
	}

	return true;

}

ICACHE_FLASH_ATTR void* jm_cli_getExtraByType(jm_msg_extra_data_t *extras, sint8_t type){
    jm_msg_extra_data_t *ex = extras;
    if(PREFIX_TYPE_MAP == type || PREFIX_TYPE_PROXY == type ) {
        jm_emap_t *m = extras->value.map;
        ex = m->_hd;
    } else if(PREFIX_TYPE_LIST == type || PREFIX_TYPE_SET == type ) {
        jm_elist_t *m = extras->value.list;
        ex = m->_hd;
    }
    return ex;
}

ICACHE_FLASH_ATTR static BOOL _c_encodeExtraMap(jm_buf_t *b, jm_msg_extra_data_t *extras){

	//JM_CLI_DEBUG("jm_extra_encode count len, extras: %u\n",extras);
	int eleCnt = _c_encodeWriteLen(b, extras);
	if(eleCnt == 0) {
		JM_CLI_ERROR("cli len zero\n");
		return true;
	}

	//JM_CLI_DEBUG("_c_encodeExtraMap encode loop begin\n");

	uint16_t logWpos = 0;

	while(extras != NULL) {
		//JM_CLI_DEBUG("cli skey=%s rpos=%d wpos=%d cap=%u f=%d\n",extras->strKey, b->rpos, b->wpos, b->capacity, jm_buf_is_full(b));
		JM_CLI_DEBUG("cli skey=%s\n",extras->strKey);

		if(!jm_buf_writeString(b, extras->strKey, jm_strlen(extras->strKey))) {
			JM_CLI_ERROR("cli 0F: %s", extras->strKey);
			return false;
		}

		/*if(jm_strcasecmp(extras->strKey, "jlogPort")==0) {
			logWpos = b->wpos;
		}*/

        jm_msg_extra_data_t *ex = jm_cli_getExtraByType(extras,extras->type);
		if(!jm_cli_encodeExtra(b,ex,extras->type)) {
			JM_CLI_ERROR("cli 1F\n");
			return false;
		}
		extras = extras->next;
	}

	if(logWpos>0) {
		//JM_CLI_DEBUG("cli: rpos=%d wpos=%d logWpos=%d cap=%d f=%d\n",b->rpos,b-> logWpos, b->capacity, jm_buf_is_full(b));
		jm_buf_print(b, logWpos, 0);
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL jm_cli_encodeExtra(jm_buf_t *b, jm_msg_extra_data_t *extras, sint8_t type) {

	if(extras == NULL  || type == PREFIX_TYPE_NULL ) {
		JM_CLI_DEBUG("cli PREFIX_TYPE_NULL\n");
		if(!jm_buf_put_s8(b, PREFIX_TYPE_NULL)) {
			JM_CLI_ERROR("cli 0F\n");
			return false;
		}
		return true;
	}

	if(!jm_buf_put_s8(b,type)) {
		JM_CLI_ERROR("cli F t: %d\n", type);
		return false;
	}

    switch((uint8_t)type) {
        case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
            //JM_CLI_DEBUG("cli List val rpos=%d wpos=%d f=%d f=%d\n",b->rpos, b->wpos, jm_buf_is_full(b));
        	{
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
        	}
            return true;
        case (uint8_t)PREFIX_TYPE_INT:
           //JM_CLI_DEBUG("cli int val=%d rpos=%d wpos=%d f=%d\n", extras->value.s32Val, b->rpos, b->wpos, jm_buf_is_full(b));
            if (!jm_buf_put_s32(b, extras->value.s32Val)) {
                return false ;
            }
            return true;
        case (uint8_t)PREFIX_TYPE_BYTE:
           // JM_CLI_DEBUG("cli b val=%d rpos=%d wpos=%d f=%d\n", extras->value.s8Val,b->rpos, b->wpos, jm_buf_is_full(b));
            if (!jm_buf_put_s8(b, extras->value.s8Val)) {
                return false ;
            }
            return true;
        case (uint8_t)PREFIX_TYPE_SHORTT:
            //JM_CLI_DEBUG("cli s val=%d rpos=%d wpos=%d f=%d\n", extras->value.s16Val,b->rpos, b->wpos, jm_buf_is_full(b));
            if (!jm_buf_put_s16(b, extras->value.s16Val)) {
                return false ;
            }
            return true;
        case (uint8_t)PREFIX_TYPE_LONG:
           // JM_CLI_DEBUG("cli l val=%d rpos=%d wpos=%d f=%d\n", extras->value.s64Val,b->rpos, b->wpos, jm_buf_is_full(b));
            if (!jm_buf_put_s64(b, extras->value.s64Val)) {
                return false ;
            }
            return true;
        case (uint8_t)PREFIX_TYPE_FLOAT:

        case (uint8_t)PREFIX_TYPE_DOUBLE:

        case (uint8_t)PREFIX_TYPE_BOOLEAN:
           // JM_CLI_DEBUG("cli b val=%d rpos=%d wpos=%d f=%d\n", extras->value.boolVal, b->rpos, b->wpos, jm_buf_is_full(b));
            if (!jm_buf_put_bool(b, extras->value.boolVal)) {
                return false ;
            }
            return true;
        case (uint8_t)PREFIX_TYPE_CHAR:
           // JM_CLI_DEBUG("cli c val=%d rpos=%d wpos=%d f=%d\n", extras->value.charVal,b->rpos, b->wpos, jm_buf_is_full(b));
            if (!jm_buf_put_char(b, extras->value.charVal)) {
                return false ;
            }
            return true;
        case (uint8_t)PREFIX_TYPE_STRINGG:
            //JM_CLI_DEBUG("cli s len=%d, value: %s rpos=%d wpos=%d f=%d\n", extras->len, extras->value.strVal,b->rpos, b->wpos, jm_buf_is_full(b));
        	{
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
					JM_CLI_ERROR("cli len exceed %d\n", len);
					return false;
				}

				if(len > 0) {
					if(!jm_buf_put_chars(b,extras->value.strVal,len)) {
						JM_CLI_ERROR("cli w F3\n");
						return false;
					}
					//JM_CLI_DEBUG("jm_extra_encode write string value success %s\n",extras->value.strVal);
				}else {
					JM_CLI_DEBUG("cli le zero %d\n",len);
				}
        	}
            return true;
        case (uint8_t)PREFIX_TYPE_MAP:
        case (uint8_t)PREFIX_TYPE_PROXY:
            //JM_CLI_DEBUG("cli map type=%d rpos=%d wpos=%d f=%d\n",type,b->rpos, b->wpos, jm_buf_is_full(b));
            if(!_c_encodeExtraMap(b, extras)) {
                //编码Map失败
                return false;
            }
            return true;
        case (uint8_t)PREFIX_TYPE_SET:
        case (uint8_t)PREFIX_TYPE_LIST:
            //JM_CLI_DEBUG("cli col type=%d rpos=%d wpos=%d\n",type,b->rpos, b->wpos, jm_buf_is_full(b));
            //请求参数是Map
            if(!_c_encodeColl(b, extras)) {
                //编码Map失败
                return false;
            }
           return true;
        default:
            JM_CLI_ERROR("cli unsupport type=%d rpos=%d wpos=%d\n", type,b->rpos, b->wpos);
            return false;
    }
}
