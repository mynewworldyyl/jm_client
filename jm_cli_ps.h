#if JM_PS_ENABLE==1
typedef struct _pubsub_listener_item{
	jm_cli_PubsubListenerFn lis;
	sint8_t type;
	struct _pubsub_listener_item *next;
} ps_listener_item_t;

typedef struct _pubsub_listener_map{
	BOOL rm;//是否可处理远程消息，有些命令只能处理本地局域网的请求，不接受通过服务器转发的远程命令
	char *topic;
	sint64_t subMsgId;
	sint32_t subId;
	ps_listener_item_t *listeners;
	struct _pubsub_listener_map *next;
} ps_listener_map;

static ps_listener_map *ps_listener = NULL;

ICACHE_FLASH_ATTR static  jm_cli_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg);
ICACHE_FLASH_ATTR static  jm_cli_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg);
ICACHE_FLASH_ATTR static void _c_subTopicAfterjmLogin();

ICACHE_FLASH_ATTR static  char* _c_getTopic();

#endif //JM_PS_ENABLE

#if JM_PS_ENABLE==1

ICACHE_FLASH_ATTR static void _c_releasePusubItem(jm_pubsub_item_t *item){
	jm_cli_getJmm()->jm_free_fn(item, sizeof(jm_pubsub_item_t));
}

ICACHE_FLASH_ATTR static  jm_pubsub_item_t* _c_createPubsubItem(){
	return jm_utils_mallocWithError(sizeof(jm_pubsub_item_t),PREFIX_TYPE_PUBSUB_ITEM,"_c_createPubsubItem");
	//return cache_get(CACHE_PUBSUB_ITEM,true,MODEL_JCLIENT);
}

ICACHE_FLASH_ATTR static  uint8_t _psitem_getDataType(uint8_t flag) {
    return (flag >> FLAG_DATA_TYPE) & 0x03;
}

ICACHE_FLASH_ATTR static  void _c_pubsubItemRelease(jm_pubsub_item_t *it){
	if(!it) return;

	if(it->data) {
        uint8 dt = _psitem_getDataType(it->flag);
        if(FLAG_DATA_BIN == dt) {
            jm_buf_release(it->data);
        }else if(FLAG_DATA_STRING == dt){
           jm_utils_releaseStr(it->data, 0);
        }else if(FLAG_DATA_JSON== dt){
            jm_utils_releaseStr(it->data, 0);
        } else {
            if(PREFIX_TYPE_MAP == it->dt || PREFIX_TYPE_PROXY == it->dt) {
                jm_emap_release(it->data);
            } else if(PREFIX_TYPE_LIST == it->dt || PREFIX_TYPE_SET == it->dt) {
                jm_elist_release(it->data);
            } else {
                jm_extra_release(it->data);
            };
        }
		it->data = NULL;
	}

	if(it->cxt) {
        jm_emap_release(it->cxt);
		it->cxt = NULL;
	}

    if(it->extraMap) {
        jm_emap_release(it->extraMap);
        it->extraMap = NULL;
    }

	if(it->topic) {
       jm_utils_releaseStr(it->topic, 0);
		it->topic = NULL;
	}

    if(it->callback) {
        jm_utils_releaseStr(it->callback, 0);
        it->callback = NULL;
    }

	//jmm->jm_free_fn(it);
    _c_releasePusubItem(it);

}

ICACHE_FLASH_ATTR static  ps_listener_map* _c_getPubsubListenerMap(char *topic){
	ps_listener_map *h;
	if(ps_listener != NULL) {
		h = ps_listener;
		while(h != NULL) {
			if(0 == strcmp(topic, h->topic)) {
				return h;
			}else {
				h = h->next;
			}
		}
	}
	 return NULL;
}

ICACHE_FLASH_ATTR static  ps_listener_map* _c_createPubsubListenerMap(char *topic){
	ps_listener_map *h = _c_getPubsubListenerMap(topic);
	if(h) return h;

    JM_CLI_DEBUG("_ccpl %u\n",sizeof(struct _pubsub_listener_map));
	h = (ps_listener_map*)jm_utils_mallocWithError(sizeof(struct _pubsub_listener_map),PREFIX_TYPE_PUBSUB_LISTENER_MAP
		,"_ccpl");

	h->listeners = NULL;
	h->next = NULL;
	h->topic = jm_utils_copyStr(topic);

	if(ps_listener == NULL) {
		ps_listener = h;
	} else {
		h->next = ps_listener;
		ps_listener = h;
	}

	return h;
}

