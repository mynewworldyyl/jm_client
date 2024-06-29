
ICACHE_FLASH_ATTR BOOL jm_cli_isLogin(){
	return loginCode == LSUCCESS;
}

static ICACHE_FLASH_ATTR void _c_setMsgUdp(jm_msg_t *msg){
#if JM_TCP==1
	jm_msg_setUdp(msg,false);
#else
	jm_msg_setUdp(msg,true);
#endif
}

ICACHE_FLASH_ATTR static  jm_cli_msg_result_t* _c_GetRpcWaitForResponse(sint32_t msgId){
	return jm_hashmap_get(wait_for_resps, (void*)msgId);
}

ICACHE_FLASH_ATTR static  jm_cli_msg_result_t* _c_createRpcWaitForResponse(uint32_t msgId){

    JM_CLI_DEBUG("cli cresp %u\n", sizeof(struct _c_msg_result));
    jm_cli_msg_result_t *m = jm_utils_mallocWithError(sizeof(struct _c_msg_result),
		PREFIX_TYPE_CLIENT_MSG_RESULT, "_ccrwfr");

	m->callback = NULL;
	m->msg_id = msgId;
	m->cbArg = NULL;

    jm_hashmap_put(wait_for_resps, (void*)msgId, m);

	return m;
}

/**
 */
ICACHE_FLASH_ATTR static  void _c_rebackRpcWaitRorResponse(jm_cli_msg_result_t *m){
    jm_cli_getJmm()->jm_free_fn(m, sizeof(jm_cli_msg_result_t));
}

ICACHE_FLASH_ATTR static BOOL _c_checkRpcTimeout(){

    if(wait_for_resps == NULL || wait_for_resps->size == 0) {
        return true;
    }

    //JM_CLI_DEBUG("cli chk to s=%d\n",wait_for_resps->size);

    jm_hash_map_iterator_t ite = { wait_for_resps, NULL, -1,wait_for_resps->ver };

    //BOOL to = false;
    void  *key = 0;
    uint32_t  cur = jm_cli_getSysTime();
    while((key = jm_hashmap_iteNext(&ite)) > 0) {
        jm_cli_msg_result_t *waits = jm_hashmap_get(wait_for_resps, key);
        if(waits == NULL) continue;

        if((cur - waits->startTime) > rpcTimeout) {
            if(waits->startTime) {
                JM_CLI_DEBUG("cli to msgId: %d, %uMS, to: %u\n", waits->msg_id,
                     (cur - waits->startTime), rpcTimeout);
                //void *resultMap, sint32_t code, char *errMsg, void *arg
                //to = true;
                waits->callback(NULL, 1, "timeout", waits->cbArg);//RPC超时
            }

            jm_hashmap_iteRemove(&ite);
            _c_rebackRpcWaitRorResponse(waits);
        }
        //waits = waits->next;
    }

    /*if(to && reconnect) {
    	JM_CLI_DEBUG("cli reconnect\n");
    	reconnect();
    }*/

    //JM_CLI_DEBUG("_c_checkRpcTimeout finish\n");
    return true;
}

