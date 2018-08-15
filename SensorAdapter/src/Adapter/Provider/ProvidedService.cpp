
#include "ProvidedService.h"
#include <fstream>

ProvidedService::ProvidedService() {
	readInputJsonFile();
	printTable();
}

ProvidedService::~ProvidedService() {

}

void ProvidedService::readInputJsonFile() {
     std::ifstream inputFile;

	std::string line;
	inputFile.open(filePath);

	if (!inputFile.good()) {
		printf("Error: Cannot open %s\n", filePath.c_str());
		inputFile.close();
		return;
	}

	std::string tmp;

	int numberOfOpen = 0;

	while (std::getline(inputFile, line)) {
		if (line.size() < 1) continue;

		numberOfOpen += line.find("{") != std::string::npos;
		numberOfOpen -= line.find("}") != std::string::npos;

		tmp += line;

		if (numberOfOpen == 0) {
			insertNewService(tmp);
			tmp = "";
		}
	}

	inputFile.close();
}

void ProvidedService::insertNewService(std::string s) {
	json j;

	try {
		j = json::parse(s.c_str());
	}
	catch (...) {
		printf("Error: Cannot parse json: %s\n", s.c_str());
	}

	try {
		std::lock_guard<std::mutex> lock(m_MoteTable);
		table.insert(std::pair<std::string, json>(j.at("sensorID").get<std::string>(), j));
	}
	catch (...) {
		printf("Error: Cannot insert Sensor into ProvidedServiceTable: %s\n", s.c_str());
	}
}

void ProvidedService::printTable() {
	printf("\n-----------------------------\nProvidedServiceTable\n-----------------------------\n");

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		printf("%s : %s\n", i->first.c_str(), i->second.dump().c_str());
	}

}

bool ProvidedService::getServiceGroup(std::string sensorID, std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if ( (i->first == sensorID) && (i->second.at("moteID").get<std::string>() == moteID)) {
			r = i->second.at("serviceGroup").get<std::string>();
			return true;
		}
	}

	return false;
}

bool ProvidedService::getServiceDefinition(std::string sensorID, std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if ((i->first == sensorID) && (i->second.at("moteID").get<std::string>() == moteID)) {
			r = i->second.at("serviceDefinition").get<std::string>();
			return true;
		}
	}

	return false;
}

bool ProvidedService::getServiceInterface(std::string sensorID, std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if ((i->first == sensorID) && (i->second.at("moteID").get<std::string>() == moteID)) {
			r = i->second.at("serviceInterface").get<std::string>();
			return true;
		}
	}

	return false;
}

bool ProvidedService::getMoteID(std::string sensorID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if (i->first == sensorID) {
			r = i->second.at("moteID").get<std::string>();
			return true;
		}
	}
	return false;
}

bool ProvidedService::getMetaUnit(std::string sensorID, std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if ((i->first == sensorID) && (i->second.at("moteID").get<std::string>() == moteID)) {
			r = i->second["meta"].at("unit").get<std::string>();
			return true;
		}
	}

	return false;
}

bool ProvidedService::getPrivateKeyPath(std::string sensorID, std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if (i->first == sensorID) {
			r = i->second.at("privateKeyPath").get<std::string>();
			if(r.size() == 0)
			    return false;
			else
			    return true;
		}
	}
	return false;
}


bool ProvidedService::getPublicKeyPath(std::string sensorID, std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if (i->first == sensorID) {
			r = i->second.at("publicKeyPath").get<std::string>();
			if(r.size() == 0)
			    return false;
			else
			    return true;
		}
	}
	return false;
}