ICACHE_FLASH_ATTR static  jm_cli_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg) {
	sint8_t code = jm_emap_getByte(msg->extraMap,(void*)EXTRA_KEY_PS_OP_CODE);

	JM_CLI_DEBUG("_cpom B c=%d \n",code);

	if(code == MSG_OP_CODE_SUBSCRIBE) {
		sint32_t subId = jm_emap_getInt(msg->extraMap, (void*)EXTRA_KEY_EXT0);
		char *topic = jm_emap_getStr(msg->extraMap, (void*)EXTRA_KEY_PS_ARGS);

		ps_listener_map *m = _c_getPubsubListenerMap(topic);
		if(m == NULL) {
			JM_CLI_ERROR("%s \n",topic);
			return MEMORY_OUTOF_RANGE;
		}
		if(m->subMsgId == msg->msgId){
			m->subId = subId;
		}
	}

	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR BOOL _c_addListenerItem(ps_listener_map *m, jm_cli_PubsubListenerFn listener, sint8_t type){
	if(m->listeners) {
		ps_listener_item_t *item = m->listeners;
		while(item) {
			if(item->lis == listener){
				item->type = type;
				JM_CLI_DEBUG("cli t: %s \n",m->topic);
				return true;
			}
			item = item->next;
		}
	}

    JM_CLI_DEBUG("cli addlis 1 %u\n",sizeof(struct _pubsub_listener_item));
	ps_listener_item_t *item = (ps_listener_item_t*)jm_utils_mallocWithError(sizeof(struct _pubsub_listener_item),
           PREFIX_TYPE_PUBSUB_LISTENER_ITEM, "_c_addListenerItem create topic");

	JM_CLI_DEBUG("cli t1: %s, %d \n", m->topic, type);

	item->lis = listener;
	item->next = NULL;
	item->type = type;

	if(m->listeners == NULL) {
		m->listeners = item;
	} else {
		item->next = m->listeners;
		m->listeners = item;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL _c_doSubscribeTopic(ps_listener_map *m){

	JM_CLI_DEBUG("_c_doSubscribeTopic B\n");

	jm_msg_t *msg = NULL;

	msg = jm_msg_create_msg(MSG_TYPE_PUBSUB,NULL);
	if(msg == NULL) {
		JM_CLI_ERROR("cli: %s \n",m->topic);
		return false;
	}

	_c_setMsgUdp(msg);

	m->subMsgId = msg->msgId;
	m->subId = 0;

    jm_emap_putByte(msg->extraMap, (void*)EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_SUBSCRIBE,false);
    jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_PS_ARGS, m->topic, false,false);
	/*jm_cli_send_msg_result_t subRes = */

    jm_emap_putByte(msg->extraMap, (void*)EXTRA_KEY_CHANNEL_NO, _c_getChanelNo(),false);

    if(jm_msg_isUdp(msg)) {
    	 jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_UDP_HOST, (char*)sysCfg.jmHost,false,false);
    	 jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_UDP_PORT, sysCfg.jmPort,false);
    }

	if(loginKey) {
		jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_LOGIN_KEY, loginKey, false,false);
	}

    jm_cli_send_msg_result_t resultCode = jm_cli_sendMessage(msg);
	if(resultCode != JM_SUCCESS) {
		JM_CLI_ERROR("cli m: %s, c:%d \n", m->topic, resultCode);
		return false;
	}
	/*if(!subRes) {
		return false;
	}*/
	jm_msg_release(msg);

	JM_CLI_DEBUG("_c_doSubscribeTopic E\n");

	return true;
}

ICACHE_FLASH_ATTR BOOL jm_cli_subscribe(char *topic, jm_cli_PubsubListenerFn listener, sint8_t type, BOOL rm){

	/*if(rm && !jm_cli_isLogin()) {
		JM_CLI_DEBUG("jm_cli_subscribe need login to regist subcribe topic:%s\n",topic);
		return false;
	}*/

    if(topic == NULL || jm_strlen(topic) == 0) {
        JM_CLI_ERROR("cli 1N %s\n",topic);
        return false;
    }

	if(listener == NULL) {
		JM_CLI_ERROR("cli 2N %s, t:%d\n",topic,type);
        jm_utils_releaseStr(topic, 0);
		return false;
	}

	BOOL isNewTopic = false;
	ps_listener_map *m = _c_getPubsubListenerMap(topic);
	if(m == NULL) {
		m = _c_createPubsubListenerMap(topic);
		if(!m) {
			JM_CLI_ERROR("cli MO: %s\n",topic);
			return false;
		}
		isNewTopic = true;
		m->rm = rm;
	}

	if(!_c_addListenerItem(m, listener, type)) {
		JM_CLI_ERROR("cli F t: %s \n",topic);
		return false;
	}

	if(rm && isNewTopic && jm_cli_isLogin()) {
		_c_doSubscribeTopic(m);
		JM_CLI_ERROR("cli S\n");
	}

	JM_CLI_DEBUG("cli E: %s, T:%d\n",m->topic,type);

	return true;
}

ICACHE_FLASH_ATTR BOOL jm_cli_subscribeByType(jm_cli_PubsubListenerFn listener, sint8_t type, BOOL rm){
	char *topic = TOPIC_P2P;
    BOOL relTopic = false;
	if(rm) {
		if(!jm_cli_isLogin()) {
			//远程控制必须先登录才能订阅
			JM_CLI_DEBUG("cli not login\n");
			return false;
		} else {
			topic = _c_getTopic();
			if(!topic) {
				JM_CLI_ERROR("cli tN \n");
				return false;
			}
            relTopic = true;
		}
	}

	if(!jm_cli_subscribe(topic, listener, type, rm)) {
		JM_CLI_ERROR("cli F t:%s\n",topic);
	}

    if(topic && relTopic)
        jmm.jm_free_fn(topic, jm_strlen(topic)+1);

	return true;
}

ICACHE_FLASH_ATTR BOOL jm_cli_subscribeP2PByType(jm_cli_PubsubListenerFn listener, sint8_t type){
	//BOOL isNewTopic = false;
	ps_listener_map *m = _c_getPubsubListenerMap(TOPIC_P2P);
	if(m == NULL) {
		m = _c_createPubsubListenerMap(TOPIC_P2P);
		if(!m) {
			JM_CLI_ERROR("cli mo t: %s \n",TOPIC_P2P);
			return false;
		}
	}

	if(!_c_addListenerItem(m, listener, type)) {
		JM_CLI_ERROR("cli: %s \n",TOPIC_P2P);
		return false;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL jm_cli_unsubscribe(char *topic, jm_cli_PubsubListenerFn listener){
	if(listener == NULL) return false;
	if(topic == NULL || jm_strlen(topic) == 0) return false;

	ps_listener_map *m = _c_getPubsubListenerMap(topic);
	if(m == NULL || m->listeners == NULL) {
		return true;
	}

	ps_listener_item_t *it, *pre, *cit;

	it = pre = NULL;
	cit = m->listeners;

	while(cit) {
		if(cit->lis == listener) {
			it = cit;
			break;
		}
		pre = cit;
		cit = cit->next;
	}

	if(it == NULL) return true;

	if(pre != NULL) {
		pre->next = cit->next;
		cit->next = NULL;
        jmm.jm_free_fn(it, sizeof(ps_listener_item_t));
		return true;
	} else {
		m->listeners = NULL;
        jmm.jm_free_fn(cit, sizeof(ps_listener_item_t));
	}

	jm_msg_t* msg = jm_msg_create_msg(MSG_TYPE_PUBSUB,NULL);
	if(msg == NULL) {
		JM_CLI_ERROR("cu: %s \n",topic);
		return false;
	}

	/**
	 let ps = [{k:Constants.EXTRA_KEY_PS_OP_CODE, v:MSG_OP_CODE_UNSUBSCRIBE, t:Constants.PREFIX_TYPE_BYTE},
	{k:Constants.EXTRA_KEY_PS_ARGS, v:callback.id, t:Constants.PREFIX_TYPE_INT}]
	 */
    jm_emap_putByte(msg->extraMap, (void*)EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_UNSUBSCRIBE,false);
	jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_PS_ARGS, m->subId,false);
    jm_emap_putByte(msg->extraMap, (void*)EXTRA_KEY_CHANNEL_NO, (sint8_t)jm_hashmap_get(name2ChannelNo,JM_UDP_SENDER_NAME),false);
	if(loginKey != NULL)
            jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_LOGIN_KEY, loginKey, false,false);

	jm_cli_send_msg_result_t subRes = jm_cli_sendMessage(msg);
	/*if(!subRes) {
		return false;
	}*/
	jm_msg_release(msg);

	return true;
}

