#pragma once
#include <string>
#include <vector>
#include "redisImp.hpp"
#include "address.hpp"

class singleDb{
public:
	singleDb();
	singleDb(address &addr);
	int setAddr(address &addr);
	address &getAddr(){return addr_;}

	int init();
	int close();
	
	//set
	int setAllPids(long long gid, std::vector<long long> &vec);
	int addGidPid(long long gid, long long pid);
	int delGidPid(long long gid, long long pid);
	int setString(long long pid, std::string &str);
	
	//get
	int getAllPids(long long gid, std::vector<long long> &vec);
	int getProductInfo(long long pid, std::string &str);

	int getAllPidsPipe(const std::vector<long long> &gids, std::vector< std::vector<long long> > &pids);
	int getProductInfoPipe(const std::vector<long long> &pids, std::vector< std::string >&infos);

private:
	address addr_;
	redisImp rc_;

	bool hasInit_;
};