ICACHE_FLASH_ATTR static  uint8_t _c_loginResult(jm_emap_t *resultMap, sint32_t code, char *errMsg, void *arg){

	/*if(loginMsg) {
        jmm.jm_free_fn(loginMsg);
		loginMsg = NULL;
	}*/

	loginCode = 0;

	//jm_msg_extra_data_t *resultMap = jm_extra_decode(buf);
	if(code != 0) {
		loginCode = code;
		JM_CLI_ERROR("cli err c:%d, msg:%s!\n",code, errMsg);
		goto finish;
	}

	if(resultMap == NULL) {
		loginCode = HANDLE_MSG_FAIL;
		JM_CLI_DEBUG("cli err: %s!",errMsg);
		return loginCode;
	}

	//RespJRso.data
	jm_emap_t *respDataEx1 = jm_emap_getMap(resultMap,"data");
	if(respDataEx1 == NULL) {
		JM_CLI_ERROR("cli inv r!\n");
		return INVALID_RESP_DATA;
	}

	//Map<String,Object>
	char *lk = jm_emap_getStr(respDataEx1,"loginKey");
	if(lk) {
        JM_CLI_DEBUG("cli alm %s\n",lk);
        if(loginKey) jm_utils_releaseStr(loginKey, 0);
		loginKey = jm_utils_copyStr(lk);
	}

	int32_t cid = jm_emap_getInt(respDataEx1,"clientId");
	
	//sysCfg.actId =  jm_emap_getInt(respDataEx1,"actId");
	int32_t aid = jm_emap_getInt(respDataEx1,"actId");
	int8_t gid = jm_emap_getByte(respDataEx1,"grpId");
	if(gid != grpId || cid != clientId || aid != sysCfg.actId) {
		if(aid) sysCfg.actId = aid;
		sysCfg.clientId = cid;
		sysCfg.grpId = gid;
		jm_cli_getJmm()->jm_postEvent(TASK_APP_SAVE_CFG,0,NULL,0);//subType=0不重启
	}

	JM_CLI_DEBUG("cli c=%ld, msg=%s, lk=%s\n", loginCode, errMsg, loginKey);
	JM_CLI_DEBUG("cli actId=%ld, grpId=%d, clientId=%ld\n", aid, gid, cid);

finish:

	jm_cli_getJmm()->jm_postEvent(TASK_APP_LOGIN_RESULT,loginCode, NULL, 0);

	/*if(jm_cli_isLogin()) {
		_c_subTopicAfterjmLogin();
	}*/

	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_login(){

	 //JM_CLI_DEBUG("jm_cli_login check\n");

	 if(jm_cli_netStatus() != NetConnected) {
		JM_CLI_ERROR("cli conn disable\n");
		return SOCKET_SENDER_NULL;
	}

	if(jm_cli_isLogin()){
		 //JM_CLI_DEBUG("cli logined\n");
		 return JM_SUCCESS;//已经登录
	}

	uint64_t cur = jm_cli_getSysTime();

	if(lastLoginTime != 0 && (cur - lastLoginTime) < 60000) {
		//最多30秒后重新请求登录
		//JM_CLI_DEBUG("cli log too %d\n", (cur - lastLoginTime));
		return BUSSY;
	}

	lastLoginTime = cur;

	if(jm_strlen((char*)sysCfg.invokeCode) == 0) {
		JM_CLI_ERROR("cli ic NULL\n");
		return SEND_INVALID_ACCOUNT;
	}

	if(jm_strlen((char*)sysCfg.deviceId) == 0) {
		JM_CLI_ERROR("cli deviceId is N\n");
		return SEND_INVALID_DEVICE_ID;
	}

	JM_CLI_DEBUG("cli ic=%s, dev=%s\n", sysCfg.invokeCode, sysCfg.deviceId);

	jm_elist_t *rpcPsList = jm_elist_create();
	if(rpcPsList == NULL) {
		JM_CLI_ERROR("cli mmo %s\n");
		return MEMORY_OUTOF_RANGE;
	}

	jm_elist_add(rpcPsList, sysCfg.invokeCode, PREFIX_TYPE_STRINGG, false);
	jm_elist_add(rpcPsList, sysCfg.deviceId, PREFIX_TYPE_STRINGG,false);

	//login jm server
	sint64_t msgId = jm_cli_invokeRpc(-1239310325, rpcPsList, _c_loginResult, NULL);

	jm_elist_release(rpcPsList);

	if(msgId < 0) {
		JM_CLI_ERROR("cli call E %ld\n",msgId);
		return msgId;
	} else {
		return msgId;
	}

}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t _c_client_login(){
	return jm_cli_login();
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_logout(){
	loginCode = LOGOUT;
	loginKey = NULL;
	clientId = 0;
	inited = false;
	lastLoginTime = 0;
	return JM_SUCCESS;
}


ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_sendMessage(jm_msg_t *msg){

	//jm_msg_extra_data_t* eh = jm_extra_get(msg->extraMap, EXTRA_KEY_UDP_HOST);
	//uint32_t port = jm_extra_getS32(msg->extraMap, EXTRA_KEY_UDP_PORT);

	JM_CLI_DEBUG("jm_cli_sendMessage B\n");

    sint8_t channelNo = jm_emap_getByte(msg->extraMap, (void*)EXTRA_KEY_CHANNEL_NO);
    if(channelNo <= 0) {
        JM_CLI_ERROR("cli channelNo is N\n");
        return SOCKET_SENDER_NULL;
    }

    BOOL isUdp = jm_msg_isUdp(msg);

	jm_cli_p2p_msg_sender_fn sender = (jm_cli_p2p_msg_sender_fn)jm_hashmap_get(sendChannel, (void*)channelNo);
	if(sender == NULL) {
		JM_CLI_ERROR("cli s: %d is N\n",channelNo);
		return SOCKET_SENDER_NULL;
	}

	//sint8_t jmchannelno = jm_hashmap_get(name2ChannelNo, JM_UDP_SENDER_NAME);

    char *host = NULL;
    uint16_t port = 0;

    //BOOL udp = jm_emap_getStr(msg->extraMap,EXTRA_KEY_UDP_ACK);

    //JM_CLI_DEBUG("jm_cli_sendMessage 1\n");
    jm_emap_remove(msg->extraMap, (void*)EXTRA_KEY_CHANNEL_NO,true);

    //JM_CLI_DEBUG("jm_cli_sendMessage 2\n");
    if(isUdp) {
    	 host = jm_emap_getStr(msg->extraMap,(void*)EXTRA_KEY_UDP_HOST);
    	 port = jm_emap_getInt(msg->extraMap,(void*)EXTRA_KEY_UDP_PORT);
    	 jm_emap_remove(msg->extraMap,(void*)EXTRA_KEY_UDP_HOST,true);
    	 jm_emap_remove(msg->extraMap,(void*)EXTRA_KEY_UDP_PORT,true);
    	 jm_emap_remove(msg->extraMap,(void*)EXTRA_KEY_UDP_ACK,true);
    }

#if JM_STM32==1
	jm_buf_t *sendBuf = jm_buf_create(MEM_REALLOC_BLOCK_SIZE);
#else
	jm_buf_t *sendBuf = jm_buf_create(MEM_REALLOC_BLOCK_SIZE*4);
#endif
	
	//JM_CLI_DEBUG("jm_cli_sendMessage 3\n");
	if(sendBuf == NULL) {
		JM_CLI_ERROR("cli MO");
		return ENCODE_MSG_FAIL;
	}

	//JM_CLI_DEBUG("jm_cli_sendMessage 4\n");
	if(!jm_msg_encode(msg, sendBuf)) {
		jm_buf_release(sendBuf);
		JM_CLI_ERROR("cli 1F\n");
		return ENCODE_MSG_FAIL;
	}

	//JM_CLI_DEBUG("jm_cli_sendMessage 5\n");
	 if(isUdp) {
		  if(host) jm_emap_putStr(msg->extraMap,(void*)EXTRA_KEY_UDP_HOST,host,false,false);
		  if(port) jm_emap_putInt(msg->extraMap,(void*)EXTRA_KEY_UDP_PORT,port,false);
	}

	//JM_CLI_DEBUG("jm_cli_sendMessage 6\n");
    if(channelNo) jm_emap_putByte(msg->extraMap,(void*)EXTRA_KEY_CHANNEL_NO,channelNo,false);

	//JM_CLI_DEBUG("jm_cli_sendMessage udp host: %s, port: %u\n",host, port);
	jm_cli_send_msg_result_t rst = sender(sendBuf, msg->extraMap);

	//JM_CLI_DEBUG("jm_cli_sendMessage 7\n");
	jm_buf_release(sendBuf);
	
	return rst;

}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_onMessage(jm_msg_t *msg){

	lastActiveTime = jm_cli_getSysTime(); //更新与服务器交互的最后活动时间
	JM_CLI_DEBUG("cli onmsg t=%d msgId=%ld\n", msg->type, msg->msgId);
	CHRI *h = (CHRI *)_c_GetMsgHandler(msg->type);
	if(h == NULL) {
		JM_CLI_ERROR("cli1 msg type=%d\n",msg->type);
		return HANDLE_MSG_FAIL;
	}

	//JM_CLI_DEBUG("cli2:%d\n",msg->type);
	return h->handler(msg);
}

ICACHE_FLASH_ATTR BOOL jm_cli_registMessageHandler(jm_cli_msg_hander_fn hdl, sint8_t type){
	CHRI *h = _c_GetMsgHandler(type);
	if(h != NULL) {
		return false;
	}

    JM_CLI_DEBUG("registMessageHandler type=%d\n",type);
	h = (CHRI *)jm_utils_mallocWithError(sizeof(struct jm_msg_handler_register_item),PREFIX_TYPE_CHRI,
		"jm_cli_registMessageHandler");
	h->handler = hdl;
	h->type = type;
	h->next = NULL;

	if(handlers == NULL) {
		handlers = h;
	} else {
		h->next = handlers;
		handlers = h;
	}

	return true;

}

ICACHE_FLASH_ATTR sint64_t jm_cli_invokeRpcByChannelNo(sint32_t mcode, jm_elist_t *params,
		jm_cli_rpc_callback_fn callback, void *cbArgs, sint8_t channelNo){

    sint64_t msgId;

	jm_buf_t *paramBuf = NULL;
	if(params) {
		paramBuf = jm_buf_create(MEM_REALLOC_BLOCK_SIZE*1);
		JM_CLI_DEBUG("cli rpc encode B\n");
		if(!jm_cli_encodeExtra(paramBuf, params->_hd, PREFIX_TYPE_LIST)) {
			JM_CLI_ERROR("cli 1F\n");
			msgId = MEMORY_OUTOF_RANGE;
            goto end;
		}
		//JM_CLI_DEBUG("cli E\n");
	}

	jm_msg_t *msg = jm_msg_create_rpc_msg(mcode, paramBuf);
    msgId = msg->msgId;

	if(msg == NULL) {
		JM_CLI_ERROR("cli msg mmo");
		msgId = MEMORY_OUTOF_RANGE;
        goto end;
	}

    jm_cli_msg_result_t *wait = NULL;

    JM_CLI_DEBUG("cli H:%s:%u msgId=%ld mcode=%ld\n",sysCfg.jmHost, sysCfg.jmPort, msgId, mcode);

#if JM_TCP==1
    jm_msg_setUdp(msg,false);
#else
     jm_msg_setUdp(msg,true);
	 jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_UDP_HOST, sysCfg.jmHost, false,false);
	 jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_UDP_PORT, sysCfg.jmPort,false);
#endif

    jm_emap_putByte(msg->extraMap, (void*)EXTRA_KEY_CHANNEL_NO, channelNo,false);

	if(mcode != -1239310325 && loginKey) {
		jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_LOGIN_KEY, loginKey, false,false);
	}

    if(callback == NULL) {
        msgId = jm_cli_sendMessage(msg);
        if(msgId != JM_SUCCESS) {
            JM_CLI_ERROR("cli F \n");
			if(paramBuf) jm_buf_release(paramBuf);
			paramBuf=NULL;
            goto end;
        } else {
            msgId = msg->msgId;
        }
    } else {
        wait = _c_createRpcWaitForResponse(msg->msgId);
        if(wait == NULL) {
            JM_CLI_ERROR("cli mmo1\n");
            msgId = MEMORY_OUTOF_RANGE;
            //if(wait) _c_rebackRpcWaitRorResponse(wait);
            goto end;
        }

        wait->msg_id = msg->msgId;
        //wait->msg = msg;
        wait->callback = callback;
        //wait->in_used = true;
        wait->cbArg = cbArgs;
        wait->startTime = jm_cli_getSysTime();

        msgId = jm_cli_sendMessage(msg);

        if(msgId == JM_SUCCESS) {
            msgId = msg->msgId;
        } else {
			if(paramBuf) jm_buf_release(paramBuf);
			paramBuf=NULL;
		}
    }

	JM_CLI_DEBUG("cli msgId %ld\n", msgId);
    end:
        if(paramBuf) jm_buf_release(paramBuf);
        if(msg) {
            msg->payload = NULL;
            jm_msg_release(msg);
        }
    return msgId;
}

