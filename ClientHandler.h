#pragma once
#include <winsock2.h>
#include <string>

class ClientHandler {
public:
	explicit ClientHandler(SOCKET clientSocket);
	void handle();
private:
	SOCKET clientSocket;
	bool readRequest(std::string& request);
	void sendResponse(const std::string& response);
	void setSocketTimeout(int seconds);
	std::string urlDecode(const std::string& src);
};