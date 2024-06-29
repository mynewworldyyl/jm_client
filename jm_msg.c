/*
 * jm_msg.h
 *
 *  Created on: 2023锟斤拷4锟斤拷10锟斤拷
 *      Author: yeyulei
 */

#include "jm_cons.h"
#include "jm_msg.h"
#include "jm_mem.h"
#include "jm_constants.h"
#include "debug.h"

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

static uint32_t msgId = 0;

extern jm_mem_op *jmm;

/****************************************Extra Map Method Begin*********************************************/
#if JM_EMAP_ENABLE==1
#include "jm_msg_emap.h"
#endif
/****************************************Extra Map Method End*********************************************/


/****************************************Extra List Method Begin*********************************************/
#if JM_ELIST_ENABLE==1
#include "jm_msg_elist.h"
#endif
/****************************************Extra List Method END*********************************************/

/********************EXTRA DATA OPERATION BEGIN**********************/

#if JM_ELIST_ENABLE==1 ||  JM_EMAP_ENABLE==1 || JM_MSG_ENABLE==1
#include "jm_msg_extra.h"
#endif

/**************************************EXTRA DATA OPERATION END**********************************/


#if JM_MSG_ENABLE==1
ICACHE_FLASH_ATTR void jm_msg_release(jm_msg_t *msg) {
	if(msg == NULL) return;

	if(msg->payload) jm_buf_release(msg->payload);

	if(msg->extraMap) jm_emap_release(msg->extraMap);

	//cache_back(CACHE_MESSAGE, msg, MODEL_JMSG);
	jmm->jm_free_fn(msg, sizeof(jm_msg_t));
}

ICACHE_FLASH_ATTR jm_msg_t* jm_msg_create() {
	return jm_utils_mallocWithError(sizeof(jm_msg_t),PREFIX_TYPE_MESSAGE,"jm_msg_create");
	//return cache_get(CACHE_MESSAGE,true,MODEL_JMSG);
}


ICACHE_FLASH_ATTR jm_msg_t* jm_msg_create_msg(sint8_t type, jm_buf_t *payload) {

	//, sint8_t up, sint8_t dp
	jm_msg_t *msg = jm_msg_create();
	if(!msg) {
		JM_MSG_ERROR("msg mo\n");
		return NULL;
	}

	msg->extraMap = jm_emap_create(PREFIX_TYPE_BYTE);
	msg->extrFlag = 0;
	msg->flag = 0;
	msg->payload = payload;
	//msg->startTime = 0;
	msg->type = type;
	msg->msgId = ++msgId;

	/*
	ApiRequestJRso req = new ApiRequestJRso();
	req.setReqId(reqId);
	req.setArgs(args);

	if(loginKey != null) {
		req.getParams().put(Constants.LOGIN_KEY, loginKey);
	}
	*/

	//Message msg = new Message();
	//jm_msg_setType(msg, MSG_TYPE_REQ_JRPC);

	//jm_msg_setMsgId(msg, req.getReqId());

	jm_msg_setUpProtocol(msg, PROTOCOL_EXTRA);
	jm_msg_setDownProtocol(msg, PROTOCOL_EXTRA);

	//jm_msg_setRpcMk(msg, true);
	//jm_msg_setSmKeyCode(msg, mcode);
	//jm_msg_putIntExtra(msg, EXTRA_KEY_SM_CODE, mcode);

	jm_msg_setDumpDownStream(msg, false);
	jm_msg_setDumpUpStream(msg, false);
	jm_msg_setRespType(msg, MSG_TYPE_PINGPONG);

	jm_msg_setOuterMessage(msg, true);
	jm_msg_setMonitorable(msg, false);
	jm_msg_setDebugMode(msg, false);

	//
	jm_msg_setUpSsl(msg, false);
	//jm_msg_setInsId(msg, 0);
	//jm_extra_put(msg->extraMap, EXTRA_KEY_INSID, 0, PREFIX_TYPE_INT);

	jm_msg_setForce2Json(msg, false);//响应消息不转JSON
	jm_msg_setFromApiGateway(msg, true);

	jm_msg_setDev(msg,true);//设备消息

	return msg;
}

