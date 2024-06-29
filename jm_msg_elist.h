
ICACHE_FLASH_ATTR BOOL _elist_eleEqual( jm_msg_extra_data_t *ex, void *val, sint8_t type){
    if(ex->type != type) {
        return false;
    }

    switch((uint8_t)type) {
        case (uint8_t)PREFIX_TYPE_BYTE:
            if(ex->value.s8Val == (sint8_t)val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_SHORTT:
            if(ex->value.s16Val == (sint16_t)val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_INT:
            if(ex->value.s32Val == (sint32_t)val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_LONG:
            if(ex->value.s64Val ==(sint64_t) val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_STRINGG:
            if(jm_strcmp(ex->value.strVal,(char*)val)) return true;
            break;
        case (uint8_t)PREFIX_TYPE_LIST:
        case (uint8_t)PREFIX_TYPE_SET:
            if((jm_elist_t*)val == ex->value.list) return true;
            break;
        case (uint8_t)PREFIX_TYPE_MAP:
        case (uint8_t)PREFIX_TYPE_PROXY:
            if((jm_emap_t*)val == ex->value.map) return true;
            break;
        case (uint8_t)PREFIX_TYPE_BOOLEAN:
            if(ex->value.boolVal == (BOOL)val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_CHAR:
            if(ex->value.charVal == (char)val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
            if(ex->value.bytesVal == (uint8_t *)val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_SHORT:
            if(ex->value.anyVal == val) return true;
            break;
        default:
            break;
    }
    return false;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* _elist_getPreExtra(jm_elist_t *list, void *val, sint8_t type){

    if(_elist_eleEqual(list->_hd,val,type)) return list->_hd;

    jm_msg_extra_data_t *ex, *pre = list->_hd;
    while(pre->next != NULL) {
        if(_elist_eleEqual(pre->next,val,type)) return pre;
        pre = pre->next;
    }

    return NULL;
}

ICACHE_FLASH_ATTR void* jm_elist_next(jm_elist_iterator_t *ite){
    if(ite == NULL || ite->cur == NULL) return NULL;
    jm_msg_extra_data_t *c = ite->cur;
    ite->cur = ite->cur->next;
    return jm_extra_sgetVal(c);
}

ICACHE_FLASH_ATTR BOOL jm_elist_add(jm_elist_t *list, void* val, sint8_t type, BOOL needFreeMem){
	jm_msg_extra_data_t *ex = jm_extra_sputByType(list->_hd, "", val, type);//往列表加一个元素
	if(ex) {
        ex->neddFreeBytes = needFreeMem;
		list->size++;
		if(list->_hd == NULL) list->_hd = ex;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL jm_elist_addExtra(jm_elist_t *list, jm_msg_extra_data_t *ex){
	if(ex) {
		list->size++;
		if(list->_hd == NULL) list->_hd = ex;
		else {
			jm_msg_extra_data_t *tail = list->_hd;
			while(tail->next != NULL) tail = tail->next; //找到最后一个元素
			tail->next = ex; //加到最后一个元素
		}
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL jm_elist_removeByIdx(jm_elist_t *list, sint16_t idx){
    if(list == NULL || list->_hd == NULL) return true;
    if(idx < 0) return false;

    jm_msg_extra_data_t *ex = NULL, *cur = list->_hd;

    if(idx == 0) {
        ex = list->_hd;
    } else {
        sint16_t curIdx = 0;
        jm_msg_extra_data_t *cur = list->_hd;
        while(cur != NULL) {
            if(curIdx+1 != idx) {
                ex = cur;
                break;
            }else {
                cur = cur->next;
                curIdx++;
            }
        }
    }

    if(ex != NULL) {
        if(ex == list->_hd) {
            list->_hd = ex->next;
            ex->next = NULL;
            jm_extra_release(ex);
        } else {
            jm_msg_extra_data_t *delEle = ex->next;
            ex->next = delEle->next;
            delEle->next = NULL;
            jm_extra_release(delEle);
        }
        list->size--;
    }
    return true;

}

ICACHE_FLASH_ATTR BOOL jm_elist_remove(jm_elist_t *list, void *val, sint8_t type){

    if(list == NULL || list->_hd == NULL) return true;

    jm_msg_extra_data_t *ex = NULL;
    if(_elist_eleEqual(list->_hd, val,type)) {
        ex = list->_hd;
    }else {
        ex = _elist_getPreExtra(list,val,type);
    }

    if(ex != NULL) {
        if(ex == list->_hd) {
            //删除首元素
            list->_hd = ex->next;
            ex->next = NULL;
            jm_extra_release(ex);
        } else {
            jm_msg_extra_data_t *delEle = ex->next;
            ex->next = delEle->next;
            delEle->next = NULL;
            jm_extra_release(delEle);
        }
        list->size--;
    }
    return true;
}

ICACHE_FLASH_ATTR uint16_t jm_elist_size(jm_elist_t *list){
	return list->size;
}

ICACHE_FLASH_ATTR void* jm_elist_get(jm_elist_t *list, sint16_t idx){
	if(list->size < idx) return NULL;

	uint16_t c = 0;
	jm_msg_extra_data_t *ex = list->_hd;
	while(c++ < idx) {
		ex = ex->next;
	}

	if(ex == NULL) {
		return NULL;
	}

    return jm_extra_sgetVal(ex);
}

ICACHE_FLASH_ATTR BOOL jm_elist_exist(jm_elist_t *list, void *val, sint8_t type){

	jm_msg_extra_data_t *ex = list->_hd;
	while(ex != NULL) {
		if(ex->type != type) {
			continue;
		}

		switch((uint8_t)type) {
		case (uint8_t)PREFIX_TYPE_BYTE:
			if(ex->value.s8Val == (uint8_t)val) return true;
			break;
		case (uint8_t)PREFIX_TYPE_SHORTT:
			if(ex->value.s16Val == (uint16_t)val) return true;
			break;
		case (uint8_t)PREFIX_TYPE_INT:
			if(ex->value.s32Val == (uint32_t)val) return true;
			break;
		case (uint8_t)PREFIX_TYPE_LONG:
			if(ex->value.s64Val == (uint64_t)val) return true;
			break;
		case (uint8_t)PREFIX_TYPE_STRINGG:
			if(jm_strcmp(ex->value.strVal,(char*)val)) return true;
			break;
		case (uint8_t)PREFIX_TYPE_LIST:
		case (uint8_t)PREFIX_TYPE_SET:
            if((jm_elist_t*)val == ex->value.list) return true;
            break;
		case (uint8_t)PREFIX_TYPE_MAP:
		case (uint8_t)PREFIX_TYPE_PROXY:
			if((jm_emap_t*)val == ex->value.map) return true;
			break;
		case (uint8_t)PREFIX_TYPE_BOOLEAN:
			if(ex->value.boolVal == (BOOL)val) return true;
			break;
		case (uint8_t)PREFIX_TYPE_CHAR:
			if(ex->value.charVal == (char)val) return true;
			break;
        case (uint8_t)PREFIX_TYPE_BYTEBUFFER:
            if(ex->value.bytesVal == (uint8_t*)val) return true;
            break;
        case (uint8_t)PREFIX_TYPE_SHORT:
            if(ex->value.anyVal == val) return true;
            break;
		default:
			break;
		}
		ex = ex->next;
	}

	return false;
}

ICACHE_FLASH_ATTR jm_msg_extra_data_t* _elist_getExtra(jm_elist_t *list, void *val, sint8_t type){
    if(_elist_eleEqual(list->_hd, val, type)) return list->_hd;//头部即为要查找的元素
    jm_msg_extra_data_t *ex = _elist_getPreExtra(list,val,type);
    if(ex != NULL) return ex->next;
    return NULL;
}

#if DEBUG_CACHE_ENABLE == 1
static uint32_t elistCnt = 0;
#endif

ICACHE_FLASH_ATTR jm_elist_t* jm_elist_create(){

    JM_MSG_DEBUG("Alloc jm_elist_create %u\n",sizeof(jm_elist_t));

	jm_elist_t *m = (jm_elist_t *)jm_utils_mallocWithError(sizeof(jm_elist_t), PREFIX_TYPE_LIST, "jm_elist_create");

	m->_hd = NULL;
	m->size = 0;

#if DEBUG_CACHE_ENABLE == 1
    elistCnt++;
    JM_MSG_DEBUG("msg %u\n",elistCnt);
#endif
	return m;
}

ICACHE_FLASH_ATTR void jm_elist_release(jm_elist_t *list){
	if(list == NULL) {
		return;
	}
	if(list->_hd) jm_extra_release(list->_hd);
    jmm->jm_free_fn(list,sizeof(jm_elist_t));

#if DEBUG_CACHE_ENABLE == 1
    elistCnt--;
    JM_MSG_DEBUG("msg %u\n",elistCnt);
#endif
}
