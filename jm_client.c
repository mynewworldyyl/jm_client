
#include "jm_client.h"
#include "jm_msg.h"
#include "jm_mem.h"
#include "jm_stdcimpl.h"
#include "jm_constants.h"
#include "debug.h"

#if ESP8266==1
#include "jm_net.h"
#endif

#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"
#endif

#if JM_TIMER_ENABLE
static jm_hashmap_t *timerCheckers;
#endif //JM_RPC_ENABLE

#if JM_PS_ENABLE
static ICACHE_FLASH_ATTR void _c_setMsgUdp(jm_msg_t *msg);
#endif //JM_RPC_ENABLE

#if JM_RPC_ENABLE

typedef struct jm_msg_handler_register_item{
	sint8_t type;
	jm_cli_msg_hander_fn handler;
	struct jm_msg_handler_register_item *next;
} CHRI;

typedef struct _c_msg_result {
	//BOOL in_used;
    uint32_t startTime;
	sint32_t msg_id;
	//jm_msg_t *msg;
	jm_cli_rpc_callback_fn callback;
	void *cbArg;
	//struct _c_msg_result *next;
} jm_cli_msg_result_t;



ICACHE_FLASH_ATTR static BOOL _c_checkRpcTimeout();
ICACHE_FLASH_ATTR static  CHRI* _c_GetMsgHandler(sint8_t type);
ICACHE_FLASH_ATTR static  jm_cli_send_msg_result_t _c_rpcMsgHandle(jm_msg_t *msg);
ICACHE_FLASH_ATTR uint8_t _c_getChanelNo();

ICACHE_FLASH_ATTR static  BOOL _c_isValidLoginInfo();

static CHRI *handlers = NULL;
static sint32_t msgId = 0;

static uint64_t lastSendHearbeatTime=0;
static uint64_t lastActiveTime=0; //最后一次连接JM平台时间，用于发送心跳包

static uint64_t lastLoginTime=0; //最后一次登录时间
static char *loginKey = NULL;
//static sint32_t actId = 0; //设备关联账号
//static char *deviceId = NULL; //当前登录设备ID
static uint32_t clientId = 0;
static sint8_t grpId = 0;
static sint32_t loginCode = LOGOUT;//登录结果码，0表示 成功，其他失败

//static char *loginMsg  = NULL;

//static net_event_t connected = NetDisconnected;

static uint32_t rpcTimeout = 1000*30;
//static jm_cli_msg_result_t *wait_for_resps = NULL;
static jm_hashmap_t *wait_for_resps = NULL;

ICACHE_FLASH_ATTR static void _c_commonRpcResult(jm_emap_t *resultMap, sint32_t code, char *errMsg, void *arg);
ICACHE_FLASH_ATTR static void _c_netListener_syncStdTime(jconn_type connType, net_event_t evtType);

#endif //JM_RPC_ENABLE

typedef struct {
	char *key;
	jm_cli_timer_check_fn fn;
	uint16_t interval; //时间间隔
	uint32_t lastCallTime; //上一次调用时间
	BOOL oneTime;
	//BOOL del;
} jm_cli_timer_check_t;

//static jm_cli_send_msg_fn jm_msg_sender = NULL;

//static jm_cli_p2p_msg_sender_fn jm_msg_p2p_sender = NULL;

static jm_cli_reconnect_fn reconnect;

static jm_hashmap_t *name2ChannelNo;

static uint8_t channelNoId = 1;

static jm_hashmap_t *sendChannel;

/*
static uint16_t jmPort = 0;
static char *jmHost = NULL;
static uint8_t useUdp = NULL;
 */

const static char *TOPIC_PREFIX = "/__act/dev/";

static jm_cli_getSystemTime_fn _client_getSystemTime;

//const static char *MSG_TYPE = "__msgType";
//static char *DEVICE_ID = "/testdevice001";

static BOOL inited = 0;

static uint32_t sysStartTime=0;//系统启动时间，单位毫秒

static timer_check check;

static jm_mem_op jmm;

extern sys_config_t sysCfg;

static BOOL wifiEn = false;

static net_event_t tcpConnected = NetDisconnected;

static net_event_t udpConnected = NetDisconnected;

#if JM_STD_TIME_ENABLE==1
static uint32_t stdTime = 0;//世界标准时间，需要联网从服务端获取,单位秒
#endif

extern ICACHE_FLASH_ATTR void _bb_setMemOps(jm_mem_op *ops);

