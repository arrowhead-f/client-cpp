Arrowhead C++ client skeletons

The following Linux packages are prerequisities:
openssl, libgnutls28-dev, libgnutlsxx28, libssl1.1, libssl1.0-dev, libcurl3, libcurl3-gnutls, libcurl4-gnutls-dev, libcrypto++-dev, libcrypto++-utils, libcrypto++6, libgpg-error-dev, automake, texinfo, g++, libjson-c-dev

The project uses libmicrohttpd-0.9.59 as well. Download, compile and install it from source with HTTPS support: https://ftp.gnu.org/gnu/libmicrohttpd/
  >tar -xvzf libmicrohttpd-0.9.59.tar.gz

  >./configure --with-gnutls

  >make

  >sudo make install

Create libmicrohttpd.so.12 file in /usr/lib or usr/local/lib directory (or where the ”ldd ProviderExample” command points):
  >cd /usr/lib

  >sudo ln –s /usr/local/lib/libmicrohttpd.so.12.46.0 libmicrohttpd.so.12

In a case of missing libcrypto.so:
>	cd /usr/lib

>	sudo ln –s libcrypto.so.1.0.0 /lib/arm-linux-gnueabihf/libcrypt-2.24.so

The example projects contain a Makefile to build the sources for the current architecture. 


To support UDP/SenML -> MQTT translation, download, build and install the Eclipse Mosquitto project, using the following commands. Before make, copy (if necessary, overwrite) the source files from SensorAdapter/changes/MQTT directory to the downloaded mosquitto/src directory.
>	tar -xvzf mosquitto-1.4.15.tar.gz
>	make
>	sudo make install

To build the Eclipse Mosquitto project, the following linux packages must be installed (sudo apt-get install):
>	libc-ares-dev
>	uuid-dev
>	xsltproc

There is a Provider and Consumer example Linux project (tested on Raspbian) written on C++. The Provider example registers a basic REST/JSON-SenML provider into the ArrowheadServiceRegistry module, and defines a HTTP or HTTPS interface for the Consumer. The Consumer example project orchestrates from the Arrowhead Orchestrator the Provider’s URI (on HTTP or HTTPS), and sends HTTP or HTTPS requests to receive the latest measured value
