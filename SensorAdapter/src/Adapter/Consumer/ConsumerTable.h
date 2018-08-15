#pragma once

#include <map>
#include <mutex>
#include <thread>
#include "ConsumerInterface.h"

#include <string>
#include <sstream>

#ifdef __linux__
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#endif

#define DEFAULT_BUFLEN 512


class ConsumerTable {

public:

	struct ConsumerInfo {
		std::string serviceIP;
		std::string servicePort;
		uint32_t serviceInterface;
		bool enableOperation;
		std::thread ConsumerInterfaceThread;
		lastValues lastValue;
	};

	std::mutex m_ConsumerTable;
	std::map<std::string, ConsumerInfo> consumerTable; //consumerID, ConsumerInfo

	ConsumerTable();
	~ConsumerTable();

	bool insertNewConsumer(std::string _consumerID, uint32_t _interfaceType, std::string sIP, std::string sPort);
	
	bool getConsumerLastValue(std::string _consumerID, lastValues &rLastValue);
	uint32_t getConsumerInterfaceType(std::string consumerID);
	bool consumerExists(std::string _consumerID);
	void deleteAllConsumer();
	void terminateConsumerThread(bool &_rEnableOperation, std::thread &_rThread);

	//
	// Consumer Interface Definitions
	//
	void parseTCPRobotArmData(char *recvbuf, int length, std::string &x, std::string &y, std::string &z, std::string &s);
	void createTCPRobotArmInterface(std::string sIPAddr, std::string sPort, bool &_ready, bool &rbthreadStartedSuccessfully, std::string _consumerID);
};

