#pragma once

#include <inttypes.h>
#include <string>
#include <map>
#include <mutex>
#include "../Interface/OrchestratorInterface.hpp"
#include "ConsumedService.h"

#ifdef __linux__

#include "curl.h"
#include "microhttpd.h"

#elif _WIN32

extern "C" {
#include "include\curl\curl.h"
#include "include\mhttpd\microhttpd.h"
}
#endif

class SensorHandler : OrchestratorInterface
{

public:
	SensorHandler();
	~SensorHandler();

     bool bSecureProviderInterface;

	void processConsumer(std::string consumerID, bool _bSecureArrowheadInterface);

	//Overload - OrchestratorInterface callback
	size_t Callback_OrchestrationResponse(char *ptr, size_t size);

     void sendRequestToProvider(std::string _sProviderURI);
     void sendHttpsRequestToProvider(std::string _sProviderURI);
     size_t providerHttpResponseCallback(char *ptr, size_t size);

	/*ConsumedServices*/
	ConsumedService oConsumedService;

};
