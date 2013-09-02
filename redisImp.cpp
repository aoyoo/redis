#include "redisImp.hpp"
#include "store.hpp"
#include <errno.h>
#include "Logger.h"

//redisImp start
redisImp::redisImp()
	:addr_("", -1), hasInit_(false), rc_(NULL)
{}

redisImp::redisImp(address &addr)
	:addr_(addr), hasInit_(false), rc_(NULL)
{}

int redisImp::setAddr(address &addr){
	addr_ = addr;
	return 0;
}

int redisImp::init(){
	if(hasInit_){
		return 0;
	}
	struct timeval timeout = { 0, 10000 }; // 10 ms
	//rc_ = redisConnectWithTimeout(addr_.ip().c_str(), addr_.port(), timeout);
	rc_ = redisConnect(addr_.ip().c_str(), addr_.port()); //i need block model
	if (rc_ == NULL || rc_->err) {
		if (rc_) {
			LOG_ERROR("Connection error: " << rc_->errstr);
			//fprintf(stderr, "Connection error: %s\n", rc_->errstr);
			redisFree(rc_);
			rc_ = NULL;
		} else {
			LOG_ERROR("Connection error: can't allocate redis context");
			//fprintf(stderr, "Connection error: can't allocate redis context\n");
		}
		return -1;
	}
	hasInit_ = true;
	return 0;
}

int redisImp::close(){
	if(rc_ != NULL){
		redisFree(rc_);
		rc_ = NULL;
	}
	return 0;
}

#if 0
//in hiredis.h as follow
#define REDIS_REPLY_STRING 1
#define REDIS_REPLY_ARRAY 2
#define REDIS_REPLY_INTEGER 3
#define REDIS_REPLY_NIL 4
#define REDIS_REPLY_STATUS 5
#define REDIS_REPLY_ERROR 6
/* This is the reply object returned by redisCommand() */
typedef struct redisReply {
    int type; /* REDIS_REPLY_* */
    long long integer; /* The integer when type is REDIS_REPLY_INTEGER */
    int len; /* Length of string */
    char *str; /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
    size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
    struct redisReply **element; /* elements vector for REDIS_REPLY_ARRAY */
} redisReply;
#endif

int redisImp::replyHandle(redisReply *rep){
	//TODO handle all redis error
	if(!hasInit_){
		return -2;
	}
	if(rep == NULL){
		LOG_ERROR("redisReply NULL: " << rc_->err << ":" << rc_->errstr << " errno: " << strerror(errno));
		//fprintf(stderr, "redisReply NULL: %d:%s errno: %s\n", rc_->err, rc_->errstr, strerror(errno));
		return -1;
	}else if(rep->type == REDIS_REPLY_ERROR){
		LOG_ERROR("redisReply ERROR: " << rep->str);
		//fprintf(stderr, "redisReply ERROR: %s\n", rep->str);
		freeReplyObject(rep);
		return -1;
	}else if(rep->type == REDIS_REPLY_STATUS){
		if(strcmp(rep->str, "OK") != 0){
			LOG_ERROR("redisReply STATUS NOT OK: " << rep->str);
			//fprintf(stderr, "redisReply STATUS NOT OK: %s\n", rep->str);
			freeReplyObject(rep);
			return -1;
		}else{
			freeReplyObject(rep);
			return 0;
		}
	}else{
		freeReplyObject(rep);
		return 0;
	}
}

int redisImp::setLongLongToSet(long long key, std::vector<long long> &vec){
	if(!hasInit_){
		return -2;
	}
	
	int num = vec.size();
	if(num == 0)
		return -1;
	//del the key first
	//then set the new
	
	redisReply *reply = (redisReply *)redisCommand(rc_, "DEL %lld", key);
	int ret = replyHandle(reply);
	if(ret< 0){
		//error happend
		LOG_ERROR("setLongLongToSet REDIS_REPLY_ERROR DEL: " << key);
		//fprintf(stderr, "setLongLongToSet REDIS_REPLY_ERROR DEL: %lld\n", key);
		return -1;
	}

	std::string cmd;

	cmd.append("SADD ");
	char buf[20]; //for %lld 
	memset(buf, 0 ,20);
	sprintf(buf,"%lld  ", key);
	cmd.append(buf);
	for(unsigned int i = 0; i < vec.size(); i++){
		sprintf(buf,"%lld  ", vec[i]);
		cmd.append(buf);
	}
	//reply = (redisReply *)redisCommand(rc_, "%s", cmd.c_str()); 
	//why this error: 
	//setLongLongToSet REDIS_REPLY_ERROR SADD: ERR unknown command 'SADD 100000001  1577772199  1577772198  1577772197  '
	
	reply = (redisReply *)redisCommand(rc_, cmd.c_str());
	ret = replyHandle(reply);
	if(ret < 0){
		LOG_ERROR("setLongLongToSet REDIS_REPLY_ERROR SADD: " << key);
		//fprintf(stderr, "setLongLongToSet REDIS_REPLY_ERROR SADD: %lld\n", key);
		return -1;
	}

	return 0;
}

int redisImp::addLongLongToSet(long long key, long long val){
	if(!hasInit_){
		return -2;
	}
	
	redisReply *reply = (redisReply *)redisCommand(rc_, "SADD %lld %lld", key, val);
	int ret = replyHandle(reply);
	if(ret < 0){
		LOG_ERROR("addLongLongToSet ERROR: " << key << ":" << val);
		//fprintf(stderr, "addLongLongToSet ERROR: %lld:%lld\n", key,val);
		return -1;
	}
	
	return 0;
}

