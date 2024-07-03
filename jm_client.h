/*
 * jm_client.h
 *
 *  Created on: 2023��4��16��
 *      Author: yeyulei
 */

#ifndef _JM_CLIENT_H_
#define _JM_CLIENT_H_

#include "jm_cons.h"

#include "jm_cfg_def.h"
#include "jm_cfg.h"

#include "jm_msg.h"
#include "jm_buffer.h"
#include "c_types.h"
#include "jm_mem.h"

#if ESP8266==1
#include "jm_net.h"
#endif

#ifdef STM32
#include "stm32_adapter.h"
#endif

#define JM_HEARBEET_INTERVAL 30000//向JM平台发送心跳间隔，超过30秒发送一次

//第5，6两位一起表示data字段的编码类型
#define FLAG_DATA_TYPE 5

#define FLAG_DATA_STRING 0
#define FLAG_DATA_BIN 1
#define FLAG_DATA_JSON 2
#define FLAG_DATA_EXTRA 3

#define BROADCAST_HOST "255.255.255.255"

#define  JM_DEVICE_PS_TYPE_CODE_BASE 0

#define  PS_TYPE_CODE(n) (JM_DEVICE_PS_TYPE_CODE_BASE + (n))

#define ITEM_TYPE_CTRL PS_TYPE_CODE(-128)
#define ITEM_TYPE_LOG PS_TYPE_CODE(-127)
#define ITEM_TYPE_SLAVE PS_TYPE_CODE(-126)

#define jm_cli_setPSItemDataType(v,flag) (*flag = (v << FLAG_DATA_TYPE) | *flag)

#define TOPIC_P2P "/__act/dev/p2pctrl" //设备与手机端对端消息主题，不需要登录设备，也不能从服务器下发

#define IPADDR_NONE         ((uint32)0xffffffffUL)

#define JM_TCP_SENDER_NAME "jmtcpc"
#define JM_UDP_SENDER_NAME "jmudpc"
#define JM_SERIAL_SENDER_NAME "jmserial"

/***************************系统事件码***********************************/

#define MSG_OP_CODE_SUBSCRIBE 1//订阅消息操作码
#define MSG_OP_CODE_UNSUBSCRIBE 2//取消订阅消息操作码
#define MSG_OP_CODE_FORWARD 3//依据账号ID做消息转发
#define MSG_OP_CODE_FORWARD_BY_TOPIC 4//根据主题做消息转发

#define  MT_INVALID_LOGIN_INFO 0x004C //76

#define JM_TASK_APP_MAINLOOP (1)
#define JM_TASK_APP_RX_DATA (2)
#define JM_TASK_APP_CHECKER (3)
#define JM_TASK_APP_IR_DATA (4)
#define JM_TASK_APP_MAIN_CHECK (5)
#define JM_TASK_APP_OLED_DATA  (6)//更新屏幕温湿度信息
#define JM_TASK_APP_WIFI_GOT_IP (7)
#define JM_TASK_APP_WIFI_DISCONN (8) //wifi断开

#define JM_TASK_APP_DNS_GO_IP (9)

#define JM_TASK_APP_RESTART_SYSTEM (10)

//#define JM_TASK_APP_JM_CHECKER ((os_signal_t)3)

#define JM_TASK_APP_SAVE_CFG (11)

#define JM_TASK_APP_GPTCHAT_RECORD_START (12)
#define JM_TASK_APP_GPTCHAT_RECORD_END (13)
#define JM_TASK_APP_AUDIO_PLAY (14)
#define JM_TASK_APP_SCREEN_ONOFF (15)

#define JM_TASK_APP_IR_SEND (16)  //发送红外命令
#define JM_TASK_APP_IR_RECV (17)  //接收红外命令
#define JM_TASK_APP_SPEECH_CMD (18)  //语音命令

#define JM_TASK_APP_DEV_CHANGE (19)  //设备信息更新

#define JM_TASK_APP_ESP_NOW_REQ (20)  //做ESP—NOW请求

#define JM_TASK_APP_PS_MSG_REQ (21)  //异步消息请求
#define JM_TASK_APP_PS_MSG_RESP (22)  //异步消息响应

#define JM_TASK_APP_LOGIN_RESULT (23)  //账号登录结果

#define JM_TASK_APP_TCP (24)  //TCP事件，连接和断开
#define JM_TASK_APP_UDP (25)  //UDP事件，UDP创建成功
#define JM_TASK_APP_WIFI (26)//wifi相关命令，具体功能由subType定义

