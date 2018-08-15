
#include "ApplicationServiceInterface.hpp"

ApplicationServiceInterface::ApplicationServiceInterface( )
{

}

ApplicationServiceInterface::ApplicationServiceInterface( string ini_file )
{
	init_ApplicationServiceInterface(ini_file);
}

ApplicationServiceInterface::~ApplicationServiceInterface()
{
	deinit();
}

// HTTP_Handler overload
int ApplicationServiceInterface::httpGETCallback(const char *Id, string *pData_str)
{
	return Callback_Serve_HTTP_GET(Id, pData_str);
}

int ApplicationServiceInterface::Callback_Serve_HTTP_GET(const char *Id, string *pData_str)
{
	*pData_str = "5678";
	return 1;
}

// HTTPs_Handler overload
int ApplicationServiceInterface::httpsGETCallback(const char *Id, string *pData_str, string _sToken, string _sSignature, string _clientDistName)
{
	return Callback_Serve_HTTPs_GET(Id, pData_str, _sToken, _sSignature, _clientDistName);
}

int ApplicationServiceInterface::Callback_Serve_HTTPs_GET(const char *Id, string *pData_str, string _sToken, string _sSignature, string _clientDistName)
{
	*pData_str = "5678";
	return 1;
}

bool ApplicationServiceInterface::init_ApplicationServiceInterface( string ini_file )
{
	pini = Load_IniFile( (char *)ini_file.c_str() );
	if( !pini ) {
		printf("Error: Cannot load ApplicationServiceInterface.ini\n");
		return false;
	}

	SR_BASE_URI       = iniparser_getstring(pini, "Server:sr_base_uri", (char *)"http://10.0.0.10:8442/serviceregistry/");
	SR_BASE_URI_HTTPS = iniparser_getstring(pini, "Server:sr_base_uri_https", (char *)"https://10.0.0.10:8443/serviceregistry/");
	ADDRESS           = iniparser_getstring(pini, "Server:address", (char *)"10.0.0.11");
	ADDRESS6          = iniparser_getstring(pini, "Server:address6", (char *)"[::1]");
	PORT              = iniparser_getint(   pini, "Server:port", 8452);

	if(ADDRESS.size() != 0){

	    URI      = "http://"  + ADDRESS + ":" + to_string(PORT);
	    HTTPsURI = "https://" + ADDRESS + ":" + to_string(PORT+1);

	    if( MakeServer(PORT) ) {
          printf("Error: Unable to start HTTP Server (%s:%d)!\n", ADDRESS.c_str(), PORT);
          return false;
	    }

	    if(MakeHttpsServer(PORT+1)){
		printf("Error: Unable to start HTTPs Server (%s:%d)!\n", ADDRESS.c_str(), PORT);
		return false;
	    }

	    printf("\n(HTTP Server) started - %s:%d\n", ADDRESS.c_str(), PORT);
	    printf("(HTTPs Server) started - %s:%d\n", ADDRESS.c_str(), PORT+1);
	}
	else{
	    printf("Warning: Could not parse IPv4 address from config, trying to use IPv6!\n");

	    URI      = "http://"  + ADDRESS6 + ":" + to_string(PORT);
	    HTTPsURI = "https://" + ADDRESS6 + ":" + to_string(PORT+1);


	    if( MakeServer(PORT) ) {
		printf("Error: Unable to start HTTP Server (%s:%d)!\n", ADDRESS6.c_str(), PORT);
		return false;
	    }


	    if( MakeHttpsServer(PORT+1) ) {
		printf("Error: Unable to start HTTPs Server (%s:%d)!\n", ADDRESS6.c_str(), PORT+1);
		return false;
	    }

	    printf("\n(HTTP Server) started - %s:%d\n", ADDRESS6.c_str(), PORT);
	    printf("\n(HTTPs Server) started - %s:%d\n", ADDRESS6.c_str(), PORT+1);

	}

	return true;
}

int ApplicationServiceInterface::deinit( )
{
	Unload_IniFile();

	KillServer();
	KillHttpsServer();

    return 0;
}

dictionary *ApplicationServiceInterface::Load_IniFile(char *fname)
{
	pini = iniparser_load(fname);
	if( pini )
		iniparser_dump(pini, NULL);

	return pini;
}

int ApplicationServiceInterface::Unload_IniFile()
{
	if( pini )
	{
		iniparser_freedict(pini);
		pini = NULL;
		return 0;
	}
	return 1;
}