ICACHE_FLASH_ATTR sint64_t jm_cli_invokeRpc(sint32_t mcode, jm_elist_t *params,
		jm_cli_rpc_callback_fn callback, void *cbArgs){
	return jm_cli_invokeRpcByChannelNo(mcode, params, callback, cbArgs,  _c_getChanelNo());
}

ICACHE_FLASH_ATTR static  CHRI* _c_GetMsgHandler(sint8_t type){
	CHRI *h;
	if(handlers != NULL) {
		h = handlers;
		while(h != NULL) {
			if(h->type == type) {
				return h;
			}else {
				h = h->next;
			}
		}
	}
	return NULL;
}

ICACHE_FLASH_ATTR static  void* _c_parseRpcPayload(jm_msg_t *msg, sint32_t *code, char **errMsg, sint8_t *t) {

	JM_CLI_DEBUG("_cprp: %d\n", *code);

	if(msg->type != MSG_TYPE_RRESP_JRPC) {
		*code = 1;
        JM_CLI_DEBUG("_cprp1 %u\n",25);
		*errMsg = "Not MSG_TYPE_RRESP_JRPC msg";
		JM_CLI_ERROR(*errMsg);
		return NULL;
	}

	//JM_CLI_DEBUG("_c_parseRpcPayload 1\n");

	sint8_t pro = jm_msg_getDownProtocol(msg);

	//PROTOCOL_BIN和JSON数据由接收者处理
	//if(pro == PROTOCOL_BIN || pro == PROTOCOL_JSON || pro == PROTOCOL_RAW) return msg->payload;
	if(pro != PROTOCOL_EXTRA) {
		JM_CLI_DEBUG("_cprp12 pro: %d\n",pro);
		return msg->payload;//平台只处理PROTOCOL_EXTRA解码，其他由接收者处理
	}

	void *resultMap = NULL;

	sint8_t type=0;
    *t = 0;

    if(msg->payload != NULL) {
		if(!jm_buf_get_s8(msg->payload,&type)) {
			JM_CLI_ERROR("_cprp3 E!");
			return NULL;
		}
		resultMap = jm_cli_decodeExtra(msg->payload, type);
        *t = type;
	} else {
		JM_CLI_DEBUG("_cprp1 N p!");
	}

	if(jm_msg_isError(msg)) {

		if(resultMap == NULL) {
			JM_CLI_ERROR("_cprp3 N r:%d",msg->msgId);
			return NULL;
		}

		if(type == PREFIX_TYPE_PROXY || type == PREFIX_TYPE_MAP) {
			jm_emap_t *m = (jm_emap_t*)resultMap;
			sint32_t rcode = jm_emap_getInt(m,"code");
			char *rmsg = jm_emap_getStr(m,"msg");
			JM_CLI_DEBUG("_cprp4 c=%d, m=%s\n",rcode,rmsg);
			*code = rcode;
			if(rmsg) {
				*errMsg = jm_utils_copyStr(rmsg);
			}
		}

		return resultMap;
	}

	*code = 0;
	*errMsg = NULL;

	JM_CLI_DEBUG("_cprp5 E: %d\n", *code);
	return resultMap;

}

