#include "jm.h" 
#include "jm_port.h" 

#if JM_STM32==1

static void user_postEvent(uint8_t evt, uint16_t subType, void *data, uint8_t flag);

static jm_hashmap_t *eventListeners = NULL;

/**
队头为0， 队尾为 N-1，
*/
static jm_event_t taskQueue[TASK_QUEUE_SIZE] = {0};
static uint8_t taskHeaderIdx = 0;
static uint8_t taskTailIdx = 0;
//任务队列是否为空，当taskHeaderIdx==taskTailIdx，通过此值判断是空还是满
//当taskHeaderIdx!=taskTailIdx，时，此值无效
static BOOL taskEmpty = true;

static uint64 msEque = 0; //从系统启动到现在的时间值，单位毫秒

static jm_eventHandler_fn jmUserEventRunner = NULL;

static ICACHE_FLASH_ATTR uint32_t _jm_getTime() {
	return msEque;//毫秒
}

ICACHE_FLASH_ATTR void jm_ElseMs(uint32_t mseq) {
	msEque += mseq;//毫秒
	if(msEque % 1000 == 0) {
		jm_cli_getJmm()->jm_postEvent(TASK_APP_JM_CHECKER, 0, NULL, JM_EVENT_FLAG_DEFAULT);
	}
}

void jm_setUserEventRunner(jm_eventHandler_fn eh) {
	jmUserEventRunner = eh;
}

static void user_postEvent(uint8_t evt, uint16_t subType, void *data, uint8_t flag) {

	if(taskHeaderIdx==taskTailIdx && !taskEmpty) {
		JM_MAIN_DEBUG("user_postEvent task queue full taskHeaderIdx=%d taskTailIdx=%d taskEmpty=%d\n",
			taskHeaderIdx, taskTailIdx, taskEmpty);
		return;
	}
	
	if(evt != 3 && TASK_APP_RX_DATA != evt) {
		JM_MAIN_DEBUG("user_postEvent B evt=%u st=%u t=%d h=%d\n", evt,  subType, taskTailIdx, taskHeaderIdx);
	}

	jm_event_t *e = taskQueue + taskTailIdx;
	
	e->type = evt;
	//e->needFree = true;
	e->subType = subType;
	e->data = data;
	e->flag = flag;
	
	taskTailIdx = (taskTailIdx+1)%TASK_QUEUE_SIZE;
	if(taskHeaderIdx==taskTailIdx) {
		//标识队列满
		taskEmpty = false;
	}
	
	if(evt != 3 && TASK_APP_RX_DATA != evt) {
		JM_MAIN_DEBUG("user_postEvent E evt=%u st=%u t=%d h=%d\n",evt, subType, taskTailIdx, taskHeaderIdx);
	}
	
}

static jm_event_t* _jm_user_popEvent() {

	if(taskHeaderIdx==taskTailIdx && taskEmpty) {
		return NULL;
	}

	jm_event_t *e = taskQueue + taskHeaderIdx;
	taskHeaderIdx = (taskHeaderIdx+1)%TASK_QUEUE_SIZE;
	if(taskHeaderIdx==taskTailIdx) {
		//标识队列空
		taskEmpty = true;
	}
	
	if(e->type != 3) {
		JM_MAIN_DEBUG("popevent B evt=%u st=%u t=%d h=%d\n",e->type, e->subType, taskTailIdx, taskHeaderIdx);
	}
	return e;
}

/*********************************事件监听器开始***************************************/

