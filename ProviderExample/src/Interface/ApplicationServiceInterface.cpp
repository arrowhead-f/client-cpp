
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

	SR_BASE_URI       = iniparser_getstring(pini, "Server:sr_base_uri", (char *)"http://10.0.0.77:8443/serviceregistry/");
	SR_BASE_URI_HTTPS = iniparser_getstring(pini, "Server:sr_base_uri_https", (char *)"https://10.0.0.77:8444/serviceregistry/");
	ADDRESS           = iniparser_getstring(pini, "Server:address", (char *)"10.0.0.77");
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

/*
inline std::string int_to_string(int i){
	return i > 9 ? to_string(i) : ("0" + to_string(i));
}
*/

inline const char *GetHttpPayload(Arrowhead_Data_ext &stAH_data, string ADDRESS, string ADDRESS6, unsigned short PORT)
{
//Expected content, example:
/*
{
 "serviceDefinition": "IndoorTemperature",
 "serviceUri": "temperature",
 "endOfValidity": "2019-12-05T12:00:00",
 "secure": "TOKEN",
 "version": 1,

 "providerSystem":
 {
   "systemName": "InsecureTemperatureSensor",
   "address": "192.168.0.2",
   "port": 8080,
   "authenticationInfo": "eyJhbGciOiJIUzI1Ni..."
 },

 "metadata": {
   "unit": "celsius"
},

 "interfaces": [
   "HTTP-SECURE-JSON"
 ]
}
*/
    json_object *jobj            = json_object_new_object();
    json_object *providerSystem  = json_object_new_object();
    json_object *jstring;
    json_object *jint;

/*
	time_t now = time(0);
	tm *lt = localtime(&now);

	std::string sValidity = to_string(lt->tm_year + 1900)   + "-" + int_to_string(lt->tm_mon + 1)  + "-" + int_to_string(lt->tm_mday) + " " +
	                        int_to_string(lt->tm_hour + 6)  + ":" + int_to_string(lt->tm_min)      + ":" + int_to_string(lt->tm_sec);
*/

	jstring = json_object_new_string(stAH_data.sServiceDefinition.c_str());
	json_object_object_add(jobj, "serviceDefinition", jstring);

	jstring = json_object_new_string(stAH_data.sServiceURI.c_str());
    json_object_object_add(jobj, "serviceUri", jstring);

	//jstring = json_object_new_string(sValidity.c_str());
	//json_object_object_add(jobj, "endOfValidity", jstring);

	jint = json_object_new_int(1);
    json_object_object_add(jobj, "version", jint);

/*
*   providerSystem section
*/
    jstring = json_object_new_string(stAH_data.sSystemName.c_str());
    json_object_object_add(providerSystem, "systemName", jstring);

    jstring = json_object_new_string( ADDRESS.size() != 0 ? ADDRESS.c_str() : ADDRESS6.c_str());
    json_object_object_add(providerSystem, "address", jstring);

    if(stAH_data.sAuthenticationInfo.size() != 0){
		jstring = json_object_new_string("TOKEN");
		json_object_object_add(jobj, "secure", jstring);

		jstring = json_object_new_string(stAH_data.sAuthenticationInfo.c_str());
		json_object_object_add(providerSystem, "authenticationInfo", jstring);
    }
    else{
		jstring = json_object_new_string("NOT_SECURE");
		json_object_object_add(jobj, "secure", jstring);
    }

	jint = json_object_new_int(PORT);
	json_object_object_add(providerSystem, "port", jint);

	json_object_object_add(jobj, "providerSystem", providerSystem);

/*
*   Interfaces, Metadata
*/

	json_object *jarray = json_object_new_array();

	if(stAH_data.sAuthenticationInfo.size() != 0)
		jstring = json_object_new_string("HTTP-SECURE-JSON");
	else
		jstring = json_object_new_string("HTTP-INSECURE-JSON");

	json_object_array_add(jarray, jstring);
	json_object_object_add(jobj, "interfaces", jarray);

	json_object *serviceMetadata = json_object_new_object();
	jstring = json_object_new_string(stAH_data.vService_Meta["unit"].c_str());
	json_object_object_add(serviceMetadata, "unit", jstring);

	if(stAH_data.sAuthenticationInfo.size() != 0){
		jstring = json_object_new_string(stAH_data.vService_Meta["security"].c_str());
		json_object_object_add(serviceMetadata, "security", jstring);
	}

	json_object_object_add(jobj, "metadata", serviceMetadata);

/*
*   Return
*/

	printf("\n%s\n", json_object_to_json_string(jobj));

    return json_object_to_json_string(jobj);

}

int ApplicationServiceInterface::registerToServiceRegistry(Arrowhead_Data_ext &stAH_data, bool _bSecureArrowheadInterface, bool _bProviderIsSecure )
{
	if(_bSecureArrowheadInterface)
          return SendHttpsRequest(GetHttpPayload(stAH_data, ADDRESS, ADDRESS6, _bProviderIsSecure ? PORT+1 : PORT), SR_BASE_URI_HTTPS + "register", "POST");
     else
          return SendRequest(GetHttpPayload(stAH_data, ADDRESS, ADDRESS6, _bProviderIsSecure ? PORT+1 : PORT), SR_BASE_URI + "register", "POST");
}

int ApplicationServiceInterface::unregisterFromServiceRegistry(Arrowhead_Data_ext &stAH_data, bool _bSecureArrowheadInterface, bool _bProviderIsSecure )
{
	std::string sParams = "service_definition=" + stAH_data.sServiceDefinition +
						  "&system_name=" + stAH_data.sSystemName +
						  "&address=";

    if(ADDRESS.size())
		sParams += ADDRESS;
	else
		sParams += ADDRESS6;

	sParams += "&port=";

	if(_bProviderIsSecure)
		sParams += std::to_string(PORT+1);
	else
		sParams += std::to_string(PORT);

	if(_bSecureArrowheadInterface)
		return SendHttpsRequest("", SR_BASE_URI_HTTPS + "unregister?" + sParams, "DELETE");
	else
		return SendRequest("",      SR_BASE_URI       + "unregister?" + sParams, "DELETE");
}
