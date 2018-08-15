
#include "ConsumedService.h"
#include <fstream>

ConsumedService::ConsumedService() {
	readInputJsonFile();
	printTable();
}

ConsumedService::~ConsumedService() {

}

void ConsumedService::readInputJsonFile() {
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
			insertNewRequestForm(tmp);
			tmp = "";
		}
	}

	inputFile.close();
}

void ConsumedService::insertNewRequestForm(std::string s) {
	json j;

	try {
		j = json::parse(s.c_str());
	}
	catch (...) {
		printf("Error: Cannot parse json: %s\n", s.c_str());
		return;
	}

	json form = j.at("requestForm");

	try {
		table.insert(std::pair<std::string, json>(j.at("consumerID").get<std::string>(), form));
	}
	catch (...) {
		printf("Error: Cannot insert Sensor into ConsumedServiceTable: %s\n", s.c_str());
	}
}

void ConsumedService::printTable() {
	printf("\n-----------------------------\nConsumedServiceTable\n-----------------------------\n");

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		printf("%s : %s\n\n\n", i->first.c_str(), i->second.dump().c_str());
	}
}

bool ConsumedService::getRequestForm(std::string condumerID, std::string &rRequestForm) {
	for (auto it = table.begin(); it != table.end(); ++it) {
		if (it->first == condumerID) {
			rRequestForm = it->second.dump();
			return true;
		}
	}

	return false;
}
