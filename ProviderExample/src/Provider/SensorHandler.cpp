
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

using namespace nlohmann;

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

	json jsonSenML;

	try {
		jsonSenML = json::parse(pJsonSenML.c_str());
		baseName  = jsonSenML.at("bn").get<std::string>();
		baseUnit  = jsonSenML.at("bu").get<std::string>();
		value    = jsonSenML["e"][0].at("v").get<double>(); //SenML must contain the measured value
	}
	catch (exception &e) {
		printf("Error: %s\n", e.what());
		return;
	}

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

          ah_dta_ext.sAuthenticationInfo = pubkeyContent;
	}

     for(std::map<std::string,std::string>::iterator it = oProvidedService.metadata.begin(); it != oProvidedService.metadata.end(); ++it )
          ah_dta_ext.vService_Meta.insert(std::pair<string,string>(it->first, it->second));

	int returnValue = registerToServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface);

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

		returnValue = unregisterFromServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface);

		if (returnValue == 200 /*OK*/ || returnValue == 204 /*No Content*/) {
			printf("Unregistration is successful\n");
		}
		else {
			printf("Unregistration is unsuccessful\n");
			return false;
		}

		returnValue = registerToServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface);

		if (returnValue == 201 /*Created*/) {
               sensorIsRegistered = true;
			lastMeasuredValue = _jsonSenML;
               return true;
		}
		else {
			return false; //unsuccessful registration
		}
	}
}

//Send HTTP PUT to ServiceRegistry
bool SensorHandler::deregisterSensor(std::string _baseName, bool _bSecureArrowheadInterface){

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

	int returnValue = unregisterFromServiceRegistry(ah_dta_ext, _bSecureArrowheadInterface);

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

//Set the own private key path for RSA algorithms

     RSASecurity oRSASecurity;
	oRSASecurity.privateKeyPath = privateKeyPath;

	if(oRSASecurity.privateKeyPath.size() == 0){
	    printf("Error: Unknown Provider Private Key File Path\n");
	    return 1;
	}

//We have to replace the 0x20 ascii code (space) with 0x2B (plus), before base64 decryption

     if( strstr(_sToken.c_str(), " ") != NULL )
          replace(_sToken.begin(),     _sToken.end(),     ' ', '+');

     if( strstr(_sSignature.c_str(), " ") != NULL )
          replace(_sSignature.begin(), _sSignature.end(), ' ', '+');

	oRSASecurity.sB64EncodedRSAEncryptedToken     = _sToken;
	oRSASecurity.sB64EncodedSignature             = _sSignature;

//Verify RSA signature

	if(oRSASecurity.getVerificationResult()){
	    printf("Successful RSA Signature verification\n");
	}
	else{
	    printf("Error: Unsuccessful RSA Signature verification - Wrong signature?\n");
	    return 1;
	}

//Parse the base64 decrypted token information, which should be a json string

     printf("\nRaw token info:\n%s\n\n", oRSASecurity.getDecryptedToken().c_str());

     json jsonRawToken;
     try{
          jsonRawToken = json::parse(oRSASecurity.getDecryptedToken().c_str());
     }
     catch (exception& e) {
		printf("Error: %s\n", e.what());
		return false;
	}

	std::string service = "s";
	try {
		service = jsonRawToken.at("s").get<std::string>();
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return false;
	}

	std::string consumerCommonName = "c";
	try {
		consumerCommonName = jsonRawToken.at("c").get<std::string>();
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return 1;
	}

	uint64_t expired = 0;

	try {
		expired = jsonRawToken.at("e").get<uint64_t>();
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return 1;
	}

//check s - format: interface.serviceDefinition
//
     std::string serviceInterface;
     if(!oProvidedService.getServiceInterface(serviceInterface)){
          printf("Error: Could not get Provider's Service Interface\n");
     }

     std::string serviceDefinition;
     if(!oProvidedService.getServiceDefinition(serviceDefinition)){
          printf("Error: Could not get Provider's Service Definition\n");
     }

     std::string expectedService = serviceInterface + (std::string)"." + serviceDefinition;

     if(strcmp(service.c_str(), expectedService.c_str()) != 0){
          printf("Error: s (%s) parameter from Raw token info is not equal to the expected %s value\n", service.c_str(), expectedService.c_str() );
          return 1;
     }
     else{
          printf("service identification is successful\n");
     }

//check c - consumer certification common name
//
//Example:
//client distinguished name: C=HU,CN=client1.testcloud1.aitia.arrowhead.eu
//consumerCommonName : client1.SmartGrid.SmartGridOperator

     vector<string> dn_content   = split<string>(_clientDistName, ",");
     vector<string> clientCN_v   = split<string>(dn_content[1], ".");
     vector<string> rawTokenCN_v = split<string>(consumerCommonName, ".");

     if(strcmp( clientCN_v[0].substr( 3, clientCN_v[0].size() ).c_str(), rawTokenCN_v[0].c_str() ) != 0){
          printf("Error: Client CN (%s) is not equal to raw token CN (%s)\n", clientCN_v[0].substr( 3, clientCN_v[0].size() ).c_str(), rawTokenCN_v[0].c_str() );
          return 1;
     }

//check e - expiration time
	if(expired != 0){
          time_t linuxEpochTime = std::time(0);

          if(expired < (uint64_t)linuxEpochTime){
               printf("Error: Expired time(%llu) is smaller than linux epoch time(%llu)!", expired, (uint64_t)linuxEpochTime);
               return 1;
          }
	}

     *pResponse = lastMeasuredValue;
     printf("Response:\n%s\n\n", lastMeasuredValue.c_str());

	return 1;
}