#define JM_TASK_APP_ML (27)//ML事件处理

#define JM_TASK_APP_KEY (28)//按键事件， subType为GPIO编号， data为按击次数

#define JM_TASK_APP_NETPROXY (29)//

#define JM_TASK_APP_SERIAL (30) //串口命令相关

#define JM_TASK_APP_PROXY_TCP (31) //串口命令相关
#define JM_TASK_APP_PROXY_WRITE_UART (32) //串口命令相关


/***************************系统事件码结束***********************************/

#define JM_APP_EVENT_SUC 0  //成功事件，具体由事件实现者定义
//#define JM_LOGIN_RESULT_FAIL 1  //账号登录失败

#define JM_EVENT_FLAG_DEFAULT 0 //默认
#define JM_EVENT_FLAG_FREE_DATA 0X01 //释放data占用内存

// Task priority for main loop
#define JM_TASK_APP_QUEUE USER_TASK_PRIO_0

typedef enum _jconn_type {
    JCONN_INVALID    = 0,
    /* ESPCONN_TCP Group */
    JCONN_TCP        = 0x10,
    /* ESPCONN_UDP Group */
    JCONN_UDP        = 0x20,
}jconn_type;

typedef enum _dev_status{
    DEV_STATUS_INIT, //: 0, 未绑定
    DEV_STATUS_BUND, //: 1, 已绑定  物理设备的物理地址已经在后台绑定设备，但还没同步信息到物理设备
    DEV_STATUS_SYNC_INFO, // : 2, 已同步信息到设备，
    DEV_STATUS_UNBUND, // : 3, 已解绑
    DEV_STATUS_FREEZONE, // : 4, 冻结
    DEV_STATUS_RESET, // : 5, 重置
} dev_status_t;

typedef enum _client_act_login_status{
	LSUCCESS=0,//登录成功
	LOGIN_FAIL,//登录失败
	LOGOUT,//登出
} jm_cli_act_login_status;

#if JM_PS_ENABLE==1
typedef struct _c_pubsub_item{

	uint8_t dataFlag;

	//private byte  对应msg所属flag
	uint8_t mflag;

	//private byte
	uint8_t flag;

	//private Integer
	sint32_t fr;

	//private long
	sint64_t id; //0

	sint8_t type;

	//private String
	char *topic; //2

	//private int
	sint32_t srcClientId; //3

	//private Integer
	sint32_t to; //4

	//private String
	char *callback ; //5

	//private long
	uint8_t delay;  //6

	/*private Map<String,Object>*/
	jm_emap_t *cxt; //7

    //对应消息的extraMap,不做序列化
    jm_emap_t *extraMap; //7

	//private Object
	//jm_buf_t *data;  //8

    sint8_t dt;// PREFIX_TYPE_MAP PREFIX_TYPE_LIST PREFIX_TYPE_STRING
	void *data;

	//private transient ILocalCallback localCallback;

	//private transient int failCnt = 0;

} jm_pubsub_item_t;

typedef uint8_t (*jm_cli_PubsubListenerFn)(jm_pubsub_item_t *psItem);
#endif

typedef enum {
    NetConnected=1,//连通
    NetDisconnected,//断开
    NetSendTimeOut,//超时
} net_event_t;

typedef uint8_t (*jm_cli_rpc_callback_fn)(void *resultMap, sint32_t code, char *errMsg, void *arg);

typedef struct _c_msg_result {
	//BOOL in_used;
    uint32_t startTime;
	sint32_t msg_id;
	//jm_msg_t *msg;
	jm_cli_rpc_callback_fn callback;
	void *cbArg;
	//struct _c_msg_result *next;
} jm_cli_msg_result_t;

#if JM_RPC_ENABLE==1 && JM_MSG_ENABLE==1

typedef jm_cli_send_msg_result_t (*jm_cli_msg_hander_fn)(jm_msg_t *msg);
typedef jm_cli_send_msg_result_t (*jm_cli_send_msg_fn)(jm_buf_t *buf);
//typedef void (*MqttCallback)(uint32_t *args);
//typedef uint8_t (*jm_cli_on_async_msg_fn)(jm_pubsub_item_t *item);

#endif //JM_RPC_ENABLE

//消息广播发送器 char* host, uint16_t port, uint16_t hlen
typedef jm_cli_send_msg_result_t (*jm_cli_p2p_msg_sender_fn)(jm_buf_t *buf, jm_emap_t *params);