/***********************************PS BEGIN*******************************************/
#if JM_PS_ENABLE==1
#include "jm_cli_ps.h"
#endif // JM_PS_ENABLE

/***********************************PS END*********************************************/

ICACHE_FLASH_ATTR uint8_t _c_getChanelNo(){
#if JM_TCP==1
	return (uint8_t)jm_hashmap_get(name2ChannelNo, JM_TCP_SENDER_NAME);
#else
	return (uint8_t)jm_hashmap_get(name2ChannelNo, JM_UDP_SENDER_NAME);
#endif
}

ICACHE_FLASH_ATTR BOOL jm_cli_isMaster(){
	return (BOOL)(jm_cli_getJmInfo()->deviceRole == DevRoleMaster);
}

ICACHE_FLASH_ATTR BOOL jm_cli_isSlave(){
	return (int8_t)jm_cli_getJmInfo()->deviceRole == DevRoleSlave;
}

ICACHE_FLASH_ATTR BOOL jm_cli_isStandalone(){
	return (int8_t)jm_cli_getJmInfo()->deviceRole == DevRoleStandalone;
}


ICACHE_FLASH_ATTR BOOL jm_cli_setMemOps(jm_mem_op ops){
    if(ops.jm_zalloc_fn == NULL) {
        JM_CLI_ERROR("csop jm_zalloc_fn NULL\n");
        return false;
    }

    if(ops.jm_free_fn == NULL) {
        JM_CLI_ERROR("csop jm_free_fn NULL\n");
        return false;
    }

    if(ops.jm_memset == NULL) {
        JM_CLI_ERROR("csop jm_memset NULL\n");
        return false;
    }

    if(ops.jm_memcpy == NULL) {
        JM_CLI_ERROR("csop jm_memcpy NULL\n");
        return false;
    }

    if(ops.jm_realloc_fn == NULL) {
        JM_CLI_ERROR("csop jm_realloc_fn NULL\n");
        return false;
    }

    /*
    jmm.jm_free_fn = ops.jm_free_fn;
    jmm.jm_memcpy = ops.jm_memcpy;
    jmm.jm_memset = ops.jm_memset;
    jmm.jm_realloc_fn = ops.jm_realloc_fn;
    jmm.jm_zalloc_fn = ops.jm_zalloc_fn;
    jmm.jm_postEvent = ops.jm_postEvent;
    jmm = ops.jm_zalloc_fn;
    jmm.jm_postEvent = ops.jm_postEvent;
    */
    ops.jm_memcpy(&jmm, &ops, sizeof(ops));

    _bb_setMemOps(&jmm);
}

ICACHE_FLASH_ATTR timer_check* jm_cli_getCheck() {
	return &check;
}

ICACHE_FLASH_ATTR jm_mem_op* jm_cli_getJmm() {
	return &jmm;
}


//系统启动时间，单位毫秒
ICACHE_FLASH_ATTR uint32_t jm_cli_getSysStartTime(){
	return sysStartTime;
}

ICACHE_FLASH_ATTR void jm_cli_setSysTimeFn(jm_cli_getSystemTime_fn sfn) {
	 _client_getSystemTime = sfn;
	 sysStartTime = jm_cli_getSysTime();//记录系统启动时间
}

//取得系统运行时长，单位秒
ICACHE_FLASH_ATTR uint32_t jm_cli_getSysRunTime() {
	//ESP_LOGI(TAG,"jm_cli_getSysTime=%u,jm_cli_getSysStartTime=%d",jm_cli_getSysTime(), jm_cli_getSysRunTime());
	uint32_t interval = jm_cli_getSysTime() - jm_cli_getSysStartTime();
	return (uint32_t)(interval/1000);
}

#if JM_STD_TIME_ENABLE==1 && JM_RPC_ENABLE==1

//世界标准时间，单位秒
ICACHE_FLASH_ATTR uint32_t jm_cli_getCurStdTime(){
	//ESP_LOGI(TAG,"stdTime=%u,jm_cli_getSysRunTime=%d",stdTime, jm_cli_getSysRunTime());
	return stdTime + jm_cli_getSysRunTime();
}

//取得系统启动时的标准时间 单位秒
ICACHE_FLASH_ATTR uint32_t jm_cli_getStartStdTime(){
	if(stdTime == 0 && jm_cli_netStatus() == NetConnected) {
		if(jm_cli_tcpEnable()){
			_c_netListener_syncStdTime(JCONN_TCP, NetConnected);
		}else {
			_c_netListener_syncStdTime(JCONN_UDP, NetConnected);
		}
	}
	return stdTime;
}