ICACHE_FLASH_ATTR static  jm_cli_send_msg_result_t _c_rpcMsgHandle(jm_msg_t *msg){
	JM_CLI_DEBUG("cli rpc rst: %ld \n",msg->msgId);
	jm_cli_msg_result_t * wait = _c_GetRpcWaitForResponse(msg->msgId);
	if(wait == NULL) {
		JM_CLI_ERROR("rpc F:%ld \n",msg->msgId);
		//jm_msg_release(msg);
		return MSG_WAIT_NOT_FOUND;
	}

	JM_CLI_DEBUG("rpc mid: %ld\n",msg->msgId);

	sint32_t code = 0;
	char *p = NULL;

    sint8_t type;
	void *rst = _c_parseRpcPayload(msg, &code, &p, &type);
	if(code != 0) {
		JM_CLI_ERROR("rpc c:%d, err: %s\n", code, p);
	}

    if(code == MT_INVALID_LOGIN_INFO) {
        JM_CLI_DEBUG("rpc in lk\n");
        jm_cli_logout();
        jm_cli_login();
    }

	wait->callback(rst, code, p, wait->cbArg);

	if(rst!= NULL && jm_msg_getDownProtocol(msg) == PROTOCOL_EXTRA) {
        if(type == PREFIX_TYPE_MAP || type == PREFIX_TYPE_PROXY) {
            jm_emap_release(rst);
        }else if(type == PREFIX_TYPE_LIST || type == PREFIX_TYPE_SET) {
            jm_elist_release(rst);
        } else {
            jm_extra_release(rst);
        }
	}else if(rst){
		jmm.jm_free_fn(rst,0);
	}

	if(p) {
        jm_utils_releaseStr(p,0);
	}

	//jm_msg_release(msg);
	//JM_CLI_DEBUG("cli rpc: %d\n",msg->msgId);

    //jm_hashmap_remove(wait_for_resps, msg->msgId);
    wait->startTime = 0;

	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR static void _c_commonRpcResult(jm_emap_t *r, sint32_t code, char *errMsg, void *arg){

	JM_CLI_DEBUG("crst cbcode:%u\n",arg);

	if(code != 0) {
		JM_CLI_ERROR("crst code=%d, msg=%s\n", code, errMsg?errMsg:"NULL");
		goto end;
	}

	uint8_t cbcode =(uint8_t)arg;

	switch(cbcode) {
#if JM_STD_TIME_ENABLE==1
		case 1:
		{
			uint32_t val = jm_emap_getInt(r,"data");
			JM_CLI_DEBUG("std t:%u\n", val);
			if(val > 0) {
				stdTime = val;
				sysStartTime = _client_getSystemTime();//记录系统启动时间,单位毫秒
			}
			break;
		}
#endif //JM_STD_TIME_ENABLE
		default:
			JM_CLI_ERROR("crst err code: %d\n",cbcode);
			break;
	}

end:
	jm_emap_release(r);
	JM_CLI_DEBUG("crst end cbcode:%d\n",arg);

}
