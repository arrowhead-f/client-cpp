
#include "ConsumerTable.h"


ConsumerTable::ConsumerTable() {

}

ConsumerTable::~ConsumerTable() {

}

bool ConsumerTable::insertNewConsumer(std::string _consumerID, uint32_t _interfaceType, std::string sIP, std::string sPort) {
	printf("Insert new Consumer: %s, intftype: %d, ip: %s, port: %s\n", _consumerID.c_str(), _interfaceType, sIP.c_str(), sPort.c_str());
	
	ConsumerInfo cInfo;

	cInfo.serviceInterface = _interfaceType;
	cInfo.serviceIP = sIP;
	cInfo.servicePort = sPort;
	cInfo.enableOperation = true;

	bool threadStartedSuccessfully = false;

	switch (_interfaceType) {
		case CONSUMER_INTERFACE_TCPRobotArm:
			cInfo.lastValue.TCPRobotArmValue.lastValueX = "x=0.0\n";
			cInfo.lastValue.TCPRobotArmValue.lastValueY = "y=0.0\n";
			cInfo.lastValue.TCPRobotArmValue.lastValueZ = "z=0.0\n";
			cInfo.lastValue.TCPRobotArmValue.lastValueS = "s=0.0\n";
		
			printf("cInfo initialization is successful\n");
			printf("CreateTCPRobotArmInterface...\n");
			
			bool ready = false;

			cInfo.ConsumerInterfaceThread = std::thread(&ConsumerTable::createTCPRobotArmInterface, this, sIP, sPort, std::ref(ready), std::ref(threadStartedSuccessfully), _consumerID);

			while (!ready) {
#ifdef __linux__
				usleep(100);
#elif _WIN32
				Sleep(100);
#endif
			}

			if ( threadStartedSuccessfully ) {
				std::lock_guard<std::mutex> lock(m_ConsumerTable);
				
				consumerTable.insert( std::pair<std::string, ConsumerInfo>( _consumerID, std::move(cInfo)));
				printf("ConsumerInterfaceThread created successfully, insert new consumer into consumerTable\n");

				return true;
			}
			else {
				if (cInfo.ConsumerInterfaceThread.joinable()) {
					cInfo.ConsumerInterfaceThread.join();
				}

				return false;
			}

		//default:
		//	printf("Error: Unknown interface type\n");
		//	return false;
	}
	return false;
}

uint32_t ConsumerTable::getConsumerInterfaceType(std::string _consumerID) {
	std::lock_guard<std::mutex> lock(m_ConsumerTable);

	for (auto i = consumerTable.begin(); i != consumerTable.end(); ++i) {
		if (i->first == _consumerID) {
			return i->second.serviceInterface;
		}
	}

	return CONSUMER_INTERFACE_UNKNOWN;
}

bool ConsumerTable::getConsumerLastValue(std::string _consumerID, lastValues &rLastValue) {
	std::lock_guard<std::mutex> lock(m_ConsumerTable);

	for (auto i = consumerTable.begin(); i != consumerTable.end(); ++i) {
		if (i->first == _consumerID) {
			rLastValue = i->second.lastValue;
		}
		return true;
	}
	
	return false;
}

bool ConsumerTable::consumerExists(std::string _consumerID) {
	std::lock_guard<std::mutex> lock(m_ConsumerTable);

	for (auto i = consumerTable.begin(); i != consumerTable.end(); ++i)
		if (i->first == _consumerID)
			return true;

	return false;
}

void ConsumerTable::deleteAllConsumer() {
	std::lock_guard<std::mutex> lock(m_ConsumerTable);

	for (auto i = consumerTable.begin(); i != consumerTable.end(); ++i) {
		terminateConsumerThread(i->second.enableOperation, i->second.ConsumerInterfaceThread);
	}
	consumerTable.clear();
}

void ConsumerTable::terminateConsumerThread(bool &_rEnableOperation, std::thread &_thread) {
	_rEnableOperation = false;

	try {
		_thread.join();
	}
	catch (...) {
		printf("Error: cannot join TCP client thread!\n");
	}
}

//
// Consumer Interface definitions
//

