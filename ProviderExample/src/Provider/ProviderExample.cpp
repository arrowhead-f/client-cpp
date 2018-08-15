
#pragma warning(disable:4996)

#include "SensorHandler.h"
#include <sstream>
#include <string>
#include <stdio.h>
#include <thread>
#include <list>
#include <time.h>
#include <iomanip>

#ifdef __linux__
     #include <unistd.h>
#elif _WIN32
     #include <windows.h>
#endif

const std::string version = "4.0";

bool bSecureProviderInterface = false; //Enables HTTPS interface on the application service (with token enabled)
bool bSecureArrowheadInterface = false; //Enables HTTPS interface towards ServiceRegistry AH module

inline void parseArguments(int argc, char* argv[]){
     for(int i=1; i<argc; ++i){
          if(strstr("--secureArrowheadInterface", argv[i]))
               bSecureArrowheadInterface = true;
          else if(strstr("--secureProviderInterface", argv[i]))
               bSecureProviderInterface = true;
     }
}

int main(int argc, char* argv[]){

	printf("\n=============================\nProvider Example - v%s\n=============================\n", version.c_str());

     parseArguments(argc, argv);

	SensorHandler oSensorHandler;

//SenML format
//todo:
//generate own measured value into "measuredValue"
//"value" should be periodically updated
//"sLinuxEpoch" should be periodically updated

     std::string measuredValue; //JSON - SENML format
     time_t linuxEpochTime = std::time(0);
     std::string sLinuxEpoch = std::to_string((uint64_t)linuxEpochTime);

     double value = 26.0;
//convert double to string
     std::ostringstream streamObj;
     streamObj << std::fixed;
     streamObj << std::setprecision(1);
     streamObj << value;
     std::string sValue = streamObj.str();

     measuredValue =
          "{"
               "\"e\":[{"
                    "\"n\": \"this_is_the_sensor_id\","
                    "\"v\":" + sValue +","
                    "\"t\": \"" + sLinuxEpoch + "\""
                    "}],"
               "\"bn\": \"this_is_the_sensor_id\","
               "\"bu\": \"Celsius\""
          "}";

//do not modify below this

     oSensorHandler.processProvider(measuredValue, bSecureProviderInterface, bSecureArrowheadInterface);

	while (true) {
	    #ifdef __linux__
               sleep(1);
          #elif _WIN32
               Sleep(1000);
          #endif
	}

	return 0;
}

