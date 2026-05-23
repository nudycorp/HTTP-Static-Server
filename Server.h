#pragma once
#include <winsock2.h>
#include <string>
#include <memory>
#include "ThreadPool.h"

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
	std::unique_ptr<ThreadPool> pool;
	void handleClient(SOCKET clientSocket);
};