int ApplicationServiceInterface::registerToServiceRegistry(Arrowhead_Data_ext &stAH_data, bool _bSecureArrowheadInterface )
{
	//Expected content, example:
	//{
	//	"providedService": {
	//		"serviceDefinition" : "IndoorTemperature",
	//		"interfaces" : ["json"],
	//		"serviceMetadata" : {
	//			"unit" : "celsius"
	//		}
	//	},
	//	"provider":{
	//		"systemName" : "SecureTemperatureSensor",
	//		"address" : "10.0.0.2",
	//		"port" : 8454,
	//	},
	//	"serviceURI": "moteID/sensorID/interface-type",
	//
	//	"version" : 1
	//	"udp": false,
	//	"ttl": 255
	//}

	json jHTTPpayload;
	json jProvidedService;
	json jServiceMetadata;
	json jProvider;

     for(std::map<std::string,std::string>::iterator it = stAH_data.vService_Meta.begin(); it!=stAH_data.vService_Meta.end(); ++it ){
          if( (stAH_data.sAuthenticationInfo.size() == 0) && (strcmp(it->first.c_str(), "security") == 0) )
               continue;

          jServiceMetadata[it->first] = it->second;
     }

	jProvidedService["serviceDefinition"] = stAH_data.sServiceDefinition;
	jProvidedService["interfaces"] = { stAH_data.sserviceInterface };
	jProvidedService["serviceMetadata"] = jServiceMetadata;

	jProvider["systemName"] = stAH_data.sSystemName;
	jProvider["address"] = ADDRESS.size()==0 ? ADDRESS6 : ADDRESS;

	if(stAH_data.sAuthenticationInfo.size() != 0){
          jProvider["port"] = PORT+1;
          jProvider["authenticationInfo"] = stAH_data.sAuthenticationInfo;
	}
	else{
          jProvider["port"] = PORT;
          //jProvider["authenticationInfo"] = "null";
	}

	jHTTPpayload["providedService"] = jProvidedService;
	jHTTPpayload["provider"] = jProvider;
	jHTTPpayload["serviceURI"] = stAH_data.sServiceURI;
	jHTTPpayload["version"] = 1;
	//jHTTPpayload["udp"] = "false";
	//jHTTPpayload["ttl"] = 255;

	//printf("register\n");
	//printf("payload: %s\n", jHTTPpayload.dump().c_str());
	//printf("https uri: %s\n", SR_BASE_URI_HTTPS.c_str());

     if(_bSecureArrowheadInterface)
          return SendHttpsRequest(jHTTPpayload.dump(), SR_BASE_URI_HTTPS + "register", "POST");
     else
          return SendRequest(jHTTPpayload.dump(), SR_BASE_URI + "register", "POST");
}

int ApplicationServiceInterface::unregisterFromServiceRegistry(Arrowhead_Data_ext &stAH_data, bool _bSecureArrowheadInterface)
{
	json jHTTPpayload;
	json jProvidedService;
	json jServiceMetadata;
	json jProvider;

     for(std::map<std::string,std::string>::iterator it = stAH_data.vService_Meta.begin(); it!=stAH_data.vService_Meta.end(); ++it ){
          if( (stAH_data.sAuthenticationInfo.size() == 0) && (strcmp(it->first.c_str(), "security") == 0) )
               continue;

          jServiceMetadata[it->first] = it->second;
     }

	jProvidedService["serviceDefinition"] = stAH_data.sServiceDefinition;
	jProvidedService["interfaces"] = { stAH_data.sserviceInterface };
	jProvidedService["serviceMetadata"] = jServiceMetadata;

	jProvider["systemName"] = stAH_data.sSystemName;
	jProvider["address"] = ADDRESS.size()==0 ? ADDRESS6 : ADDRESS;
	jProvider["port"] = PORT;

	jHTTPpayload["providedService"] = jProvidedService;
	jHTTPpayload["provider"] = jProvider;
	jHTTPpayload["serviceURI"] = stAH_data.sServiceURI;
	jHTTPpayload["version"] = 1;
	//jHTTPpayload["udp"] = "false";
	//jHTTPpayload["ttl"] = 255;

     if(_bSecureArrowheadInterface)
          return SendHttpsRequest(jHTTPpayload.dump(), SR_BASE_URI_HTTPS + "remove", "PUT");
     else
          return SendRequest(jHTTPpayload.dump(), SR_BASE_URI + "remove", "PUT");
}
