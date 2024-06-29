#if JM_KE_ENABLE == 1
ICACHE_FLASH_ATTR BOOL _kv_getStrVal(uint8_t type, void *val, char *str) {

	if(val == NULL) {
		JM_CLI_ERROR("kv NULL val\n");
		return false;
	}

	if(type== PREFIX_TYPE_BYTE){
		jm_itoa(*((sint8_t*)val), str);
	}else if(type==PREFIX_TYPE_SHORTT) {
		jm_itoa(*((sint16_t*)val), str);
	}else if(type==PREFIX_TYPE_INT) {
		jm_itoa(*((sint32_t*)val), str);
	}else if(type==PREFIX_TYPE_LONG) {
		jm_itoa(*((sint64_t*)val), str);
	}/*else if(type==PREFIX_TYPE_STRINGG) {
		vstr = (char*)val;
	}*/else if(type==PREFIX_TYPE_BOOLEAN) {
		if(jm_strcmp("0",val)) {
			str[0] = '0';
		}else {
			str[0] = '1';
		}
		str[1] = '\0';
	}else if(type==PREFIX_TYPE_CHAR) {
		char *strp = (char*)val;
		str[0] = *strp;
		str[1] = '\0';
	} else {
		JM_CLI_ERROR("kv NS type: %d!\n",type);
		return false;
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL kv_add(char *name, void *val, char *desc, sint8_t type, jm_cli_rpc_callback_fn cb) {
	if(!jm_cli_isLogin()) {
		JM_CLI_ERROR("kv n login!\n");
		return false;
	}

	if(name == NULL || jm_strlen(name) == 0) {
		JM_CLI_ERROR("kv name N!\n");
		return false;
	}

	jm_elist_t *ps = jm_elist_create();
	if(ps == NULL) {
		JM_CLI_ERROR("kv mo!\n");
		goto jmerror;
	}

	jm_elist_add(ps, name, PREFIX_TYPE_STRINGG,false);
	char *vstr;
	//全部值传为字符串存储
	char str[9];

	if(type == PREFIX_TYPE_STRINGG) {
		vstr = (char*)val;
	} else if(_kv_getStrVal(type, val, str)) {
		vstr = str;
	} else {
		JM_CLI_ERROR("kv n support: %d!\n",type);
		goto jmerror;
	}

	jm_elist_add(ps,  vstr, PREFIX_TYPE_STRINGG, false);
	jm_elist_add(ps,  desc, PREFIX_TYPE_STRINGG, false);
	jm_elist_add(ps,  (void*)type, PREFIX_TYPE_BYTE, false);

	sint32_t msgId = jm_cli_invokeRpc(360214249, ps, cb, NULL);

	if(msgId <= 0) {
		JM_CLI_ERROR("kv F =%\n",name);
		goto jmerror;
	}

	jmerror:
		if(ps) jm_elist_release(ps);

	return true;
}

ICACHE_FLASH_ATTR void _kv_getResult(
		jm_emap_t *resultMap, sint32_t code, char *errMsg, void *arg
/*jm_msg_extra_data_t *rr, sint32_t code, char *errMsg, jm_cli_rpc_callback_fn cb*/) {

	jm_cli_rpc_callback_fn cb = (jm_cli_rpc_callback_fn)arg;

	if(code != 0) {
		JM_CLI_ERROR("kv F code:%d: err: %s!\n",code,errMsg);
		cb(NULL, code, errMsg, NULL);
		return;
	}
	jm_emap_t *data = jm_emap_getMap(resultMap, "data");

	sint8_t type = jm_emap_getByte(data,"type");
	char* val = jm_emap_getStr(data,"val");

	if(val == NULL || jm_strlen(val) == 0) {
		cb(NULL, code, errMsg, NULL);
		return;
	}

	jm_msg_extra_data_t *v = jm_extra_sput(NULL,"v", type,false);

	if(type== PREFIX_TYPE_BYTE){
		v->value.s8Val = (sint8_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_SHORTT) {
		v->value.s16Val = (sint16_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_INT) {
		v->value.s32Val = (sint32_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_LONG) {
		v->value.s64Val = (sint64_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_STRINGG) {
		v->value.strVal = (char*)val;
	}else if(type==PREFIX_TYPE_BOOLEAN) {
		v->value.boolVal = jm_strcmp("1",val);
	}else if(type==PREFIX_TYPE_CHAR) {
		v->value.charVal = val[0];
	} else {
		JM_CLI_ERROR("kv n support: %d!\n",type);
		if(v) jm_extra_release(v);
		v = NULL;
	}

    jm_emap_t *rst = jm_emap_create(PREFIX_TYPE_STRINGG);
    if(v) {
        v->neddFreeBytes = true;
        jm_emap_putExtra(rst,v);
        jm_emap_put(rst, "type", (void*)type, PREFIX_TYPE_BYTE,true,false);
    }

	cb(rst, code, errMsg, NULL);
    if(rst) jm_emap_release(rst);
	//if(v) jm_extra_release(v);
}

ICACHE_FLASH_ATTR BOOL kv_get(char *name, jm_cli_rpc_callback_fn cb) {
	if(!jm_cli_isLogin()) {
		JM_CLI_ERROR("kv not login!\n");
		return false;
	}

	if(name == NULL || jm_strlen(name) == 0) {
		JM_CLI_ERROR("kv name N\n");
		return false;
	}

	jm_elist_t *ps = jm_elist_create();
	if(ps == NULL) {
		JM_CLI_ERROR("kv mo!\n");
	}

	jm_elist_add(ps, name, PREFIX_TYPE_STRINGG,false);

	sint32_t msgId = jm_cli_invokeRpc(-1246466186, ps, _kv_getResult, cb);

	jm_elist_release(ps);

	if(msgId <= 0) {
		JM_CLI_ERROR("kv F name=%\n",name);
		return false;
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL kv_update(char *name, char *desc, void *val, sint8_t type, jm_cli_rpc_callback_fn cb) {

	if(!jm_cli_isLogin()) {
		JM_CLI_ERROR("kv not login!\n");
		return false;
	}

	if(name == NULL || jm_strlen(name) == 0) {
		JM_CLI_ERROR("kv name is N!\n");
		return false;
	}

	jm_elist_t *ps = jm_elist_create();
	if(ps == NULL) {
		JM_CLI_ERROR("kv mo!\n");
		goto jmerror;
	}

	jm_elist_add(ps, name, PREFIX_TYPE_STRINGG,false);

	char *vstr;
	//全部值传为字符串存储
	char str[9];

	if(type == PREFIX_TYPE_STRINGG) {
		vstr = (char*)val;
	} else if(_kv_getStrVal(type, val, str)) {
		vstr = str;
	} else {
		JM_CLI_ERROR("kv not support: %d!\n",type);
		return false;
	}

	jm_elist_add(ps, vstr, PREFIX_TYPE_STRINGG,false);
	jm_elist_add(ps, desc, PREFIX_TYPE_STRINGG,false);

	sint32_t msgId = jm_cli_invokeRpc(247475691, ps, cb, NULL);

	jm_elist_release(ps);

	if(msgId <= 0) {
		JM_CLI_ERROR("kv F name=%s\n",name);
		goto jmerror;
	}

    return true;

	jmerror:
		JM_CLI_ERROR("kv err name=%s\n",name);
		return true;
}

ICACHE_FLASH_ATTR BOOL kv_delete(char *name, jm_cli_rpc_callback_fn cb) {
	if(!jm_cli_isLogin()) {
		JM_CLI_ERROR("kv not login!\n");
		return false;
	}

	if(name == NULL || jm_strlen(name) == 0) {
		JM_CLI_ERROR("kv name N\n");
		return false;
	}

	jm_elist_t *ps = jm_elist_create();
	if(ps == NULL) {
		JM_CLI_ERROR("kv mo!\n");
		return false;
	}

	jm_elist_add(ps, name, PREFIX_TYPE_STRINGG,false);

	sint32_t msgId = jm_cli_invokeRpc(1604442957, ps, cb, NULL);

	jm_elist_release(ps);

	if(msgId <= 0) {
		JM_CLI_ERROR("kv F name=%s\n",name);
		return false;
	}

	return true;
}

#endif //JM_KE_ENABLE