ICACHE_FLASH_ATTR jm_emap_t * jm_cli_getUdpExtraByHost(char *host, sint32_t port, BOOL isUdp){

	//JM_CLI_DEBUG("jm_cli_getUdpExtraByHost begin\n");
    JM_CLI_DEBUG("cli ip: %s, p:%u \r\n",host,port);// windows 输出错误，ESP输出正常

    jm_emap_t *nex = jm_emap_create(PREFIX_TYPE_BYTE);
    jm_emap_putStr(nex, (void*)EXTRA_KEY_UDP_HOST, host, false,false);
    jm_emap_putInt(nex, (void*)EXTRA_KEY_UDP_PORT, port,false);
    jm_emap_putByte(nex, (void*)EXTRA_KEY_UDP_ACK, isUdp,false);
    jm_emap_putByte(nex,(void*)EXTRA_KEY_CHANNEL_NO, (sint8_t)jm_hashmap_get(name2ChannelNo,JM_UDP_SENDER_NAME),false);
    JM_CLI_DEBUG("cli E\n");
    return nex;

}

ICACHE_FLASH_ATTR jm_emap_t* jm_cli_getUdpExtra(jm_pubsub_item_t *it){
	//JM_CLI_DEBUG("jm_cli_getUdpExtra cxt is: %u\n", it->cxt);
	BOOL isUdp = jm_emap_getBool(it->cxt, EXTRA_SKEY_UDP_ACK);
	//JM_CLI_DEBUG("jm_cli_getUdpExtra 1\n", it->cxt);
	//jm_msg_extra_data_t *ex = NULL;
	if(!isUdp) {
		return NULL;
	}

	//JM_CLI_DEBUG("jm_cli_getUdpExtra 2\n", it->cxt);
    char *host = jm_emap_getStr(it->extraMap, (void*)EXTRA_KEY_UDP_HOST);
    sint32_t port = jm_emap_getInt(it->extraMap, (void*)EXTRA_KEY_UDP_PORT);
   // JM_CLI_DEBUG("jm_cli_getUdpExtra %s : %d\n", host, port);

	return jm_cli_getUdpExtraByHost(host, port, true);

}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_respExtra(jm_pubsub_item_t *it, jm_emap_t *payload){

	JM_CLI_DEBUG("cli B it->id=%ld\n",it->id);
	//jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true,MODEL_JCLIENT);
	jm_pubsub_item_t *item = _c_createPubsubItem();

	//指示响应包，对方收到后，不用再给返回值，以避免进入死循环
	jm_emap_putBool(payload, "_r", true, false);

	item->id = it->id;//应答报文

	jm_cli_initPubsubItem(item, FLAG_DATA_EXTRA);

	if(jm_strcasecmp(it->callback,"cb") == 0) {
		item->callback = it->callback;
		item->topic = it->topic;
		JM_CLI_DEBUG("cli cb=%s", item->topic);
	} else {
		if(jm_strlen(it->callback) > 0) {
			item->topic = it->callback;
		}else {
			item->topic = it->topic;
		}
		JM_CLI_DEBUG("cli topic=%s", item->topic);
	}

	item->type = it->type;
	item->fr = it->to;
	item->to = it->fr;

	item->dt = PREFIX_TYPE_MAP;
	item->data = payload;

	jm_cli_send_msg_result_t sendResult = jm_cli_publishPubsubItem(item, it->extraMap);
	JM_CLI_DEBUG("cli r %d t: %s, %u\n",sendResult, item->topic, item->id);

	//jm_buf_release(buf);
	jm_emap_release(item->cxt);

	//cache_back(CACHE_PUBSUB_ITEM,item,MODEL_JCLIENT);
	_c_releasePusubItem(item);

	//JM_CLI_DEBUG("cre E");
	return sendResult;
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_respError(jm_pubsub_item_t *it, sint8_t code, char *msg){
    jm_emap_t *m = jm_emap_create(PREFIX_TYPE_STRINGG);
    jm_emap_putByte(m,"code",code,false);
    jm_emap_putStr(m,"msg",msg,false,false);
    jm_cli_send_msg_result_t r = jm_cli_respExtra(it,m);
    jm_emap_release(m);
    return r;
}


ICACHE_FLASH_ATTR static void _c_dispachPubsubItem(jm_pubsub_item_t *it){
	if(it==NULL) return;

	JM_CLI_DEBUG("dis it.type=%d\n",it->type);
	
	BOOL isResp = false;
	if(it->type == ITEM_TYPE_CTRL && it->data) {
		isResp = jm_emap_getBool(it->data,"_r");
	}

	int32_t cid = jm_emap_getInt(it->extraMap, (void*)EXTRA_KEY_CLIENT_ID);
	int8_t gid = jm_emap_getByte(it->extraMap, (void*)EXTRA_KEY_GROUP_ID);

	if(!isResp && sysCfg.clientId != 0 && (cid != sysCfg.clientId || gid != sysCfg.grpId)) {

		BOOL err = true;
		if(it->type == ITEM_TYPE_SLAVE) {
			//未绑定账号的设备上线
			char *deviceId = jm_emap_getStr(it->data,"deviceId");
			if(deviceId == NULL || jm_strlen(deviceId) == 0) {
				err = false;
			}
		}

		if(err) {
			JM_CLI_ERROR("cli perm rjt clientId=%d groupId=%d cid=%d gid=%d\n", sysCfg.clientId , sysCfg.grpId, cid, gid);
			//JM_CLI_DEBUG("1 ");
			jm_emap_t *ps = jm_emap_create(PREFIX_TYPE_STRINGG);
			jm_emap_putInt(ps ,RESP_CODE, 10, false);
			jm_emap_putStr(ps, RESP_MSG, "Rej", false, false);

			jm_cli_respExtra(it, ps);

			jm_emap_release(ps);

			return;
		}
	}

	//JM_CLI_DEBUG("2 ");
	ps_listener_map *m = _c_getPubsubListenerMap(it->topic);
	if(m == NULL || m->listeners == NULL) {
		JM_CLI_ERROR("cli Nt: %s",it->topic);
		return;
	}

	//JM_CLI_DEBUG("3 ");
	BOOL find = false;
	ps_listener_item_t *lis_item = m->listeners;
	while(lis_item) {
		//JM_CLI_DEBUG("4 ");
		if(lis_item->type == 0 || lis_item->type == it->type) {
			JM_CLI_DEBUG("dis one");
			find = true;
			lis_item->lis(it);
		}
		lis_item = lis_item->next;
	}

	//JM_CLI_DEBUG("5 ");
	if(!find) {
		JM_CLI_ERROR("cli N L: %s type:%d",it->topic,it->type);
	}
}

ICACHE_FLASH_ATTR static  uint8_t _psitem_setDataFlag(int idx, uint8_t dataFlag) {
	return dataFlag | (1 << idx);
}

/*private void clearDataFlag(int idx) {
	this.dataFlag &= ~(1 << idx);
}*/

ICACHE_FLASH_ATTR static  BOOL _psitem_isDataFlag(int idx, uint8_t dataFlag) {
	return (dataFlag & (1 << idx)) != 0;
}

ICACHE_FLASH_ATTR void psitem_pubsubItemParseBin(jm_buf_t *buf, jm_msg_t *msg){
	jm_pubsub_item_t *it = _c_createPubsubItem();
	if(!it){
        JM_CLI_ERROR("cli mo\n");
		goto jmerror;
	}

	if(!jm_buf_get_u8(buf,&it->dataFlag)) {
		JM_CLI_ERROR("cli F1\n");
        goto jmerror;
	}

	if(!jm_buf_get_u8(buf,&it->flag)) {
		JM_CLI_ERROR("cli F2\n");
        goto jmerror;
	}

	if(!jm_buf_get_s32(buf,&it->fr)) {
		JM_CLI_ERROR("cli F3\n");
        goto jmerror;
	}

	if(_psitem_isDataFlag(0,it->dataFlag)) {
		if(!jm_buf_get_s64(buf,&it->id)) {
			JM_CLI_ERROR("cli F4\n");
            goto jmerror;
		}
	}

	JM_CLI_DEBUG("cli psitem id=%d msgId=%ld\n",it->id, msg->msgId);

	if(_psitem_isDataFlag(1,it->dataFlag)) {
		if(!jm_buf_get_s8(buf,&it->type)) {
			JM_CLI_ERROR("cli F5\n");
            goto jmerror;
		}
	}

	if(_psitem_isDataFlag(2,it->dataFlag)) {
		sint8_t flag;
		it->topic  = jm_buf_readString(buf,&flag);
		if(flag != JM_SUCCESS){
			JM_CLI_ERROR("cli F6\n");
            goto jmerror;
		}
	}

	if(_psitem_isDataFlag(3,it->dataFlag)) {
		if(!jm_buf_get_s32(buf,&it->srcClientId)) {
			JM_CLI_ERROR("cli F7\n");
            goto jmerror;
		}
	}

	if(_psitem_isDataFlag(4,it->dataFlag)) {
		if(!jm_buf_get_s32(buf,&it->to)) {
			JM_CLI_ERROR("cli F8\n");
            goto jmerror;
		}
	}

	if(_psitem_isDataFlag(5,it->dataFlag)) {
		sint8_t flag;
		it->callback  = jm_buf_readString(buf, &flag);
		if(flag != JM_SUCCESS){
			JM_CLI_ERROR("cli F9\n");
            goto jmerror;
		}
	}

	/*if(_psitem_isDataFlag(6,it->dataFlag)) {
		if(!jm_buf_get_u8(buf,&it->delay)) {
			JM_CLI_DEBUG("_psitem_pubsubItemParseBin fail to read delay\n");
			return;
		}
	}*/

	if(_psitem_isDataFlag(6,it->dataFlag)) {
		//extra
		//it->cxt = jm_extra_decode(buf);
        /*if(jm_buf_get_s8(buf,&it->dt)) {
            JM_CLI_DEBUG("_psitem_pubsubItemParseBin get cxt type fail topic: %s",it->topic );
            return;
        }*/
        it->cxt = jm_cli_decodeExtra(buf,PREFIX_TYPE_MAP);
    }

	if(_psitem_isDataFlag(7,it->dataFlag)) {
		uint8 dt = _psitem_getDataType(it->flag);
		if(FLAG_DATA_BIN == dt) {
			uint16_t size = jm_buf_readable_len(buf);
			if(size > 0) {
				jm_buf_t *b = jm_buf_create(size);
				if(!jm_buf_get_buf(buf,b,size)) {
					JM_CLI_ERROR("ppip F10:%d\n",size);
					jm_buf_release(b);
                    goto jmerror;
				}
				it->data = b;
			}else {
				it->data = NULL;
			}

		}else if(FLAG_DATA_STRING == dt){
			//this.data = in.readUTF();
			sint8_t flag;
			it->data  = jm_buf_readString(buf,&flag);
			if(flag != JM_SUCCESS){
				JM_CLI_ERROR("ppip F11\n");
                goto jmerror;
			}
		}else if(FLAG_DATA_JSON== dt){
			sint8_t flag;
			char *p  = jm_buf_readString(buf,&flag);
			if(flag != JM_SUCCESS){
				JM_CLI_ERROR("ppip F12\n");
                goto jmerror;
			}
			it->data = p;
		} else {
			//it->data = jm_extra_decode(buf);
            if(!jm_buf_get_s8(buf,&it->dt)) {
                JM_CLI_ERROR("cli F13: %s",it->topic );
                goto jmerror;
            }
            it->data = jm_cli_decodeExtra(buf,it->dt);

			if(it->data) {
				if(!jm_emap_exist(it->data,"host")) {
					jm_emap_putStr(it->data, "host", jm_emap_getStr(msg->extraMap,(void*)EXTRA_KEY_UDP_HOST),false,false);
				}
				if(!jm_emap_exist(it->data,"port")) {
					jm_emap_putInt(it->data, "port", jm_emap_getInt(msg->extraMap,(void*)EXTRA_KEY_UDP_PORT),false);
				}
			}
		}
	}

    it->extraMap = msg->extraMap;
    msg->extraMap = NULL;

	JM_CLI_DEBUG("dispachPubsubItem t=%d\n",it->type);
	_c_dispachPubsubItem(it);

	jmerror:
		_c_pubsubItemRelease(it);
		return;
}

ICACHE_FLASH_ATTR static  jm_cli_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg){
	JM_CLI_DEBUG("cli pubsubMsgHandle msgId=%ld\n",msg->msgId);
	if(jm_msg_getDownProtocol(msg) == PROTOCOL_JSON) {
        JM_CLI_ERROR("cli not support PROTOCOL_JSON\n");
		//_c_pubsubItemParseJson(msg);
		//os_printf("");
	} else {
		psitem_pubsubItemParseBin(msg->payload, msg);
	}
	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_publishStrItemByTopic(char *topic, sint8_t type, char *content/*, jm_msg_extra_data_t *extra*/){
	if(topic == NULL || jm_strlen(topic) == 0) {
		JM_CLI_ERROR("cli Nt\n");
		return INVALID_PS_DATA;
	}

    jm_emap_t *ps = jm_emap_create(PREFIX_TYPE_INT);
    jm_emap_putByte(ps, (void*)EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_FORWARD_BY_TOPIC,false);
    jm_emap_putStr(ps, (void*)EXTRA_KEY_PS_ARGS, topic, false,false);

	//jm_msg_extra_data_t *msgExtra = jm_extra_putByte(NULL, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_FORWARD_BY_TOPIC);
	//jm_extra_putStr(msgExtra, EXTRA_KEY_PS_ARGS, topic, strlen(topic));
    jm_cli_send_msg_result_t rst = jm_cli_publishStrItem(topic, type, content, ps);
    jm_emap_release(ps);
    return rst;
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_publishStrItem(char *topic, sint8_t type, char *content, jm_emap_t *ps){
	if(topic == NULL || jm_strlen(topic) == 0) {
		JM_CLI_ERROR("cli ps topic N\n");
		return INVALID_PS_DATA;
	}

	//jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true,MODEL_JCLIENT);
	jm_pubsub_item_t *item = _c_createPubsubItem();
	//(jm_pubsub_item_t*)jmm.jm_free_fn(sizeof(struct _c_pubsub_item));
	//os_memset(item,0,sizeof(struct _c_pubsub_item));

	if(item == NULL) {
		JM_CLI_ERROR("cli F1\n");
		return MEMORY_OUTOF_RANGE;
	}

	item->flag = 0;
	item->cxt = NULL;
	//item->data = buf;
	item->delay = 0;
	
//#ifndef JM_STM32
	item->fr = sysCfg.actId;
//#endif
	
	item->to = 0;
	item->srcClientId = 0;
	item->topic = topic;
	item->id = ++msgId;
	item->type = type;

	item->data = content;
	jm_cli_setPSItemDataType(FLAG_DATA_STRING, &item->flag);

	JM_CLI_DEBUG("cli F2: %s, %u\n",item->topic,item->id);
	jm_cli_send_msg_result_t rst = jm_cli_publishPubsubItem(item, ps);

	jm_emap_release(item->cxt);

	//cache_back(CACHE_PUBSUB_ITEM,item,MODEL_JCLIENT);
	_c_releasePusubItem(item);
	return rst;
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_publishStrBufItem(char *topic, sint8_t type, jm_buf_t *buf,
		jm_emap_t *extra){
	if(topic == NULL || jm_strlen(topic) == 0) {
		JM_CLI_ERROR("cli topic is N\n");
		return INVALID_PS_DATA;
	}

	/*jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true,MODEL_JCLIENT);
	if(item == NULL) {
		JM_CLI_DEBUG("jm_cli_publishStrItem create PS item fail\n");
		return MEMORY_OUTOF_RANGE;
	}*/

	jm_pubsub_item_t *item = _c_createPubsubItem();

	item->flag = 0;
	item->cxt = NULL;
	//item->data = buf;
	item->delay = 0;
	
//#ifndef JM_STM32
	item->fr = sysCfg.actId;
//#endif
	
	item->to = 0;
	item->srcClientId = 0;
	item->topic = topic;
	item->id = ++msgId;
	item->type = type;
	item->dt = PREFIX_TYPE_BYTEBUFFER;

	item->data = buf;
	jm_cli_setPSItemDataType(FLAG_DATA_STRING, &item->flag);

	JM_CLI_DEBUG("cli item: %s, %u\n", item->topic, item->id);
	jm_cli_send_msg_result_t rst = jm_cli_publishPubsubItem(item, extra);

	jm_emap_release(item->cxt);

	//cache_back(CACHE_PUBSUB_ITEM,item,MODEL_JCLIENT);
	_c_releasePusubItem(item);
	return rst;
}

ICACHE_FLASH_ATTR jm_buf_t *_c_serialPsItem(jm_pubsub_item_t *it){

	jm_buf_t *buf = jm_buf_create(MEM_REALLOC_BLOCK_SIZE);
	if(buf == NULL) {
		JM_CLI_ERROR("cli mom");
		return NULL;
	}

	if(it->id != 0) it->dataFlag=_psitem_setDataFlag(0,it->dataFlag);
	if(it->type != 0) it->dataFlag=_psitem_setDataFlag(1,it->dataFlag);
	if(it->topic != NULL) it->dataFlag=_psitem_setDataFlag(2,it->dataFlag);
	if(it->srcClientId != 0) it->dataFlag=_psitem_setDataFlag(3,it->dataFlag);
	if(it->to != 0) it->dataFlag=_psitem_setDataFlag(4,it->dataFlag);
	if(it->callback != NULL) it->dataFlag=_psitem_setDataFlag(5,it->dataFlag);
	//if(it->delay != 0) it->dataFlag=_psitem_setDataFlag(6,it->dataFlag);
	if(it->cxt != NULL) it->dataFlag=_psitem_setDataFlag(6,it->dataFlag);
	if(it->data != NULL) it->dataFlag=_psitem_setDataFlag(7,it->dataFlag);

	jm_buf_put_u8(buf,it->dataFlag);
	jm_buf_put_u8(buf,it->flag);
	jm_buf_put_s32(buf,it->fr);

	if(it->id != 0) {
		if(!jm_buf_put_s64(buf,it->id)) {
			JM_CLI_ERROR("cli err1 %s",it->id);
			jm_buf_release(buf);
			return NULL;
		}
	}

	if(it->type != 0) {
		if(!jm_buf_put_u8(buf,it->type)) {
			JM_CLI_ERROR("_cspi err2 %s",it->type);
			jm_buf_release(buf);
			return NULL;
		}
	}

	if(it->topic != NULL) {
		if(!jm_buf_writeString(buf,it->topic,jm_strlen(it->topic))) {
			JM_CLI_ERROR("cli err3 %s",it->topic);
			jm_buf_release(buf);
			return NULL;
		}
	}

	if(it->srcClientId != 0) {
		if(!jm_buf_put_s32(buf,it->srcClientId)) {
			JM_CLI_ERROR("cli err4 %s",it->srcClientId);
			jm_buf_release(buf);
			return NULL;
		}
	}

	if(it->to != 0) {
		if(!jm_buf_put_s32(buf,it->to)) {
			JM_CLI_ERROR("cli err5 %s",it->to);
			jm_buf_release(buf);
			return NULL;
		}
	}

	if(it->callback != NULL) {
		if(!jm_buf_writeString(buf,it->callback,jm_strlen(it->callback))) {
			JM_CLI_ERROR("cli err6 %s",it->callback);
			jm_buf_release(buf);
			return NULL;
		}
	}

	/*if(it->delay != 0) {
		if(!jm_buf_put_u8(buf,it->delay)) {
			JM_CLI_ERROR("_client_serialItem write delay jmerror %s",it->delay);
			return NULL;
		}
	}*/

	if(it->cxt != NULL) {
		/*uint16_t len;
		if(!jm_extra_encode(it->cxt, buf, &len, EXTRA_KEY_TYPE_STRING)){
			JM_CLI_ERROR("_client_serialItem write cxt jmerror");
			return NULL;
		}*/
        if(!jm_cli_encodeExtra(buf,it->cxt->_hd, PREFIX_TYPE_MAP)){
            JM_CLI_ERROR("cli err7");
			jm_buf_release(buf);
            return NULL;
        }
	}

	if(it->data != NULL) {
		uint8_t dt = _psitem_getDataType(it->flag);
		if(FLAG_DATA_BIN == dt) {
			jm_buf_t *bb = (jm_buf_t*)it->data;
			uint16_t len = jm_buf_readable_len(bb);
			jm_buf_put_u16(buf,len);
			if(len > 0) {
				if(!jm_buf_put_buf(buf,bb)) {
					JM_CLI_ERROR("cli err8 %d",jm_buf_readable_len(bb));
					jm_buf_release(buf);
					return NULL;
				}
			}
		}else if(FLAG_DATA_STRING == dt){
			if(it->dt == PREFIX_TYPE_BYTEBUFFER) {
				//JM_CLI_DEBUG("_client_serialItem write buffer string: %d",jm_buf_readable_len(buf));
                if(!jm_buf_writeStringLen(buf, jm_buf_readable_len(it->data))) {
                    JM_CLI_ERROR("cli err9");
					jm_buf_release(buf);
                    return NULL;
                }

				if(!jm_buf_put_buf(buf, it->data)) {
					JM_CLI_ERROR("cli err10");
					jm_buf_release(buf);
					return NULL;
				}
			} else {
				JM_CLI_DEBUG("cli data: %d",jm_strlen(it->data));
				char *bb = (char*)it->data;
				if(!jm_buf_writeString(buf, bb, jm_strlen(bb))) {
					JM_CLI_ERROR("_cspi err12 %s",bb);
					jm_buf_release(buf);
					return NULL;
				}
			}
		}else if(FLAG_DATA_JSON== dt){
			char *bb = (char*)it->data;
			if(!jm_buf_writeString(buf,bb,jm_strlen(bb))) {
				JM_CLI_ERROR("cli JSON error %s",bb);
				jm_buf_release(buf);
				return NULL;
			}
		} else if(FLAG_DATA_EXTRA == dt){
			/*uint16_t wl;
			if(!jm_extra_encode(it->data, buf, &wl, EXTRA_KEY_TYPE_STRING)){
				JM_CLI_ERROR("_client_serialItem write extra data error %d",wl);
				return NULL;
			}*/

            jm_msg_extra_data_t *ex = it->data;
            if(PREFIX_TYPE_MAP == it->dt || PREFIX_TYPE_PROXY == it->dt ) {
                jm_emap_t *m = (jm_emap_t*)it->data;
                ex = m->_hd;
            } else if(PREFIX_TYPE_LIST == it->dt || PREFIX_TYPE_SET == it->dt ) {
                jm_elist_t *m = (jm_elist_t*)it->data;
                ex = m->_hd;
            }

            //jm_msg_extra_data_t *ex = jm_cli_getExtraByType(it->data->_hd,it->dt);
            if(!jm_cli_encodeExtra(buf,ex, it->dt)){
                JM_CLI_ERROR("cli cxt error");
				jm_buf_release(buf);
                return NULL;
            }
		} else {
			JM_CLI_ERROR("cli not support %d",dt);
			jm_buf_release(buf);
			return NULL;
		}
	}

	return buf;
}

/*
ICACHE_FLASH_ATTR static jm_buf_t* _c_psItem2Json(jm_pubsub_item_t *item) {
	cJSON *json = cJSON_CreateObject();

	//cJSON *ji = cJSON_CreateNumber(item->flag);
	cJSON_AddNumberToObject(json,"flag", item->flag);

	//ji = cJSON_CreateNumber(item->id);
	cJSON_AddNumberToObject(json,"id", item->id);

	//ji = cJSON_CreateNumber(item->srcClientId);
	cJSON_AddNumberToObject(json,"srcClientId", item->srcClientId);

	//ji = cJSON_CreateNumber(item->fr);
	cJSON_AddNumberToObject(json,"fr", item->fr);

	//ji = cJSON_CreateNumber(item->to);
	cJSON_AddNumberToObject(json,"to", item->to);

	//ji = cJSON_CreateNumber(item->delay);
	cJSON_AddNumberToObject(json,"delay", item->delay);

	cJSON *ji = cJSON_CreateString(item->topic);
	cJSON_AddItemToObject(json,"topic", ji);

	if(item->data) {
		ji = cJSON_CreateRaw(item->data);
		cJSON_AddItemToObject(json,"data", ji);
		//cJSON_AddItemReferenceToArray(ji,json);
	}

	if(item->cxt) {
		jm_msg_extra_data_t *ic = item->cxt;
		while(ic) {
			ic = ic->next;
		}
	}

	char *itemData = cJSON_PrintUnformatted(json);
	int len = jm_strlen(itemData);

	JM_CLI_DEBUG("%s",itemData);

	jm_buf_t *buf = jm_buf_create(len);
	jm_buf_put_chars(buf,itemData,len);

	cJSON_Delete(json);
	jmm.jm_free_fn(itemData);

	return buf;
}
*/

ICACHE_FLASH_ATTR void jm_cli_initPubsubItem(jm_pubsub_item_t *item,uint8_t dataType){
	item->flag = 0;
	item->cxt = NULL;
	item->delay = 0;
	item->to = 0;
	item->topic = NULL;
	item->type = 0;
		
//#ifndef JM_STM32
	item->fr = sysCfg.actId;
//#endif

	if(item->id <= 0) {
		item->id = ++msgId;
	}

	item->srcClientId = clientId;
	jm_cli_setPSItemDataType(dataType, &item->flag);
}

ICACHE_FLASH_ATTR jm_emap_t * jm_cli_topicForwardExtra(char *topic) {
	jm_emap_t *msgExtra = jm_emap_create(PREFIX_TYPE_BYTE);
    jm_emap_putByte(msgExtra, (void*)EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_FORWARD_BY_TOPIC,false);
    jm_emap_putStr(msgExtra, (void*)EXTRA_KEY_PS_ARGS, topic, false,false);
    jm_emap_putByte(msgExtra, (void*)EXTRA_KEY_CHANNEL_NO, (int8_t)jm_hashmap_get(name2ChannelNo,JM_UDP_SENDER_NAME),false);
    jm_emap_putStr(msgExtra, (void*)EXTRA_KEY_UDP_HOST, (char*)sysCfg.jmHost, false,false);
    jm_emap_putInt(msgExtra, (void*)EXTRA_KEY_UDP_PORT, sysCfg.jmPort,false);
	return msgExtra;
}

ICACHE_FLASH_ATTR sint32_t jm_cli_publishEmap2Device(jm_emap_t *ps, sint8_t type, char *host, uint16_t port){

   /* jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true,MODEL_JCLIENT);
    if(item == NULL) {
        JM_CLI_ERROR("jm_cli_publishEmap2Device create PS item fail\n");
        return MSG_CREATE_FAIL;
    }*/

	jm_pubsub_item_t *item = _c_createPubsubItem();

    jm_emap_t* extra = jm_cli_getUdpExtraByHost(host, port, true);

    item->flag = 0;

    jm_cli_initPubsubItem(item, FLAG_DATA_EXTRA);

    item->cxt = NULL;
    //item->data = buf;
    item->delay = 0;
    item->fr = sysCfg.actId;
    item->to = 0;
    //item->srcClientId = 0;
    item->topic = TOPIC_P2P;
    item->type = type;
    item->dt = PREFIX_TYPE_MAP;
    item->data = ps;

    sint32_t  msgId = item->id;

    jm_cli_send_msg_result_t rst = jm_cli_publishPubsubItem(item, extra);

    if(rst != JM_SUCCESS) {
        JM_CLI_ERROR("cli F host: %s\n",host);
    }

    jm_emap_release(extra);

    item->topic = NULL;
    item->data = NULL;
    item->cxt = NULL;
    item->callback = NULL;

    //cache_back(CACHE_PUBSUB_ITEM, item,MODEL_JCLIENT);
    _c_releasePusubItem(item);
    return msgId;
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_publishPubsubItem(jm_pubsub_item_t *item, jm_emap_t *ps){
    //jm_msg_extra_data_t *extra
	if(item == NULL || item->topic == NULL || jm_strlen(item->topic) == 0) {
		JM_CLI_ERROR("cli topic N: %s, %d\n",item->topic, item->id);
		return INVALID_PS_DATA;
	}

    jm_buf_t *buf = _c_serialPsItem(item);
    if(buf == NULL) {
		JM_CLI_ERROR("cli Serial E %s, %d",item->topic, item->id);
		return INVALID_PS_DATA;
	}

	jm_msg_t* msg = jm_msg_create_ps_msg(buf);
	if(msg == NULL) {
		JM_CLI_ERROR("cli msg is N : %s, %u\n", item->topic, item->id);
        jm_buf_release(buf);
		return MEMORY_OUTOF_RANGE;
	}

	jm_msg_setDownProtocol(msg, PROTOCOL_BIN);
	jm_msg_setUpProtocol(msg, PROTOCOL_RAW);

	//msg->extraMap = jm_extra_pullAll(ps->_hd, msg->extraMap);

    char *host = jm_emap_getStr(ps, (void*)EXTRA_KEY_UDP_HOST);
    uint16_t port = jm_emap_getInt(ps, (void*)EXTRA_KEY_UDP_PORT);
    BOOL udp = jm_emap_getBool(ps, (void*)EXTRA_KEY_UDP_ACK);
    sint8_t channelNo = jm_emap_getByte(ps, (void*)EXTRA_KEY_CHANNEL_NO);

    if(host==NULL || (jm_strcasecmp(host, (char*)jm_cli_getJmInfo()->jmHost) == 0) && (port == jm_cli_getJmInfo()->jmPort)) {
    	//发往云端消息
    	if(loginKey) {
			jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_LOGIN_KEY, loginKey, false,false);
		}
    	channelNo = _c_getChanelNo();

    	if(!jm_emap_exist(ps,(void*)EXTRA_KEY_PS_OP_CODE)) {
			jm_emap_putByte(ps, (void*)EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_FORWARD_BY_TOPIC, false);
			//JM_CLI_DEBUG("ps opcode=%d",MSG_OP_CODE_FORWARD_BY_TOPIC);
		}

		jm_emap_putStr(ps, (void*)EXTRA_KEY_PS_ARGS, jm_utils_copyStr(item->topic), true, false);
    }

    /*
    if(host) jm_emap_putStr(msg->extraMap,EXTRA_KEY_UDP_HOST,host,false);
    if(port) jm_emap_putInt(msg->extraMap,EXTRA_KEY_UDP_PORT,port);
    if(channelNo) jm_emap_putInt(msg->extraMap,EXTRA_KEY_CHANNEL_NO,channelNo);
     */

    jm_emap_putAll(msg->extraMap,ps);

    jm_msg_setUdp(msg, udp);

    if(host==NULL) jm_emap_putStr(msg->extraMap,(void*)EXTRA_KEY_UDP_HOST,(char*)sysCfg.jmHost,false,false);
    if(port <= 0) jm_emap_putInt(msg->extraMap,(void*)EXTRA_KEY_UDP_PORT,sysCfg.jmPort,false);
    if(channelNo <= 0) jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_CHANNEL_NO,_c_getChanelNo(),false);

	jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_GROUP_ID, grpId,false);
	jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_CLIENT_ID, clientId,false);

	JM_CLI_DEBUG("jm_cli_publishPubsubItem Begin send msg\n");

	jm_cli_send_msg_result_t sendRst = jm_cli_sendMessage(msg);
	//JM_CLI_DEBUG("jm_cli_publishPubsubItem End send result: \n",sendRst);

	//jm_extra_release(msg->extraMap);
	jm_msg_release(msg);

	return sendRst;
}


