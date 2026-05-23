#pragma once
#include <winsock2.h>
#include <string>

class ClientHandler {
public:
	explicit ClientHandler(SOCKET clientSocket);
	void handle();
private:
	SOCKET clientSocket;
};