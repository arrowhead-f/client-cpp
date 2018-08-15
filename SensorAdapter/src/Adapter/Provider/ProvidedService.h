#pragma once

#include <string>
#include <map>
#include <mutex>
#include "../json.hpp"

using namespace nlohmann;

class ProvidedService {
private:
	const std::string filePath = "config/providedService.json";
	std::map<std::string, json> table;

	std::mutex m_MoteTable;

public:
	ProvidedService();
	~ProvidedService();

	void readInputJsonFile();
	void insertNewService(std::string s);
	void printTable();

	bool getServiceGroup(std::string sensorID, std::string moteID, std::string &r);
	bool getServiceDefinition(std::string sensorID, std::string moteID, std::string &r);
	bool getServiceInterface(std::string sensorID, std::string moteID, std::string &r);
	bool getMoteID(std::string sensorID, std::string &r);
	bool getMetaUnit(std::string sensorID, std::string moteID, std::string &r);
	bool getPrivateKeyPath(std::string sensorID, std::string moteID, std::string &r);
	bool getPublicKeyPath(std::string sensorID, std::string moteID, std::string &r);
};
