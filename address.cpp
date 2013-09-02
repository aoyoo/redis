#include <stdio.h>
#include <errno.h>
#include <sstream>

#include "store.hpp"
#include "singleDb.hpp"

//#define print(format, args...) fprintf(stdout, "[%s@%s,%d] " format "\n", __FUNCTION__, __FILE__, __LINE__, ## args ); 
#define print(format, args...) 

//address start
address::address()
	:ip_(""), port_(-1)
{}

address::address(std::string ip, int port)
	:ip_(ip), port_(port)
{}

address::address(const address &addr)
	:ip_(addr.ip_), port_(addr.port_)
{}

address &address::operator=(const address &addr){
	ip_ = addr.ip_;
	port_ = addr.port_;
	return *this;
}

int address::setAddr(std::string ip, int port){
	ip_ = ip;
	port_ = port;
	return 0;
}

std::string &address::ip(){
	return ip_;
}

int address::port(){
	return port_;
}
//address stop 


//addressCluster start
addressCluster::addressCluster(){
}

addressCluster::addressCluster(address &masterAddr, std::vector<address> &slaveAddrVec){
	masterAddr_ = masterAddr;
	for(unsigned int i = 0; i < slaveAddrVec.size(); i++){
		slaveAddr_.push_back(slaveAddrVec[i]);
	}
}

int addressCluster::setMasterAddr(address &masterAddr){
	masterAddr_ = masterAddr;
	return 0;
}

int addressCluster::setSlaveAddr(std::vector<address> &slaveAddrVec){
	for(unsigned int i = 0; i < slaveAddrVec.size(); i++){
		slaveAddr_.push_back(slaveAddrVec[i]);
	}
	return 0;
}

//addressCluster stop 

