
#include "SensorHandler.h"
#include <map>
#include <mutex>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <thread>
#include <algorithm>
#include <time.h>
#include "../Security/RSASecurity.h"


SensorHandler::SensorHandler(){
	if (!init_ApplicationServiceInterface("ApplicationServiceInterface.ini")) {
		printf("Error: Unable to start Service Registry Interface!\n");
	}

	sensorIsRegistered = false;
}

SensorHandler::~SensorHandler(void){

}

void SensorHandler::processProvider(std::string pJsonSenML, bool _bProviderIsSecure, bool _bSecureArrowheadInterface) {
// todo:
// Delete the following source code if not using json/SenML

	json_object *obj = json_tokener_parse(pJsonSenML.c_str());

	if(obj == NULL){
	    printf("Error: Could not parse SenML: %s\n", pJsonSenML.c_str());
	    return;
	}

    json_object *jBN;
	if(!json_object_object_get_ex(obj, "bn", &jBN)){
	    printf("Error: received json does not contain bn field!\n");
	    return;
	}

	baseName = std::string(json_object_get_string(jBN));


//todo: check baseUnit and value... SenML must contain the measured value


//do not modify below this

	if (sensorIsRegistered) {
		lastMeasuredValue = pJsonSenML;
		printf("New measurement received from: %s\n", baseName.c_str());
		printf("LastValue updated.\n");
		return;
	}

	printf("\nMeasured value received from: (Base Name: %s)\n", baseName.c_str());
	printf("Provider is not registered yet!\n");

	if (registerSensor(pJsonSenML, _bProviderIsSecure, _bSecureArrowheadInterface)) {
		printf("Provider Registration is successful!\n");
	}
	else {
		printf("Provider Registration is unsuccessful!\n");
	}
}

/*
--
--
-- Provider
-- Registration, deregistration
--
*/
bool SensorHandler::registerSensor(std::string _jsonSenML, bool _bProviderIsSecure, bool _bSecureArrowheadInterface){
	printf("\nREGISTRATION (%s, %s)\n\n", _bProviderIsSecure ? "Secure Provider" : "Insecure Provider", _bSecureArrowheadInterface ? "Secure AHInterface" : "Insecure AHInterface");

	Arrowhead_Data_ext ah_dta_ext;

	if (!oProvidedService.getSystemName(ah_dta_ext.sSystemName)) {
		printf("Error: Cannot find SystemName\n");
		return false;
	}

	if(!oProvidedService.getServiceDefinition(ah_dta_ext.sServiceDefinition)) {
		printf("Error: Cannot find ServiceDefinition\n");
		return false;
	}

	if (!oProvidedService.getServiceInterface(ah_dta_ext.sserviceInterface)) {
		printf("Error: Cannot find ServiceInterface\n");
		return false;
	}

	if (_bProviderIsSecure && !oProvidedService.getPrivateKeyPath(privateKeyPath)) {
		printf("Error: Cannot find privateKeyPath for secure sensor: %s\n", baseName.c_str());
		return false;
	}

	if (_bProviderIsSecure && !oProvidedService.getPublicKeyPath(publicKeyPath)) {
		printf("Error: Cannot find publicKeyPath for secure sensor: %s\n", baseName.c_str());
		return false;
	}

	oProvidedService.getCustomURL(customURL);
    ah_dta_ext.sServiceURI = customURL;

	if(_bProviderIsSecure){
		std::ifstream ifs(publicKeyPath.c_str());
		std::string pubkeyContent( (std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()) );

		pubkeyContent.erase(0, pubkeyContent.find("\n") + 1);
		pubkeyContent = pubkeyContent.substr(0, pubkeyContent.size()-25);

        pubkeyContent.erase(std::remove(pubkeyContent.begin(), pubkeyContent.end(), '\n'), pubkeyContent.end());

		printf("pubkeyContent: %s\n\n", pubkeyContent.c_str());

		ah_dta_ext.sAuthenticationInfo = pubkeyContent;

	}

    for(std::map<std::string,std::string>::iterator it = oProvidedService.metadata.begin(); it != oProvidedService.metadata.end(); ++it )
		ah_dta_ext.vService_Meta.insert(std::pair<string,string>(it->first, it->second));

	int returnValue = registerToServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface, _bProviderIsSecure);

	printf("%s Post sent (SenML baseName = %s)\n", _bSecureArrowheadInterface? "HTTPs" : "HTTP", baseName.c_str());
	printf("%s Post return value: %d\n", _bSecureArrowheadInterface? "HTTPs" : "HTTP", returnValue);

	if (returnValue == 201 /*Created*/){
        sensorIsRegistered = true;
		lastMeasuredValue = _jsonSenML;
		return true;
	}
	else{
        printf("Already registered?\n");
		printf("Try re-registration\n");

		returnValue = unregisterFromServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface, _bProviderIsSecure);

		if (returnValue == 200 /*OK*/ || returnValue == 204 /*No Content*/) {
			printf("Unregistration is successful\n");
		}
		else {
			printf("Unregistration is unsuccessful\n");
			return false;
		}

		returnValue = registerToServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface, _bProviderIsSecure);

		if (returnValue == 201 /*Created*/) {
            sensorIsRegistered = true;
			lastMeasuredValue  = _jsonSenML;
            return true;
		}
		else {
			return false; //unsuccessful registration
		}
	}
}

