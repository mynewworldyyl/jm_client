/*
 * jm_buffer.c
 *
 *  Created on: 2023锟斤拷4锟斤拷9锟斤拷
 *      Author: yeyulei
 */
 
#ifdef WIN32
#include "stdio.h"
#include "string.h"
#include "test.h"
#endif

#if ESP8266==1
#include "user_interface.h"
#include "osapi.h"
#include "mem.h"

#elif ESP32==1
#include "stdio.h"
#include "string.h"
#include "esp32_adapter.h"

#elif STM32
#include "stm32_adapter.h"
#endif

#include "debug.h"

#if ESP8266==1 || ESP32==1
#include "jm_utils.h"
#endif

#include "jm_stdcimpl.h"

static uint32_t allocSize = 0;//当前已经分配的内存大小

jm_mem_op *jmm;

ICACHE_FLASH_ATTR static void  _bb_set_full(jm_buf_t *buf, BOOL v);

ICACHE_FLASH_ATTR static void  _bb_set_empty(jm_buf_t *buf, BOOL v);

ICACHE_FLASH_ATTR void _bb_setMemOps(jm_mem_op *ops){
    if(ops != NULL) {
        jmm = ops;
    }
}

ICACHE_FLASH_ATTR void jm_buf_print(jm_buf_t *buf, uint32_t startPos, uint32_t endPos){
	if(!buf) {
		JM_BUF_ERROR("No data buff\n");
		return;
	}

	//int len = jm_buf_readable_len(buf);

	if(endPos == 0 || endPos > buf->capacity)  {
		endPos = buf->capacity;
	}

	JM_BUF_DEBUG("jm_buf_print rpos=%d wpos=%d cap=%d startPos=%d endPos=%d f=%d\n",
			buf->rpos, buf->wpos, buf->capacity, startPos, endPos, jm_buf_is_full(buf));

	/*
	char c = 0;
	for(int i = startPos; i < endPos; i++) {
		//c = jm_buf_get_by_index(buf,i);
		JM_BUF_DEBUG("%u,",buf->data[i]);
	}
	JM_BUF_DEBUG("\n");
	*/

}

ICACHE_FLASH_ATTR static BOOL  _bb_is_wrap(jm_buf_t *buf) {
	return buf->wrap_buf != NULL;
}

ICACHE_FLASH_ATTR void jm_buf_clear(jm_buf_t *buf) {
	 buf->status = BB_EMPTY; //锟斤拷始状态锟角匡拷
	 buf->rpos = 0;
	 buf->wpos = 0;
	 buf->wrap_buf  = NULL;
	 buf->rw_flag = true;
	 buf->rmark = -1;
	 buf->rmark_status = 0;
}

ICACHE_FLASH_ATTR static BOOL  jm_buf_buffer_init0(jm_buf_t *buf, uint8_t *data, uint16_t cap) {
	 //JM_BUF_DEBUG("jm_buf_buffer_init0 cap: %u\n",cap);
	 buf->capacity = cap;
	 //JM_BUF_DEBUG("jm_buf_buffer_init0 23\n");
	 jm_buf_clear(buf);
	 if(data) {
		 buf->data = data;
		 //数据由申请者负责释放
		 buf->needReleaseData = false;
		 return true;
	 }

	_bb_set_empty(buf,true);
	_bb_set_full(buf,false);

	 buf->needReleaseData = true;
	 if(cap == 0) return true;//只锟斤拷锟斤拷一buffer,之锟斤拷锟劫碉拷锟斤拷锟斤拷始锟斤拷

	 if(!data) {
         allocSize += cap;
		 
		 buf->data = (uint8_t*)jm_utils_mallocWithError(cap, PREFIX_TYPE_JM_BUFFER_ARRAY, "jm_buf_buffer_init0");
#if ESP8266==0
		 //assert(buf->data);
#endif
		 if(buf->data == NULL) {
			 JM_BUF_ERROR("F alloc=%u cap=%d\n",allocSize,cap);
			 return false;
		 }
	 }
	 //JM_BUF_DEBUG("jm_buf_buffer_init0 3\n");
	 return true;
}

ICACHE_FLASH_ATTR jm_buf_t*  jm_buf_wrapArrayReadBuf(uint8_t *data, int dataLenInBytes) {
	if (dataLenInBytes < 0) {
		JM_BUF_ERROR("buf err cap: %d\n", dataLenInBytes);
		return NULL;
	}

	jm_buf_t *bb = jm_utils_mallocWithError(sizeof(struct _jm_buffer),PREFIX_TYPE_JM_BUFFER,"jm_buf_create");
	if(bb != NULL) {
		jm_buf_buffer_init0(bb, data, dataLenInBytes);
		_bb_set_full(bb, true);
		_bb_set_empty(bb, false);
	}
	//JM_BUF_DEBUG("buf is full=%d", jm_buf_is_full(bb));

	return bb;
}