typedef BOOL (*jm_cli_timer_check_fn)();

typedef uint32_t (*jm_cli_getSystemTime_fn)();

typedef void (*jm_cli_reconnect_fn)();

typedef struct _c_timer_check{
	jm_cli_timer_check_fn jm_checkNet;//本地网络状态检测
	jm_cli_timer_check_fn jm_checkConCheck;//与JM物联网平台的连接
	jm_cli_timer_check_fn jm_checkLocalServer;//本地设备端对端连接
	jm_cli_timer_check_fn jm_checkLoginStatus;//物联网平台登录状态
} timer_check;

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR int jm_strcmp(const char *s1, const char *s2);
		
ICACHE_FLASH_ATTR int32_t jm_cli_getClientId();

ICACHE_FLASH_ATTR int8_t jm_cli_getGrpId();

ICACHE_FLASH_ATTR uint8_t jm_cli_tcpEnable();

ICACHE_FLASH_ATTR BOOL jm_cli_isMaster();

ICACHE_FLASH_ATTR BOOL jm_cli_isSlave();

ICACHE_FLASH_ATTR BOOL jm_cli_isStandalone();

ICACHE_FLASH_ATTR BOOL jm_cli_unregistTimerChecker(char *key);

ICACHE_FLASH_ATTR BOOL jm_cli_registTimerChecker(char *key, jm_cli_timer_check_fn checker,
	 uint16_t interval, uint16_t delay, BOOL oneTime);

//系统启动时间，单位毫秒
ICACHE_FLASH_ATTR uint32_t jm_cli_getSysStartTime();

ICACHE_FLASH_ATTR uint32_t jm_cli_getSysRunTime();

ICACHE_FLASH_ATTR char* jm_cli_getLoginKey();

ICACHE_FLASH_ATTR void jm_cli_setLoginKey(char* loginKey);

ICACHE_FLASH_ATTR int32_t  jm_cli_clientId();

ICACHE_FLASH_ATTR int8_t  jm_cli_grpId();

#if JM_STD_TIME_ENABLE==1
//取得系统启动时的标准时间 单位秒
ICACHE_FLASH_ATTR uint32_t jm_cli_getStartStdTime();

//取得当前的标准时间  单位秒
ICACHE_FLASH_ATTR uint32_t jm_cli_getCurStdTime();
#endif

ICACHE_FLASH_ATTR sint8_t jm_cli_registSenderChannel(jm_cli_p2p_msg_sender_fn sender, char *chname);

//取得设备时钟记录的是时间
ICACHE_FLASH_ATTR uint32_t jm_cli_getSysTime();

ICACHE_FLASH_ATTR jm_mem_op* jm_cli_getJmm();

ICACHE_FLASH_ATTR BOOL jm_cli_setMemOps(jm_mem_op ops);

//设置系统时间获取器
ICACHE_FLASH_ATTR void jm_cli_setSysTimeFn(jm_cli_getSystemTime_fn fn);

//设备JM平台服务地址
//ICACHE_FLASH_ATTR void jm_cli_setJmInfo(jm_info *jmInfo/*char *jmh, uint16_t port, uint8_t udp*/);
ICACHE_FLASH_ATTR sys_config_t* jm_cli_getJmInfo();

//定时检测系统各服务状态，确保系统在条件充许性况下可以正常运行
ICACHE_FLASH_ATTR BOOL jm_cli_main_timer();

ICACHE_FLASH_ATTR timer_check* jm_cli_getCheck();

ICACHE_FLASH_ATTR BOOL jm_cli_isBind();//设备是否已经被绑定

ICACHE_FLASH_ATTR BOOL jm_cli_init();

ICACHE_FLASH_ATTR BOOL jm_cli_isInit();

ICACHE_FLASH_ATTR BOOL  jm_cli_wifiEnable();

//ICACHE_FLASH_ATTR BOOL jm_cli_socketDisconCb(jconn_type connType);

//ICACHE_FLASH_ATTR BOOL _client_socketConedCb(jconn_type connType);

ICACHE_FLASH_ATTR BOOL jm_cli_socketSendTimeoutCb(jconn_type connType);

//ICACHE_FLASH_ATTR BOOL jm_cli_registMessageSender(jm_cli_send_msg_fn sender);

ICACHE_FLASH_ATTR void jm_cli_registReconnect(jm_cli_reconnect_fn fn);

