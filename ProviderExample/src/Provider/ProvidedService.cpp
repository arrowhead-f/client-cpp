
#include "ProvidedService.h"

ProvidedService::ProvidedService(){
     printService();
     fillMetadata();
}

ProvidedService::~ProvidedService(){

}

void ProvidedService::printService() {
	printf("\n-----------------------------\nProvidedService\n-----------------------------\n");
     printf("%s : %s\n", "Custom URL", customURL.c_str());
     printf("%s : %s\n", "System name", systemName.c_str());
     printf("%s : %s\n", "Service definition", serviceDefinition.c_str());
     printf("%s : %s\n", "Service interface", serviceInterface.c_str());
     printf("%s : %s\n", "Private key path", privateKeyPath.c_str());
     printf("%s : %s\n", "Public key path", publicKeyPath.c_str());
     printf("Meta values:\n");
     for(std::map<std::string,std::string>::iterator it = metadata.begin(); it!=metadata.end(); ++it )
          printf("%s : %s\n", it->first.c_str(), it->second.c_str());
}

void ProvidedService::fillMetadata(){
     metadata.insert(std::pair<std::string, std::string>("unit", "Celsius"));
     metadata.insert(std::pair<std::string, std::string>("security", "token")); //not used, when the provider is unsecure

//todo:
//add new/custom meta data
     //metadata.insert(std::pair<std::string, std::string>("customKey", "customValue"));
}

bool ProvidedService::getCustomURL(std::string &r) {
     if(customURL.length() == 0)
          return false;

     r = customURL;
	return true;
}

bool ProvidedService::getSystemName(std::string &r) {
     if(systemName.length() == 0)
          return false;

     r = systemName;
	return true;
}

bool ProvidedService::getServiceDefinition(std::string &r) {
     if(serviceDefinition.length() == 0)
          return false;

     r = serviceDefinition;
	return true;
}

bool ProvidedService::getServiceInterface(std::string &r) {
     if(serviceInterface.length() == 0)
          return false;

     r = serviceInterface;
	return true;
}

bool ProvidedService::getPrivateKeyPath(std::string &r) {
     if(privateKeyPath.length() == 0)
          return false;

     r = privateKeyPath;
          return true;
}

bool ProvidedService::getPublicKeyPath(std::string &r) {
     if(publicKeyPath.length() == 0)
          return false;

     r = publicKeyPath;
          return true;
}

