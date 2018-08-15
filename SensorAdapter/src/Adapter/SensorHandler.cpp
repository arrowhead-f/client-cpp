
#include <map>
#include <mutex>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <thread>
#include <algorithm>
#include <time.h>

#include "SensorHandler.h"

using namespace nlohmann;

SensorHandler::SensorHandler(){

     dictionary *iniFile = iniparser_load((char *)sIniFilePath.c_str());

     if (iniFile)
          iniparser_dump(iniFile, NULL);

	if( !iniFile ) {
		printf("Error: Cannot load %s\n", sIniFilePath.c_str());
	}

     std::string sServRegHttpUri  = iniparser_getstring(iniFile, "AHServiceRegistry:baseUri",       (char *)"http://arrowhead.tmit.bme.hu:8442/serviceregistry/");
     std::string sServRegHttpsUri = iniparser_getstring(iniFile, "AHServiceRegistry:secureBaseUri", (char *)"https://arrowhead.tmit.bme.hu:8443/serviceregistry/");
     uint16_t    uServRegPort     = iniparser_getint   (iniFile, "AHServiceRegistry:port",           8452);

     std::string sOrchHttpUri     = iniparser_getstring(iniFile, "AHOrchestrator:baseUri",          (char *)"http://arrowhead.tmit.bme.hu:8440/orchestrator/orchestration");
     std::string sOrchHttpsUri    = iniparser_getstring(iniFile, "AHOrchestrator:baseUri",          (char *)"https://arrowhead.tmit.bme.hu:8441/orchestrator/orchestration");
     uint16_t    uOrchIntfPort    = iniparser_getint   (iniFile, "AHOrchestrator:port",              8454);

     std::string sIPv4Addr        = iniparser_getstring(iniFile, "AHAdapter:address",                 (char *)"127.0.0.1");
     std::string sIPv6Addr        = iniparser_getstring(iniFile, "AHAdapter:address6",                (char *)"[::1]");

     uint16_t    uSecAHIntf       = iniparser_getint(iniFile, "AHAdapter:secureAHIntf",             0);
     uint16_t    uMoteIntfUDPPort = iniparser_getint(iniFile, "AHAdapter:moteIntfUDPPort",          65000);

     if (iniFile){
          iniparser_freedict(iniFile);
          iniFile = NULL;
     }

     bSecureArrowheadInterface = (uSecAHIntf == 0) ? false : true;

     printf("bSecureArrowheadInterface: %d\n", bSecureArrowheadInterface);

     if(!init_ServiceRegistryInterface(sServRegHttpUri, sServRegHttpsUri, sIPv4Addr, sIPv6Addr, uServRegPort)){
		printf("Error: Unable to start Service Registry Interface!\n");
	}

     if(!init_OrchestratorInterface( sOrchHttpUri, sOrchHttpsUri, sIPv4Addr, sIPv6Addr, uOrchIntfPort)){
		printf("Error: Unable to start Orchestrator Interface!\n");
	}

	startMoteUDPInterface(uMoteIntfUDPPort);

     printf("Adapter mode: %s Arrowhead interface\n\n", bSecureArrowheadInterface ? (char *)"Secure" : (char *)"Insecure");

	oWhiteList.startPeriodicRefresh();
}

SensorHandler::~SensorHandler(void){

}

void SensorHandler::stopMoteInterface() {
	stopMoteUDPInterface();
}

