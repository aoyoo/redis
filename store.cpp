#include <stdio.h>
#include <errno.h>
#include <sstream>

#include "Logger.h"
#include "store.hpp"
#include "singleDb.hpp"

//#define print(format, args...) fprintf(stdout, "[%s@%s,%d] " format "\n", __FUNCTION__, __FILE__, __LINE__, ## args ); 
#define print(format, args...) 
using namespace std;

int store::setGidPidDb(addressCluster &addr){
	if(gidPidDbMaster_ != NULL) //has been set
		return -1;
	gidPidDbMaster_ = new singleDb(addr.getMasterAddr());
	for (std::vector<address>::iterator it = addr.getSlaveAddr().begin(); it != addr.getSlaveAddr().end(); it++) {
		singleDb *tmpSlave = new singleDb(*it);
		gidPidDbSlaveVec_.push_back(tmpSlave);
	}
	return 0; 
}

int store::setPidInfoDb(std::vector<addressCluster> &addrVec){
	if(pidInfoDbMasterVec_.size() > 0) //has been set
		return -1;
	for(unsigned int i = 0; i < addrVec.size(); i++){
		singleDb *tmpMaster = new singleDb(addrVec[i].getMasterAddr());
		pidInfoDbMasterVec_.push_back(tmpMaster);
		std::vector<singleDb *> tmpSlaveVec;
		for (std::vector<address>::iterator it = addrVec[i].getSlaveAddr().begin(); it != addrVec[i].getSlaveAddr().end(); it++) {
			singleDb *tmpSlave = new singleDb(*it);
			tmpSlaveVec.push_back(tmpSlave);
		}
		pidInfoDbSlaveVec_.push_back(tmpSlaveVec);
	}
	return 0;
}


//set
int store::setAllPids(long long gid, std::vector<long long> &pidVec){
	return gidPidDbMaster_->setAllPids(gid, pidVec);
}

int store::addGidPid(long long gid, long long pid){
	return gidPidDbMaster_->addGidPid(gid, pid);
}

int store::delGidPid(long long gid, long long pid){
	return gidPidDbMaster_->delGidPid(gid, pid);
}

int store::setPidInfo(long long pid, std::string &pidInfo){
	int serial = pid%getPidInfoDbNum();
	return getPidInfoDbMaster(serial)->setString(pid, pidInfo);
}



//get
int store::getAllPids(long long gid, std::vector<long long> &vec){
	int ret = 0;
	for(std::vector<singleDb *>::iterator it = gidPidDbSlaveVec_.begin(); it != gidPidDbSlaveVec_.end(); it++) {
		ret = (*it)->getAllPids(gid, vec);
		if(ret < 0){
			LOG_WARN("getAllPids from " << (*it)->getAddr().ip() << ":" << (*it)->getAddr().port() << " failed");
			continue;
		}else{
			return ret;
		}
	}
	LOG_ERROR("getAllPids " << gid << " from all slave failed");
	return ret;
}

int store::getAllPidsPipe(const std::vector<long long> &gids, std::vector< std::vector<long long> > &pidVec){
	int ret = 0;
	for(std::vector<singleDb *>::iterator it = gidPidDbSlaveVec_.begin(); it != gidPidDbSlaveVec_.end(); it++) {
		ret = (*it)->getAllPidsPipe(gids, pidVec);
		if(ret < 0){
			LOG_WARN("getAllPidsPipe from " << (*it)->getAddr().ip() << ":" << (*it)->getAddr().port() << " failed");
			continue;
		}else{
			return ret;
		}
	}
	LOG_ERROR("getAllPidsPipe from all slave failed");
	return ret;
}

int store::getProductInfo(long long pid, std::string &str){
	int index = pid%getPidInfoDbNum();
	int ret = 0;
	std::vector<singleDb *> slaveVec = getPidInfoDbSlave(index);
	for(std::vector<singleDb *>::iterator it = slaveVec.begin(); it != slaveVec.end(); it++) {
		ret = (*it)->getProductInfo(pid, str);
		if(ret < 0){
			LOG_WARN("getProductInfo from " << (*it)->getAddr().ip() << ":" << (*it)->getAddr().port() << " failed");
			continue;
		}else{
			return ret;
		}
	}
	LOG_ERROR("getProductInfo " << pid << " from all slave failed");
	return ret;
}

