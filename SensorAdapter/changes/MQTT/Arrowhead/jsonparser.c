
//
// compilation:
// gcc jsonparser.c -ljson-c
//

#include "jsonparser.h"

/*
*Expected serviceRegistryRegistrationMsg payload:
        {
            "providedService": {
                "serviceDefinition": "IndoorTemperature",
                "interfaces": [
                    "MQTT"
                ],
                "serviceMetadata": {
                    "unit": "celsius",
                }
            },
            "provider": {
                "id": 5,
                "systemName": "SecureTemperatureSensor",
                "address": "10.0.0.40",
                "port": 8455
            },
            "serviceURI": "temperature",
            "version": 1,
            "udp": false,
            "ttl": 0
        }
*/

/*AH core system version compatibility: M3.0 and M4.0*/

const char *serviceRegistryRegistrationMsg(char *_serviceDefinition, char *_unit, char *_ip, uint16_t _port, char *_systemName, char *_uri){
     json_object *jobj = json_object_new_object();
     json_object *providedService = json_object_new_object();
     json_object *provider = json_object_new_object();
     json_object *jstring;
     json_object *jint;
/*
*   ProvidedService section
*/
     jstring = json_object_new_string(_serviceDefinition);
     json_object_object_add(providedService, "serviceDefinition", jstring);

     json_object *jarray = json_object_new_array();
     jstring = json_object_new_string("MQTT");
     json_object_array_add(jarray, jstring);
     json_object_object_add(providedService, "interfaces", jarray);

     json_object *serviceMetadata = json_object_new_object();
     jstring = json_object_new_string(_unit);
     json_object_object_add(serviceMetadata, "unit", jstring);
     json_object_object_add(providedService, "serviceMetadata", serviceMetadata);
/*
*  Provider section
*/
    jstring = json_object_new_string(_systemName);
    json_object_object_add(provider, "systemName", jstring);

    jstring = json_object_new_string(_ip);
    json_object_object_add(provider, "address", jstring);

    jint = json_object_new_int(_port);
    json_object_object_add(provider, "port", jint);
/*
*  Concatenation
*/
    json_object_object_add(jobj, "providedService", providedService);
    json_object_object_add(jobj, "provider", provider);
    jstring = json_object_new_string(_uri);
    json_object_object_add(jobj, "serviceURI", jstring);
    jint = json_object_new_int(1);
    json_object_object_add(jobj, "version", jint);
/*
* Return
*/
    return json_object_to_json_string(jobj);
}