ICACHE_FLASH_ATTR jm_msg_t* jm_msg_create_rpc_msg(sint32_t mcode, jm_buf_t *payload) {
	jm_msg_t *msg = jm_msg_create_msg(MSG_TYPE_REQ_JRPC,payload);
	if(!msg) {
		return NULL;
	}
	jm_msg_setRpcMk(msg, true);
	//jm_msg_setSmKeyCode(msg, mcode);
	jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_SM_CODE, mcode,false);
	
	return msg;
}

ICACHE_FLASH_ATTR jm_msg_t* jm_msg_create_ps_msg(jm_buf_t *payload) {
	return jm_msg_create_msg(MSG_TYPE_PUBSUB, payload);
}


ICACHE_FLASH_ATTR static BOOL jm_msg_is_s16(sint16_t flag, sint16_t mask) {
	return (flag & mask) != 0;
}

ICACHE_FLASH_ATTR static BOOL jm_msg_is_s32(sint32_t flag, sint32_t mask) {
	return (flag & mask) != 0;
}

ICACHE_FLASH_ATTR static sint32_t jm_msg_set_s32(BOOL isTrue,sint32_t f,sint32_t mask) {
	return isTrue ?(f |= mask) : (f &= ~mask);
}

ICACHE_FLASH_ATTR static uint16_t jm_msg_set_s16(BOOL isTrue, sint16_t f,sint16_t mask) {
	return isTrue ?(f |= mask) : (f &= ~mask);
}

//
ICACHE_FLASH_ATTR static BOOL jm_msg_isWriteExtra(jm_msg_t *msg) {
	return msg->extraMap != NULL;
}

//
ICACHE_FLASH_ATTR static BOOL jm_msg_isReadExtra(jm_msg_t *msg) {
	return jm_msg_is_s16(msg->flag,FLAG_EXTRA);
}

ICACHE_FLASH_ATTR static void jm_msg_setExtraFlag(jm_msg_t *msg, BOOL f) {
	msg->flag = jm_msg_set_s16(f,msg->flag,FLAG_EXTRA);
}

ICACHE_FLASH_ATTR BOOL jm_msg_isUpSsl(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_UP_SSL);
}

ICACHE_FLASH_ATTR void jm_msg_setUpSsl(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f,msg->extrFlag, EXTRA_FLAG_UP_SSL);
}

ICACHE_FLASH_ATTR BOOL jm_msg_isUdp(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_UDP);
}

ICACHE_FLASH_ATTR void jm_msg_setUdp(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f,msg->extrFlag, EXTRA_FLAG_UDP);
}

ICACHE_FLASH_ATTR BOOL jm_msg_isDownSsl(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_DOWN_SSL);
}

ICACHE_FLASH_ATTR  void jm_msg_setDownSsl(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f,msg->extrFlag,EXTRA_FLAG_DOWN_SSL);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isFromApiGateway(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_FROM_APIGATEWAY);
}

ICACHE_FLASH_ATTR  void jm_msg_setFromApiGateway(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f,msg->extrFlag, EXTRA_FLAG_FROM_APIGATEWAY);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isRsaEnc(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_ENC_TYPE);
}

ICACHE_FLASH_ATTR  void jm_msg_setEncType(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f, msg->extrFlag,EXTRA_FLAG_ENC_TYPE);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isSecretVersion(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_SECTET_VERSION);
}

ICACHE_FLASH_ATTR  void jm_msg_setSecretVersion(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_SECTET_VERSION);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isSign(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_IS_SIGN);
}

ICACHE_FLASH_ATTR  void jm_msg_setSign(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_IS_SIGN);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isSec(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_IS_SEC);
}

ICACHE_FLASH_ATTR  void jm_msg_setSec(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f,msg->extrFlag, EXTRA_FLAG_IS_SEC);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isRpcMk(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_RPC_MCODE);
}

ICACHE_FLASH_ATTR  void jm_msg_setRpcMk(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_RPC_MCODE);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isDumpUpStream(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag, EXTRA_FLAG_DUMP_UP);
}