int store::getProductInfoPipe(const std::vector<long long> &pids, std::vector<std::string> &infos){
	int pidNum = pids.size();
	
	for(int i = 0; i < pidInfoDbSlaveVec_.size(); i++){ //分片i里面各搜一次
		for(std::vector<singleDb *>::iterator it = pidInfoDbSlaveVec_[i].begin(); it != pidInfoDbSlaveVec_[i].end(); it++) {
			int ret = (*it)->getProductInfoPipe(pids, infos); 
			if(ret < 0){ //一个从库找失败了
				LOG_WARN("getProductInfoPipe from " << (*it)->getAddr().ip() << ":" << (*it)->getAddr().port() << " failed");
				continue;
			}else{ //在一个从库里面找到了就返回，继续到下一个分片里面去找
				break;
			}
		}
	}
	return 0; //return 0 也可能infos为空。。。
}

int store::getProductInfoPipe(int index, const vector<long long> &pids, vector<string> &infos){
	vector<singleDb *> slaveVec = getPidInfoDbSlave(index);

	for(std::vector<singleDb *>::iterator it = slaveVec.begin(); it != slaveVec.end(); it++) {
		int ret = (*it)->getProductInfoPipe(pids, infos); 
		if(ret < 0){ //一个从库找失败了
			LOG_WARN("getProductInfoPipe from " << (*it)->getAddr().ip() << ":" << (*it)->getAddr().port() << " failed");
			continue;
		}else{
			break;
		}
	}

	return 0;
}

int store::getAllProductInfo(long long gid, std::vector<std::string> &pidInfoVec){
	std::vector<long long> tmppidVec;
	
	if(getAllPids(gid, tmppidVec) < 0){
		LOG_ERROR("getAllProductInfo getAllPids error gid: " << gid);
		return -1;
	}
	bool hasGet = false;
	for(unsigned int i = 0; i < tmppidVec.size(); i++){
		std::string tmpPidInfo;
		if(getProductInfo(tmppidVec[i], tmpPidInfo) >= 0){
			hasGet = true;
			pidInfoVec.push_back(tmpPidInfo);
		}else{
			LOG_WARN("getAllProductInfo getProductInfo error gid: " << gid << " pid: " <<tmppidVec[i]);
		}
	}
	if(hasGet)
		return 0;
	else
		return -1;
}

int store::getAllProductInfo(long long gid, std::vector<long long> &pidVec, std::vector<std::string> &pidInfoVec){
	std::vector<long long> tmppidVec;
	
	if(getAllPids(gid, tmppidVec) < 0){
		LOG_ERROR("getAllProductInfo getAllPids error gid: " << gid);
		return -1;
	}
	
	bool hasGet = false;
	for(unsigned int i = 0; i < tmppidVec.size(); i++){
		std::string tmpPidInfo;
		if(getProductInfo(tmppidVec[i], tmpPidInfo) >= 0){
			hasGet = true;
			pidVec.push_back(tmppidVec[i]);
			pidInfoVec.push_back(tmpPidInfo);
		}else{
			LOG_WARN("getAllProductInfo getProductInfo error gid: " << gid << " pid: " <<tmppidVec[i]);
		}
	}
	if(hasGet)
		return 0;
	else
		return -1;
}

int store::getAllProductInfoPipe(const std::vector<long long> &gids, 
								std::vector< std::vector<std::string> > &pidInfoVec){
	
	std::vector< std::vector<long long> > pidVec;

	if(getAllPidsPipe(gids, pidVec) < 0){
		LOG_ERROR("getAllProductInfoPipe getAllPidsPipe error");
		return -1;
	}
	
	int gidNum = gids.size();
	if(gidNum != pidVec.size()){
		LOG_ERROR("getAllProductInfoPipe gids.size != pids.size");
		return -1;
	}

	//get gid->index_in_vec
	map<long long, int> gidVecIndex;
	for(int i = 0; i < gidNum; i++){
		gidVecIndex[gids[i]] = i; //输入gids必须各不相同
	}
	
	multimap<long long, int> pidVecIndex; //pid -> gidIndex, pid maybe multi

	int pidInfoDbNum = getPidInfoDbNum();
	vector< vector<long long> > forPidsVec;
	forPidsVec.resize(pidInfoDbNum);

	vector< vector<string> > forPidInfosVec;
	forPidInfosVec.resize(pidInfoDbNum);

	for(int i = 0; i < gidNum; i++){
		for(int j = 0; j < pidVec[i].size(); j++){
			int index = pidVec[i][j]%pidInfoDbNum;
			long long tmpPid = pidVec[i][j];
			forPidsVec[index].push_back(tmpPid); //pid may multi
			pidVecIndex.insert(make_pair(tmpPid, i)); 
		}
	}
	
	for(int i = 0; i < pidInfoDbNum; i++){
		if(!forPidsVec[i].empty()){
			if(getProductInfoPipe(i, forPidsVec[i], forPidInfosVec[i]) < 0){
				LOG_ERROR("getAllProductInfoPipe getProductInfoPipe error");
			}
		}
	}
	
	pidInfoVec.resize(gidNum);
	
	for(int i = 0; i < pidInfoDbNum; i++){
		if(!forPidInfosVec[i].empty()){
			for(int j = 0; j < forPidInfosVec[i].size(); j++){
				//forPidInfosVec[i][j] //pidInfo
				//forPidsVec[i][j] //pid
				//伪代码
				//vec = pidVecIndex.find(forPidsVec[i][j]);
				//for i in vec
				//	pidInfoVec[i].push_back(forPidInfosVec[i][j]);
				multimap<long long, int>::const_iterator itStart  = pidVecIndex.lower_bound(forPidsVec[i][j]);
				multimap<long long, int>::const_iterator itEnd = pidVecIndex.upper_bound(forPidsVec[i][j]);
				for(; itStart != itEnd; itStart++){
					int tmpGidIndex = itStart->second;
					pidInfoVec[tmpGidIndex].push_back(forPidInfosVec[i][j]);
				}
			}
		}
	}



	return 0;
}

