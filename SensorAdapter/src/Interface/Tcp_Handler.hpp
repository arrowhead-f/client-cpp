#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h> 

class Tcp_Handler
{
private:
	SOCKET sock;
	SOCKET srv;
	struct sockaddr_in client_addr;
	int connected = 0;

public:

int Connect(string address, int port);
int Disconnect();
int SendData(char *pdata, int len);
int RecvData(char *pdata, int len);

int SRV_Start(int port);
int SRV_Listen(); // Blocking!
int SRV_Stop();

};
