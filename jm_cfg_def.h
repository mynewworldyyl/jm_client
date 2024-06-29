#ifndef JMICRO_cfg_def_H_
#define JMICRO_cfg_def_H_
#include "jm_cons.h"
#include "c_types.h"

//必须4字节对齐读写
typedef struct{
    uint8_t inited;//1 是否已经初始化过系统，初次启动，此值为0，否则为1
    uint8_t deviceTypeName[32]; // 34 设备类型名称，比如 视频监控，电灯， 洗衣机等
    uint8_t deviceId[13]; // 47
    uint8_t jlogEnable;// 71 启用JLOG日志
    uint8_t slogEnable;// 72 启用串口日志
    uint8_t logEnable;// 73 启用日志
    uint8_t deviceRole;// 74 是否主控机
    uint8_t devStatus; //75    0:"未绑定", 1:"已绑定", 2:"已绑定", 3:"已解绑", 4:"冻结"
    uint8_t storeGpioStatus; //76
    uint8_t key[33];//109 设备密钥，长度为32个字符，加一个结束字符
    uint8_t invokeCode[33]; //142 设备登录授权码 长度为32个字符，加一个结束字符
    sint32_t clientId; //146 设备绑定的账号ID
    sint8_t grpId; //147 设备所属组ID
	uint8_t jmHost[16];//168 JM后台网关IP
    uint16_t jmPort;// 170 JM后台网关端口
	
#ifdef JM_STM32
	uint8_t padding[2]; //148  保证4字节对齐
#endif	

#ifndef JM_STM32
	uint8_t jlogHost[16];// 68 接收日志的主机
    uint16_t jlogPort;// 70 接收日志主机端口
    uint8_t useUdp; // 52 是否使用UDP与JM平台通信，1：是，0：否，使用UDP后，不再使用
    uint8_t ap_ssid[32];// 34 热点 ssid
    uint8_t ap_pwd[32];// 66 热点 密码
    uint8_t sta_ssid[32];// 98 连接的wifi ssid
    uint8_t sta_pwd[32];// 130 wifi 密码
    uint8_t sta_type;// 131
    uint8_t sta_useStore;// 132 使用定制WIFI账号密码，而不是系统记录的账号密码
    uint16_t devicePort;//134 本地开启的端口
    uint8_t jm_hostNetorder[8];//142 JM后台网关IP,网络字节序形式
	uint8_t jmDomain[64];//JM后台网关域名
	uint8_t padding[4]; //392  保证4字节对齐
#endif

	sint32_t actId; //396 设备绑定的账号ID
    //uint32_t gpioStatuEnable; //gpioStatus对应的引脚状态是否有效，1为有空，0为无效
    //uint32_t gpioStatus; //记录ctrl_remote_ctrlGpio控制的GPIO状态，系统重启后，可以恢复引状态

} sys_config_t;

#endif /* JMICRO_cfg_def_H_ */