ICACHE_FLASH_ATTR  void jm_msg_setDumpUpStream(jm_msg_t *msg, BOOL f) {
	//flag0 |= f ? FLAG0_DUMP_UP : 0 ;
	msg->extrFlag = jm_msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_DUMP_UP);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isDumpDownStream(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag,EXTRA_FLAG_DUMP_DOWN);
}

ICACHE_FLASH_ATTR  void jm_msg_setDumpDownStream(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_DUMP_DOWN);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isLoggable(jm_msg_t *msg) {
	return jm_msg_getLogLevel(msg) > 0 ? true : false;
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isDebugMode(jm_msg_t *msg) {
	return jm_msg_is_s32(msg->extrFlag,EXTRA_FLAG_DEBUG_MODE);
}

ICACHE_FLASH_ATTR  void jm_msg_setDebugMode(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = jm_msg_set_s32(f,msg->extrFlag,EXTRA_FLAG_DEBUG_MODE);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isMonitorable(jm_msg_t *msg) {
	return jm_msg_is_s16(msg->flag,FLAG_MONITORABLE);
}

ICACHE_FLASH_ATTR  void jm_msg_setMonitorable(jm_msg_t *msg, BOOL f) {
	msg->flag = jm_msg_set_s16(f,msg->flag,FLAG_MONITORABLE);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isError(jm_msg_t *msg) {
	return jm_msg_is_s16(msg->flag,FLAG_ERROR);
}

ICACHE_FLASH_ATTR  void jm_msg_setError(jm_msg_t *msg, BOOL f) {
	msg->flag = jm_msg_set_s16(f,msg->flag,FLAG_ERROR);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isOuterMessage(jm_msg_t *msg) {
	return jm_msg_is_s16(msg->flag,FLAG_OUT_MESSAGE);
}

ICACHE_FLASH_ATTR  void jm_msg_setOuterMessage(jm_msg_t *msg, BOOL f) {
	msg->flag = jm_msg_set_s16(f,msg->flag,FLAG_OUT_MESSAGE);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isForce2Json(jm_msg_t *msg) {
	return jm_msg_is_s16(msg->flag, FLAG_FORCE_RESP_JSON);
}

ICACHE_FLASH_ATTR  void jm_msg_setForce2Json(jm_msg_t *msg, BOOL f) {
	msg->flag = jm_msg_set_s16(f,msg->flag,FLAG_FORCE_RESP_JSON);
}

ICACHE_FLASH_ATTR  void jm_msg_setDev(jm_msg_t *msg, BOOL f) {
	msg->flag = jm_msg_set_s16(f,msg->flag,FLAG_DEV);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isDev(jm_msg_t *msg) {
	return jm_msg_is_s16(msg->flag, FLAG_DEV);
}


ICACHE_FLASH_ATTR  BOOL jm_msg_isNeedResponse(jm_msg_t *msg) {
	sint8_t rt = jm_msg_getRespType(msg);
	return rt != MSG_TYPE_NO_RESP;
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isPubsubMessage(jm_msg_t *msg) {
	sint8_t rt = jm_msg_getRespType(msg);
	return rt == MSG_TYPE_MANY_RESP;
}

ICACHE_FLASH_ATTR  BOOL jm_msg_isPingPong(jm_msg_t *msg) {
	sint8_t rt = jm_msg_getRespType(msg);
	return rt != MSG_TYPE_PINGPONG;
}

/**
 * @param f true 锟斤拷示锟斤拷锟斤拷锟斤拷false锟斤拷示锟斤拷锟斤拷锟斤拷
 */
ICACHE_FLASH_ATTR  void jm_msg_setLengthType(jm_msg_t *msg, BOOL f) {
	//flag |= f ? FLAG_LENGTH_INT : 0 ;
	msg->flag = jm_msg_set_s16(f, msg->flag, FLAG_LENGTH_INT);
}

ICACHE_FLASH_ATTR BOOL jm_msg_isLengthInt(jm_msg_t *msg) {
	return jm_msg_is_s16(msg->flag,FLAG_LENGTH_INT);
}

ICACHE_FLASH_ATTR  sint8_t jm_msg_getPriority(jm_msg_t *msg) {
	return (sint8_t)((msg->extrFlag >> EXTRA_FLAG_PRIORITY) & 0x03);
}

ICACHE_FLASH_ATTR  BOOL jm_msg_setPriority(jm_msg_t *msg, sint32_t l) {
	if(l > PRIORITY_3 || l < PRIORITY_0) {
		return false;
	}
	msg->extrFlag = (l << EXTRA_FLAG_PRIORITY) | msg->extrFlag;
	return true;
}

ICACHE_FLASH_ATTR  sint8_t jm_msg_getLogLevel(jm_msg_t *msg) {
	sint8_t v = (sint8_t)((msg->flag >> FLAG_LOG_LEVEL) & 0x07);
	return v;
}

//000 001 010 011 100 101 110 111
ICACHE_FLASH_ATTR  BOOL jm_msg_setLogLevel(jm_msg_t *msg, sint16_t v) {
	if(v < 0 || v > 6) {
		 return false;
	}
	msg->flag = (uint16_t)((v << FLAG_LOG_LEVEL) | (msg->flag & FLAG_LOG_LEVEL_MASK));
	return true;
}

ICACHE_FLASH_ATTR sint8_t jm_msg_getRespType(jm_msg_t *msg) {
	sint8_t v =  (sint8_t)((msg->flag >> FLAG_RESP_TYPE) & 0x03);
	return v;
}

ICACHE_FLASH_ATTR  BOOL jm_msg_setRespType(jm_msg_t *msg, sint16_t v) {
	if(v < 0 || v > 3) {
		 return false;
	}
	msg->flag = (v << FLAG_RESP_TYPE) | (msg->flag & FLAG_RESP_TYPE_MASK);
	return true;
}

ICACHE_FLASH_ATTR  sint8_t jm_msg_getUpProtocol(jm_msg_t *msg ) {
	//return jm_msg_is_s16(msg->flag, FLAG_UP_PROTOCOL);
	return (sint8_t)((msg->flag >> FLAG_UP_PROTOCOL) & 0x03);
}

ICACHE_FLASH_ATTR  void jm_msg_setUpProtocol(jm_msg_t *msg, sint16_t protocol) {
	//flag |= protocol == PROTOCOL_JSON ? FLAG_UP_PROTOCOL : 0 ;
	//msg->flag = jm_msg_set_s16(protocol == PROTOCOL_JSON, msg->flag, FLAG_UP_PROTOCOL);
	msg->flag = (msg->flag & FLAG_UP_PROTOCOL_MASK) | (protocol << FLAG_UP_PROTOCOL);
}

ICACHE_FLASH_ATTR  sint8_t jm_msg_getDownProtocol(jm_msg_t *msg) {
	//return jm_msg_is_s16(msg->flag, FLAG_DOWN_PROTOCOL);
	return (sint8_t)((msg->flag >> FLAG_DOWN_PROTOCOL) & 0x03);
}

ICACHE_FLASH_ATTR  void jm_msg_setDownProtocol(jm_msg_t *msg, sint16_t protocol) {
	//flag |= protocol == PROTOCOL_JSON ? FLAG_DOWN_PROTOCOL : 0 ;
	//msg->flag = jm_msg_set_s16(protocol == PROTOCOL_JSON, msg->flag, FLAG_DOWN_PROTOCOL);
	//0000 0000 0000 0011
	msg->flag = (msg->flag & FLAG_DOWN_PROTOCOL_MASK) | (protocol << FLAG_DOWN_PROTOCOL);
}

ICACHE_FLASH_ATTR static void freeMem(jm_buf_t *buf){
	if(buf) jm_buf_release(buf);
}

ICACHE_FLASH_ATTR jm_msg_t *jm_msg_decode(jm_buf_t *b) {

	jm_msg_t *msg = jm_msg_create();
	if(msg == NULL) {
		JM_MSG_ERROR("msg mo\n");
		goto jmerror;
	}

	//锟斤拷0,1锟斤拷锟街斤拷
	//uint16_t flag;
	if(!jm_buf_get_s16(b,&(msg->flag))) {
		JM_MSG_ERROR("msg F0\n");
		goto jmerror;
	}

	//ByteBuffer b = ByteBuffer.wrap(data);
	sint32_t len = 0;
	if(jm_msg_isLengthInt(msg)) {
		//len = b.readInt();
		if(!jm_buf_get_s32(b, &len)) {
			JM_MSG_ERROR("msg F1\n");
			goto jmerror;
		}
	} else {
		//len = b.readUnsignedShort(); // len = 锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷
		sint16_t slen = 0;
		if(!jm_buf_get_s16(b, &slen)) {
			JM_MSG_ERROR("msg F2\n");
			goto jmerror;
		}
		len = slen;
	}

	if(jm_buf_readable_len(b) < len){
		//throw new CommonException("Message len not valid");
		JM_MSG_ERROR("msg F3 \n");
		goto jmerror;
	}

	//锟斤拷3锟斤拷锟街斤拷
	//msg.setVersion(b.readByte());

	//read type
	//锟斤拷4锟斤拷锟街斤拷

	//msg.setType(b.readByte());
	if(!jm_buf_get_u8(b, &msg->type)) {
		JM_MSG_ERROR("msg F4\n");
		goto jmerror;
	}

	//msg.setMsgId(b.readLong());
	sint64_t mid;
	if(!jm_buf_get_s64(b, &mid)) {
		JM_MSG_ERROR("msg F5\n");
		goto jmerror;
	}
	msg->msgId = (uint32_t)mid;

	if(jm_msg_isReadExtra(msg)) {
		uint16_t curLen = jm_buf_readable_len(b);
        //jm_emap_t *em = jm_emap_create(PREFIX_TYPE_BYTE);
		msg->extraMap = jm_extra_decode(b);
		if(msg->extraMap == NULL) {
			JM_MSG_ERROR("msg F6\n");
			goto jmerror;
		}

		len = len - (curLen - jm_buf_readable_len(b));
		//msg->len = len;//锟斤拷效锟斤拷锟截的筹拷锟斤拷

        msg->extrFlag = jm_emap_getInt(msg->extraMap, (void*)EXTRA_KEY_FLAG);

		/*if(extraFlag) {
			msg->extrFlag = extraFlag->value.s32Val;
		}*/
	} else {
		//msg->len = len;//锟斤拷效锟斤拷锟截的筹拷锟斤拷
	}

	if(len > 0){
		jm_buf_t *pl = jm_buf_create(len);
		if(!jm_buf_get_buf(b,pl,len)) {
			JM_MSG_ERROR("msg F7\n");
            jm_buf_release(pl);
			goto jmerror;
		}
		msg->payload = pl;
		//msg.setPayload(ByteBuffer.wrap(payload));
	} else {
		msg->payload = NULL;
	}

	return msg;

	jmerror:
		if(msg) {
            jm_msg_release(msg);
		}
		return NULL;
}

ICACHE_FLASH_ATTR BOOL jm_msg_encode(jm_msg_t *msg, jm_buf_t *buf) {

	jm_buf_t *data = msg->payload;

	int len = 0;//锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷,锟斤拷锟斤拷锟斤拷头锟斤拷锟斤拷锟斤拷
	if(data != NULL){
		len = jm_buf_readable_len(data);
	}

	if(msg->extrFlag != 0) {
        jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_FLAG, msg->extrFlag, false);
	}

	jm_buf_t *extBuf = NULL;
	uint16_t extraLen = 0;
	if(jm_msg_isWriteExtra(msg)) {
		extBuf = jm_buf_create(MEM_REALLOC_BLOCK_SIZE);
		if(!jm_extra_encode(msg->extraMap->_hd, extBuf, &extraLen, msg->extraMap->keyType)) {
			//写锟斤拷锟斤拷锟斤拷息失锟斤拷
			goto doerror;
		}

		/*if(extraLen > 4092) {
			return false;
		}*/

		len += extraLen;
		jm_msg_setExtraFlag(msg,true);
	}

	//锟斤拷1锟斤拷2锟斤拷锟街斤拷 ,len = 锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷模式时锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷
	if(len <= MAX_SHORT_VALUE) {
		jm_msg_setLengthType(msg,false);
	} else if(len < MAX_INT_VALUE){
		jm_msg_setLengthType(msg,true);
	} else {
		//throw new CommonException("Data length too long than :"+MAX_INT_VALUE+", but value "+len);
		goto doerror;
	}

	//锟斤拷0,1,2,3锟斤拷锟街节ｏ拷锟斤拷志头
	//b.put(this.flag);
	//b.writeShort(this.flag);
	//jm_msg_writeUnsignedShort(buf, this.flag);
	if(!jm_buf_put_s16(buf,msg->flag)){
		goto doerror;
	}

	if(len <= MAX_SHORT_VALUE) {
		//锟斤拷2锟斤拷3锟斤拷锟街斤拷 ,len = 锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷模式时锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷
		if(!jm_buf_put_s16(buf,len)){
			goto doerror;
		}
		//b.writeUnsignedShort(len);
	}else if(len < MAX_INT_VALUE){
		//锟斤拷息锟斤拷锟斤拷锟斤拷锟斤拷蟪ざ锟轿狹AX_VALUE 2,3,4,5
		//b.writeInt(len);
		if(!jm_buf_put_s32(buf,len)){
			goto doerror;
		}
	} else {
		goto doerror;
		//throw new CommonException("Max int value is :"+ Integer.MAX_VALUE+", but value "+len);
	}

	//b.putShort((short)0);

	//锟斤拷3锟斤拷锟街斤拷
	//b.put(this.version);
	//b.writeByte(this.method);

	//锟斤拷4锟斤拷锟街斤拷
	//writeUnsignedShort(b, this.type);
	//b.put(this.type);
	//b.writeByte(this.type);
	if(!jm_buf_put_s8(buf,msg->type)){
		goto doerror;
	}

	//b.writeLong(this.msgId);
	if(!jm_buf_put_s64(buf,msg->msgId)){
		goto doerror;
	}

	if(jm_msg_isWriteExtra(msg)) {
		//b.writeInt(this.extrFlag);
		/*if(!jm_buf_put_u16(buf,extraLen)){
			goto doerror;
		}*/

		if(!jm_buf_put_buf(buf,extBuf)){
			goto doerror;
		}

		freeMem(extBuf);
		/*b.writeUnsignedShort(this.extra.remaining());
		b.write(this.extra);*/
	}

	if(data != NULL){
		//b.put(data);
		/*b.write(data);
		data.reset();*/
		jm_buf_put_buf(buf,data);
		//freeMem(data);
	}

	JM_MSG_DEBUG("msg: %d, mcode: %d, flag: %d\n",jm_msg_getDownProtocol(msg),
         jm_emap_getInt(msg->extraMap,(void*)EXTRA_KEY_SM_CODE), msg->flag);

	return true;

	doerror:
		freeMem(extBuf);
		//freeMem(data);
		return false;
}

ICACHE_FLASH_ATTR jm_msg_t *jm_msg_readMessage(jm_buf_t *buf){

	//保存当前读位置
	//uint16_t rpos = buf->rpos;
	jm_buf_rmark(buf);

	//可读数据数量
	uint16_t totalLen = jm_buf_readable_len(buf);
	//JM_MSG_DEBUG("jm_msg_readMessage totalLen %d\n",totalLen);

	if(totalLen < HEADER_LEN) {
		//可读数据小于消息头部长度
		JM_MSG_ERROR("msg t %d\n",totalLen);
		return NULL;
	}

	//标识头
	sint16_t f ;

	if(!jm_buf_get_s16(buf, &f)) {
		JM_MSG_ERROR("msg F1 \n");
		return NULL;
	}

	uint32 len = 0;
	uint32 headerLen = HEADER_LEN;
	//取锟节讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟街斤拷 锟斤拷锟捷筹拷锟斤拷
	if(jm_msg_is_s16(f,FLAG_LENGTH_INT)) {
		//32位锟斤拷锟斤拷
		if(!jm_buf_get_u32(buf, &len)) {
			JM_MSG_ERROR("msg F2 \n");
			return NULL;
		}

		//锟斤拷原锟斤拷锟斤拷锟捷癸拷位锟斤拷
		/*if(!jm_buf_set_rpos(buf,rpos)) {
			JM_MSG_ERROR("msg F3 \n");
			return NULL;
		}*/

		//len = Message.readUnsignedShort(cache);
		//锟斤拷原锟斤拷锟斤拷锟捷癸拷位锟斤拷
		//cache.position(pos);
		headerLen += 2;  //int锟酵憋拷默锟斤拷short锟斤拷2锟街斤拷
		if(totalLen < len + headerLen){
			JM_MSG_DEBUG("msg F4:%d, need len: %d\n",totalLen,(len + headerLen));
			//锟斤拷锟斤拷锟杰癸拷锟斤拷一锟斤拷锟姐够锟斤拷锟饺碉拷锟斤拷锟捷帮拷
			return NULL;
		}
	} else {
		//16位锟斤拷锟斤拷
		//锟斤拷锟捷筹拷锟饺诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值
		//len = cache.getInt();
		//len = cache.getInt();
		uint16_t sl;
		if(!jm_buf_get_s16(buf, &sl)) {
			JM_MSG_ERROR("msg F5 \n");
			return NULL;
		}
		len = sl;

		//锟斤拷原锟斤拷锟斤拷锟捷癸拷位锟斤拷
		//cache.position(pos);
		/*if(!jm_buf_set_rpos(buf,rpos)) {
			JM_MSG_ERROR("msg F6 \n");
			return NULL;
		}*/
		/*if(len > (Integer.MAX_VALUE-10000) || len < 0) {
			throw new CommonException("Got invalid message len: " + len + ",flag: " + f+",buf: " + cache.toString());
		}*/
		if(totalLen < len + headerLen){
			//锟斤拷锟斤拷锟杰癸拷锟斤拷一锟斤拷锟姐够锟斤拷锟饺碉拷锟斤拷锟捷帮拷
			JM_MSG_ERROR("msg F7:%d, need len: %d\n",totalLen,(len + headerLen));
			return NULL;
		}
	}

	jm_buf_rmark_reset(buf);

	//JM_MSG_DEBUG("jm_msg_readMessage 3 len=%u headerLen=%u\n", len, headerLen);
	//byte[] data = new byte[len + headerLen];
	//锟接伙拷锟斤拷锟叫讹拷一锟斤拷锟斤拷,cache锟斤拷position锟斤拷前锟斤拷
	//cache.get(data, 0, len + headerLen);
	//return Message.decode(new JDataInput(ByteBuffer.wrap(data)));

	jm_msg_t *msg = NULL;
	jm_buf_t *cache = jm_buf_create(len + headerLen);
	//JM_MSG_DEBUG("jm_msg_readMessage 1 \n");
	if(!cache) {
		JM_MSG_ERROR("msg F8\n");
		return NULL;
	}

	JM_MSG_DEBUG("jm_msg_readMessage 2  jm_buf_readable_len(buf)=%u rpos=%d, wpos=%d full=%d\n", jm_buf_readable_len(buf), buf->rpos, buf->wpos, jm_buf_is_full(buf));
	if(jm_buf_get_buf(buf, cache, len + headerLen)) {
		//JM_MSG_DEBUG("jm_msg_readMessage 4 \n");
		msg = jm_msg_decode(cache);
		jm_buf_release(cache);
	} else {
		//JM_MSG_DEBUG("jm_msg_readMessage 5 \n");
		jm_buf_release(cache);
		JM_MSG_ERROR("msg F9\n");
		return NULL;
	}

	//JM_MSG_DEBUG("jm_msg_readMessage success: %d\n",msg->msgId);
	//jm_buf_t *cache = jm_buf_buffer_wrap(buf, ,true);
	//jm_buf_move_forward(buf,len + headerLen);//前锟斤拷一锟斤拷锟斤拷息锟斤拷锟斤拷
	return msg;

}

#endif // JM_MSG_ENABLE==1

