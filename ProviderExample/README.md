Arrowhead Provider Example code

This example code registers a provider to the Arrowhead ServiceRegistry module, 
using HTTP or HTTPS for the registration procedure.
The "measured" value by the provider is a predefined value (in json/SenML format), 
which is located in the src/Provider/ProviderExample.cpp file.

The operation is based on the "ApplicationServiceInterface.ini" configuration files:
  sr_base_uri: address of the ServiceRegistry module
  port: this is the TCP port of the HTTP server, which serves the consumer requests (HTTP/GET requests).
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

4) Compile ProviderExample using make
> ./ProviderExample

To grant a secure (HTTPS) communication  to the ServiceRegistry module, use the "./ProviderExample --secureArrowheadInterface" command.
To grant a secure (HTTPS) communication between the comsumer - provider path, use the "./ProviderExample --secureProviderInterface" command.
You can use both options at the same time.
The keys folder contains the required key files for the secure communications. 

About the project sources:
The main() function is located in the ProviderExample.cpp file. It contains a predefined measured value in json/SenML string.
To update the measured value for the current sensor, you should call the procssProvider() function in the main(), with the proper json content.
The SensorHandler.cpp contains the functions for the SensorHandler class, which registers the sensor and serves the consumer requests, using an HTTP/HTTPS server.
The Interface folder contains the sources for the HTTP and HTTPS server. Only modify, if other HTTP methods required than HTTP GET.

The Security folder contains the required RSA functions for the token decryption. Please do not modify the content of this folder.

The code is commented with TODOs and "do not touch below" remarks.
