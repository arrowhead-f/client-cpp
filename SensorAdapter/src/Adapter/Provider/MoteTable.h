#pragma once

#include <string>
#include <map>
#include <mutex>
#include "../json.hpp"

using namespace nlohmann;

class MoteTable {

private:
	const std::string filePath = "config/moteTable.json";
	std::map<std::string, json> table;

	std::mutex m_MoteTable;

public:
	MoteTable();
	~MoteTable();

	void readInputJsonFile();
	void insertNewMote(std::string s);
	void printTable();

	bool getMoteSystemGroup(std::string moteID, std::string &r);
	bool getMoteSystemName(std::string moteID, std::string &r);
};
