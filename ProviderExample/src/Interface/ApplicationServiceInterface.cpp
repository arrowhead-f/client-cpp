
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

inline const char *GetHttpPayload(Arrowhead_Data_ext &stAH_data, string ADDRESS, string ADDRESS6, unsigned short PORT)
{

//Expected content, example:
//{
//	"providedService": {
//		"serviceDefinition" : "IndoorTemperature",
//		"interfaces" : ["json"],
//		"serviceMetadata" : {
//			"unit" : "celsius",
//			"security" : "token"
//		}
//	},
//	"provider":{
//		"systemName" : "SecureTemperatureSensor",
//		"address" : "10.0.0.2",
//		"port" : 8454,
//		"authenticationInfo" : "dfasfdsafdsa"
//	},
//	"serviceURI": "moteID/sensorID/interface-type",
//
//	"version" : 1
//	"udp": false,
//	"ttl": 255
//}

    json_object *jobj            = json_object_new_object();
    json_object *providedService = json_object_new_object();
    json_object *provider        = json_object_new_object();
    json_object *jstring;
    json_object *jint;

/*
*   ProvidedService section
*/

    jstring = json_object_new_string(stAH_data.sServiceDefinition.c_str());
    json_object_object_add(providedService, "serviceDefinition", jstring);

    json_object *jarray = json_object_new_array();
    jstring = json_object_new_string(stAH_data.sserviceInterface.c_str());
    json_object_array_add(jarray, jstring);
    json_object_object_add(providedService, "interfaces", jarray);

    json_object *serviceMetadata = json_object_new_object();
    jstring = json_object_new_string(stAH_data.vService_Meta["unit"].c_str());
    json_object_object_add(serviceMetadata, "unit", jstring);

    if(stAH_data.sAuthenticationInfo.size() != 0){
	jstring = json_object_new_string(stAH_data.vService_Meta["security"].c_str());
	json_object_object_add(serviceMetadata, "security", jstring);
    }

    json_object_object_add(providedService, "serviceMetadata", serviceMetadata);

/*
*   Provider section
*/
    jstring = json_object_new_string(stAH_data.sSystemName.c_str());
    json_object_object_add(provider, "systemName", jstring);

    jstring = json_object_new_string( ADDRESS.size() != 0 ? ADDRESS.c_str() : ADDRESS6.c_str());
    json_object_object_add(provider, "address", jstring);

    if(stAH_data.sAuthenticationInfo.size() != 0){

	jstring = json_object_new_string(stAH_data.sAuthenticationInfo.c_str());
	json_object_object_add(provider, "authenticationInfo", jstring);

	jint = json_object_new_int(PORT+1);
	json_object_object_add(provider, "port", jint);
    }
    else{
	jint = json_object_new_int(PORT);
	json_object_object_add(provider, "port", jint);
    }

/*
*   Concatenation
*/

    json_object_object_add(jobj, "providedService", providedService);
    json_object_object_add(jobj, "provider", provider);

    jstring = json_object_new_string(stAH_data.sServiceURI.c_str());
    json_object_object_add(jobj, "serviceURI", jstring);

    jint = json_object_new_int(1);
    json_object_object_add(jobj, "version", jint);

/*
*   Return
*/

    return json_object_to_json_string(jobj);

}

int ApplicationServiceInterface::registerToServiceRegistry(Arrowhead_Data_ext &stAH_data, bool _bSecureArrowheadInterface )
{
	if(_bSecureArrowheadInterface)
          return SendHttpsRequest(GetHttpPayload(stAH_data, ADDRESS, ADDRESS6, PORT), SR_BASE_URI_HTTPS + "register", "POST");
     else
          return SendRequest(GetHttpPayload(stAH_data, ADDRESS, ADDRESS6, PORT), SR_BASE_URI + "register", "POST");
}

int ApplicationServiceInterface::unregisterFromServiceRegistry(Arrowhead_Data_ext &stAH_data, bool _bSecureArrowheadInterface)
{
     if(_bSecureArrowheadInterface)
          return SendHttpsRequest(GetHttpPayload(stAH_data, ADDRESS, ADDRESS6, PORT), SR_BASE_URI_HTTPS + "remove", "PUT");
     else
          return SendRequest(GetHttpPayload(stAH_data, ADDRESS, ADDRESS6, PORT), SR_BASE_URI + "remove", "PUT");
}