/*
--
-- MoteInterface
--
-- Called, when Adapter receives a new measurement value through UDPMoteInterface
*/
void SensorHandler::processReceivedSensorDataThroughUDPMoteInterface(std::string pJsonSenML) {
	json jsonSenML;

	try {
		jsonSenML = json::parse(pJsonSenML.c_str());
	}
	catch (exception &e) {
		printf("Error: %s\n", e.what());
		return;
	}

	std::string baseName;
	try {
		baseName = jsonSenML.at("bn").get<std::string>();
	}
	catch (...) {
		printf("Error: received json does not contain bn field!\n");
		return;
	}

	vector<string> v = split<string>(baseName, ":");

	if(v.size() != 3){
	    printf("Error: Unknown base name format. Expected: moteID:SensorID:Flags");
	    return;
	}

	bool bProvider	          = strstr(v[2].c_str(),"P") != NULL;
	bool bConsumer 		= strstr(v[2].c_str(),"C") != NULL;
	bool bTranslationToMQTT  = strstr(v[2].c_str(),"M") != NULL;
	bool bProviderIsSecure 	= strstr(v[2].c_str(),"S") != NULL;

     if(bProvider){
          if(bTranslationToMQTT){
               processProviderMQTT(pJsonSenML); //secure mode is not supported yet
          }
          else{
               processProvider(pJsonSenML, bProviderIsSecure);
          }
     }
     else if(bConsumer){
          processConsumer(pJsonSenML);
     }
	else{
          printf("Error: Unknown Flag(s): %s\n", v[2].c_str());
          printf("Expected: P | PS | PM | C");
	}
}

int SensorHandler::Callback_Serve_HTTPs_POST(const char *_url, const char *_payload){
     return Callback_Serve_HTTP_POST(_url, _payload);
}

int SensorHandler::Callback_Serve_HTTP_POST(const char *_url, const char *_payload){
     json jsonSenML;

	try {
		jsonSenML = json::parse(_payload);
	}
	catch (exception &e) {
		printf("Error: %s\n", e.what());
		return 0;
	}

	std::string baseName;
	try {
		baseName = jsonSenML.at("bn").get<std::string>();
	}
	catch (...) {
		printf("Error: received json does not contain bn field!\n");
		return 0;
	}

	vector<string> v = split<string>(baseName, ":");

	if(v.size() != 3){
	    printf("Error: Unknown base name format. Expected: moteID:SensorID:Flags");
	    return 0;
	}

     bool bProvider	          = strstr(v[2].c_str(),"P") != NULL;
	bool bTranslationToMQTT  = strstr(v[2].c_str(),"M") != NULL;
	bool bProviderIsSecure 	= strstr(v[2].c_str(),"S") != NULL;

     if(bProvider){
          if(bTranslationToMQTT)
               processProviderMQTT((string)_payload);
          else
               processProvider((string)_payload, bProviderIsSecure);
     }
     else{
          printf("Error: Unknown Flag(s): %s\n", v[2].c_str());
          printf("Expected: P | PS | PM | C");
	}

     return 1;
}

void SensorHandler::processProvider(std::string pJsonSenML, bool _bProviderIsSecure) {
	json jsonSenML;
     std::string baseName;

	try {
		jsonSenML = json::parse(pJsonSenML.c_str());
		baseName  = jsonSenML.at("bn").get<std::string>();
	}
	catch (exception &e) {
		printf("Error: %s\n", e.what());
		return;
	}

	if (oSensorTable.sensorExists(baseName)) {
		oSensorTable.setLastValue(baseName, pJsonSenML);
		printf("New measurement received from: %s\n", baseName.c_str());
		printf("LastValue updated.\n");
		return;
	}

	if (!oWhiteList.sensorIsEnabled(baseName)) {
		printf("Warning: Received data from a not allowed Sensor: %s!\n", baseName.c_str());
		printf("Skipping registration procedure.\n\n");
		return;
	}

	printf("Provider is allowed but not registered yet!\n");

	if (registerSensor(pJsonSenML, _bProviderIsSecure)) {
		printf("Provider Registration is successful!\n");
	}
	else {
		printf("Provider Registration is unsuccessful!\n");
	}
}

void SensorHandler::processProviderMQTT(std::string pJsonSenML){
	json jsonSenML;

	try{
		jsonSenML = json::parse(pJsonSenML.c_str());
	}
	catch(exception &e){
		printf("Error: could not parse json:  %s\n", e.what());
		return;
	}

	std::string baseName;
	std::string unit;

	try{
		baseName = jsonSenML.at("bn").get<std::string>();
		unit     = jsonSenML.at("bu").get<std::string>();
	}
	catch(exception &e){
		printf("Error: could not parse bn or bu:  %s\n", e.what());
		return;
	}

	std::replace(baseName.begin(), baseName.end(), ':', '/');

	std::string command = (std::string)("mosquitto_pub -t ") + baseName;

	command += (std::string)(" -q 1 -m \"") + pJsonSenML + (std::string)("\"");

	system(command.c_str());
}


