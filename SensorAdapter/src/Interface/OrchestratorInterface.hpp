#pragma once

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <list>

#include "Http_Handler.hpp"
#include "Https_Handler.hpp"

#ifdef __linux__
#include "ini/iniparser.h"
#elif _WIN32
extern "C" {
#include "ini/iniparser.h"
}
#endif

class OrchestratorInterface : Http_Handler, Https_Handler {

private:

	dictionary  *pini = NULL;
	string OR_BASE_URI;
	string OR_BASE_URI_HTTPS;
	string ADDRESS;
	string ADDRESS6;
	uint16_t PORT;
	string URI;

	dictionary *Load_IniFile(char *fname);
	int Unload_IniFile();

public:
	string sConsumerID;

	OrchestratorInterface(string ini_file);
	OrchestratorInterface();
	~OrchestratorInterface();

	bool init_OrchestratorInterface(string ini_file);
	bool init_OrchestratorInterface(string _sHttpUri, string _sHttpsUri, string _sAddr, string _sAddr6, uint16_t _uPort);
	int deinit();

	int sendOrchestrationRequest(string rResult, bool _bSecureArrowheadInterface);

	size_t httpResponseCallback(char *ptr, size_t size);
	size_t httpsResponseCallback(char *ptr, size_t size);

	virtual size_t Callback_OrchestrationResponse(char *ptr, size_t size);
};
