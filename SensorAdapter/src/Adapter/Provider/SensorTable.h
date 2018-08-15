#pragma once

#include <inttypes.h>
#include <string>
#include <map>
#include <mutex>

#define State_Unregistered 0
#define State_Registered   1
#define State_Idle         2
#define State_Active       3 //already provides data to costumer(s)

class SensorTable {

public:
	typedef struct Consumer {
		uint32_t IPv4_Addr;
		uint16_t Port;
	} Consumer;

	typedef struct SensorTableContent {
		uint8_t sensorState;
		//std::map<Consumer, uint64_t> consumerList;
		std::map<std::string, uint64_t> consumerList; // consumer string format: consumer IPv4_Addr:consumer Port, lastQueryTS
		uint64_t lastValueTS;
		std::string lastValue;
		std::string privateKeyPath;
	} SensortableContent;

//private:
	mutable std::mutex m_sensorTable;
	std::map<std::string, SensortableContent> sensorTable;

//public:

	void insert(std::string sensorURI, SensortableContent content);
	void remove(std::string sensorURI);

	void insertNewConsumer(std::string sensorURI, std::string consumer, uint64_t timeStamp);
	void removeConsumer(std::string sensorURI, std::string consumer);

	void setLastValue(std::string sensorURI, std::string newValue);
	std::string getLastValue(std::string sensorURI);

	bool sensorExists(std::string sensorURI); //return true, if sensor already known and registered
	void setSensorState(std::string sensorURI, uint8_t newState);
	uint8_t getSensorState(std::string sensorURI);

	std::string getPrivateKeyPath(std::string sensorURI);
};