void SensorHandler::processConsumer(std::string pJsonSenML) {

	json jsonSenML;

	try {
		jsonSenML = json::parse(pJsonSenML.c_str());
	}
	catch (exception &e) {
		printf("Error: %s\n", e.what());
		return;
	}

	std::string consumerID;

	try {
		consumerID = jsonSenML.at("bn").get<std::string>();
	}
	catch (exception &e) {
		printf("Error: %s\n", e.what());
		return;
	}

	printf("consumerID: %s\n", consumerID.c_str());

	lastValues lastValue;

	if( oConsumerTable.consumerExists(consumerID) ) {
		printf("Consumer already registered into the ConsumerTable!\n");

		oConsumerTable.getConsumerLastValue(consumerID, lastValue);

		uint32_t inftType = oConsumerTable.getConsumerInterfaceType(consumerID);

		switch (inftType) {
			case CONSUMER_INTERFACE_TCPRobotArm:
				printf("Send back: %s%s%s%s\n", lastValue.TCPRobotArmValue.lastValueX.c_str(), lastValue.TCPRobotArmValue.lastValueY.c_str(), lastValue.TCPRobotArmValue.lastValueZ.c_str(), lastValue.TCPRobotArmValue.lastValueS.c_str());

				//Send back the last measured value through UDP
				std::string lv = lastValue.TCPRobotArmValue.lastValueX + lastValue.TCPRobotArmValue.lastValueY + lastValue.TCPRobotArmValue.lastValueZ + lastValue.TCPRobotArmValue.lastValueS;

				if (sendto(s, lv.c_str(), lv.size(), 0, (struct sockaddr*) &si_other, slen) == -1) {
					printf("Error sending back data!\n");
				}

				break;
		}
	}
	else {
		printf("Unknown consumer (%s), orchestrate for consumer information and then register into the ConsumerTable!\n", consumerID.c_str());

		std::string requestForm;
		oConsumedService.getRequestForm(consumerID, requestForm);

		if (requestForm.size() == 0) {
			printf("Error: Request Form is missing for %s!\n", consumerID.c_str());
			return;
		}

		printf("Send orchestration request:\n%s\n", requestForm.c_str());

		sConsumerID = consumerID;
		int returnValue = sendOrchestrationRequest(requestForm, bSecureArrowheadInterface); //send consumerID to get back in httpResponseHandler function

		printf("Return value: %d\n", returnValue);
	}
}


