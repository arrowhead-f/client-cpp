
#include "MoteInterface.h"
#include <stdio.h>
#include <thread>

MoteInterface::MoteInterface(){
	enableOperation = true;
}

MoteInterface::~MoteInterface(){
//#ifdef __linux__
//	close(s);
//#elif _WIN32
//	closesocket(s);
//	WSACleanup();
//#else
//
//#endif
}

void MoteInterface::startMoteUDPInterface(uint16_t port) {

#ifdef __linux__
	slen = sizeof(si_other);

	//create a UDP socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		printf("Error: Cannot create UDP socket for MoteInterface!\n");
		return;
	}

	// zero out the structure
	memset((char *)&si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me)) == -1){
		printf("Error: Cannot bind MoteInterface UDP port: %d\n\n", port);
		return;
	}

	int iOptVal = 100; //ms
	int r = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iOptVal, sizeof(iOptVal));

#elif _WIN32

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\n-----------------------------\nMoteInterface init process.\n-----------------------------\n");
	printf("\nMoteInterface: Initialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("MoteInterface: Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("MoteInterface: Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("MoteInterface: Could not create socket : %d", WSAGetLastError());
	}
	printf("MoteInterface: UDP Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	//server.sin_addr.s_addr = inet_addr("192.168.42.1"); -- RPI code
	server.sin_addr.s_addr = inet_addr("127.0.0.1"); // TEST Code
	server.sin_port = htons(port);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("MoteInterface: Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("MoteInterface: Bind done, port: %d\n\n", port);

	int iOptVal = 100; //ms
	int r = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iOptVal, sizeof(iOptVal));
	//printf("setsockopt: %d\n", r);
#else

#endif

     printf("MoteInterface started, UDP port: %d\n\n", port);

	thProcessIncomingPacket = std::thread(&MoteInterface::processIncomingPacket, this);
}

void MoteInterface::processIncomingPacket(){

#ifdef __linux__
	//keep listening for data
	while (enableOperation){

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1){
			//printf("if MoteInterface recv_len: %d\n", recv_len);
			//int errorCode = WSAGetLastError();

			//if (errorCode == WSAETIMEDOUT)
			//continue;
			//else {
			//printf("recvfrom() failed with error code : %d", errorCode);
			//	break;
			//}
		}

		//printf("MoteInterface recv_len: %d\n", recv_len);
		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n\n", buf);

		processReceivedSensorDataThroughUDPMoteInterface(std::string(buf));
	}
#elif _WIN32
	//keep listening for data
	while (enableOperation){
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR) {
			int errorCode = WSAGetLastError();

			if (errorCode == WSAETIMEDOUT)
				continue;
			else {
				printf("recvfrom() failed with error code : %d", errorCode);
				break;
			}
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n\n", buf);

		processReceivedSensorDataThroughUDPMoteInterface(std::string(buf));
	}
#endif
}

void MoteInterface::processReceivedSensorDataThroughUDPMoteInterface(std::string s) {
	printf("moteinterface Data: %s\n\n", s.c_str());
}

void MoteInterface::stopMoteUDPInterface() {

	printf("stopMoteUDPInterface\n");
	enableOperation = false;

	if (thProcessIncomingPacket.joinable()) {
		thProcessIncomingPacket.join();
	}
	else {
		printf("Warning: MoteUDPInterface is not joinable!\n");
	}

#ifdef __linux__
	close(s);
#elif _WIN32
	closesocket(s);
	WSACleanup();
#endif

	printf("MoteUDPInterface stopped successfully!\n");
}
