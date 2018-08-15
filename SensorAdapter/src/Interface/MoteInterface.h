
#pragma once

#include <inttypes.h>
#include <thread>

#ifdef __linux__
	#include<string.h> //memset
	#include<stdlib.h> //exit(0);
	#include<arpa/inet.h>
	#include<sys/socket.h>
	#include<unistd.h>
#elif _WIN32
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<winsock2.h>
	#pragma comment(lib,"ws2_32.lib") //Winsock Library
#else
//
#endif

class MoteInterface{

private:
	static const uint32_t BUFLEN = 1510;
	std::thread thProcessIncomingPacket;

	bool enableOperation;

public:
	MoteInterface();
	~MoteInterface();

#ifdef __linux__
	struct sockaddr_in si_me, si_other;
	int s, i, recv_len;
	socklen_t slen;
	char buf[BUFLEN];
#elif _WIN32
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen , recv_len;
	char buf[BUFLEN];
	WSADATA wsa;
#else

#endif

//Process incoming SenML-JSon packet, over Eth/IP/UDP
	void startMoteUDPInterface(uint16_t port);
	void processIncomingPacket();
	void stopMoteUDPInterface();

	virtual void processReceivedSensorDataThroughUDPMoteInterface(std::string pJsonSenML);
};
