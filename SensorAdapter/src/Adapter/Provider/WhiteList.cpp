
#include "WhiteList.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <inttypes.h>
#include <string>
#include <list>
#include <thread>

#ifdef __linux__
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#endif

WhiteList::WhiteList(void) {
	enableRefresh = true;
}

WhiteList::~WhiteList(void) {

}

void WhiteList::startPeriodicRefresh() {
	thPeriodicRefresh = std::thread(&WhiteList::thWhiteListPeriodicRefresh, this);
}

void WhiteList::stopPeriodicRefresh() {
	printf("stop whitelist refresh\n");
	enableRefresh = false;

	if (thPeriodicRefresh.joinable())
		thPeriodicRefresh.join();
	else
		printf("Warning: Whitelist Refresh Thread is not joinable!\n");

	printf("WhiteList refresh stopped!\n");
}

//a sensorhandler constructor-ából indítjuk
void WhiteList::thWhiteListPeriodicRefresh() {
	while (enableRefresh) {
		readWhiteListCsv("config/whitelist.csv");

		#ifdef __linux__
			sleep(5);
		#elif _WIN32
			Sleep(5000);
		#endif
	}
}

void WhiteList::insert(std::string sensorURI, std::string moteIP) {
	//insert into whiteList table
	std::lock_guard<std::mutex> lock(m_whileListTable);

	whiteListTable.insert(std::pair<std::string, std::string>(sensorURI, moteIP));
}

void WhiteList::remove(std::string sensorURI) {
	std::lock_guard<std::mutex> lock(m_whileListTable);
	//remove from whiteList table
	whiteListTable.erase(sensorURI);

	//and also deregister sensor in ServiceRegistry and in ServiceTable
	//ezt egy külön szál végezheti a SensorHandler-bõl
}

//return true, if sensor exists in WhiteList
bool WhiteList::sensorIsEnabled(std::string sensorURI) {
	std::lock_guard<std::mutex> lock(m_whileListTable);

	if (whiteListTable.size() == 0) return false;

	return whiteListTable.find(sensorURI) == whiteListTable.end() ? false : true;
}

bool findSensorInList(std::list<std::string> _list, std::string _p) {
	std::list<std::string>::iterator listIt;

	for (listIt = _list.begin(); listIt != _list.end(); ++listIt) {
		if (*listIt == _p) {
			return true;
		}
	}
	return false;
}

//SensorURI, MoteIP
//
void WhiteList::readWhiteListCsv(std::string filePath) {
	std::ifstream inputFile;
	std::string delimiter = ",";
	std::string line;
	inputFile.open(filePath);

	if (!inputFile.good()) {
		printf("Error: Cannot open %s\n", filePath.c_str());
		inputFile.close();
		return;
	}
	//else {
	//	printf("WhiteList.csv opened successfully\n");
	//}

	//Check version
	while (std::getline(inputFile, line)) {
		if (line.at(0) == '#') {
			continue;
		}
		else {
			if (line != "version=0.0.2") {
				printf("WhiteList csv version is not correct!\n");
				return;
			}
			else {
				//printf("WhiteList csv version is OK.\n");
				break;
			}
		}
	}

	std::string sensorURI;
	std::string moteIP;

	std::list<std::string> csvList;
	//std::list<std::string>::iterator listIt;

	uint32_t unsuccessfulRegistration = 0;
	uint32_t maybeDoubled = 0;

	while (std::getline(inputFile, line)) {
		if(line.size() < 1) continue;
		if(line.at(0) == '#') continue;

		std::string::size_type pos = 0;
		std::string::size_type prev = 0;

		if ((pos = line.find(delimiter, prev)) != std::string::npos) {
			sensorURI = line.substr(prev, pos - prev).c_str();
			prev = pos + 1;
		}
		else {
			printf("Error: Sensor.csv contains unknown format or missing data!\n");
			inputFile.close();
			return;
		}

		if ((pos = line.find(delimiter, prev)) != std::string::npos) {
			for (uint32_t i = prev; i < pos; ++i) {
				if (isspace(line.at(i))) {
					++prev;
				}
			}
			if (prev == pos) {
				printf("Error: Sensor.csv contains unknown format or missing data!\n");
				inputFile.close();
				return;
			}

			moteIP = line.substr(prev, pos - prev).c_str();
			prev = pos + 1;
		}
		else {
			printf("Error: Sensor.csv contains unknown format or missing data!\n");
			return;
		}

		//auto it = std::find(csvList.begin(), csvList.end(), sensorURI);

		/*for (listIt = csvList.begin(); listIt != csvList.end(); ++listIt) {
			if (*listIt == sensorURI) {
				break;
			}
		}*/

		//if (listIt != csvList.end()) {
		if( findSensorInList(csvList, sensorURI) ){
			++maybeDoubled;
			printf("Warning: ""%s, %s"" Sensor is already exists in WhiteList csv file. Maybe wrong configuration!\n", sensorURI.c_str(), moteIP.c_str());
			printf("Skipping update.\n\n");
			continue;
		}

		csvList.push_back(sensorURI);
		if (sensorIsEnabled(sensorURI)) continue;
		insert(sensorURI, moteIP);
		printf("%s added to whitelist\n", sensorURI.c_str());
		//printf("white list table size: %d\n", whiteListTable.size());
	}//while

	/*printf("white list table size: %d\n", whiteListTable.size());
	printf("csv list size: %d\n", csvList.size());*/

	//Deregister and Remove, if sensorURI is missing from the csv file
	std::map<std::string, std::string>::iterator tableIt;
	//std::list<std::string>::iterator listIt;

	while (csvList.size() != whiteListTable.size()) {
		for (tableIt = whiteListTable.begin(); tableIt != whiteListTable.end(); ++tableIt) {
			//listIt = std::find(csvList.begin(), csvList.end(), tableIt->first);

			//if (listIt == csvList.end()) {
			if (!findSensorInList(csvList, tableIt->first)){
				printf("Remove %s from WhiteList Table\n", tableIt->first.c_str());

				//Remove from WhiteList table, remove from SensorTable and deregister in ServiceRegistry
				remove(tableIt->first);

				break;
			}
		}
	}

	//printf("Periodic WhiteList.csv read is done.\n");
	inputFile.close();
}
