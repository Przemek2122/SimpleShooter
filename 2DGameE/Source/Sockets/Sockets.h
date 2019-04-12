#pragma once
#include <string>
#include <map>
#include "Util.h"
#include "game.h"
#include "Sockets/SocketClientTCP.h"
#include "Sockets/SocketServerTCP.h"


class SocketsManager
{
public:
	SocketsManager(Game * mGame);
	~SocketsManager();

	Game * game;


#ifndef _DEDICATED
	/* TCP socket
	 * Local socket used to connect and use later
	 * Only used on client
	 * Doesn't exists on dedicated */
	SocketClientTCP * localSocket;
#endif 

	/* Server listener socket.
	 * Only used on server */
	SocketServerTCP * socketsServer;

	/* Connected clients sockets:
	 * Clients connected in map
	 * With ip as string and
	 * With names as strings. */
	//std::map<>


	/* Attempts to connect with
	 * server at @domain with @port
	 * return true on success false otherwise.
	 * Default tag is "local". */
	bool Connect(const char * domain, Uint16 port, std::string tag = "local");

	/* Creates listener socket.
	 * Default tag is "server". */
	bool Listen(Uint16 port, std::string tag = "server");

	/* Assuming this is not server.
	 * Probably crash if called on server.
	 * Because it will return empty variable.
	 * Inline for performance. */
	inline SocketClientTCP& GetLocalSocketTCP();

};