int store::getAllProductInfoPipe(const std::vector<long long> &gids, 
								std::vector< std::vector<long long> > &pidVec, 
								std::vector< std::vector<std::string> > &pidInfoVec){
	
	if(getAllPidsPipe(gids, pidVec) < 0){
		LOG_ERROR("getAllProductInfoPipe getAllPidsPipe error");
		return -1;
	}
	
	int gidNum = gids.size();
	if(gidNum != pidVec.size()){
		LOG_ERROR("getAllProductInfoPipe gids.size != pids.size");
		return -1;
	}

	//get gid->index_in_vec
	map<long long, int> gidVecIndex;
	for(int i = 0; i < gidNum; i++){
		gidVecIndex[gids[i]] = i; //输入gids必须各不相同
	}
	
	multimap<long long, int> pidVecIndex; //pid -> gidIndex, pid maybe multi

	int pidInfoDbNum = getPidInfoDbNum();
	vector< vector<long long> > forPidsVec;
	forPidsVec.resize(pidInfoDbNum);

	vector< vector<string> > forPidInfosVec;
	forPidInfosVec.resize(pidInfoDbNum);

	for(int i = 0; i < gidNum; i++){
		for(int j = 0; j < pidVec[i].size(); j++){
			int index = pidVec[i][j]%pidInfoDbNum;
			long long tmpPid = pidVec[i][j];
			forPidsVec[index].push_back(tmpPid); //pid may multi
			pidVecIndex.insert(make_pair(tmpPid, i)); 
		}
	}
	
	for(int i = 0; i < pidInfoDbNum; i++){
		if(!forPidsVec[i].empty()){
			if(getProductInfoPipe(i, forPidsVec[i], forPidInfosVec[i]) < 0){
				LOG_ERROR("getAllProductInfoPipe getProductInfoPipe error");
			}
		}
	}
	
	pidInfoVec.resize(gidNum);
	
	for(int i = 0; i < pidInfoDbNum; i++){
		if(!forPidInfosVec[i].empty()){
			for(int j = 0; j < forPidInfosVec[i].size(); j++){
				//forPidInfosVec[i][j] //pidInfo
				//forPidsVec[i][j] //pid
				//伪代码
				//vec = pidVecIndex.find(forPidsVec[i][j]);
				//for i in vec
				//	pidInfoVec[i].push_back(forPidInfosVec[i][j]);
				multimap<long long, int>::const_iterator itStart  = pidVecIndex.lower_bound(forPidsVec[i][j]);
				multimap<long long, int>::const_iterator itEnd = pidVecIndex.upper_bound(forPidsVec[i][j]);
				for(; itStart != itEnd; itStart++){
					int tmpGidIndex = itStart->second;
					pidInfoVec[tmpGidIndex].push_back(forPidInfosVec[i][j]);
				}
			}
		}
	}
	return 0;
}

store::store()
	:gidPidDbMaster_(NULL)
{}

store::~store(){
	closeAll();
	
	if(gidPidDbMaster_ != NULL){
		delete gidPidDbMaster_;
		gidPidDbMaster_ = NULL;
	}
	for(unsigned int i = 0; i < gidPidDbSlaveVec_.size(); i++){
		if(gidPidDbSlaveVec_[i] != NULL){
			delete gidPidDbSlaveVec_[i];
			gidPidDbSlaveVec_[i] = NULL;
		}
	}

	for(unsigned int i = 0; i < pidInfoDbMasterVec_.size(); i++){
		if(pidInfoDbMasterVec_[i] != NULL){
			delete pidInfoDbMasterVec_[i];
			pidInfoDbMasterVec_[i] = NULL;
		}
	}
	for(unsigned int i = 0; i < pidInfoDbSlaveVec_.size(); i++){
		for(unsigned int j = 0; j < pidInfoDbSlaveVec_[i].size(); j++){
			if(pidInfoDbSlaveVec_[i][j] != NULL){
				delete pidInfoDbSlaveVec_[i][j];
				pidInfoDbSlaveVec_[i][j] = NULL;
			}
		}
	}
}