ICACHE_FLASH_ATTR static BOOL _jm_regEventListener(uint8_t k, jm_evnet_listener lis){
	// JM_MAIN_DEBUG("_jm_regEventListener B",key,lis);
	uint32_t key = k;
	jm_elist_t *list = jm_hashmap_get(eventListeners, (void*)key);
	if(list == NULL) {
		//JM_MAIN_DEBUG("_jm_regEventListener create list");
		list = jm_elist_create();
		jm_hashmap_put(eventListeners, (void*)key, list);
		//JM_MAIN_DEBUG("_jm_regEventListener 1 list=%p", list);
		//JM_MAIN_DEBUG("_jm_regEventListener list._hd=%p NULL=%d", list->_hd, list->_hd == NULL);
	}

   if(jm_elist_exist(list, lis, PREFIX_TYPE_PROXY)) {
	   JM_MAIN_ERROR("reg evt lis: %d",key);
       return true;
   }

    //JM_MAIN_DEBUG("_jm_regEventListener 2");
	if(!jm_elist_add(list, lis, PREFIX_TYPE_PROXY, false)) {
		JM_MAIN_ERROR("reg evt fail: %d",key);
       return true;
	}

	//JM_MAIN_DEBUG("_jm_regEventListener E %d",key);
   return true;
}

ICACHE_FLASH_ATTR static BOOL _jm_unregEventListener(uint8_t key, jm_evnet_listener lis){

	jm_elist_t *list = jm_hashmap_get(eventListeners, (void*)key);
	if(list == NULL) {
		return true;
	}

   if(!jm_elist_exist(list,lis,PREFIX_TYPE_PROXY)) {
       return true;
   }

   if(!jm_elist_remove(list,lis,PREFIX_TYPE_PROXY)){
       JM_MAIN_DEBUG("unreg evt unr F: %d",key);
       return false;
   }

   return true;
}

ICACHE_FLASH_ATTR static void _jm_invokeEventListener(jm_event_t *evt){

   if(eventListeners->size <= 0) return;

	jm_elist_t *list = jm_hashmap_get(eventListeners, (void*)evt->type);
	if(list == NULL || jm_elist_size(list) == 0) {
		return;
	}

	jm_elist_iterator_t ite = {list->_hd};
	jm_evnet_listener fn = NULL;
	while((fn = jm_elist_next(&ite)) != NULL) {
		  fn(evt);
	}

	return;
}

/**********************************事件监听器结束**************************************/



ICACHE_FLASH_ATTR static void _doChecker(){

	//JM_MAIN_DEBUG("start_timer_wrapper\n");
#if JLOG_ENABLE == 1
	if(jm_cli_getJmInfo()->jlogEnable) {
		JM_MAIN_DEBUG("%s - %d - %s -%s Test jinfo device log\n", __FILE__, __LINE__, __DATE__, __TIME__);
		//_jlog_checkLogBuf();
	}
#endif

#if JM_TIMER_ENABLE==1
	jm_cli_main_timer();
#endif // JM_TIMER_ENABLE
	
}

