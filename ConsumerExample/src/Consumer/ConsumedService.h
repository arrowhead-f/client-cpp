#pragma once

#include <map>
#include <string>
#include <json-c/json.h>

class ConsumedService {

private:
	std::string filePath = std::string("consumedServices.json");
	std::map<std::string, std::string> table; //consumerID + requestForm pair

public:
	ConsumedService();
	~ConsumedService();

	void readInputJsonFile();
	void insertNewRequestForm(std::string s);
	void printTable();
	bool getRequestForm(std::string consumerID, std::string &rRequestForm);
};