ICACHE_FLASH_ATTR static void _c_netListener_syncStdTime(jconn_type connType, net_event_t evtType) {
    if(jm_cli_netStatus() == NetConnected) {
	   //JM_CLI_ERROR("std N dis");
       return;
    }

	if(jm_cli_tcpEnable() && connType != JCONN_TCP){
		//TCP启用，则只响应TCP类型的事件
		//JM_CLI_ERROR("std TCP dis tcpE=%d, conType=%d",jm_cli_tcpEnable(),connType);
		return;
	}

	//JM_CLI_DEBUG("std TCP dis tcpE=%d, conType=%d",jm_cli_tcpEnable(),connType);
 	sint64_t msgId = jm_cli_invokeRpc(-2072516138, NULL, (jm_cli_rpc_callback_fn)_c_commonRpcResult, (void*)1);
	if(msgId < 0 && msgId != JM_SUCCESS) {
		JM_CLI_ERROR("c up time E");
	}
}

ICACHE_FLASH_ATTR static void _ctrl_syncStdTime_checker(){
	if(stdTime > 0 || !jm_cli_wifiEnable()) return;

	if(jm_cli_tcpEnable()){
		_c_netListener_syncStdTime(JCONN_TCP, NetConnected);
	}else {
		_c_netListener_syncStdTime(JCONN_UDP, NetConnected);
	}

	jm_cli_unregistTimerChecker("_syncStdT");
}

#endif //JM_STD_TIME_ENABLE


ICACHE_FLASH_ATTR uint32_t jm_cli_getSysTime() {
    return _client_getSystemTime();
}

ICACHE_FLASH_ATTR sys_config_t* jm_cli_getJmInfo(){
	return &sysCfg;
}

#if JM_HB_ENABLE==1 && JM_RPC_ENABLE==1

ICACHE_FLASH_ATTR void _c_hearbeetResult(jm_emap_t *rst, sint32_t code, char *msg, void *args) {
	JM_CLI_DEBUG("_chr c:%d, m:%s\n",code,msg?msg:"");//获取心跳结果
}

ICACHE_FLASH_ATTR void _c_sendHearbeet() {
	uint64_t cur = jm_cli_getSysTime();
	if((cur - lastSendHearbeatTime) < JM_HEARBEET_INTERVAL) return;
	if((cur - lastActiveTime) < JM_HEARBEET_INTERVAL) return;//最后一次活动时间小于30秒，不需要发心跳
	//JM_CLI_DEBUG("cshb send hearbeet!\n");
	lastSendHearbeatTime = cur;
	jm_cli_invokeRpc(885612323, NULL, _c_hearbeetResult, NULL);
}
#endif //JM_HB_ENABLE



ICACHE_FLASH_ATTR uint8_t jm_cli_tcpEnable(){
#if JM_TCP==1
	return true;
#else
	return false;
#endif
}

#if JM_RPC_ENABLE==1
static ICACHE_FLASH_ATTR BOOL _c_check_net() {
	//JM_CLI_DEBUG("jm_cli_main_timer check");
	timer_check* checker = jm_cli_getCheck();
	if(checker == NULL) {
		JM_CLI_ERROR("cli c N!");
		return false;
	}

	if(checker->jm_checkNet) {
		if(!checker->jm_checkNet()) {
			//网络不可用
			JM_CLI_ERROR("Wifi disable\n");
			return false;
		}
	} else {
		JM_CLI_ERROR("cli checker net disable!\n");
		return false;
	}

	//网络连接成功才做登录校验
	//JM_CLI_DEBUG("cli tcp login B!");
	if(!checker->jm_checkLoginStatus()) {
		//账号登录验证失败
		JM_CLI_DEBUG("cli jm_checkLoginStatus F!");
	} /*else {
		JM_CLI_DEBUG("jm_main_timer check login success!");
	}*/
	return true;
}
#endif //JM_RPC_ENABLE


ICACHE_FLASH_ATTR BOOL jm_cli_distroy() {

#if JM_RPC_ENABLE==1
	CHRI *chri = handlers;
	while(chri) {
		CHRI *n = chri->next;
        jmm.jm_free_fn(chri, sizeof(CHRI));
		chri = n;
	}
	handlers = NULL;

#endif

#if JM_PS_ENABLE==1
	ps_listener_map *pi = ps_listener;
	while(pi) {
		ps_listener_map *n = pi->next;
        jmm.jm_free_fn(pi,sizeof(ps_listener_map));
		pi = n;
	}
#endif
	
	return true;
}