ICACHE_FLASH_ATTR jm_buf_t*  jm_buf_create(int capacity) {
	if (capacity < 0) {
		JM_BUF_ERROR("buf err cap: %d", capacity);
		return NULL;
	}
	//JM_BUF_DEBUG("jm_buf_create 1\n");
    //MINFO("Alloc jm_buf_create %u\n",sizeof(struct _jm_buffer));
	jm_buf_t * bb = jm_utils_mallocWithError(sizeof(struct _jm_buffer),PREFIX_TYPE_JM_BUFFER,"jm_buf_create");
	//JM_BUF_DEBUG("jm_buf_create 2\n");
	if(bb == NULL) {
		JM_BUF_ERROR("F c buf\n");
		return NULL;
	}
	
	if(jm_buf_buffer_init0(bb, NULL, capacity)) {
		return bb;
	}
	//JM_BUF_DEBUG("jm_buf_create 3\n");
	jm_buf_release(bb);
	return NULL;
}

ICACHE_FLASH_ATTR jm_buf_t*  jm_buf_buffer_wrap(jm_buf_t *src,  uint16_t cap, BOOL rw_flag) {

	uint16_t slen;
	if(rw_flag) {
		//只读
		slen = jm_buf_readable_len(src);
	} else {
		//只写
		slen = jm_buf_writeable_len(src);
	}

	if(slen <= 0 || slen < cap) return NULL;//锟斤拷锟姐够锟缴讹拷锟斤拷锟捷伙拷锟叫达拷占锟�

	jm_buf_t *dest = jm_buf_create(0);
	dest->wrap_buf = src;
	dest->rw_flag = rw_flag;
	dest->capacity = cap;
	//源BUF锟缴讹拷锟斤拷锟捷达拷锟斤拷锟斤拷要锟斤拷锟斤拷
	dest->rpos = 0;
	dest->wpos = 0;
	dest->data = 0;
	dest->status = 0;

	return dest;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_reset(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) return false;//锟斤拷wrap锟斤拷Buf锟斤拷效
	 buf->status = BB_EMPTY; //锟斤拷始状态锟角匡拷
	 buf->rpos = 0;
	 buf->wpos = 0;
	 return true;
}

//锟斤拷录锟斤拷前锟斤拷位锟斤拷
ICACHE_FLASH_ATTR void  jm_buf_rmark(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) {
		 jm_buf_rmark(buf->wrap_buf);
	} else {
		buf->rmark = buf->rpos;
		buf->rmark_status = buf->status;
	}
}

ICACHE_FLASH_ATTR BOOL  jm_buf_rmark_reset(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) {
		return jm_buf_rmark_reset(buf->wrap_buf);
	} else {
		if(buf->rmark <= -1) return false;//锟斤拷效状态

		buf->rpos = buf->rmark;
		buf->status = buf->rmark_status;

		buf->rmark = -1;
		buf->rmark_status = 0;

		return true;
	}

}

ICACHE_FLASH_ATTR BOOL  jm_buf_set_rpos(jm_buf_t *buf, uint16_t rpos) {
	if(_bb_is_wrap(buf)) return false;//锟斤拷wrap锟斤拷Buf锟斤拷效
	buf->rpos = rpos;
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_set_wpos(jm_buf_t *buf, uint16_t wpos) {
	if(_bb_is_wrap(buf)) return false;//锟斤拷wrap锟斤拷Buf锟斤拷效
	buf->wpos = wpos;
	return true;
}

//锟斤拷录锟斤拷前锟斤拷位锟斤拷
ICACHE_FLASH_ATTR uint16_t  jm_buf_get_rpos(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) return buf->wrap_buf->rpos;
	else return buf->rpos;
}

ICACHE_FLASH_ATTR uint16_t  jm_buf_get_wpos(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) return buf->wrap_buf->wpos;
	else return buf->wpos;
}

ICACHE_FLASH_ATTR static void  _bb_set_full(jm_buf_t *buf, BOOL v) {
	if(_bb_is_wrap(buf)) return;
	if(v) {
		buf->status |= BB_FULL;
	} else {
		buf->status &= ~BB_FULL;
	}
	//JM_BUF_DEBUG("_bb_set_full status=%u",buf->status);
}

ICACHE_FLASH_ATTR static void  _bb_set_empty(jm_buf_t *buf, BOOL v) {
	if(_bb_is_wrap(buf)) return;
	if(v) {
		buf->status |= BB_EMPTY;
	} else {
		buf->status &= ~BB_EMPTY;
	}
	//JM_BUF_DEBUG("_bb_set_empty status=%u",buf->status);
}

