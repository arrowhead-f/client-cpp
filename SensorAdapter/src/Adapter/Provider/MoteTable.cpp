#include "MoteTable.h"
#include <fstream>
#include <string>

MoteTable::MoteTable() {
	readInputJsonFile();

	printTable();
}

MoteTable::~MoteTable() {

}

void MoteTable::readInputJsonFile() {
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
			insertNewMote(tmp);
			tmp = "";
		}
	}

	inputFile.close();
}

void MoteTable::insertNewMote(std::string s) {
	json j;

	try {
		j = json::parse(s.c_str());
	}
	catch (...) {
		printf("Error: Cannot parse json: %s\n", s.c_str());
	}

	try {
		std::lock_guard<std::mutex> lock(m_MoteTable);
		table.insert(std::pair<std::string, json>(j.at("moteID").get<std::string>(), j));
	}
	catch (...) {
		printf("Error: Cannot insert Mote into MoteTable: %s\n", s.c_str());
	}

}

void MoteTable::printTable() {
	printf("\n-----------------------------\nMoteTable\n-----------------------------\n");

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		printf("%s : %s\n", i->first.c_str(), i->second.dump().c_str());
	}
}

bool MoteTable::getMoteSystemGroup(std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if (i->first == moteID) {
			r = i->second.at("systemGroup").get<std::string>();
			return true;
		}
	}

	return false;
}

bool MoteTable::getMoteSystemName(std::string moteID, std::string &r) {
	std::lock_guard<std::mutex> lock(m_MoteTable);

	std::map<std::string, json>::iterator i;

	for (i = table.begin(); i != table.end(); ++i) {
		if (i->first == moteID) {
			r = i->second.at("systemName").get<std::string>();
			return true;
		}
	}

	return false;
}