//Send HTTP DELETE to ServiceRegistry
bool SensorHandler::deregisterSensor(std::string _baseName, bool _bSecureArrowheadInterface, bool _bProviderIsSecure){

	Arrowhead_Data_ext ah_dta_ext;

	if (!oProvidedService.getSystemName(ah_dta_ext.sSystemName)) {
		printf("Error: Cannot find SystemName\n");
		return false;
	}

	if (!oProvidedService.getServiceDefinition(ah_dta_ext.sServiceDefinition)) {
		printf("Error: Cannot find ServiceDefinition\n");
		return false;
	}

	if (!oProvidedService.getServiceInterface(ah_dta_ext.sserviceInterface)) {
		printf("Error: Cannot find ServiceInterface\n");
		return false;
	}

     for(std::map<std::string,std::string>::iterator it = oProvidedService.metadata.begin(); it != oProvidedService.metadata.end(); ++it )
          ah_dta_ext.vService_Meta.insert(std::pair<string,string>(it->first, it->second));

     if (!oProvidedService.getCustomURL(customURL)) {
		printf("Error: Cannot find customURL\n");
		return false;
	}

    ah_dta_ext.sServiceURI = customURL;

	int returnValue = unregisterFromServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface, _bProviderIsSecure);

	if( returnValue == 200 /*OK*/ || returnValue == 204 /*No Content*/) {
		return true;
	}

	return false; //unsuccessful unregistration
}

/*
--
-- Called, when Consumer request arrives -- HTTP GET Request
--
*/
int SensorHandler::Callback_Serve_HTTP_GET(const char *URL, string *pResponse){
     printf("\nHTTP GET request received\n");

     printf("Received URL: %s\n", URL);
     std::string tmp = "/" + customURL;
	if (strcmp(tmp.c_str(), URL) != 0) {
		printf("Error: Unknown URL: %s\n", URL);
		return 1;
	}

     *pResponse = lastMeasuredValue;
     printf("Response:\n%s\n\n", lastMeasuredValue.c_str());

	return 1;
}

int SensorHandler::Callback_Serve_HTTPs_GET(const char *URL, string *pResponse, string _sToken, string _sSignature, string _clientDistName){
	printf("\nHTTPs GET request received\n");

     printf("Received URL: %s\n", URL);
     std::string tmp = "/" + customURL;
	if (strcmp(tmp.c_str(), URL) != 0) {
		printf("Error: Unknown URL: %s\n", URL);
		return 1;
	}
/*
	if( strstr(_sToken.c_str(), " ") != NULL )
		 replace(_sToken.begin(),     _sToken.end(),     ' ', '+');

	printf("Token: %s\n\n\n", _sToken.c_str());
*/

//Todo: Decrypt, verify and decode token

     *pResponse = lastMeasuredValue;
     printf("Response:\n%s\n\n", lastMeasuredValue.c_str());

	return 1;
}
