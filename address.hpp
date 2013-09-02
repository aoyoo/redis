#pragma once
#include <string>

class address{
public:
	address();
	address(std::string ip, int port);
	address(const address &addr);
	address &operator=(const address &addr);
	
	int setAddr(std::string ip, int port);
	
	std::string &ip();
	int port();
private:
	std::string ip_;
	int port_;
};

class addressCluster{
public:
	addressCluster();
	addressCluster(address &masterAddr, std::vector<address> &slaveAddrVec);
	
	int setMasterAddr(address &masterAddr);
	int setSlaveAddr(std::vector<address> &slaveAddrVec);

	address &getMasterAddr(){return masterAddr_;}
	std::vector<address> &getSlaveAddr(){return slaveAddr_;}
	
private:
	address masterAddr_;
	std::vector<address> slaveAddr_;
};








