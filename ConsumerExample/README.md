Arrowhead Consumer Example code

This example code orchestrates for a predefined provider from the Arrowhead Orchestrator module, 
using HTTP or HTTPS for the orchestration procedure.
The json content of the request form is predefined in the consumedServices.json file. 
This file contains the request forms for the predefined consumers, the form selection is based on the consumerID field.
To communicate with the ProviderEample project, you should set the same IP address and port in the consumedServices.json/preferredProviders/providerSystem section as in the provider's "ApplicationServiceInterface.ini" configuration file.


The communication is based on the "OrchestratorInterface.ini" configuration files:
  or_base_uri: address of the Orchestrator module
  port: this is the TCP port of the HTTP server, which communicates with the Orchestrator module
  In a case of HTTPs operation, the port will be the value + 1.
  address: this is the local IP address (e.g. the IP address of the eth0 interface on a RaspBerry)

Installation steps on RaspBerry:
1) Install the following packages:
> sudo apt-get install openssl libgnutls28-dev libgnutlsxx28 libssl1.1 libssl1.0-dev libcurl3 libcurl3-gnutls libcurl4-gnutls-dev libcrypto++-dev libcrypto++-utils libcrypto++6 libgpg-error-dev automake texinfo g++ libjson-c-dev

2) Download libmicrohttpd-0.9.59
> tar -xvzf libmicrohttpd-0.9.59.tar.gz
> ./configure --with-gnutls
> make; sudo make install

> cd /usr/lib
> sudo ln -s /usr/local/lib/libmicrohttpd.so.12.46.0 libmicrohttpd.so.12

3) In a case of missing libcrypto.so:
> cdr /usr/lib
> sudo ln -s libcrypto.so.1.0.0 /lib/arm-linux-gnueabihf/libcrypt-2.24.so

4) Compile ConsumerExample using make
> ./ConsumerExample

To grant a secure (HTTPS) communication  to the Orchestrator module, use the "./ConsumerExample --secureArrowheadInterface" command.
To grant a secure (HTTPS) communication between the comsumer - provider path, use the "./ConsumerExample --secureProviderInterface" command.
You can use both options at the same time.
The keys folder contains the required key files for the secure communications. 

About the project sources:
The main() function is located in the ConsumerExample.cpp file. 
It calls the processConsumer() function with a predefined consumerID, which will be used to select the orchestration payload from the consumedServices.json file.
The SensorHandler.cpp contains the functions for the SensorHandler class, which sends orchestration requests and HTTP/HTTPS requests for the provider, using an HTTP/HTTPS server.
The Interface folder contains the sources for the HTTP/HTTPS communication.