//设备是否已经被绑定
ICACHE_FLASH_ATTR uint8_t jm_cli_isBind(){
    return /*sysCfg.actId >0 &&*/ jm_strlen((char*)sysCfg.deviceId) > 0 && jm_strlen((char*)sysCfg.invokeCode) > 0;
}

ICACHE_FLASH_ATTR void jm_cli_registReconnect(jm_cli_reconnect_fn fn){
	reconnect = fn;
}

ICACHE_FLASH_ATTR sint8_t jm_cli_registSenderChannel(jm_cli_p2p_msg_sender_fn sender, char *chname){
	JM_CLI_DEBUG("cli r sender=%p\n", sender);

	if(sender == NULL) {
		JM_CLI_ERROR("cli s N\n");
		return -1;
	}

	if(channelNoId < 0) {
		JM_CLI_ERROR("cli cid: %p\n",sender);
		return -2;//渠道号用完
	}

	uint8_t chno = channelNoId++;
	if(!jm_hashmap_put(sendChannel, (void*)chno, sender)){
        JM_CLI_ERROR("cli s F\n");
		return -3;
	}

	//给通道起名
	if(chname != NULL) {
		jm_hashmap_put(name2ChannelNo, chname, (void*)chno);
	}

	JM_CLI_DEBUG("cli [%d] suc!\n",chno);
	return chno;
}

ICACHE_FLASH_ATTR jm_cli_p2p_msg_sender_fn jm_cli_getUdpSenderByName(char *channelName){
    return jm_hashmap_get(sendChannel, jm_hashmap_get(name2ChannelNo,channelName));
}

ICACHE_FLASH_ATTR static  BOOL _c_isValidLoginInfo(){
	if(jm_strlen((char*)sysCfg.deviceId) == 0) {
		return false;
	}

//#ifndef JM_STM32
	if(sysCfg.actId <= 0) {
		return false;
	}
//#endif
	
	return true;
}


ICACHE_FLASH_ATTR net_event_t jm_cli_netStatus() {
	return jm_cli_tcpEnable() ? tcpConnected : udpConnected;
}

ICACHE_FLASH_ATTR BOOL  jm_cli_wifiEnable() {
	return wifiEn;
}

ICACHE_FLASH_ATTR BOOL jm_cli_socketDisconCb(jconn_type connType) {
	if(connType == JCONN_TCP) {
		tcpConnected = NetDisconnected;
	} else if(connType == JCONN_UDP) {
		udpConnected = NetDisconnected;
	}
	//_c_invokeNetListener(connType, NetDisconnected);
	return true;
}

ICACHE_FLASH_ATTR void _client_socketConedCb(jm_event_t *event){
	sint8_t connType = event->subType;
	JM_CLI_DEBUG("jm_cli_socketConedCb B connType=%d\n",connType);

	if(event->type == TASK_APP_UDP) {
		udpConnected = connType;
	}else if(event->type == TASK_APP_TCP) {
		tcpConnected = NetConnected;
	}else {
		JM_CLI_ERROR("cli invalid conn\n");
	}
	/*if(connType == JCONN_TCP) {
		tcpConnected = NetConnected;
	} else if(connType == JCONN_UDP) {
		udpConnected = NetConnected;
	}*/
#if JM_RPC_ENABLE==1
	if(jm_cli_tcpEnable() ){
		if(!jm_cli_isLogin()) {
			JM_CLI_DEBUG("cli tlogin\n");
			jm_cli_login();
		}
	} else {
		if(!jm_cli_isLogin()) {
			JM_CLI_DEBUG("cli ulogin\n");
			jm_cli_login();
		}
	}
#endif //JM_RPC_ENABLE
	
	JM_CLI_DEBUG("jm_cli_socketConedCb return");
}

ICACHE_FLASH_ATTR BOOL jm_cli_socketSendTimeoutCb(jconn_type connType){
    //_c_invokeNetListener(connType, NetSendTimeOut);
	return true;
}

//Wifi连接上后，开始下载需要缓存的语音信息
#ifndef JM_STM32
static void _jm_c_wifi_conn_event_listener(jm_event_t* event){ // The tts stream comes to an end
    if(event->type == TASK_APP_WIFI_GOT_IP) {
		wifiEn = true;
	}else if(event->type == TASK_APP_WIFI_DISCONN) {
		wifiEn = false;
	}
}
#endif