/*
--
--
-- Provider
-- Registration, deregistration
--
*/
//+-------------------------- - +------ + -------- +
//| SenML | JSON | Type |
//+-------------------------- - +------ + -------- +
//| Base Name | bn | String |     OPTIONAL
//| Base Time | bt | Number |     OPTIONAL
//| Base Units | bu | Number |    OPTIONAL
//| Version | ver | Number |      OPTIONAL
//| Measurement or Parameters | e | Array |
//+-------------------------- - +------ + -------- +
//
//Measurement or Parameter Entries :
//
//+-------------- - +------ + ---------------- +
//| SenML | JSON | Notes |
//+-------------- - +------ + ---------------- +
//| Name | n | String |   Sensor name
//| Units | u | String |
//| Value | v | Floating point |
//| String Value | sv | String |
//| Boolean Value | bv | Boolean |
//| Value Sum | s | Floating point |
//| Time | t | Number |
//| Update Time | ut | Number |
//+-------------- - +------ + ---------------- +
//
bool SensorHandler::registerSensor(std::string _jsonSenML, bool _bProviderIsSecure){
     printf("\nREGISTRATION (%s, %s)\n\n", _bProviderIsSecure ? "Secure Provider" : "Insecure Provider", bSecureArrowheadInterface ? "Secure AHInterface" : "Insecure AHInterface");

	json jsonSenML = json::parse(_jsonSenML.c_str());
	std::string baseName;
	std::string baseUnit;
	std::string value;

	try {
		baseName = jsonSenML.at("bn").get<std::string>();
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return false;
	}

	try {
		baseUnit = jsonSenML.at("bu").get<std::string>();
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return false;
	}

	try {
		string sv = jsonSenML["e"][0].at("sv").get<std::string>();
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return false;
	}

	Arrowhead_Data_ext ah_dta_ext;

	vector<string> v = split<string>(baseName, ":");

	if (v.size() != 3) {
		printf("Error: unknown baseName format! Expected: moteID:sensorID:Flags\n");
		return false;
	}

	string baseNameMoteID = v[0];
	string baseNameSensorID = v[1];

	if (!oMoteTable.getMoteSystemName(baseNameMoteID, ah_dta_ext.sSystemName)) {
		printf("Error: Cannot find SystemName in MoteTable for moteID: %s\n", baseNameMoteID.c_str());
		return false;
	}

	if(!oProvidedService.getServiceDefinition(baseNameSensorID, baseNameMoteID, ah_dta_ext.sServiceDefinition)) {
		printf("Error: Cannot find ServiceDefinition in ProvidedServicesTable for sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

	if (!oProvidedService.getServiceInterface(baseNameSensorID, baseNameMoteID, ah_dta_ext.sserviceInterface)) {
		printf("Error: Cannot find ServiceInterface in ProvidedServicesTable for sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

	string meta_unit;
	if (!oProvidedService.getMetaUnit(baseNameSensorID, baseNameMoteID, meta_unit)) {
		printf("Error: Cannot find ""Unit"" meta data in ProvidedServicesTable for sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

	string privateKeyPath;
	if (_bProviderIsSecure && !oProvidedService.getPrivateKeyPath(baseNameSensorID, baseNameMoteID, privateKeyPath)) {
		printf("Error: Cannot find privateKeyPath for secure sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

     FILE *fp = fopen(privateKeyPath.c_str(),"r");
     if(!fp){
          printf("Error: Cannot open privateKeyPath for secure sensorID: %s\n", baseNameSensorID.c_str());
          return false;
     }
     else
          fclose(fp);

	string publicKeyPath;
	if (_bProviderIsSecure && !oProvidedService.getPublicKeyPath(baseNameSensorID, baseNameMoteID, publicKeyPath)) {
		printf("Error: Cannot find publicKeyPath for secure sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

     fp = fopen(publicKeyPath.c_str(),"r");
     if(!fp){
          printf("Error: Cannot open publicKeyPath for secure sensorID: %s\n", baseNameSensorID.c_str());
          return false;
     }
     else
          fclose(fp);

	if(_bProviderIsSecure){
		std::ifstream ifs(publicKeyPath.c_str());
		std::string pubkeyContent(  (std::istreambuf_iterator<char>(ifs) ),
					    (std::istreambuf_iterator<char>()    )
					 );

		pubkeyContent.erase(0, pubkeyContent.find("\n") + 1);
		pubkeyContent = pubkeyContent.substr(0, pubkeyContent.size()-25);

          pubkeyContent.erase(std::remove(pubkeyContent.begin(), pubkeyContent.end(), '\n'), pubkeyContent.end());

          ah_dta_ext.sAuthenticationInfo = pubkeyContent;
	}

	ah_dta_ext.vService_Meta.insert(std::pair<string,string>("unit", meta_unit));

	if(_bProviderIsSecure)
          ah_dta_ext.vService_Meta.insert(std::pair<string,string>("security", "token"));

	ah_dta_ext.sServiceURI = baseNameMoteID + "/" + baseNameSensorID + "/" + ah_dta_ext.sserviceInterface;

	int returnValue = registerToServiceRegistry(ah_dta_ext, bSecureArrowheadInterface);

	printf("%s Post sent (SenML baseName = %s)\n", bSecureArrowheadInterface? "HTTPs" : "HTTP", baseName.c_str());
	printf("%s Post return value: %d\n", bSecureArrowheadInterface? "HTTPs" : "HTTP", returnValue);

	if (returnValue == 201 /*Created*/){
		SensorTable::SensortableContent content;
		content.lastValue = _jsonSenML;
		content.sensorState = State_Registered;

		if(_bProviderIsSecure)
		    content.privateKeyPath = privateKeyPath;

		oSensorTable.insert(baseName, content);

		return true;
	}
	else{
		printf("Already registered?\n");
		printf("Try re-registration\n");

		returnValue = unregisterFromServiceRegistry(ah_dta_ext, bSecureArrowheadInterface);

		if (returnValue == 200 /*OK*/ || returnValue == 204 /*No Content*/) {
			printf("Unregistration is successful\n");
		}
		else {
			printf("Unregistration is unsuccessful\n");
			return false;
		}

		returnValue = registerToServiceRegistry(ah_dta_ext, bSecureArrowheadInterface);

		if (returnValue == 201 /*Created*/) {
			SensorTable::SensortableContent content;
			content.lastValue = _jsonSenML;
			content.sensorState = State_Registered;

               if(_bProviderIsSecure)
                    content.privateKeyPath = privateKeyPath;

			oSensorTable.insert(baseName, content);

			return true;
		}
		else {
			return false; //unsuccessful registration
		}
	}
}

//Send HTTP PUT to ServiceRegistry
bool SensorHandler::deregisterSensor(std::string _baseName){

	Arrowhead_Data_ext ah_dta_ext;

	//Parse basename to MoteID and SensorID
	vector<string> v = split<string>(_baseName, ":");

	if (v.size() != 3) {
		printf("Error: unknown baseName format! Expected: moteID:sensorID:P\n");
		return false;
	}

	string baseNameMoteID = v[0];
	string baseNameSensorID = v[1];

	if (!oMoteTable.getMoteSystemName(baseNameMoteID, ah_dta_ext.sSystemName)) {
		printf("Error: Cannot find SystemName in MoteTable for moteID: %s\n", baseNameMoteID.c_str());
		return false;
	}

	if (!oProvidedService.getServiceDefinition(baseNameSensorID, baseNameMoteID, ah_dta_ext.sServiceDefinition)) {
		printf("Error: Cannot find ServiceDefinition in ProvidedServicesTable for sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

	if (!oProvidedService.getServiceInterface(baseNameSensorID, baseNameMoteID, ah_dta_ext.sserviceInterface)) {
		printf("Error: Cannot find ServiceInterface in ProvidedServicesTable for sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

	string meta_unit;
	if (!oProvidedService.getMetaUnit(baseNameSensorID, baseNameMoteID, meta_unit)) {
		printf("Error: Cannot find ""Unit"" meta data in ProvidedServicesTable for sensorID: %s\n", baseNameSensorID.c_str());
		return false;
	}

	ah_dta_ext.vService_Meta.insert(std::pair<string, string>("unit", meta_unit));

	ah_dta_ext.sServiceURI = "/" + baseNameMoteID + "/" + baseNameSensorID + "/" + "json-SenML";

	int returnValue = unregisterFromServiceRegistry(ah_dta_ext, bSecureArrowheadInterface);

	if( returnValue == 200 /*OK*/ || returnValue == 204 /*No Content*/) {
		return true;
	}

	return false; //unsuccessful unregistration
}

/*
--
--
-- Arrowhead Interface -- Service Registry Callback
--
-- Called, when Consumer request arrives -- HTTP GET Request
--
*/

int SensorHandler::Callback_Serve_HTTP_GET(const char *Id, string *pString){

     printf("\nHTTP GET request received\n");

	vector<string> params = split<string>(string(Id), "/");

	printf("Received URI: %s\n", Id);

	if (params.size() != 3) {
          *pString = "Error: Unknown sensor URI format.";
		printf("Error: Unknown sensor URI format\n");
		return 1;
	}

	if (!oSensorTable.sensorExists(params[0] + ":" + params[1] + ":P")) {
		*pString = "Error: Unknown sensor.";
		printf("Error: Unknown sensor: %s.%s.P\n", params[0].c_str(), params[1].c_str());
		return 1;
	}

     std::string serviceInterface;
     if(!oProvidedService.getServiceInterface(params[0], params[1], serviceInterface)){
          *pString = "Error: Could not get Provider's Service Interface";
          printf("Error: Could not get Provider's Service Interface\n");
     }

	if ( strcmp(params[2].c_str(), serviceInterface.c_str()) == 0) {
		std::string lastValue_in_JsonSenML_Format = oSensorTable.getLastValue(params[0] + ":" + params[1] + ":P");
		*pString = lastValue_in_JsonSenML_Format;
		printf("Response:\n%s\n\n", lastValue_in_JsonSenML_Format.c_str());
	}
	else{
          *pString = "Error: Provider's Service Interface is not equal to the expected value.";
          printf("Error: Provider's Service Interface (%s) is not equal to %s\n", serviceInterface.c_str(), params[2].c_str());
	}

	return 1;
}

int SensorHandler::Callback_Serve_HTTPs_GET(const char *URI, string *pString, string _sToken, string _sSignature, string _clientDistName){

	printf("\nHTTPs GET request received\n");

	vector<string> params = split<string>(string(URI), "/");

	if (params.size() != 3) {
		printf("Error: Unknown sensor URI format\n");
		return 1;
	}

	if (!oSensorTable.sensorExists(params[0] + ":" + params[1] + ":PS")) {
		printf("Error: Unknown sensor: %s.%s.PS\n", params[0].c_str(), params[1].c_str());
		return 1;
	}

     RSASecurity oRSASecurity;
	oRSASecurity.privateKeyPath = oSensorTable.getPrivateKeyPath(params[0] + ":" + params[1] + ":PS");

	if(oRSASecurity.privateKeyPath.size() == 0){
	    printf("Error: Unknown Provider Private Key File Path\n");
	    return 1;
	}

     if( strstr(_sToken.c_str(), " ") != NULL )
          replace(_sToken.begin(),     _sToken.end(),     ' ', '+');

     if( strstr(_sSignature.c_str(), " ") != NULL )
          replace(_sSignature.begin(), _sSignature.end(), ' ', '+');

	oRSASecurity.sB64EncodedRSAEncryptedToken     = _sToken;
	oRSASecurity.sB64EncodedSignature             = _sSignature;

	if(oRSASecurity.getVerificationResult()){
	    printf("Successful RSA Signature verification\n");
	}
	else{
	    printf("Error: Unsuccessful RSA Signature verification - Wrong signature?\n");
	    return 1;
	}

     printf("\nRaw token info:\n%s\n\n", oRSASecurity.getDecryptedToken().c_str());

     if( strcmp("error", oRSASecurity.getDecryptedToken().c_str()) == 0)
          return false;

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
     if(!oProvidedService.getServiceInterface(params[0], params[1], serviceInterface)){
          printf("Error: Could not get Provider's Service Interface\n");
     }

     std::string serviceDefinition;
     if(!oProvidedService.getServiceDefinition(params[0], params[1], serviceDefinition)){
          printf("Error: Could not get Provider's Service Definition\n");
     }

     std::string concatenation = serviceInterface + (std::string)"." + serviceDefinition;

     if(strcmp(service.c_str(), concatenation.c_str()) != 0){
          printf("Error: s (%s) parameter from Raw token info is not equal to the expected %s value\n", service.c_str(), concatenation.c_str() );
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

	//check e
	if(expired != 0){
          time_t linuxEpochTime = std::time(0);

          if(expired < (uint64_t)linuxEpochTime){
               printf("Error: Expired time(%llu) is smaller than linux epoch time(%llu)!", expired, (uint64_t)linuxEpochTime);
               return 1;
          }
	}

     std::string lastValue_in_JsonSenML_Format = oSensorTable.getLastValue(params[0] + ":" + params[1] + ":PS");
     *pString = lastValue_in_JsonSenML_Format;
     printf("Response:\n%s\n\n", lastValue_in_JsonSenML_Format.c_str());

	return 1;
}


/*
--
--
-- Arrowhead Interface -- Orchestration Callback
--
-- Called, when Response received for the Orchestration HTTP POST message
--
*/
//todo: rename to Callback_OrchestrationResponse
size_t SensorHandler::Callback_OrchestrationResponse(char *ptr, size_t size) {
//
//Expected Response -- example
//
//{
//	"response": [{
//		"provider":
//		{
//			"address": "127.0.0.1",
//			"authenticationInfo" : "Base64 coded Public Key",
//			"port" : 8083,
//			"systemGroup" : "SmartGrid",
//			"systemName" : "SmartGridManagerSystem3"
//		},
//		"service":
//		{
//			"interfaces": ["JSON"],
//			"serviceDefinition": "ChargingReservation",
//			"serviceGroup" : "Charging",
//			"serviceMetadata" : {
//			"entry": [{
//				"key": "security",
//				"value" : "token"
//				},
//				{
//				"key": "carID",
//				"value" : "ID"
//				}
//			]}
//		},
//		"serviceURI": "reserve_charging"
//	}]
//}
	printf("\n\nOrchestration response: %s\n\n", ptr);

	json jHTTPResponsePayload;
	string sHTTPResponse;
	json jProvider;
	json jService;

	try {
		jHTTPResponsePayload	= json::parse(ptr);
		sHTTPResponse			= jHTTPResponsePayload["response"][0].at("provider").dump();
		jProvider				= json::parse( jHTTPResponsePayload["response"][0].at("provider").dump().c_str() );
		jService				= json::parse( jHTTPResponsePayload["response"][0].at("service").dump().c_str());
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return 1;
	}

	string sIPAddress;
	uint32_t uPort;
	string sInterface;
	string sURI;

	try {
		sIPAddress	= jProvider.at("address").dump();
		sIPAddress	= sIPAddress.substr(1, sIPAddress.size() - 2); //cut ""

		uPort		= jProvider.at("port").get<uint32_t>();

		sInterface	= jService["interfaces"][0].dump();
		sInterface	= sInterface.substr(1, sInterface.size() - 2); //cut ""

		//sURI		= jHTTPResponsePayload["response"][0].at("serviceURI").dump();
		//sURI		= sURI.substr(1, sURI.size() - 2); //cut ""
	}
	catch (exception& e) {
		printf("Error: %s\n", e.what());
		return 1;
	}

	printf("\n\nParsed from HTTP Response: \n");
	printf("IPAddress: %s\n", sIPAddress.c_str());
	printf("Port: %d\n", uPort);
	printf("Interface: %s\n", sInterface.c_str());
	//printf("URI: %s\n", sURI.c_str());

	char cbuff[10];
#ifdef __linux__
	snprintf(cbuff, sizeof(cbuff),"%d",uPort);
#elif _WIN32
	_itoa(uPort, cbuff, 10);
#endif
	lastValues lastValue;

	uint32_t intfType = getInterfaceCode(sInterface);

	if ( oConsumerTable.insertNewConsumer(sConsumerID, intfType, sIPAddress, string(cbuff)) ) {
		switch (intfType) {
			case CONSUMER_INTERFACE_TCPRobotArm:
				oConsumerTable.getConsumerLastValue(sConsumerID, lastValue);
				std::string r = lastValue.TCPRobotArmValue.lastValueX + lastValue.TCPRobotArmValue.lastValueY + lastValue.TCPRobotArmValue.lastValueZ + lastValue.TCPRobotArmValue.lastValueS;

				if (sendto(s, r.c_str(), r.size(), 0, (struct sockaddr*) &si_other, slen) == -1) {
					printf("Error sending back data!\n");
				}
				break;
               //case CONSUMER_INTERFACE_HTTP_REST:
               //   sendHTTPRequestToProvider...
               //case CONSUMER_INTERFACE_HTTPS_REST:
               //   sendHTTPSRequestToProvider...
               //case CONSUMER_INTERFACE_MQTT
               //   subscribeForTopic

		}
	}
	else {
		switch (intfType) {
			case CONSUMER_INTERFACE_TCPRobotArm:
				std::string r = "x=0\ny=0\nz=0\ns=0\n";
				if (sendto(s, r.c_str(), r.size(), 0, (struct sockaddr*) &si_other, slen) == -1) {
					printf("Error sending back data!\n");
				}
				break;
               //case CONSUMER_INTERFACE_HTTP_REST:
               //case CONSUMER_INTERFACE_HTTPS_REST:
               //case CONSUMER_INTERFACE_MQTT:
		}

	}

	printf("return size\n");

	return size;
}

void SensorHandler::unregisterAllProvider() {
	printf("unregister all provider\n");
	for (auto i = oSensorTable.sensorTable.begin(); i != oSensorTable.sensorTable.end(); ++i) {
		deregisterSensor(i->first);
	}
	oSensorTable.sensorTable.clear();
	printf("done\n");
}

