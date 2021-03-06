#include "SocketClientTCP.h"
#include "SocketDataParser.h"
#include "Util.h"


AsyncConnection::AsyncConnection(SocketClientTCP * mClientTCP) : clientTCP(mClientTCP)
{
	shouldCheck = false;
	connected = false;
}

AsyncConnection::~AsyncConnection() {}

int SocketClientTCP::ConnectionThread(void * ptr)
{
	ConnectionData * connectionData = (ConnectionData *)ptr;

	connectionData->asyncConnection->clientTCP->mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectionData->asyncConnection->clientTCP->mainSocket == INVALID_SOCKET)
	{
		Util::Error("Error creating socket: " + getSockError());
		closesocket(connectionData->asyncConnection->clientTCP->mainSocket);

		return false;
	}

	sockaddr_in service;
	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(connectionData->domain);
	service.sin_port = htons(connectionData->port);

	// Probably should be in thread
	if (connect(connectionData->asyncConnection->clientTCP->mainSocket, (SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR)
	{
		Util::Warning("Failed to connect.");
		sockClose(connectionData->asyncConnection->clientTCP->mainSocket);
		return false;
	}
}

SocketClientTCP::SocketClientTCP(std::string tag) : SocketClient(tag + "_t"){}

SocketClientTCP::~SocketClientTCP(){}

bool SocketClientTCP::Connect(const char * domain, unsigned short port)
{
	mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mainSocket == INVALID_SOCKET)
	{
		Util::Error("Error creating socket: " + getSockError());
		closesocket(mainSocket);

		return false;
	}

	sockaddr_in service;
	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(domain);
	service.sin_port = htons(port);

	// Probably should be in thread
	if (connect(mainSocket, (SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR)
	{
		Util::Warning("Failed to connect.");
		sockClose(mainSocket);
		return false;
	}

	// Initial send data variable
	InitialData initalData; 
	initalData.nick = (char *)"Przemek";

	// Log 
	Util::Debug(initalData.nick);

	// Send data
	Send((char *)SocketDataParser::ParseDataInitalIn(initalData).c_str());

	SDL_Delay(2000);
	Send((char *)"Test over network with 2000ms delay.");

	return true;
}

void SocketClientTCP::AsyncConnect(const char * domain, unsigned short port)
{
	asyncConnect = new AsyncConnection(this);

	ConnectionData * connectionData = new ConnectionData();

	thread = SDL_CreateThread(SocketClientTCP::ConnectionThread, "AsyncConnect", connectionData);
}

bool SocketClientTCP::IsAsyncReady()
{
	return asyncConnect->shouldCheck;
}

bool SocketClientTCP::IsAsyncConnected()
{
	return asyncConnect->connected;
}

// @Todo move to thread
void SocketClientTCP::Send(char * data)
{
#ifdef _DEBUG
	if (!mainSocket || mainSocket == NULL)
	{
		Util::Error("SocketClientTCP::Send(): mainSocket is NULL.");
		Util::Error("No data was send.");
		return;
	}
	if (data == NULL)
	{
		Util::Error("data variable is nullptr.");
		Util::Error("No data was send.");
		return;
	}
	Util::Debug("Send: " + (std::string)data);
#endif 

	// Add start and end to the data
	std::string dataString;
	dataString.append(SocketDataStart);
	dataString.append(data);
	dataString.append(SocketDataEnd);

#ifdef _DEBUG
	// Log debug data. (Characters which will be attempted to send.)
	Util::Debug("Attempting to send: " + dataString);
#endif

	// Send data
	int bytesSent = send(mainSocket, dataString.c_str(), strlen(dataString.c_str()), 0);

#ifdef _DEBUG
	// Log debug data. (Ammount of send bytes.)
	Util::Debug("Send: " + std::to_string(bytesSent) + " bytes.");
#endif
}

// @Todo move to thread
char * SocketClientTCP::Recive()
{
	int bytesRecv = SOCKET_ERROR;
	char recvbuf[128] = "";

	bytesRecv = recv(mainSocket, recvbuf, 128, 0);
	std::cerr << "Bytes received: " << std::to_string(bytesRecv) << std::endl;
	std::cerr << "Received text: " << recvbuf << std::endl;

	return nullptr;
}
