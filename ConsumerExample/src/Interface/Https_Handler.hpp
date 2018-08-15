#pragma once
#include <string>

#ifdef __linux__

#include "curl.h"
#include "microhttpd.h"

#endif

using namespace std;

#define SERVERKEYFILE	"/home/tottthy/SensorGW/client1/client1.testcloud1.key"
#define SERVERCERTFILE	"/home/tottthy/SensorGW/client1/client1.testcloud1.crt"

class Https_Handler
{
private:
	struct MHD_Daemon *pmhd = NULL;
	char *key_pem;
	char *cert_pem;

public:
	int SendHttpsRequest(string pdata, string paddr, string pmethod);

	virtual int httpsGETCallback(const char *Id, string *pData_str, string sToken, string sSignature);
	virtual size_t httpsResponseCallback(char *ptr, size_t size);

	int MakeHttpsServer(unsigned short listen_port);
	int KillHttpsServer();

};
