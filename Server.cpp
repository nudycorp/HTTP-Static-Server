#include "Server.h"
#include "Logger.h"
#include "RequestParser.h"
#include "ResponseBuilder.h"
#include "FileReader.h"
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

Server::Server(int port) : port(port), serverSocket(INVALID_SOCKET), running(false) {}

Server::~Server() {
	stop();
}

bool Server::start() {
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		Logger::instance().log("WSAStartup failed");
		return false;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		Logger::instance().log("Socket creation failed: " + std::to_string(WSAGetLastError()));
		WSACleanup();
		return false;
	}

	int opt = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		Logger::instance().log("Bind failed: " + std::to_string(WSAGetLastError()));
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	if (listen(serverSocket, 10) == SOCKET_ERROR) {
		Logger::instance().log("Listen failed:" + std::to_string(WSAGetLastError()));
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	running = true;
	Logger::instance().log("Server started on port: " + std::to_string(port));
	return true;
}

void Server::stop() {
	running = false;
	if (serverSocket != INVALID_SOCKET) {
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
	}
	
	WSACleanup();
	Logger::instance().log("Server stopped");
}

void Server::run() {
	if (!running) return;

	while (running) {
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			if (running) {
				Logger::instance().log("Accept failed: " + std::to_string(WSAGetLastError()));
			}
			continue;
		}

		Logger::instance().log("Client connected");
		handleClient(clientSocket);
	}
}

void Server::handleClient(SOCKET clientSocket) {
	char buffer[4096];
	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesReceived <= 0) {
		closesocket(clientSocket);
		return;
	}
	buffer[bytesReceived] = '\0';
	std::string requestStr(buffer);

	HttpRequest request;
	if (!RequestParser::parse(requestStr, request)) {
		std::string response = ResponseBuilder::buildError(400, "Bad request");
		send(clientSocket, response.c_str(), response.size(), 0);
		shutdown(clientSocket, SD_SEND);
		closesocket(clientSocket);
		return;
	}

	const std::string baseDir = "www";
	std::string relativePath = request.path;
	if (relativePath == "/") relativePath = "/index.html";
	std::string fullPath = FileReader::sanitizePath(baseDir, relativePath);

	std::vector<char> fileData;
	if (!fullPath.empty() && FileReader::exists(fullPath) && FileReader::readFile(fullPath, fileData)) {
		std::string mime = FileReader::getMimeType(fullPath);
		std::string response = ResponseBuilder::buildSuccess(mime, fileData);
		send(clientSocket, response.c_str(), response.size(), 0);
		Logger::instance().log("200 " + request.path);
	}
	else {
		std::string response = ResponseBuilder::buildError(404, "Not found");
		send(clientSocket, response.c_str(), response.size(), 0);
		Logger::instance().log("404 " + request.path);
	}

	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
}