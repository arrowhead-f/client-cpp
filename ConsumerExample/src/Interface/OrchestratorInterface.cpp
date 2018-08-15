#include "OrchestratorInterface.hpp"

OrchestratorInterface::OrchestratorInterface( )
{

}


OrchestratorInterface::OrchestratorInterface( string ini_file )
{
	init_OrchestratorInterface(ini_file);
}

OrchestratorInterface::~OrchestratorInterface()
{
	deinit();
}

// Overload Sensor-handler callback functionality here!
size_t OrchestratorInterface::httpResponseCallback(char *ptr, size_t size)
{
	return Callback_OrchestrationResponse(ptr, size);
}

size_t OrchestratorInterface::httpsResponseCallback(char *ptr, size_t size)
{
	return Callback_OrchestrationResponse(ptr, size);
}

size_t OrchestratorInterface::Callback_OrchestrationResponse(char *ptr, size_t size)
{
	printf("Callback_OrchestrationResponse -- need to overwrite\n");
	return size;
}

bool OrchestratorInterface::init_OrchestratorInterface( string ini_file )
{
	pini = Load_IniFile( (char *)ini_file.c_str() );
	if (!pini) {
		printf("Error: Cannot load OrchestratorInterface.ini\n");
		return false;
	}

	OR_BASE_URI = iniparser_getstring(pini, "Server:or_base_uri", (char *)"http://arrowhead.tmit.bme.hu:8440/orchestrator/orchestration");
	OR_BASE_URI_HTTPS = iniparser_getstring(pini, "Server:or_base_uri_https", (char *)"https://arrowhead.tmit.bme.hu:8441/orchestrator/orchestration");
	ADDRESS = iniparser_getstring(pini, "Server:address", (char *)"10.0.0.11");
	ADDRESS6 = iniparser_getstring(pini, "Server:address6", (char *)"[::1]");
	PORT = iniparser_getint(pini, "Server:port", 8453);

	if(ADDRESS.size() != 0){
	    URI = "http://" + ADDRESS + ":" + to_string(PORT);

	    if ( MakeServer(PORT) ) {
		printf("Error: Unable to start HTTP Server (%s:%d)!\n", ADDRESS.c_str(), PORT);
		return false;
	    }

	    printf("\nOrchestratorInterface started - %s:%d\n", ADDRESS.c_str(), PORT);

	}
	else{
	    printf("Warning: Could not parse IPv4 address from config, trying to use IPv6!\n");
	    URI = "http://" + ADDRESS6 + ":" + to_string(PORT);

	    if ( MakeServer(PORT) ) {
	    printf("Error: Unable to start HTTP Server (%s:%d)!\n", ADDRESS6.c_str(), PORT);
	    return false;
	    }

	    printf("\nOrchestratorInterface started - %s:%d\n", ADDRESS6.c_str(), PORT);
	}

	return true;
}

int OrchestratorInterface::deinit( )
{
	Unload_IniFile();

	KillServer();

    return 0;
}

dictionary *OrchestratorInterface::Load_IniFile(char *fname)
{
	pini = iniparser_load(fname);
	if (pini)
		iniparser_dump(pini, NULL);

	return pini;
}

int OrchestratorInterface::Unload_IniFile()
{
	if (pini)
	{
		iniparser_freedict(pini);
		pini = NULL;
		return 0;
	}
	return 1;
}

int OrchestratorInterface::sendOrchestrationRequest(string requestForm, bool _bSecureArrowheadInterface)
{
     if(_bSecureArrowheadInterface)
          return SendHttpsRequest(requestForm, OR_BASE_URI_HTTPS, "POST");
	else
          return SendRequest(requestForm, OR_BASE_URI, "POST");
}
