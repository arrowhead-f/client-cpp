
#pragma warning(disable:4996)

#include "SensorHandler.h"
#include <sstream>
#include <string>
#include <stdio.h>
#include <thread>
#include <list>

#ifdef __linux__
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#endif

const std::string version = "4.0";

bool bSecureProviderInterface = false;
bool bSecureArrowheadInterface = false;

inline void parseArguments(int argc, char* argv[]){
     for(int i=1; i<argc; ++i){
          if(strstr("--secureArrowheadInterface", argv[i]))
               bSecureArrowheadInterface = true;
          else if(strstr("--secureProviderInterface", argv[i]))
               bSecureProviderInterface = true;
     }
}

int main(int argc, char* argv[]){

     parseArguments(argc, argv);

	#ifdef __linux__
		sleep(1);
	#elif _WIN32
		Sleep(1000);
	#endif

	printf("\n=====================\nConsumer example v%s\n=====================\n", version.c_str());

	SensorHandler oSensorHandler;

     oSensorHandler.bSecureProviderInterface = bSecureProviderInterface;

	std::string consumerMessageExample = (std::string)"{\"bn\":\"TestconsumerID\"\n}";//json - SenML format

     oSensorHandler.processConsumer( consumerMessageExample, bSecureArrowheadInterface);


	#ifdef __linux__
		sleep(2);
	#elif _WIN32
		Sleep(2000);
	#endif

     printf("\nDone.\n");

	return 0;
}

