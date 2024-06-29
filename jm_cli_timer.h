
ICACHE_FLASH_ATTR BOOL jm_cli_registTimerChecker(char *key, jm_cli_timer_check_fn checker,
	 uint16_t interval, uint16_t delay, BOOL oneTime){
    if(jm_hashmap_exist(timerCheckers, key)) {
        JM_CLI_ERROR("checker checker exist: %s\n",key);
        return false;
    }

	#if JM_ENV==0
	jm_cli_timer_check_t *c = jm_utils_mallocWithError(sizeof(jm_cli_timer_check_t), 0,"ipa");
	#else
	jm_cli_timer_check_t *c = jm_utils_malloc(sizeof(jm_cli_timer_check_t), 0);
	#endif
	
	c->fn = checker;
	c->interval = interval;
	c->key = key;
	c->oneTime = oneTime;
	//c->del = false;
	c->lastCallTime = jm_cli_getSysTime()/1000 + delay;
	JM_CLI_DEBUG("key=%s, lastCallTime: %u c->interval=%u\n",key, c->lastCallTime,c->interval);

    if(!jm_hashmap_put(timerCheckers, key, c)){
        JM_CLI_ERROR("checker regist fail: %s\n",key);
		jm_cli_getJmm()->jm_free_fn(c,sizeof(jm_cli_timer_check_t));
        return false;
    }

    return true;
}

ICACHE_FLASH_ATTR BOOL jm_cli_unregistTimerChecker(char *key){

	jm_cli_timer_check_t *c = jm_hashmap_get(timerCheckers,key);

    if(!c) {
        return true;
    }

    if(!jm_hashmap_remove(timerCheckers,key)){
        JM_CLI_ERROR("uchecker unregist fail: %s",key);
        return false;
    }

	jm_cli_getJmm()->jm_free_fn(c,sizeof(jm_cli_timer_check_t));
    return true;
}

ICACHE_FLASH_ATTR static void _c_invokeTimerChecker(){

	if(jm_cli_getSysRunTime() < 10) return;

    if(timerCheckers->size <= 0) return;

    jm_hash_map_iterator_t ite = {timerCheckers, NULL, -1, timerCheckers->ver};

    char *k = NULL;

	uint32_t curTime = jm_cli_getSysTime()/1000;

	//ESP_LOGW(TAG,"_c_invokeTimerChecker B");
    while((k = jm_hashmap_iteNext(&ite)) != NULL) {
		//JM_CLI_DEBUG("invoke B %s",k);
		jm_cli_timer_check_t *c = jm_hashmap_get(timerCheckers, k);
		int32_t inte = curTime - c->lastCallTime;
        if(c != NULL && (inte >= c->interval)) {
			/*JM_CLI_DEBUG("invoke %s, curTime=%u, c->lastCallTime=%u,c->interval=%u, curTime - c->lastCallTime=%d\n", k,
				curTime, c->lastCallTime,c->interval,inte);*/
        	//int32_t st = jm_cli_getSysTime();
			c->fn();
			//JM_CLI_DEBUG("const t=%dMS\n", jm_cli_getSysTime()-st);
			if(c->oneTime) {
				//只执行一次的定时任务
				jm_hashmap_iteRemove(&ite);
			} else {
				c->lastCallTime = curTime;
			}
			//JM_CLI_DEBUG("invoke E");
        }
    }
	//ESP_LOGW(TAG,"_c_invokeTimerChecker E");
    return;
}

ICACHE_FLASH_ATTR BOOL jm_cli_main_timer(void *arg) {
	_c_invokeTimerChecker();
	return true;
}