ICACHE_FLASH_ATTR BOOL  jm_buf_is_full(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//只锟斤拷
			return buf->capacity == jm_buf_readable_len(buf->wrap_buf);
		} else {
			//只写
			return buf->capacity == jm_buf_writeable_len(buf->wrap_buf);
		}
	}
	return  (buf->rpos == buf->wpos) && (buf->status & BB_FULL);
}

ICACHE_FLASH_ATTR BOOL  jm_buf_is_empty(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//只锟斤拷
			return 0 == jm_buf_readable_len(buf->wrap_buf);
		} else {
			//只写
			return 0 == jm_buf_writeable_len(buf->wrap_buf);
		}
	}
	return (buf->rpos == buf->wpos) && (buf->status & BB_EMPTY);
}

ICACHE_FLASH_ATTR static uint8_t  _bb_get_u8(jm_buf_t *buf) {

	if(_bb_is_wrap(buf)) {
		buf->rpos++;//锟斤拷录锟斤拷前锟斤拷装锟窖讹拷锟斤拷锟斤拷锟斤拷
		return _bb_get_u8(buf->wrap_buf);
	}

	if(jm_buf_is_full(buf)) {
		_bb_set_full(buf,false);
	}

	uint8_t v = buf->data[buf->rpos];
	buf->rpos = (buf->rpos + 1) % buf->capacity;

	if(buf->rpos == buf->wpos) {
		_bb_set_empty(buf,true);
	}
	return v;
}

ICACHE_FLASH_ATTR uint16_t  jm_buf_readable_len(jm_buf_t *buf) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			return buf->capacity - buf->rpos;
		}else {
			return 0;
		}
	}

	if(buf->wpos == buf->rpos ) {
		if(jm_buf_is_full(buf)) {
			return buf->capacity;
		}else {
			return 0;
		}
	}else if(buf->wpos > buf->rpos ) {
		return buf->wpos - buf->rpos;
	}else {
		return buf->capacity - (buf->rpos - buf->wpos);
	}
}

ICACHE_FLASH_ATTR uint16_t  jm_buf_writeable_len(jm_buf_t *buf) {
	//JM_BUF_DEBUG("jm_buf_writeable_len wpos=%u rpos=%u capacity=%u",buf->wpos,buf->rpos,buf->capacity);
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//JM_BUF_DEBUG("jm_buf_writeable_len0 readonly");
			return 0;
		}else {
			return buf->capacity - buf->wpos;
		}
	}

	if(buf->wpos == buf->rpos ) {
		if(jm_buf_is_empty(buf)) {
			return buf->capacity;
		} else {
			//JM_BUF_DEBUG("jm_buf_writeable_len full");
			return 0;
		}
	}else if(buf->wpos > buf->rpos ) {
		return buf->capacity - (buf->wpos - buf->rpos) ;
	}else {
		return buf->rpos - buf->wpos;
	}
}

ICACHE_FLASH_ATTR void  jm_buf_release(jm_buf_t *buf) {
	if (buf == NULL) {
		return;
	}

	if(!_bb_is_wrap(buf) && buf->needReleaseData) {
        //非封装类Buf，释放自身的数据
		if(buf->data) {
            allocSize -= buf->capacity;
            JM_BUF_DEBUG("buf als=%u, rlsize=%d\n", allocSize, buf->capacity);
            jmm->jm_free_fn(buf->data, buf->capacity);
			buf->data = NULL;
		}
	}

    jmm->jm_free_fn(buf, sizeof(jm_buf_t));
}

ICACHE_FLASH_ATTR BOOL  jm_buf_move_forward(jm_buf_t *buf, uint16_t forwarnCnt) {

	if(jm_buf_is_full(buf)) {
		_bb_set_full(buf,false);
	}

	buf->rpos = (buf->rpos+forwarnCnt) % buf->capacity;

	if(buf->rpos == buf->wpos) {
		_bb_set_empty(buf,true);
	}

	return true;
}