ICACHE_FLASH_ATTR static  char* _c_getTopic() {
	if(!_c_isValidLoginInfo()) {
		return NULL;
	}

	char actIdStr[32];
	 jm_itoa(sysCfg.actId, actIdStr);
	 uint16_t len = jm_strlen(TOPIC_PREFIX) + jm_strlen((char*)sysCfg.deviceId) + jm_strlen(actIdStr) + 1;
	JM_CLI_DEBUG("cli pre:%s, actId:%ld, dev:%s \n",TOPIC_PREFIX, sysCfg.actId, sysCfg.deviceId);
	
	
    //JM_CLI_DEBUG("Alloc _c_getTopic %u\n",len);
	char *topic = jm_utils_mallocStr(len,"_c_getTopic");

	
	
	jm_cli_getJmm()->jm_memset(topic,0,len);
	jm_strcpy(topic, TOPIC_PREFIX);
	
	//os_memset(topic,0,len);
	//os_strncpy(topic, TOPIC_PREFIX, jm_strlen(TOPIC_PREFIX));
//#ifndef JM_STM32
	jm_strcat(topic, actIdStr);
//#endif
	
	jm_strcat(topic, "/");
	jm_strcat(topic, (char*)sysCfg.deviceId);

	//JM_CLI_DEBUG("_c_getTopic topic:%s \n",topic);

	return topic;
}

ICACHE_FLASH_ATTR static void _c_subTopicAfterjmLogin(jm_event_t *evt) {
	//sint32_t code, char *msg, char *loginKey, sint32_t actId
	JM_CLI_DEBUG("cli sub topic B loginCode=%ld\n",loginCode);

	if(loginCode == LSUCCESS) {
		ps_listener_map *pi = ps_listener;
		while(pi) {
			if(!pi->rm) {pi = pi->next; continue;} //p2p主题不能经服务器转发
			if(jm_strcmp(pi->topic,TOPIC_P2P) == 0) {
				pi = pi->next;
				continue;
			}
			JM_CLI_DEBUG("cli resub topic: %s\n",pi->topic);
			_c_doSubscribeTopic(pi);
			pi = pi->next;
		}
	}
}

#endif // JM_PS_ENABLE == 1
