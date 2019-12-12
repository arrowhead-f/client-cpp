#pragma once
#include <string>

#ifdef __linux__

#include "curl.h"
#include "microhttpd.h"

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#endif

using namespace std;

#define SERVERKEYFILE	"keys2/tempsensor.testcloud2.privkey.pem" //password protected private key
#define SERVERCERTFILE	"keys2/tempsensor.testcloud2.clcert.pem"
#define ROOTCACERTFILE  "keys2/tempsensor.testcloud2.caCert.pem"

class Https_Handler
{
private:
	struct MHD_Daemon *pmhd = NULL;
	char *key_pem;
	char *cert_pem;
	char *root_ca_pem;

public:
	int SendHttpsRequest(string pdata, string paddr, string pmethod);

	virtual int httpsGETCallback(const char *Id, string *pData_str, string sToken, string sSignature, string clientDistName);

	virtual size_t httpsResponseCallback(char *ptr, size_t size);

	int MakeHttpsServer(unsigned short listen_port);
	int KillHttpsServer();

};
