#pragma once

#include <string>
#include <sstream>

#define CONSUMER_INTERFACE_UDP 0
#define CONSUMER_INTERFACE_TCP 1

#define CONSUMER_INTERFACE_TCPRobotArm 254
#define CONSUMER_INTERFACE_UNKNOWN 255

typedef struct {
	std::string lastValueX;
	std::string lastValueY;
	std::string lastValueZ;
	std::string lastValueS;
} TCPRobotArm;

class lastValues {
public:
	TCPRobotArm TCPRobotArmValue;
};

static inline uint32_t getInterfaceCode(std::string s) {
	if (s == "UDP")			return CONSUMER_INTERFACE_UDP;
	else if (s == "TCP")		return CONSUMER_INTERFACE_TCP;
	else if (s == "TCPRobotArm")	return CONSUMER_INTERFACE_TCPRobotArm;
	else						return CONSUMER_INTERFACE_UNKNOWN;
}