//todo: rewrite this function
void ConsumerTable::parseTCPRobotArmData(char *recvbuf, int length, std::string &x, std::string &y, std::string &z, std::string &s) {

	int pos1, pos2;
	char buff[512];

	for (int i = 0; i < length; ++i) {
		if (recvbuf[i] == 0x78) {
			pos1 = i;
			for (int j = i; j < length; ++j) {
				if (recvbuf[j] == 0x0a) {
					pos2 = j + 1;
					for (int k = pos1, l = 0; k < pos2; ++k, ++l) {
						buff[l] = recvbuf[k];
					}
					buff[pos2 - pos1] = '\0';
					x = std::string(buff);
					break;
				}
			}
		}
		else if (recvbuf[i] == 0x79) {
			pos1 = i;
			for (int j = i; j < length; ++j) {
				if (recvbuf[j] == 0x0a) {
					pos2 = j + 1;
					for (int k = pos1, l = 0; k < pos2; ++k, ++l) {
						buff[l] = recvbuf[k];
					}
					buff[pos2 - pos1] = '\0';
					y = std::string(buff);
					break;
				}
			}
		}
		else if (recvbuf[i] == 0x7a) {
			pos1 = i;
			for (int j = i; j < length; ++j) {
				if (recvbuf[j] == 0x0a) {
					pos2 = j + 1;
					for (int k = pos1, l = 0; k < pos2; ++k, ++l) {
						buff[l] = recvbuf[k];
					}
					buff[pos2 - pos1] = '\0';
					z = std::string(buff);
					break;
				}
			}
		}
		else if (recvbuf[i] == 0x73) {
			pos1 = i;
			for (int j = i; j < length; ++j) {
				if (recvbuf[j] == 0x0a) {
					pos2 = j + 1;
					for (int k = pos1, l = 0; k < pos2; ++k, ++l) {
						buff[l] = recvbuf[k];
					}
					buff[pos2 - pos1] = '\0';
					s = std::string(buff);
					break;
				}
			}
		}
	}

}


void ConsumerTable::createTCPRobotArmInterface(std::string sIPAddr, std::string sPort, bool &_ready, bool &_rbthreadStartedSuccessfully, std::string _consumerID) {

	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

#ifdef __linux__
	int sock;
	struct sockaddr_in server;

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket");
		_ready = true;
		return;
	}

	printf("TCP socket created\n");
	server.sin_addr.s_addr = inet_addr(sIPAddr.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(sPort.c_str()));

	//Connect to remote server
	if ( connect(sock, (struct sockaddr *)&server, sizeof(server) ) < 0) {
		printf("Error: Unable to connect to TCP server\n");
		_ready = true;
		return;
	}
	printf("TCP connected\n");

#elif _WIN32

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Error: WSAStartup failed with error: %d\n", iResult);
		_ready = true;
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(sIPAddr.c_str(), sPort.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("Error: getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		_ready = true;
		return;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Error: socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			_ready = true;
			return;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error: Unable to connect to server!\n");
		WSACleanup();
		_ready = true;
		return;
	}

	int iOptVal = 1000; //set receive timeout to 1000 ms
	int r = setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iOptVal, sizeof(iOptVal));

	BOOL b = TRUE;

	int r2 = setsockopt(ConnectSocket, (int)SOL_SOCKET, (int)SO_KEEPALIVE, (char *)&b, sizeof(b));
#endif

	std::string x = "0";
	std::string y = "0";
	std::string z = "0";
	std::string s = "0";

	int errorCode = 0;

	_rbthreadStartedSuccessfully = true;
	_ready = true;

