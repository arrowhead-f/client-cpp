INC=-Isrc/Interface/include/curl
LIB=-lmicrohttpd -lcurl -lcrypto -lpthread -ljson-c
CC=g++

.PHONY: all clean

all: cons

cons: orchIntf.o http.o https.o ini.o dict.o consumedService.o rsaSec.o consEx.o sensorHandler.o b64.o                                  
	$(CC) $^ -std=gnu++11 $(INC) $(LIB) -o ConsumerExample 
	rm -f *.o

orchIntf.o: src/Interface/OrchestratorInterface.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

http.o: src/Interface/Http_Handler.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

https.o: src/Interface/Https_Handler.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

ini.o: src/Interface/ini/iniparser.c
	$(CC) -c $^ -o $@ $(INC) $(LIB)

dict.o: src/Interface/ini/dictionary.c
	$(CC) -c $^ -o $@ $(INC) $(LIB)

consumedService.o: src/Consumer/ConsumedService.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

rsaSec.o: src/Security/RSASecurity.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

consEx.o: src/Consumer/ConsumerExample.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

sensorHandler.o: src/Consumer/SensorHandler.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

b64.o: src/Security/base64.cpp
	$(CC) -c $^ -o $@ $(INC) $(LIB)

clean:
	rm -f *.o cons

