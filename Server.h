#pragma once
#include <winsock2.h>
#include <string>

class Server {
public:
	Server(int port);
	~Server();
	bool start();
	void stop();
	void run();
private:
	int port;
	SOCKET serverSocket;
	bool running;
	void handleClient(SOCKET clientSocket);
};