ICACHE_FLASH_ATTR static void _eventHandler(jm_event_t *jevent) {

    if(jevent->type != 3) {
		SINFO("_eventHandler B evt=%u st=%u h=%d\n",jevent->type, jevent->subType, taskHeaderIdx);
	}

  switch (jevent->type) {
	  case TASK_APP_MAINLOOP:
		//mainLoop();
		break;

	  case TASK_APP_WIFI_GOT_IP:
	  {
		  /*
		  #if JM_ENV==1
		  ip_addr_t *addr = jm_utils_mallocWithError(sizeof(ip_addr_t), 0,"ipa");
		  #else
		  ip_addr_t *addr = jm_utils_malloc(sizeof(ip_addr_t), 0,);
		  #endif
		 if(jm_resovle_domain_ip(jm_cli_getJmInfo()->jmDomain, addr, _jm_dns_found0) == -1) {
			 //_doChecker();
		 }
		  */
	  }
     break;

	case TASK_APP_DNS_GO_IP:
	  //_doChecker();
	  //jm_udpserver_check();
      break;

	case TASK_APP_WIFI:
		JM_MAIN_DEBUG("Invoke wifi_init B\n");
		//wifi_init();
		//JM_MAIN_DEBUG("Invoke wifi_init E\n");
		break;

  case TASK_APP_JM_CHECKER:
	  //JM_MAIN_DEBUG("TASK_APP_JM_CHECKER B\n");
	  //os_timer_disarm(&worker);//先停掉定时器，防止重入
	  // start_timer_wrapper(0);
	  // os_timer_arm(&worker, WORK_INTERVAL, 1);
 /*
	  curSeconds++;
	  if(CHECK_INTERVAL_IN_SECONDS <= curSeconds) {
		  curSeconds=0;
		  mem_printMemInfo();
	  }
*/
	  _doChecker();

      break;

#if NET_PROXY
  case TASK_APP_NETPROXY:
	  JM_MAIN_DEBUG("n proxy\n");
	  jm_netproxy_recvTxData();
    break;
#endif

#if JM_IR >-1
  case TASK_APP_IR_DATA:
 	  JM_MAIN_DEBUG("_eventHandler forwar ir msg\n");
 	 // _ir_onData();
     break;
#endif

  case TASK_APP_RESTART_SYSTEM:
	  //经过jevent->subType秒后，重启系统
	  //ml_broadcastShutdownMsg();
	  //jm_cli_registTimerChecker("_rssys", user_restartSystem, 1, jevent->subType, false);
	  break;

  case TASK_APP_SAVE_CFG:
	//cfg_save();
	if(jevent->subType == 1) {
		jm_cli_getJmm()->jm_delay(100);
		JM_MAIN_DEBUG("ctrl restart");
		//user_restartSystem();
	}
	break;

  case TASK_APP_UDP:
	//通知ML模块UDP可用，优先发送上线广播
	//jm_cli_getJmm()->jm_postEvent(TASK_APP_ML, JM_ML_EVENT_SUBTYPE_UPDCONNECTED, NULL, JM_EVENT_FLAG_DEFAULT);
  	break;

  //Handle the unknown event type.
  //default:
  //  JM_MAIN_DEBUG("eventHandler: Unknown task type: %d\n",jevent->type);
   /* {
     // Get the data from the UART RX buffer.  If the size of the returned data is
     // not zero, then push it onto the Espruino processing queue for characters.
	 char pBuffer[100];
	 int size = getRXBuffer(pBuffer, sizeof(pBuffer));
	 if (size > 0) {
		 pBuffer[size] = '\0';
		 JM_MAIN_DEBUG("Rec: %s",pBuffer);
	 }
   }*/

    break;
  }
  
  if(jmUserEventRunner) {
	  jmUserEventRunner(jevent);
  }

  	 //JM_MAIN_DEBUG("_eventHandler2\n");
  	_jm_invokeEventListener(jevent);

  	//JM_MAIN_DEBUG("_eventHandler3\n");
	if(jevent->data && ((jevent->flag & JM_EVENT_FLAG_FREE_DATA))) {
		jm_cli_getJmm()->jm_free_fn(jevent->data,0);
	}

	//JM_MAIN_DEBUG("_eventHandler4\n");
	//jm_cli_getJmm()->jm_free_fn(jevent,sizeof(jm_event_t));
	//jevent = NULL;

	//JM_MAIN_DEBUG("_eventHandler5\n");
    //JM_MAIN_DEBUG("_eventHandler finish process event: %ld\n",pEvent->sig);
}

ICACHE_FLASH_ATTR void jm_runEvent(){
	jm_event_t *e = _jm_user_popEvent();
	if(e != NULL) {
		//JM_MAIN_DEBUG("pe=%d\n",e->type);
		_eventHandler(e);
	}
}

ICACHE_FLASH_ATTR void jm_runner_setEventParam(jm_mem_op *jmm){
	jmm->jm_postEvent = user_postEvent;
	jmm->jm_regEventListener = _jm_regEventListener;
	jmm->jm_unregEventListener = _jm_unregEventListener;
	jm_cli_setSysTimeFn(_jm_getTime);
}

ICACHE_FLASH_ATTR void jm_runner_init(){
	//系统事件监听器
	eventListeners = jm_hashmap_create(5,PREFIX_TYPE_SHORTT);
	eventListeners->needFreeVal = false;
	
}

#endif //JM_STM32==1