ICACHE_FLASH_ATTR BOOL jm_cli_isInit() {
	return inited;
}

ICACHE_FLASH_ATTR BOOL jm_cli_init() {
	JM_CLI_DEBUG("cli_init B\n");
	
	if(inited==true) {
		return false;//已经初始化
	}

#if JM_RPC_ENABLE==1
    clientId = sysCfg.clientId;
    grpId = sysCfg.grpId;

    sendChannel = jm_hashmap_create(3, PREFIX_TYPE_BYTE);
    name2ChannelNo = jm_hashmap_create(3,PREFIX_TYPE_STRINGG);

    timerCheckers = jm_hashmap_create(6,PREFIX_TYPE_STRINGG);
	
    wait_for_resps = jm_hashmap_create(6,PREFIX_TYPE_INT);
	
	jm_cli_registMessageHandler(_c_rpcMsgHandle, MSG_TYPE_RRESP_JRPC);
	
	jm_cli_registTimerChecker("_c_RPCC",_c_checkRpcTimeout,3,5,false);
	
#endif //JM_RPC_ENABLE

#if JM_PS_ENABLE==1
	jm_cli_registMessageHandler(_c_pubsubMsgHandle, MSG_TYPE_ASYNC_RESP);
	jm_cli_registMessageHandler(_c_pubsubMsgHandle, MSG_TYPE_PUBSUB);
	jm_cli_registMessageHandler(_c_pubsubOpMsgHandle, MSG_TYPE_PUBSUB_RESP);
	//jm_cli_subscribeByType(test_onPubsubItemType1Listener,-128,false);

	jmm.jm_regEventListener(TASK_APP_LOGIN_RESULT, _c_subTopicAfterjmLogin);
	//#endif
#endif

	/*if(doLogin && actId > 0) {
		jm_cli_login(deviceId,actId);
	}else {
		JM_CLI_DEBUG("invalid actId:%d, deviceId: %s\n",actId,deviceId);
	}*/

    //检测超时
    //jm_cli_registTimerChecker("_c_checkRpcTimeout",_c_checkRpcTimeout,2,5);
    //jm_cli_registTimerChecker("syncStdT",_ctrl_syncStdTime_checker,2,5);

    //检测超时
#ifndef JM_STM32
		jm_cli_registTimerChecker("_c_net", _c_check_net, 5,1,false);//网络状态
#endif

#if JM_STD_TIME_ENABLE==1
   	jm_cli_registTimerChecker("_syncStdT",_ctrl_syncStdTime_checker,5,1,false);
#endif

#if JM_HB_ENABLE==1 && JM_RPC_ENABLE==1
   	jm_cli_registTimerChecker("_chb",_c_sendHearbeet,60,60,false);//一分钟一次心跳
#endif //JM_HB_ENABLE

   	jmm.jm_regEventListener(TASK_APP_UDP, _client_socketConedCb);

#ifndef JM_STM32
   	if(jmm.jm_regEventListener) {		
		JM_CLI_DEBUG("cli_init 1 %u\n", jm_cli_getJmm()->jm_regEventListener);
		jmm.jm_regEventListener(TASK_APP_WIFI_GOT_IP, _jm_c_wifi_conn_event_listener);
		jmm.jm_regEventListener(TASK_APP_WIFI_DISCONN, _jm_c_wifi_conn_event_listener);
   	}
#endif

   	JM_CLI_DEBUG("cli_init E\n");
	inited = 1;
	return true;
}

/***********************************Timer Checker BEGIN*******************************************/
#if JM_TIMER_ENABLE==1
#include "jm_cli_timer.h"
#endif // JM_TIMER_ENABLE

/***********************************Timer Checker END*********************************************/

/***********************************RPC BEGIN*******************************************/
#if JM_RPC_ENABLE==1
#include "jm_cli_rpc.h"
#endif // JM_RPC_ENABLE

/***********************************RPC END*********************************************/

/***********************************KEY VALUE BEGIN*******************************************/
#if JM_KV_ENABLE==1
#include "jm_cli_kv.h"
#endif // JM_KV_ENABLE

/***********************************KEY VALUE END*********************************************/


/***********************************解码Extra数据开始*********************************************/
#if JM_RPC_ENABLE==1
#include "jm_cli_encode.h"
#endif // JM_RPC_ENABLE
/***********************************解码Extra数据结束*********************************************/