int store::init(){
	if(gidPidDbMaster_ != NULL){
		if(gidPidDbMaster_->init() < 0){
			LOG_ERROR("gidPidDbMaster " << gidPidDbMaster_->getAddr().ip() << ":" << gidPidDbMaster_->getAddr().port() 
						<< " init failed");
			closeAll();
			return -1;
		}else{
			LOG_INFO("gidPidDbMaster " << gidPidDbMaster_->getAddr().ip() << ":" << gidPidDbMaster_->getAddr().port() 
						<< " init success");
		}
	}else{
		LOG_ERROR("gidPidDbMaster not set!");
		return -1;
	}
	
	for(unsigned int i = 0; i < gidPidDbSlaveVec_.size(); i++){
		if(gidPidDbSlaveVec_[i]->init() < 0){
			LOG_ERROR("gidPidDbSlave " << gidPidDbSlaveVec_[i]->getAddr().ip() << ":" << gidPidDbSlaveVec_[i]->getAddr().port() 
						<< " init failed");
			closeAll();
			return -1;
		}else{
			LOG_INFO("gidPidDbSlave " << gidPidDbSlaveVec_[i]->getAddr().ip() << ":" << gidPidDbSlaveVec_[i]->getAddr().port() 
						<< " init success");
		}
	}

	int tmpPidInfoDbNum = pidInfoDbMasterVec_.size();
	if(tmpPidInfoDbNum != pidInfoDbSlaveVec_.size()){
		LOG_ERROR("pidInfoDb Master Num: " << tmpPidInfoDbNum << " Slave Num: " << pidInfoDbSlaveVec_.size() << " dont equal");
		return -1;
	}

	for(unsigned int i = 0; i < tmpPidInfoDbNum; i++){
		if(pidInfoDbMasterVec_[i]->init() < 0){
			LOG_ERROR("pidInfoDbMaster " << pidInfoDbMasterVec_[i]->getAddr().ip() << ":" << pidInfoDbMasterVec_[i]->getAddr().port() 
						<< " init failed");
			closeAll();
				return -1;
		}else{
			LOG_INFO("pidInfoDbMaster " << pidInfoDbMasterVec_[i]->getAddr().ip() << ":" << pidInfoDbMasterVec_[i]->getAddr().port() 
						<< " init success");
		}

		for(unsigned int j = 0; j < pidInfoDbSlaveVec_[i].size(); j++){
			if(pidInfoDbSlaveVec_[i][j]->init() < 0){
				LOG_ERROR("pidInfoDbSlave: " << pidInfoDbSlaveVec_[i][j]->getAddr().ip() << ":" 
						<< pidInfoDbSlaveVec_[i][j]->getAddr().port() << " init failed");
				closeAll();
				return -1;
			}else{
				LOG_INFO("pidInfoDbSlave: " << pidInfoDbSlaveVec_[i][j]->getAddr().ip() << ":" 
						<< pidInfoDbSlaveVec_[i][j]->getAddr().port() << " init success");
			}
		}
	}
	
	return 0;
}

int store::closeAll(){
	if(gidPidDbMaster_ != NULL)
		gidPidDbMaster_->close();
	for(unsigned int i = 0; i < gidPidDbSlaveVec_.size(); i++){
		gidPidDbSlaveVec_[i]->close();
	}
	for(unsigned int i = 0; i < pidInfoDbMasterVec_.size(); i++){
		pidInfoDbMasterVec_[i]->close();
	}
	for(unsigned int i = 0; i < pidInfoDbSlaveVec_.size(); i++){
		for(unsigned int j = 0; j < pidInfoDbSlaveVec_[i].size(); j++){
			pidInfoDbSlaveVec_[i][j]->close();
		}
	}
	return 0;
}

int store::getPidInfoDbNum(){
	return pidInfoDbMasterVec_.size();
}

singleDb *store::getPidInfoDbMaster(int serial){
	return pidInfoDbMasterVec_[serial];
}

std::vector<singleDb *> &store::getPidInfoDbSlave(int serial){
	return pidInfoDbSlaveVec_[serial];
}

//store over


