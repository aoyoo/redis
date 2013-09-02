#pragma once
#include <string>
#include <vector>
#include "hiredis.h"
#include "address.hpp"

class redisImp{
public:
	redisImp();
	redisImp(address &addr);
		
	int setAddr(address &addr);

	int init();
	int close();

	int setLongLongToSet(long long key, std::vector<long long> &vec);
	int addLongLongToSet(long long key, long long val);
	int delLongLongFromSet(long long key, long long val);
	int setString(long long key, std::string &str);

	//get
	int getLongLongFromSet(long long key, std::vector<long long> &vec);
	int getString(long long key, std::string &str);
	int getLongLongFromSetPipe(const std::vector<long long> &keys, std::vector< std::vector<long long> > &values);
	int getStringPipe(const std::vector<long long> &keys, std::vector<std::string> &infos);

	//hiredis interface
	int getReplyType(redisReply *rep);
	int replyHandle(redisReply *rep);

private:
	address addr_;
	redisContext *rc_;

	bool hasInit_;
};



