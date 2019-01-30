
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

	struct json_object *jsonString = json_tokener_parse(s.c_str());

	if(jsonString == NULL){
		printf("Error: Cannot parse json: %s\n", s.c_str());
		return;
	}

	struct json_object *json_consID;
	if(!json_object_object_get_ex(jsonString, "consumerID", &json_consID)){
	    printf("Error: Could not find \"consumerID\" in \n%s\n", s.c_str());
	    return;
	}

	std::string sConsumerID = std::string(json_object_get_string(json_consID));

	struct json_object *json_reqForm;
	if(!json_object_object_get_ex(jsonString, "requestForm", &json_reqForm)){
	    printf("Error: Could not find \"requestFrom\" in \n%s\n", s.c_str());
	    return;
	}

	std::string sRequestForm = std::string(json_object_get_string(json_reqForm));

	try {
		table.insert(std::pair<std::string, std::string>(sConsumerID, sRequestForm));
	}
	catch (...) {
		printf("Error: Cannot insert Sensor into ConsumedServiceTable: %s\n", s.c_str());
	}
}

void ConsumedService::printTable() {
	printf("\n-----------------------------\nConsumedServiceTable\n-----------------------------\n");

	std::map<std::string, std::string>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		printf("%s : %s\n\n\n", i->first.c_str(), i->second.c_str());
	}
	printf("-----------------------------\n");
}

bool ConsumedService::getRequestForm(std::string condumerID, std::string &rRequestForm) {
	for (auto it = table.begin(); it != table.end(); ++it) {
		if (it->first == condumerID) {
			rRequestForm = it->second;
			return true;
		}
	}

	return false;
}
