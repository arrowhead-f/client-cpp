
#pragma warning(disable:4996)

#include <sstream>
#include <string>
#include <stdio.h>
#include <thread>
#include <list>

#include "SensorHandler.h"

#ifdef __linux__
#include <unistd.h>
#elif _WIN32
#include <windows.h> //just for Sleep()
#endif

const std::string version = "0.2.9";

int main(int argc, char* argv[]){

	#ifdef __linux__
		sleep(1);
	#elif _WIN32
		Sleep(1000);
	#endif

	printf("\n==========================================\nArrowhead Sensor Adapter module v%s\n==========================================\n", version.c_str());

	SensorHandler oSensorHandler;

	#ifdef __linux__
		sleep(1);
	#elif _WIN32
		Sleep(1000);
	#endif

	printf("\nWaiting for Providers or Consumers...\n");

	while (true) {
		std::string ex;
		cin >> ex;

		if(ex == "unregister"){
		    oSensorHandler.unregisterAllProvider();
		    break;
		}
		else if ((ex == "exit") || (ex == "exit\n")) {
			oSensorHandler.stopMoteInterface();
			oSensorHandler.unregisterAllProvider();
			oSensorHandler.oWhiteList.stopPeriodicRefresh();
			oSensorHandler.oConsumerTable.deleteAllConsumer(); //close TCP sockets...
			break;
		}
	}

	return 0;
}