#ifdef __linux__
	int iOptVal = 5; //set receive timeout to 500 ms ??
	struct timeval timeoutTime;
	timeoutTime.tv_sec = 1;
	timeoutTime.tv_usec = 0;

	int r = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeoutTime, sizeof(timeoutTime));
	int numberOfTimeouts = 0;

	while (true) {
		iResult = recv(sock, recvbuf, recvbuflen, 0);

		if( iResult == 0 ) {
			printf("Server disconnected\n");
			close(sock);
			printf("TCP socket closed\n");
			
			new std::thread( [ _consumerID, this ] {
			    printf("Termination thread started\n");
			    m_ConsumerTable.lock();
			    if(consumerTable[_consumerID].ConsumerInterfaceThread.joinable()){
				consumerTable[_consumerID].ConsumerInterfaceThread.join();
			    }
			    consumerTable.erase(_consumerID);
			    printf("%s - consumer erased from Consumer Table\n", _consumerID.c_str());
			    m_ConsumerTable.unlock();
			});
			
			return;
		}
		else if( iResult == -1 ) { //Timeout
			printf("Server Timeout\n");
			++numberOfTimeouts;

			if(numberOfTimeouts == 30){
			    new std::thread( [ _consumerID, this ] {
				printf("Timeout threashold reached (30 sec)\n");
				m_ConsumerTable.lock();
				if(consumerTable[_consumerID].ConsumerInterfaceThread.joinable()){
				    consumerTable[_consumerID].ConsumerInterfaceThread.join();
				}
				consumerTable.erase(_consumerID);
				printf("%s - consumer erased from Consumer Table\n", _consumerID.c_str());
				m_ConsumerTable.unlock();
			    });

			    return;
			}

			continue;
		}
		else{
		    numberOfTimeouts = 0;
		    printf("iResult: %d\n", iResult);
		}

		x = std::string("0"); y = std::string("0"); z = std::string("0"); s = std::string("0");

		parseTCPRobotArmData(recvbuf, iResult, x, y, z, s);

		printf("Received: %s, %s, %s, %s\n\n", x.c_str(), y.c_str(), z.c_str(), s.c_str());

		//update lastValue in consumerTable
		m_ConsumerTable.lock();

		if (x != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueX = x;
		if (y != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueY = y;
		if (z != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueZ = z;
		if (s != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueS = s;

		m_ConsumerTable.unlock();
	}

#elif _WIN32
	int numberOfTimeouts = 0;

	while (true) {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

		if (iResult == 0) {
			printf("Server disconnected\n");
			closesocket(ConnectSocket);
			WSACleanup();

			new std::thread([_consumerID, this] {
				printf("Terminating thread\n");
				m_ConsumerTable.lock();
				if (consumerTable[_consumerID].ConsumerInterfaceThread.joinable()) {
					consumerTable[_consumerID].ConsumerInterfaceThread.join();
				}
				consumerTable.erase(_consumerID);
				printf("%s - consumer erased from Consumer Table\n", _consumerID.c_str());
				m_ConsumerTable.unlock();
			});

			return;
		}
		else if (iResult == SOCKET_ERROR) {
			errorCode = WSAGetLastError();

			if (errorCode == WSAETIMEDOUT) {
				printf("receive timeout\n");
				++numberOfTimeouts;

				if (numberOfTimeouts == 30) {
					new std::thread([_consumerID, this] {
						printf("Timeout threashold reached (30 sec)\n");
						m_ConsumerTable.lock();
						if (consumerTable[_consumerID].ConsumerInterfaceThread.joinable()) {
							consumerTable[_consumerID].ConsumerInterfaceThread.join();
						}
						consumerTable.erase(_consumerID);
						printf("%s - consumer erased from Consumer Table\n", _consumerID.c_str());
						m_ConsumerTable.unlock();
					});

					return;
				}

				continue;
			}
			else {
				printf("recvfrom() failed with error code : %d", errorCode);
				break;
			}
		}
		else {
			numberOfTimeouts = 0;
		}

		x = std::string("0"); y = std::string("0"); z = std::string("0"); s = std::string("0");

		parseTCPRobotArmData(recvbuf, iResult, x, y, z, s);

		printf("Received: %s, %s, %s, %s\n\n", x.c_str(), y.c_str(), z.c_str(), s.c_str());

		//update lastValue in consumerTable
		m_ConsumerTable.lock();

		if (x != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueX = x;
		if (y != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueY = y;
		if (z != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueZ = z;
		if (s != "0")
			consumerTable[_consumerID].lastValue.TCPRobotArmValue.lastValueS = s;

		m_ConsumerTable.unlock();
	}
#endif

#ifdef __linux__
	close(sock);
#elif _WIN32
	closesocket(ConnectSocket);
	WSACleanup();
#endif

}
