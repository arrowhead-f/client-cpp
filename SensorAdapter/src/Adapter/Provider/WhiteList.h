#pragma once

#include <thread>
#include <mutex>
#include <map>

class WhiteList {

private:
	std::thread thPeriodicRefresh;

	mutable std::mutex m_whileListTable;
	std::map<std::string, std::string> whiteListTable;
	bool enableRefresh;

public:
	WhiteList();
	~WhiteList();

	void startPeriodicRefresh();
	void thWhiteListPeriodicRefresh(); //a sensorhandler constructor-ából indítjuk
	void readWhiteListCsv(std::string filePath);
	void insert(std::string sensorURI, std::string moteIP);
	void remove(std::string sensorURI);
	bool sensorIsEnabled(std::string sensorURI); //return true, if sensor exists
	void stopPeriodicRefresh();
};