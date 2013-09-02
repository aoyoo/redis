#pragma once
#include <string>
#include <vector>
#include "address.hpp"

class singleDb;

class store{
public:
	int setGidPidDb(addressCluster &addr);
	int setPidInfoDb(std::vector<addressCluster> &addrVec);
	
	//set
	int setAllPids(long long gid, std::vector<long long> &pidVec);
	int addGidPid(long long gid, long long pid);
	int delGidPid(long long gid, long long pid);
		//set to several
	int addPidInfo(long long pid, std::string &pidInfo);
	int setPidInfo(long long pid, std::string &pidInfo);
	
	//get
	int getAllPids(long long gid, std::vector<long long> &vec);  //gid -> pids
	int getAllPidsPipe(const std::vector<long long> &gids, std::vector< std::vector<long long> > &pidVec);
	int getProductInfo(long long pid, std::string &str);  //pid -> pid info
	int getProductInfoPipe(const std::vector<long long> &pids, std::vector<std::string> &infos); //pids -> pid infos
	int getProductInfoPipe(int index, const std::vector<long long> &pids, std::vector<std::string> &infos);  //pids -> pid infos
		//get from several
	int getAllProductInfo(long long gid, std::vector<std::string> &pidInfoVec);  //gid -> all pid info
	int getAllProductInfo(long long gid, std::vector<long long> &pidVec, std::vector<std::string> &pidInfoVec);  //gid -> all pid info

	int getAllProductInfoPipe(const std::vector<long long> &gids, 
								std::vector< std::vector<std::string> > &pidInfoVec);  //gid -> all pid info

	int getAllProductInfoPipe(const std::vector<long long> &gids, 
								std::vector< std::vector<long long> > &pidVec, 
								std::vector< std::vector<std::string> > &pidInfoVec);  //gid -> all pid info

	store();
	~store();
	int init();
	int closeAll();

	int getPidInfoDbNum();
	singleDb *getPidInfoDbMaster(int serial);
	std::vector<singleDb *> &getPidInfoDbSlave(int serial);
	
private:
	singleDb *gidPidDbMaster_;
	std::vector<singleDb *> gidPidDbSlaveVec_;

	std::vector<singleDb *> pidInfoDbMasterVec_;
	std::vector< std::vector<singleDb *> > pidInfoDbSlaveVec_;

	store(const store &rhs);
	store &operator=(const store &rhs);
};







