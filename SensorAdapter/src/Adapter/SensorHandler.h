#pragma once

#include <inttypes.h>
#include <string>
#include <map>
#include <mutex>
#include "../Interface/ServiceRegistryInterface.hpp"
#include "../Interface/OrchestratorInterface.hpp"
#include "../Interface/MoteInterface.h"
#include "Provider/SensorTable.h"
#include "Provider/WhiteList.h"
#include "Provider/MoteTable.h"
#include "Provider/ProvidedService.h"
#include "Consumer/ConsumedService.h"
#include "Consumer/ConsumerTable.h"
#include "StringManipulation.hpp"
#include "../Security/RSASecurity.h"

#ifdef __linux__
#include "../Interface/ini/iniparser.h"
#elif _WIN32
extern "C" {
#include "../Interface/ini/iniparser.h"
}
#endif

class SensorHandler :
	ServiceRegistryInterface,
	OrchestratorInterface,
	MoteInterface
{

private:
     bool bSecureArrowheadInterface;
     const std::string sIniFilePath = "config/adapter.ini";

public:

	SensorHandler();
	~SensorHandler();

	//Overload - MoteInterface
	void processReceivedSensorDataThroughUDPMoteInterface(std::string pJsonSenML);

	void processProvider(std::string pJsonSenML, bool _bProviderIsSecure);
	void processProviderMQTT(std::string pJsonSenML);
	void processConsumer(std::string consumerID);

	//Overload - ServiceRegistryInterface callback
	int Callback_Serve_HTTP_GET(const char *Id, string *pStr);
	int Callback_Serve_HTTP_POST(const char *_url, const char *_payload);
	int Callback_Serve_HTTPs_GET(const char *Id, string *pStr, string sToken, string sSignature, string clientDistName);
     int Callback_Serve_HTTPs_POST(const char *_url, const char *_payload);

	//Overload - OrchestratorInterface callback
	size_t Callback_OrchestrationResponse(char *ptr, size_t size);

	/*SensorTable*/ //active Providers
	SensorTable oSensorTable;

	/*WhiteListTable*/
	WhiteList oWhiteList;

	/*MoteTable*/
	MoteTable oMoteTable;

	/*ProvidedServices*/
	ProvidedService oProvidedService;

	/*ConsumedServices*/
	ConsumedService oConsumedService;

	/*ConsumerTable*/ //active Consumers
	ConsumerTable oConsumerTable;

	/*Sensor registration, deregistration in ServiceRegistry*/
	bool registerSensor(std::string _jsonSenML, bool _bProviderIsSecure);
	bool deregisterSensor(std::string _sensorURI);
	//void th_timeoutHandler(); //remove and delete old sensors by lastValueTS

	void stopMoteInterface();
	void unregisterAllProvider();
};
