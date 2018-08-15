#pragma once

#include <map>
#include "json.hpp"
#include <string>

using namespace nlohmann;

class ConsumedService {

private:
	std::string filePath = "consumedServices.json";
	std::map<std::string, json> table;

public:
	ConsumedService();
	~ConsumedService();

	void readInputJsonFile();
	void insertNewRequestForm(std::string s);
	void printTable();
	bool getRequestForm(std::string condumerID, std::string &rRequestForm);
};
