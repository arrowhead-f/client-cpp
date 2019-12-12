
#include "SensorHandler.h"
#include <map>
#include <mutex>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <thread>
#include <algorithm>
#include <time.h>
#include <vector>
#include "StringManipulation.hpp"

#include "../Security/RSASecurity.h"

SensorHandler::SensorHandler(){
     bSecureProviderInterface = false;

	if (!init_OrchestratorInterface("OrchestratorInterface.ini")) {
		printf("Error: Unable to start Orchestrator Interface!\n");
	}
}

SensorHandler::~SensorHandler(void){

}

void SensorHandler::processConsumer(std::string pJsonSenML, bool _bSecureArrowheadInterface) {
	json_object *obj = json_tokener_parse(pJsonSenML.c_str());

	if(obj == NULL){
	    printf("Error: Could not parse payload: %s\n", pJsonSenML.c_str());
	    return;
	}

	json_object *jBN;
	if(!json_object_object_get_ex(obj, "bn", &jBN)){
	    printf("Error: received json does not contain bn field!\n");
	    return;
	}

	std::string consumerID = std::string(json_object_get_string(jBN));

	printf("consumerID: %s\n", consumerID.c_str());

     std::string requestForm;
     if(!oConsumedService.getRequestForm(consumerID, requestForm)){
          printf("Error: Request Form is missing for %s!\n", consumerID.c_str());
          return;
     }

     printf("Sending Orchestration Request: (%s)\n", _bSecureArrowheadInterface ? "Secure Arrowhead Interface" : "Insecure Arrowhead Interface");

     int returnValue = sendOrchestrationRequest(requestForm, _bSecureArrowheadInterface);
}


size_t SensorHandler::Callback_OrchestrationResponse(char *ptr, size_t size) {
//
//Expected Response -- example
/*
{
   "response":[
      {
         "provider":{
            "id":6,
            "systemName":"securetemperaturesensor",
            "address":"10.0.0.77",
            "port":8452,
            "createdAt":"2019-10-28 06:53:32",
            "updatedAt":"2019-10-28 06:53:32"
         },
         "service":{
            "id":11,
            "serviceDefinition":"indoortemperature_providerexample",
            "createdAt":"2019-10-22 13:12:07",
            "updatedAt":"2019-10-22 13:12:07"
         },
         "serviceUri":null,
         "secure":"NOT_SECURE",
         "metadata":{
            "unit":"Celsius"
         },
         "interfaces":[
            {
               "id":2,
               "interfaceName":"HTTP-INSECURE-JSON",
               "createdAt":"2019-10-22 10:50:30",
               "updatedAt":"2019-10-22 10:50:30"
            }
         ],
         "version":1,
         "authorizationTokens":{
            "interfaceName1": "token1"
         }
         "warnings":[

         ]
      }
   ]
}
*/

     printf("Orchestration response: %s\n", ptr);

     std::string token;
     std::string signature;
     string sIPAddress;
     uint32_t uPort;
     string sURI;

     struct json_object *obj = json_tokener_parse(ptr);
     if(obj == NULL){
         printf("Error: could not parse orchestration response\n");
         return 1;
     }

     struct json_object *jResponseArray;
     if(!json_object_object_get_ex(obj, "response", &jResponseArray)){
         printf("Error: could not parse response\n");
         return 1;
     }

    struct json_object *jResponse = json_object_array_get_idx(jResponseArray, 0);
    struct json_object *jProvider;

    if(!json_object_object_get_ex(jResponse, "provider", &jProvider)){
        printf("Error: could not parse provider section\n");
        return 1;
    }

     struct json_object *jAddr;
     struct json_object *jPort;
     struct json_object *jService;
     struct json_object *jUri;
     struct json_object *jToken;
     struct json_object *jSignature;

     if(!json_object_object_get_ex(jProvider, "address",    &jAddr))    {printf("Error: could not find address\n");    return 1;}
     if(!json_object_object_get_ex(jProvider, "port",       &jPort))    {printf("Error: could not find port\n");       return 1;}
     if(!json_object_object_get_ex(jResponse, "service",    &jService)) {printf("Error: could not find service\n");    return 1;}
     if(!json_object_object_get_ex(jResponse, "serviceUri", &jUri))     {printf("Error: could not find serviceURI\n"); return 1;}

	sIPAddress = string(json_object_get_string(jAddr));
	uPort      = json_object_get_int(jPort);
    sURI       = string(json_object_get_string(jUri));

    if(bSecureProviderInterface){
        struct json_object *jAuthTokens;
        struct json_object *jToken;
        std::string sToken;

        if(!json_object_object_get_ex(jResponse, "authorizationTokens", &jAuthTokens)){
            printf("Error: could not parse authorizationTokens section\n");
            return 1;
        }

        if(!json_object_object_get_ex(jAuthTokens, "HTTP-SECURE-JSON", &jToken)){
            printf("Error: could not find Token\n");
            return 1;
        }

        sToken = string(json_object_get_string(jToken));

        std::string sProviderURI = "https://" + sIPAddress + ":" + std::to_string(uPort) + "/" + sURI + "?token=" + sToken; // + "&signature=" + signature;

        sendHttpsRequestToProvider(sProviderURI);
     }
     else{
        std::string sProviderURI = "http://" + sIPAddress + ":" + std::to_string(uPort) + "/" + sURI;
        sendRequestToProvider(sProviderURI);
     }

	return size;
}