ICACHE_FLASH_ATTR static BOOL  jm_buf_check_read_len(jm_buf_t *buf, uint16_t len) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			return jm_buf_readable_len(buf) >= len;
		} else {
			return false;
		}
	}

	if(jm_buf_is_empty(buf)) {
		JM_BUF_ERROR("buf empty rpos: %d, wpos:%d\n", buf->rpos, buf->wpos);
		return false;
	}

	if(jm_buf_readable_len(buf) < len) {
		JM_BUF_ERROR("bufr err len: %d, nl: %d", jm_buf_readable_len(buf),len);
		return false;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_u8(jm_buf_t *buf, uint8_t *rst) {
	if(!jm_buf_check_read_len(buf,1)) {
		JM_BUF_ERROR("bufr8 rpos: %d", buf->rpos);
		return false;
	}
	*rst = _bb_get_u8(buf);
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_s8(jm_buf_t *buf, int8_t *rst) {
	if(!jm_buf_check_read_len(buf,1)) {
		JM_BUF_ERROR("bufrs8 rpos: %d", buf->rpos);
		return false;
	}
	*rst = (int8_t)_bb_get_u8(buf);
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_bool(jm_buf_t *buf, BOOL *rst) {
	if(!jm_buf_check_read_len(buf,1)) {
		JM_BUF_ERROR("bufrb rpos: %d", buf->rpos);
		return false;
	}
	*rst = _bb_get_u8(buf) == 0 ? false : true;
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_char(jm_buf_t *buf, char *rst) {
	if(!jm_buf_check_read_len(buf,1)) {
		JM_BUF_ERROR("bufrc rpos: %d", buf->rpos);
		return false;
	}

	*rst = _bb_get_u8(buf);
	return true;
}

//锟斤拷指锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
ICACHE_FLASH_ATTR BOOL  jm_buf_get_bytes(jm_buf_t *buf, uint8_t *dest, uint16_t len) {
	if(!jm_buf_check_read_len(buf,len)) {
		JM_BUF_ERROR("bufrbs rpos: %d, nl: %d", buf->rpos,len);
		return false;
	}

	for(;len > 0; len--) {
		*dest = _bb_get_u8(buf);
		dest++;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_chars(jm_buf_t *src, char *dest, uint16_t len) {
	//JM_BUF_DEBUG("bufr 0\n");
	if(!jm_buf_check_read_len(src,len)) {
		JM_BUF_ERROR("bufrcs rpos=%d len=%d", src->rpos, len);
		return false;
	}

	//JM_BUF_DEBUG("bufr 1\n");

	for(;len > 0; len--) {
		jm_buf_get_char(src,dest);
		dest++;
	}
	//JM_BUF_DEBUG("bufr 2\n");
	return true;
}

ICACHE_FLASH_ATTR char* jm_buf_readString(jm_buf_t *buf,int8_t *flag) {

    *flag = JM_SUCCESS;//默认成功
    int32_t len = (int8_t)_bb_get_u8(buf);

    if(len == -1) {
        return NULL;
    }else if(len == 0) {
        return "";
    }

    if(len == MAX_BYTE_VALUE) {
        int16_t sl=0;
        jm_buf_get_s16(buf,&sl);
        if(sl == MAX_SHORT_VALUE) {
            jm_buf_get_s32(buf,&len);
        }else {
            len = sl;
        }
    }

    char *p = jm_utils_mallocStr(len,"jm_buf_readString");

    if(jm_buf_get_chars(buf,p,len)) {
		JM_BUF_ERROR("len=%d s=%s\n",len,p);
        return p;
    } else {
        JM_BUF_ERROR("bufr err: %d",2);
        *flag = 2;
        return NULL;
    }
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_buf(jm_buf_t *buf, jm_buf_t *dest, uint16_t len){
	if(!jm_buf_check_read_len(buf,len)) {
		JM_BUF_ERROR("bufr rpos=%d wpos=%d, nl: %d", buf->rpos, buf->wpos, len);
		return false;
	}

	uint8_t p = 0;
	for(;len > 0; len--) {
		if(!jm_buf_get_u8(buf,&p)){
			JM_BUF_ERROR("jm_buf_get_buf GE rpos=%d len=%d", buf->rpos,len);
			return false;
		}
		if(!jm_buf_put_u8(dest,p)) {
			JM_BUF_ERROR("jm_buf_get_buf PE wpos=%d len=%d", dest->rpos,len);
			return false;
		}
	}
	return true;
}

ICACHE_FLASH_ATTR jm_buf_t*  jm_buf_read_buf(jm_buf_t *buf){

	uint16_t len;
	if(!jm_buf_get_s16(buf,&len)) {
		JM_BUF_ERROR("bufr fail", buf->rpos,len);
		return false;
	}

	if(!jm_buf_check_read_len(buf,len)) {
		JM_BUF_ERROR("bufr 1 rpos: %d, need len: %d", buf->rpos,len);
		return false;
	}

	if(len > 0){
		jm_buf_t *pl = jm_buf_create(len);
		if(!jm_buf_get_buf(buf,pl,len)) {
			JM_BUF_ERROR("bufr 2 fail");
            jm_buf_release(pl);
			return NULL;
		}
		return pl;
	}
	return NULL;
}

ICACHE_FLASH_ATTR char* jm_buf_read_chars(jm_buf_t *buf){

	uint16_t len;
	if(!jm_buf_get_s16(buf,&len)) {
		JM_BUF_ERROR("bufr 1 fail");
		return false;
	}

	if(!jm_buf_check_read_len(buf,len)) {
		JM_BUF_ERROR("bufr 2 rpos: %d, nl: %d\n", buf->rpos,len);
		return false;
	}

	if(len > 0){
        MINFO("bufr ma %u\n",len);
		char *cs = jm_utils_mallocStr(len,"jm_buf_read_chars");
		if(!jm_buf_get_chars(buf,cs,len)) {
			JM_BUF_ERROR("bufr 3");
            jm_utils_releaseStr(cs,len);
			return NULL;
		}
		return cs;
	}
	return NULL;
}


//取锟斤拷锟斤拷锟斤拷指锟斤拷位锟矫碉拷一锟斤拷锟街节ｏ拷锟剿凤拷锟斤拷锟斤拷锟侥憋拷锟街革拷锟斤拷锟�
ICACHE_FLASH_ATTR char  jm_buf_get_by_index(jm_buf_t *buf,  uint16_t index) {
	if(_bb_is_wrap(buf)) {
		return jm_buf_get_by_index(buf->wrap_buf,index);
	}
	uint16_t idx = (index+buf->rpos)%buf->capacity;
	return buf->data[idx];
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_u16(jm_buf_t *buf, uint16_t *rst) {
	if(!jm_buf_check_read_len(buf,2)) {
		JM_BUF_ERROR("bufr 1 rpos: %d", buf->rpos);
		return false;
	}

	uint16_t first = _bb_get_u8(buf);
	uint16_t second = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//锟斤拷锟�
		*rst = first<<8 | second;
	} else {
		//小锟斤拷
		*rst = second<<8 | first;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_s16(jm_buf_t *buf, int16_t *rst) {
	if(!jm_buf_check_read_len(buf,2)) {
		JM_BUF_ERROR("bufr rpos: %d", buf->rpos);
		return false;
	}

	int16_t first = _bb_get_u8(buf);
	int16_t second = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//锟斤拷锟�
		*rst = (int16_t)(first<<8 | second);
	} else {
		//小锟斤拷
		*rst = (int16_t)(first | second<<8) ;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_s32(jm_buf_t *buf, int32_t *rst) {
	if(!jm_buf_check_read_len(buf,4)) {
		JM_BUF_ERROR("bufr rpos: %d", buf->rpos);
		return false;
	}

	int32_t first = _bb_get_u8(buf);
	int32_t second = _bb_get_u8(buf);
	int32_t third = _bb_get_u8(buf);
	int32_t forth = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//锟斤拷锟�
		*rst = (int32_t)(first <<24 | second<<16 | third<<8 | forth<<0) ;
	} else {
		//小锟斤拷
		*rst = (int32_t)(first <<0 | second<<8 | third<<16 | forth<<24) ;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_u32(jm_buf_t *buf,uint32_t *rst) {
	if(!jm_buf_check_read_len(buf,4)) {
		JM_BUF_ERROR("bufr rpos: %d", buf->rpos);
		return false;
	}

	uint32_t first = _bb_get_u8(buf);
	uint32_t second = _bb_get_u8(buf);
	uint32_t third = _bb_get_u8(buf);
	uint32_t forth = _bb_get_u8(buf);


	if(NET_DATA_BIG_END) {
		*rst = first <<24 | second<<16 | third<<8 | forth<<0 ;
	} else {
		*rst =  first <<0 | second<<8 | third<<16 | forth<<24 ;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_u64(jm_buf_t *buf, uint64_t *rst) {
	return jm_buf_get_s64(buf,rst);
}

ICACHE_FLASH_ATTR BOOL  jm_buf_get_s64(jm_buf_t *buf, sint64_t *rst) {
	if(!jm_buf_check_read_len(buf,8)) {
		JM_BUF_ERROR("bufr rpos: %d", buf->rpos);
		return false;
	}

	uint64_t first = _bb_get_u8(buf);
	uint64_t second = _bb_get_u8(buf);
	uint64_t third = _bb_get_u8(buf);
	uint64_t forth = _bb_get_u8(buf);

	uint64_t five = _bb_get_u8(buf);
	uint64_t six = _bb_get_u8(buf);
	uint64_t seven = _bb_get_u8(buf);
	uint64_t eight = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//*rst = first <<24 | second<<16 | third<<8 | forth<<0 ;
		*rst = (uint64_t)(first <<56 | second<<48 | third<<40 | forth<<32 | five <<24 | six<<16 | seven<<8 | eight<<0);
	} else {
		//*rst =  first <<0 | second<<8 | third<<16 | forth<<24 ;
		*rst = (uint64_t)(first <<0 | second<<8 | third<<16 | forth<<24 | five <<32 | six<<40 | seven<<48 | eight<<56);
	}

	JM_BUF_DEBUG("jm_buf_get_s64 rst=%ld\n",*rst);

	/*
	uint8_t *arr = (uint8_t*)rst;

	if(NET_DATA_BIG_END) {
		arr[7] = first;
		arr[6] = second;
		arr[5] = third;
		arr[4] = forth;
		arr[3] = five;
		arr[2] = six;
		arr[1] = seven;
		arr[0] = eight;
		*rst = (int64_t)(first <<56 | second<<48 | third<<40 | forth<<32 | five <<24 | six<<16 | seven<<8 | eight<<0);
	} else {
		*rst = (int64_t)(first <<0 | second<<8 | third<<16 | forth<<24 | five <<32 | six<<40 | seven<<48 | eight<<56);
		arr[0] = first;
		arr[1] = second;
		arr[2] = third;
		arr[3] = forth;
		arr[4] = five;
		arr[5] = six;
		arr[6] = seven;
		arr[7] = eight;
	}
	 */

	return true;
}

/*******************************Write method begin********************************************/

ICACHE_FLASH_ATTR BOOL jm_buf_writeStringLen(jm_buf_t *buf, uint16_t len){

    if(len == 0) {
        if(!jm_buf_put_u8(buf,0)) {
            JM_BUF_ERROR("bufw 0 err");
            return false;
        }
        return true;
    }

    if(len < MAX_BYTE_VALUE) {
        if(!jm_buf_put_u8(buf,len)) {
            JM_BUF_ERROR("bufw 1 %d",len);
            return false;
        }
    }else if(len < MAX_SHORT_VALUE) {
        if(!jm_buf_put_u8(buf,MAX_BYTE_VALUE)) {
            JM_BUF_ERROR("bufw 2 %d",MAX_BYTE_VALUE);
            return false;
        }
        if(!jm_buf_put_u16(buf,len)) {
            JM_BUF_ERROR("bufw 3 %d",len);
            return false;
        }
    }else if(len < MAX_INT_VALUE) {
        if(!jm_buf_put_u8(buf, MAX_BYTE_VALUE)) {
            JM_BUF_ERROR("bufw 4 %d", MAX_BYTE_VALUE);
            return false;
        }
        if(!jm_buf_put_u16(buf, MAX_SHORT_VALUE)) {
            JM_BUF_ERROR("bufw 4 %d", MAX_SHORT_VALUE);
            return false;
        }
        if(!jm_buf_put_u32(buf, len)) {
            JM_BUF_ERROR("bufw 5 %d", len);
            return false;
        }
    } else {
        JM_BUF_ERROR("bufw 6 %d", MAX_INT_VALUE);
        return false;
    }

    return true;
}

ICACHE_FLASH_ATTR BOOL jm_buf_writeString(jm_buf_t *buf, char *s, uint16_t len){

    if(!jm_buf_writeStringLen(buf,len)) {
        return false;
    }

    if(s == NULL) {
        return true;
    }

    if(!jm_buf_put_chars(buf,s,len)) {
        JM_BUF_ERROR("bws err %d",len);
        return false;
    }

    return true;
}

// [1,0,0,4] --> [0,0,0,0,0,0]
ICACHE_FLASH_ATTR static BOOL  jm_buf_check_write_len(jm_buf_t *buf, uint16_t len) {
    if(len < 0) {
        JM_BUF_ERROR("bufw=%d\n", len);
        return false;
    }

    uint16_t wl = jm_buf_writeable_len(buf);
    if(wl >= len) {
        return true;
    }

    if(_bb_is_wrap(buf)) {
        if(buf->rw_flag) {
            //JM_BUF_ERROR("bufw");
            return false;
        } else {
            return jm_buf_check_write_len(buf->wrap_buf,len);
        }
    }

    //JM_BUF_DEBUG("bufw :%u,rl: %u, cap: %u", wl, len,buf->capacity);
    uint16_t relen = 0;
    if(len < 32) {
    	relen = buf->capacity + 32;
    }else {
    	relen = buf->capacity + len;
    }

    if(relen > MAX_SHORT_VALUE) {
        JM_BUF_ERROR("bufw mo: %d, max: %d\n", relen, MAX_SHORT_VALUE);
        return false;
    }

   /* while(relen < len) {
	   relen += MEM_REALLOC_ADD_SIZE;
    }

    if(relen > MAX_SHORT_VALUE) {
	   relen = MAX_SHORT_VALUE;//分配最大内存
    }*/

    //JM_BUF_DEBUG("rebuf %u\n",relen);
    char *ptr = jm_utils_mallocWithError(relen,PREFIX_TYPE_JM_BUFFER_ARRAY,"jm_buf_check_write_len");
    uint16_t rl = jm_buf_readable_len(buf);

    if(rl > 0 && !jm_buf_get_chars(buf,ptr,rl)){
        JM_BUF_ERROR("cpy buf fail\n");
		jmm->jm_free_fn(ptr, relen);
        return false;
    }

    jmm->jm_free_fn(buf->data, buf->capacity);//释放原来申请的内存

    buf->status = 0;
    buf->data = ptr;
    buf->capacity = relen;
    buf->rpos = 0;
    buf->wpos = rl;

    buf->rmark = -1;
    buf->rmark_status = 0;

    _bb_set_full(buf,false);

    //JM_BUF_DEBUG("rebuf newlen=%d rl=%d cap=%d rpos=%d wpos=%d\n",relen, rl, buf->capacity, buf->rpos, buf->wpos);

	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_u8(jm_buf_t *buf, uint8_t x) {
	if(!jm_buf_check_write_len(buf,1)) {
		JM_BUF_ERROR("ERR bufw wpos: %d, rpos:%d, cap:%d\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//只读的Buff
			return false;
		}else {
			if(jm_buf_put_u8(buf->wrap_buf, x)) {
				buf->wpos++;
			}
		}
	}

    if(jm_buf_is_empty(buf)) {
		_bb_set_empty(buf,false);
	}

    buf->data[buf->wpos] = x;
    buf->wpos = (buf->wpos + 1) % buf->capacity;
    //JM_BUF_DEBUG("jm_buf_put_s8 wpos=%d cap=%d\n",buf->wpos,buf->capacity);

    if(buf->wpos == buf->rpos) {
    	_bb_set_full(buf,true);
    }
    return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_s8(jm_buf_t *buf, int8_t x) {
	if(jm_buf_check_write_len(buf,1) == false) {
		JM_BUF_ERROR("bufw wpos: %d, rpos:%d, cap:%d",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			return false;
		} else {
			if(jm_buf_put_s8(buf->wrap_buf, x)) {
				buf->wpos++;
			}else {
				JM_BUF_DEBUG("bw E wpos=%d", buf->wrap_buf->wpos);
			}
		}

	}else {
	    if(jm_buf_is_empty(buf)) {
			_bb_set_empty(buf,false);
		}

		buf->data[buf->wpos] = x;
		buf->wpos = (buf->wpos + 1) % buf->capacity;
		//JM_BUF_DEBUG("jm_buf_put_s8 wpos=%d cap=%d\n",buf->wpos,buf->capacity);

		if(buf->wpos == buf->rpos) {
			_bb_set_full(buf,true);
		}
	}
    return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_bytes(jm_buf_t *buf, uint8_t *bytes, int16_t len) {
	if(jm_buf_check_write_len(buf,len) == false) {
		JM_BUF_ERROR("ERROR bufw writeable_len: %d, need len: %d", jm_buf_writeable_len(buf), len);
		return false;
	}

	while(len > 0 ) {
		jm_buf_put_u8(buf,*bytes);
		bytes++;
		--len;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_buf(jm_buf_t *buf, jm_buf_t *src) {
	uint16_t len = jm_buf_readable_len(src);
	if(jm_buf_check_write_len(buf,len) == false) {
		JM_BUF_ERROR("bufw len: %d, need len: %d", jm_buf_writeable_len(buf),len);
		return false;
	}

	while(!jm_buf_is_empty(src)) {
		jm_buf_put_u8(buf, _bb_get_u8(src));
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_chars(jm_buf_t *buf, char *chars, int16_t len){
	if(len <= 0) {
		len = jm_strlen(chars);
	}

	if(len < 0 || !jm_buf_check_write_len(buf,len)) {
		JM_BUF_ERROR("bufw rpos=%d wpos=%d nl=%d\n", buf->rpos, buf->wpos,len);
		return false;
	}

	if(len > 0) {
		//JM_BUF_DEBUG("wbuf wl=%d", len);
		for(;len > 0; len--) {
			if(!jm_buf_put_s8(buf, *chars)) {
				JM_BUF_ERROR("bufw E rpos=%d wpos=%d nl=%d\n", buf->rpos, buf->wpos,len);
				return false;
			}
			chars++;
		}
		//JM_BUF_DEBUG("wbuf suc rl=%d", jm_buf_readable_len(buf));
	}

	return true;
}


ICACHE_FLASH_ATTR BOOL  jm_buf_put_char(jm_buf_t *buf, char x) {
	return jm_buf_put_u8(buf,x);
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_bool(jm_buf_t *buf, BOOL x) {
	return jm_buf_put_u8(buf,x);
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_u16(jm_buf_t *buf, uint16_t x) {
	if(!jm_buf_check_write_len(buf,2)) {
		JM_BUF_ERROR("bufw wpos: %d, rpos:%d, cap:%d",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//锟斤拷锟�
		jm_buf_put_u8(buf,(uint8_t)(x>>8));
		jm_buf_put_u8(buf,(uint8_t)(x));
	} else {
		//小锟斤拷
		jm_buf_put_u8(buf,(uint8_t)(x));
		jm_buf_put_u8(buf,(uint8_t)(x>>8));
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_s16(jm_buf_t *buf, int16_t x) {
	if(!jm_buf_check_write_len(buf,2)) {
		JM_BUF_ERROR("bufw wpos: %d, rpos:%d, cap:%d",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		jm_buf_put_u8(buf,(int8_t)(x>>8));
		jm_buf_put_u8(buf,(uint8_t)(x));
		//JM_BUF_DEBUG("jm_buf_put_s16 %d,%d\n", buf->data[buf->wpos-2], buf->data[buf->wpos-1]);
	} else {
		jm_buf_put_u8(buf,(uint8_t)(x));
		jm_buf_put_u8(buf,(int8_t)(x>>8));
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_u32(jm_buf_t *buf, uint32_t x) {
	if(!jm_buf_check_write_len(buf,4)) {
		JM_BUF_ERROR("bufw wpos: %d, rpos:%d, cap:%d",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//锟斤拷锟�
		jm_buf_put_u8(buf,(uint8_t)(x>>24));
		jm_buf_put_u8(buf,(uint8_t)(x>>16));
		jm_buf_put_u8(buf,(uint8_t)(x>>8));
		jm_buf_put_u8(buf,(uint8_t)(x));
	} else {
		//小锟斤拷
		jm_buf_put_u8(buf,(uint8_t)(x));
		jm_buf_put_u8(buf,(uint8_t)(x>>8));
		jm_buf_put_u8(buf,(uint8_t)(x>>16));
		jm_buf_put_u8(buf,(uint8_t)(x>>24));
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_s32(jm_buf_t *buf, int32_t x) {
	if(!jm_buf_check_write_len(buf,4)) {
		JM_BUF_ERROR("bufw wpos: %d, rpos:%d, cap:%d",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		jm_buf_put_u8(buf,x>>24 & 0xFF);
		jm_buf_put_u8(buf,x>>16 & 0xFF);
		jm_buf_put_u8(buf,x>>8 & 0xFF);
		jm_buf_put_u8(buf,x & 0xFF);
	} else {
		jm_buf_put_u8(buf,x & 0xFF);
		jm_buf_put_u8(buf,x>>8 & 0xFF);
		jm_buf_put_u8(buf,x>>16 & 0xFF);
		jm_buf_put_u8(buf,x>>24 & 0xFF);
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_u64(jm_buf_t *buf, uint64_t x) {
	if(!jm_buf_check_write_len(buf,8)) {
		JM_BUF_ERROR("bufw wpos: %d, rpos:%d, cap:%d",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//锟斤拷锟�
		jm_buf_put_u8(buf,(uint8_t)(x>>56 & 0xFF));
		jm_buf_put_u8(buf,(uint8_t)(x>>48 & 0xFF));
		jm_buf_put_u8(buf,(uint8_t)(x>>40 & 0xFF));
		jm_buf_put_u8(buf,(uint8_t)(x>>32 & 0xFF));
		jm_buf_put_u8(buf,(uint8_t)(x>>24 & 0xFF));
		jm_buf_put_u8(buf,(uint8_t)(x>>16 & 0xFF));
		jm_buf_put_u8(buf,(uint8_t)(x>>8 & 0xFF));
		jm_buf_put_u8(buf,(uint8_t)(x & 0xFF));
	} else {
		//小锟斤拷
		jm_buf_put_u8(buf,x & 0xFF);
		jm_buf_put_u8(buf,x>>8 & 0xFF);
		jm_buf_put_u8(buf,x>>16 & 0xFF);
		jm_buf_put_u8(buf,x>>24 & 0xFF);
		jm_buf_put_u8(buf,x>>32 & 0xFF);
		jm_buf_put_u8(buf,x>>40 & 0xFF);
		jm_buf_put_u8(buf,x>>48 & 0xFF);
		jm_buf_put_u8(buf,x>>56 & 0xFF);
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL  jm_buf_put_s64(jm_buf_t *buf, sint64_t x) {
	return jm_buf_put_u64(buf,x);
}