#if JM_RPC_ENABLE==1
ICACHE_FLASH_ATTR BOOL jm_cli_isLogin();

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_onMessage(jm_msg_t *msg);

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_sendMessage(jm_msg_t *msg);

ICACHE_FLASH_ATTR BOOL jm_cli_registMessageHandler(jm_cli_msg_hander_fn msg, sint8_t type);

/**
 */
ICACHE_FLASH_ATTR sint64_t jm_cli_invokeRpc(sint32_t mcode, jm_elist_t *params,
		jm_cli_rpc_callback_fn callback, void *cbArgs);

#if JM_LOGIN_ENABLE==1
/**
 */
ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_login();

/**
 */
ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_logout();

#endif

#endif //#if JM_RPC_ENABLE==1

#if JM_PS_ENABLE==1
ICACHE_FLASH_ATTR jm_cli_p2p_msg_sender_fn jm_cli_getUdpSenderByName(char *channelName);



ICACHE_FLASH_ATTR sint32_t jm_cli_publishEmap2Device(jm_emap_t *ps, sint8_t type, char *host, uint16_t port);

ICACHE_FLASH_ATTR void jm_cli_initPubsubItem(jm_pubsub_item_t *item, uint8_t dataType);

ICACHE_FLASH_ATTR void psitem_pubsubItemParseBin(jm_buf_t *buf, jm_msg_t *msg);

ICACHE_FLASH_ATTR jm_emap_t * jm_cli_topicForwardExtra(char *topic);

ICACHE_FLASH_ATTR jm_emap_t* jm_cli_getUdpExtraByHost(char *host, sint32_t port, BOOL isUdp);

ICACHE_FLASH_ATTR jm_emap_t* jm_cli_getUdpExtra(jm_pubsub_item_t *it);

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_respExtra(jm_pubsub_item_t *it, jm_emap_t *payload);

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_respError(jm_pubsub_item_t *it, sint8_t code, char *msg);

ICACHE_FLASH_ATTR jm_buf_t *_c_serialPsItem(jm_pubsub_item_t *it);

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_publishStrBufItem(char *topic, sint8_t type, jm_buf_t *buf,
		jm_emap_t *ps);

/**
 */
ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_publishStrItem(char *topic, sint8_t type, char *content, jm_emap_t *ps);

/**
 *
 */
ICACHE_FLASH_ATTR jm_cli_send_msg_result_t jm_cli_publishPubsubItem(jm_pubsub_item_t *item, jm_emap_t *ps);


/**
 */
ICACHE_FLASH_ATTR BOOL jm_cli_subscribe(char *topic, jm_cli_PubsubListenerFn listener, sint8_t type, BOOL p2p);

/**
 *
 */
ICACHE_FLASH_ATTR BOOL jm_cli_subscribeByType(jm_cli_PubsubListenerFn listener, sint8_t type,BOOL p2p);

/**
 */
ICACHE_FLASH_ATTR BOOL jm_cli_unsubscribe(char *topic, jm_cli_PubsubListenerFn listener);

#endif // JM_PS_ENABLE == 1





/*************************************KEY Value Storage begin***********************************************/
#if JM_KE_ENABLE == 1
ICACHE_FLASH_ATTR BOOL kv_delete(char *name, jm_cli_rpc_callback_fn cb);
ICACHE_FLASH_ATTR BOOL kv_update(char *name, char *desc, void *val, sint8_t type, jm_cli_rpc_callback_fn cb);
ICACHE_FLASH_ATTR BOOL kv_get(char *name, jm_cli_rpc_callback_fn cb);
ICACHE_FLASH_ATTR BOOL kv_add(char *name, void *val, char *desc, sint8_t type, jm_cli_rpc_callback_fn cb);
#endif

/*************************************KEY Value Storage end***********************************************/


/*************************************Client extra begin***********************************************/
ICACHE_FLASH_ATTR BOOL jm_cli_encodeExtra(jm_buf_t *b, jm_msg_extra_data_t *extras, sint8_t type);
ICACHE_FLASH_ATTR void* jm_cli_decodeExtra(jm_buf_t *b, sint8_t type);
ICACHE_FLASH_ATTR void* jm_cli_getExtraByType(jm_msg_extra_data_t *extras, sint8_t type);
/*************************************Client extra end***********************************************/

/*************************************Net listener start***********************************************/
ICACHE_FLASH_ATTR net_event_t jm_cli_netStatus();
/*************************************Net listener end***********************************************/

#ifdef __cplusplus
}
#endif

#endif /* _JM_CLIENT_H_ */
