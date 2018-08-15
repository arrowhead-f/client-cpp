#include "Tcp_Handler.hpp"

// CLIENT
int Tcp_Handler::Connect(string address, int port)
{
#ifdef _WIN32
	WORD wVersionRequested;          /* socket dll version info */
	WSADATA wsaData;                 /* data for socket lib initialisation */
#endif

struct sockaddr_in server_addr;
struct hostent *host;
int addr_len;

#ifdef _WIN32
	wVersionRequested = MAKEWORD( 1, 1 );
	WSAStartup( wVersionRequested, &wsaData );
#endif

host = (struct hostent *)gethostbyname((char *)address.c_str());

addr_len = sizeof(struct sockaddr);

sock = socket(AF_INET, SOCK_STREAM, 0);

if ( sock == INVALID_SOCKET)
{
	perror("socket");
	return 0;
}

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(port);
server_addr.sin_addr = *((struct in_addr *)host->h_addr);
memset(&(server_addr.sin_zero), '\0', 8);

if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == INVALID_SOCKET) 
{
	perror("Connect");
	return 0;
}
connected = 1;
//printf("Connected to CS Server\n");
return 1;
}
 
int Tcp_Handler::Disconnect()
{

#ifdef _WIN32
	closesocket(sock);
	WSACleanup();
#else
	close(sock);
#endif

connected = 0;

return 1;
}

int Tcp_Handler::SendData(char *pdata, int len)
{
int ret;

ret = send(sock, pdata, len, 0);

return (ret==len);
}

int Tcp_Handler::RecvData(char *pdata, int len)
{
int ret;

ret = recv(sock, pdata,len, 0);

return (ret==len);
} 


// SERVER
int Tcp_Handler::SRV_Start(int port)
{
	struct sockaddr_in server_addr;

	#ifdef _WIN32
		wVersionRequested = MAKEWORD(1, 1);
		WSAStartup(wVersionRequested, &wsaData);
	#endif

	if ((srv = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
	{
		perror("Socket fail");
		return 0;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(server_addr.sin_zero), '\0', 8);

	if (bind(srv, (struct sockaddr *)&server_addr,
		sizeof(struct sockaddr)) == -1)
	{
		perror("Bind");
		return 0;
	} 

return 1;
}

int Tcp_Handler::SRV_Listen()
{
int addr_len;

	if (listen(srv, 5) == -1)
	{
		perror("Listen");
		return 0;
	}

	int addr_len = sizeof(struct sockaddr_in);

	sock = accept(srv, (struct sockaddr *)&client_addr, &addr_len);
 
	if (sock != INVALID_SOCKET)
		connected = 1;

return connected;
}

int Tcp_Handler::SRV_Stop()
{

if (connected)
{
	#ifdef _WIN32
		closesocket(sock);
	#else
		close(sock);
	#endif
}
connected = 0;
	#ifdef _WIN32
		closesocket(srv);
		WSACleanup();
	#else
		close(srv);
	#endif

return 1;
}