int redisImp::delLongLongFromSet(long long key, long long val){
	if(!hasInit_){
		return -2;
	}
	
	redisReply *reply = (redisReply *)redisCommand(rc_, "SREM %lld %lld", key, val);
	int ret = replyHandle(reply);
	if(ret < 0){
		LOG_ERROR("delLongLongFromSet ERROR: " << key << ":" << val);
		//fprintf(stderr, "delLongLongFromSet ERROR: %lld:%lld\n", key,val);
		return -1;
	}

	return 0;
}

int redisImp::setString(long long key, std::string &str){
	if(!hasInit_){
		return -2;
	}

	char buf[20]; //for %lld, key 
	memset(buf, '\0' ,20);
	sprintf(buf, "%lld", key);

	int num = 3;
	char *cmd[num];
	size_t len[num];

	cmd[0] = "SET";
	len[0] = strlen("SET");

	cmd[1] = buf;
	len[1] = strlen(buf);
	
	cmd[2] = const_cast<char *>(str.c_str());
	len[2] = str.size();
	
	//std::string cmd;
	//cmd.append("SET ");

	//cmd.append(buf); //add key
	//cmd.append(str); //add str
	//redisReply *reply = (redisReply *)redisCommand(rc_, cmd.c_str());
	
	//redisReply *reply = (redisReply *)redisCommand(rc_, "SET %lld %s", key, str.c_str());

	redisReply *reply = (redisReply *)redisCommandArgv(rc_, num, (const char **)cmd, len);

	int ret = replyHandle(reply);
	if(ret < 0){
		LOG_ERROR("setString ERROR: " << key);
		//fprintf(stderr, "setString ERROR: %lld:%s\n", key, str.c_str());
		return -1;
	}

	return 0;
}


//get
int redisImp::getLongLongFromSet(long long key, std::vector<long long> &vec){
	if(!hasInit_){
		return -2;
	}
	redisReply *reply = (redisReply *)redisCommand(rc_, "SMEMBERS %lld", key);

	if(reply == NULL){
		LOG_ERROR("redisReply NULL: " << rc_->err << ":" << rc_->errstr << " errno: " << strerror(errno));
		//fprintf(stderr, "redisReply NULL: %d:%s errno: %s\n", rc_->err, rc_->errstr, strerror(errno));
		return -1;
	}else if(reply->type == REDIS_REPLY_ERROR){
		LOG_ERROR("redisReply ERROR: " << reply->str);
		//fprintf(stderr, "redisReply ERROR: %s\n", reply->str);
		freeReplyObject(reply);
		return -1;
	}

	int num = reply->elements;
	if(num > 0){
		for(int i = 0; i < num; i++){
			vec.push_back(atoll(reply->element[i]->str));
		}
	}
	freeReplyObject(reply);
	return 0;
}

int redisImp::getString(long long key, std::string &str){
	if(!hasInit_){
		return -2;
	}
	redisReply *reply = (redisReply *)redisCommand(rc_, "GET %lld", key);

	if(reply == NULL){
		LOG_ERROR("redisReply NULL: " << rc_->err << ":" << rc_->errstr << " errno: " << strerror(errno));
		//fprintf(stderr, "redisReply NULL: %d:%s errno: %s\n", rc_->err, rc_->errstr, strerror(errno));
		return -1;
	}else if(reply->type == REDIS_REPLY_ERROR){
		LOG_ERROR("redisReply ERROR: " << reply->str);
		//fprintf(stderr, "redisReply ERROR: %s\n", reply->str);
		freeReplyObject(reply);
		return -1;
	}

	if(reply->str != NULL){
		//str = reply->str;
		str.assign(reply->str, reply->len);
		freeReplyObject(reply);
		return 0;
	}else {
		freeReplyObject(reply);
		return -1;
	}
}

int redisImp::getLongLongFromSetPipe(const std::vector<long long> &keys, std::vector< std::vector<long long> > &values){
	if(!hasInit_){
		return -2;
	}
	
	for(int i = 0; i < keys.size(); i++){
		if(REDIS_OK != redisAppendCommand(rc_, "SMEMBERS %lld", keys[i])){
			LOG_ERROR("redisAppendCommand SMEMBERS " << keys[i]);
			return -1;
		}
	}
	
	redisReply* reply = NULL;
	for(int i = 0; i < keys.size(); i++){
		std::vector<long long> tmpVec;
		int reply_status = redisGetReply(rc_, (void **)&reply);
		if(reply_status == REDIS_OK){
			int num = reply->elements;
			if(num > 0){
				for(int j = 0; j < num; j++){
					tmpVec.push_back(atoll(reply->element[j]->str));
				}
			}
		}
		//if status != OK or num <= 0, tmpVec is empty
		values.push_back(tmpVec);
		freeReplyObject(reply);
	}
	
	return 0;
}

int redisImp::getStringPipe(const std::vector<long long> &keys, std::vector<std::string> &infos){
	if(!hasInit_){
		return -2;
	}
	
	for(int i = 0; i < keys.size(); i++){
		if(REDIS_OK != redisAppendCommand(rc_, "GET %lld", keys[i])){
			LOG_ERROR("redisAppendCommand GET" << keys[i]);
			return -1;
		}
	}
	
	redisReply* reply = NULL;
	for(int i = 0; i < keys.size(); i++){
		std::string tmpInfo;
		int reply_status = redisGetReply(rc_, (void **)&reply);
		if(reply_status == REDIS_OK){
			if(reply->str != NULL){
				tmpInfo.assign(reply->str, reply->len);
				//push_back to infos just while get tmpInfo
				infos.push_back(tmpInfo);
			}
		}
		freeReplyObject(reply);
	}
	
	return 0;
}


//redisImp over 



