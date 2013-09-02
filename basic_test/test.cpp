#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <iostream>

#include "hiredis.h"

int main(void) {
    unsigned int j;
    redisContext *c;
    redisReply *reply;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
			exit(1);
        }
    }

	//get from SET
	long long gid = 1001;
	printf("command: SMEMBERS %lld\n", gid);
	reply = (redisReply *)redisCommand(c, "SMEMBERS %lld", gid);

	int num = reply->elements;
    printf("get SET %lld num %d\n", gid, num);
	if(num > 0){
		int i = 0;
		for(i = 0; i < num; i++){
			printf("get SET %lld[%d] %d\n", gid, i, (reply->element[i]->type));
			printf("get SET %lld[%d] %lld\n", gid, i, atoll(reply->element[i]->str));
		}
	}
    freeReplyObject(reply);

	//get from HASH
	
	//get HKEYS
	long long pid = 2001;
	printf("command: HKEYS %lld\n", pid);
	reply = (redisReply *)redisCommand(c, "HKEYS %lld", pid);
	
	num = reply->elements;
	printf("HKEYS %lld num %d\n", pid, num);
	if(num > 0){
		int i = 0;
		for(i = 0; i < num; i++){
			printf("get HKEYS %lld[%d] %d\n", gid, i, (reply->element[i]->type));
			printf("get HKEYS %lld[%d] %s\n", gid, i, (reply->element[i]->str));
		}
	}
	std::string title(reply->element[0]->str), seller(reply->element[1]->str);
	std::cout << title << " " << seller << std::endl;
	freeReplyObject(reply);

	//HGET key title
	printf("command: HGET %lld %s\n", pid, title.c_str());
	reply = (redisReply *)redisCommand(c, "HGET %lld %s", pid, title.c_str());
	printf("get title: %s\n", reply->str);
	freeReplyObject(reply);

	//HGETALL
	printf("command: HGETALL %lld\n", pid);
	reply = (redisReply *)redisCommand(c, "HGETALL %lld", pid);
	num = reply->elements;
	printf("HGETALL %lld num %d\n", pid, num);
	if(num > 0){
		int i = 0;
		for(i = 0; i < num; i++){
			printf("get HKEYS %lld[%d] %d\n", gid, i, (reply->element[i]->type));
			printf("get HKEYS %lld[%d] %s\n", gid, i, (reply->element[i]->str));
		}
	}
	freeReplyObject(reply);

	return 0;
}


