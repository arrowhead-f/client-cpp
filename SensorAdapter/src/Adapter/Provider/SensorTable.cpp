
#include "SensorTable.h"

void SensorTable::insert(std::string sensorURI, SensortableContent content){
	std::lock_guard<std::mutex> lock(m_sensorTable);

	sensorTable.insert(std::pair<std::string, SensortableContent>(sensorURI, content));
}

void SensorTable::remove(std::string sensorURI){
	std::lock_guard<std::mutex> lock(m_sensorTable);

	sensorTable.erase(sensorURI);
}

void SensorTable::setLastValue(std::string sensorURI, std::string newValue) {
	std::lock_guard<std::mutex> lock(m_sensorTable);
	std::map<std::string, SensortableContent>::iterator it = sensorTable.find(sensorURI);

	it->second.lastValue = newValue;
}

std::string SensorTable::getLastValue(std::string sensorURI){
	std::lock_guard<std::mutex> lock(m_sensorTable);
	std::map<std::string, SensortableContent>::iterator it = sensorTable.find(sensorURI);
	
	return it->second.lastValue;
}

void SensorTable::insertNewConsumer(std::string sensorURI, std::string consumer, uint64_t timeStamp) {
	std::lock_guard<std::mutex> lock(m_sensorTable);
	std::map<std::string, SensortableContent>::iterator it = sensorTable.find(sensorURI);

	it->second.consumerList.insert(std::pair<std::string, uint64_t>(consumer, timeStamp));
}

void SensorTable::removeConsumer(std::string sensorURI, std::string consumer) {
	std::lock_guard<std::mutex> lock(m_sensorTable);
	std::map<std::string, SensortableContent>::iterator it = sensorTable.find(sensorURI);

	it->second.consumerList.erase(consumer);
}

bool SensorTable::sensorExists(std::string sensorURI) {
	std::lock_guard<std::mutex> lock(m_sensorTable);

	if (sensorTable.size() == 0) return false;

	return sensorTable.find(sensorURI) == sensorTable.end() ? false : true;
}

void SensorTable::setSensorState(std::string sensorURI, uint8_t newState) {
	std::lock_guard<std::mutex> lock(m_sensorTable);
	std::map<std::string, SensortableContent>::iterator it = sensorTable.find(sensorURI);

	it->second.sensorState = newState;
}

uint8_t SensorTable::getSensorState(std::string sensorURI) {
	std::lock_guard<std::mutex> lock(m_sensorTable);
	std::map<std::string, SensortableContent>::iterator it = sensorTable.find(sensorURI);

	return it->second.sensorState;
}

std::string SensorTable::getPrivateKeyPath(std::string sensorURI){
	std::lock_guard<std::mutex> lock(m_sensorTable);
	std::map<std::string, SensortableContent>::iterator it = sensorTable.find(sensorURI);
	
	return it->second.privateKeyPath;
}
