#include "singleDb.hpp"

//singleDb start
singleDb::singleDb()
	:addr_("", -1), hasInit_(false)
{
}

singleDb::singleDb(address &addr)
	:addr_(addr), rc_(addr), hasInit_(false)
{
}

int singleDb::setAddr(address &addr){
	addr_ = addr;
	return 0;
}

int singleDb::init(){
	int ret = rc_.init();
	
	if(ret < 0){
		//fprintf(stderr, "redisImp init error: %d", ret);
		return -1;
	}
	hasInit_ = true;
	return 0;
}

int singleDb::close(){
	return rc_.close();
}

//set
int singleDb::setAllPids(long long gid, std::vector<long long> &vec){
	return rc_.setLongLongToSet(gid, vec);
}

int singleDb::addGidPid(long long gid, long long pid){
	return rc_.addLongLongToSet(gid, pid);
}

int singleDb::delGidPid(long long gid, long long pid){
	return rc_.delLongLongFromSet(gid, pid);
}

int singleDb::setString(long long pid, std::string &str){
	return rc_.setString(pid, str);
}

//get
int singleDb::getAllPids(long long gid, std::vector<long long> &vec){
	return rc_.getLongLongFromSet(gid, vec);
}

int singleDb::getProductInfo(long long pid, std::string &str){
	return rc_.getString(pid, str);
}

int singleDb::getAllPidsPipe(const std::vector<long long> &gids, std::vector< std::vector<long long> > &pids){
	return rc_.getLongLongFromSetPipe(gids, pids);
}

int singleDb::getProductInfoPipe(const std::vector<long long> &pids, std::vector< std::string >&infos){
	return rc_.getStringPipe(pids, infos);
}

//singleDb over



