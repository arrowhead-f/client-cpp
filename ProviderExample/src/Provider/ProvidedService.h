#pragma once

#include <string>
#include <map>

class ProvidedService {
private:
// todo:
// modify these parameters
     std::string customURL         = "/this_is_the_custom_url";
     std::string systemName        = "SecureTemperatureSensor";
     std::string serviceDefinition = "IndoorTemperature_ProviderExample";
     std::string serviceInterface  = "HTTP-SECURE-JSON"; // name pattern: protocol-SECURE or INSECURE format. (e.g.: HTTPS-SECURE-JSON)
     std::string privateKeyPath    = "keys2/tempsensor.testcloud2.privkey.pem";
     std::string publicKeyPath     = "keys2/tempsensor.testcloud2.pubkey.pem";

public:
//do not modify below this
     ProvidedService();
     ~ProvidedService();

     std::map<std::string, std::string> metadata;
     void fillMetadata();

     void printService();
     bool getCustomURL(std::string &r);
     bool getSystemName(std::string &r);
     bool getServiceDefinition(std::string &r);
     bool getServiceInterface(std::string &r);
     bool getPrivateKeyPath(std::string &r);
     bool getPublicKeyPath(std::string &r);
};