inline size_t providerHttpResponseHandler(char *ptr, size_t size, size_t nmemb, void *userdata){
	return ((SensorHandler *)userdata)->providerHttpResponseCallback(ptr, size*nmemb);
}

void SensorHandler::sendRequestToProvider(std::string _sProviderURI){

     printf("\nsendHttpRequestToProvider\n");

     int http_code = 0;
     CURLcode res;
  	CURL *curl;
	struct curl_slist *headers = NULL;
  	string agent;

	curl_global_init(CURL_GLOBAL_ALL);

  	curl = curl_easy_init();

  	if(curl){
		agent = "libcurl/"+string(curl_version_info(CURLVERSION_NOW)->version);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &providerHttpResponseHandler);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_URL, _sProviderURI.c_str());

		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
  	}

     curl_global_cleanup();
}

void SensorHandler::sendHttpsRequestToProvider(std::string _sProviderURI){

     //printf("\nsendHttpsRequestToProvider: %s\n\n", _sProviderURI.c_str());
     printf("\nsendHttpsRequestToProvider\n");

    int http_code = 0;
    CURLcode res;
  	CURL *curl;
	struct curl_slist *headers = NULL;
  	string agent;

	curl_global_init(CURL_GLOBAL_ALL);

  	curl = curl_easy_init();

  	if(curl){
		agent = "libcurl/"+string(curl_version_info(CURLVERSION_NOW)->version);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &providerHttpResponseHandler);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

          //---------------HTTPS SECTION--------------------------------------------------------
          //
          //--verbose
          if ( curl_easy_setopt(curl, CURLOPT_VERBOSE,        1L)            != CURLE_OK)
               printf("error: CURLOPT_VERBOSE\n");
          //--insecure
          if ( curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L)            != CURLE_OK)
               printf("error: CURLOPT_SSL_VERIFYPEER\n");
          if ( curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L)            != CURLE_OK)
               printf("error: CURLOPT_SSL_VERIFYHOST\n");
          //--cert
          if ( curl_easy_setopt(curl, CURLOPT_SSLCERT,        "keys2/clcert.pem")  != CURLE_OK)
               printf("error: CURLOPT_SSLCERT\n");
          //--cert-type
          if ( curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,    "PEM")         != CURLE_OK)
               printf("error: CURLOPT_SSLCERTTYPE\n");
          //--key
          if ( curl_easy_setopt(curl, CURLOPT_SSLKEY,         "keys2/privkey.pem") != CURLE_OK)
               printf("error: CURLOPT_SSLKEY\n");
          //--key-type
          if ( curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,     "PEM")         != CURLE_OK)
               printf("error: CURLOPT_SSLKEYTYPE\n");
          //--pass
          if ( curl_easy_setopt(curl, CURLOPT_KEYPASSWD,      "123456")       != CURLE_OK)
               printf("error: CURLOPT_KEYPASSWD\n");
          //--cacert
          if ( curl_easy_setopt(curl, CURLOPT_CAINFO,         "keys2/cacert.pem")  != CURLE_OK)
               printf("error: CURLOPT_CAINFO\n");
          //
          //---------------END OF HTTPS SECTION-------------------------------------------------

		curl_easy_setopt(curl, CURLOPT_URL, _sProviderURI.c_str());

		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

          curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
  	}

     curl_global_cleanup();
}

size_t SensorHandler::providerHttpResponseCallback(char *ptr, size_t size){
	printf("\nProvider Response:\n%s\n", ptr);

     deinit();

	return size;
}
