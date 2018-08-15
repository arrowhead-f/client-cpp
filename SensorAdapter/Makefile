INC=-Isrc/Interface/include/curl -Isrc/Interface/include/mhttpd
LIB=-L/usr/local/lib -lpthread -lmicrohttpd -lcurl -lcrypto -lgnutls
CC=g++

.PHONY: all clean

all: adapter

adapter: orchIntf.o servRegIntf.o http.o https.o ini.o dict.o consServ.o consTable.o moteIntf.o moteTable.o provServ.o rsaSec.o adapter.o sensorHandler.o sensorTable.o b64.o wlist.o
	$(CC) $^ -std=gnu++11 $(INC) $(LIB) -o adapter 

orchIntf.o: src/Interface/OrchestratorInterface.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
servRegIntf.o: src/Interface/ServiceRegistryInterface.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
http.o: src/Interface/Http_Handler.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)

https.o: src/Interface/Https_Handler.cpp		
			$(CC) -c $^ -o $@ $(INC) $(LIB)
	
ini.o: src/Interface/ini/iniparser.c
			$(CC) -c $^ -o $@ $(INC) $(LIB)

dict.o: src/Interface/ini/dictionary.c
			$(CC) -c $^ -o $@ $(INC) $(LIB)

consServ.o: src/Adapter/Consumer/ConsumedService.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
consTable.o: src/Adapter/Consumer/ConsumerTable.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
moteIntf.o: src/Interface/MoteInterface.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
moteTable.o: src/Adapter/Provider/MoteTable.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
provServ.o: src/Adapter/Provider/ProvidedService.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
rsaSec.o: src/Security/RSASecurity.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
adapter.o: src/Adapter/Adapter.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
sensorHandler.o: src/Adapter/SensorHandler.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
sensorTable.o: src/Adapter/Provider/SensorTable.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
b64.o: src/Security/base64.cpp	
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
wlist.o: src/Adapter/Provider/WhiteList.cpp
			$(CC) -c $^ -o $@ $(INC) $(LIB)
			
clean:
	rm -f *.o
	rm -f adapter
