
#ifndef AH_JSON_PARSER_H
#define AH_JSON_PARSER_H

#include <json-c/json.h>
#include <stdio.h>
#include <stdint.h>

const char *serviceRegistryRegistrationMsg(char *servDef, char *unit, char *ip, uint16_t port, char *systemName, char *uri);

#endif
