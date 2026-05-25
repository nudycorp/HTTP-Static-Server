#include "ClientHandler.h"
#include "Logger.h"
#include "RequestParser.h"
#include "ResponseBuilder.h"
#include "FileReader.h"
#include <vector>
#include <string>
#include <cstring>

ClientHandler::ClientHandler(SOCKET clientSocket) : clientSocket(clientSocket) {}

void ClientHandler::setSocketTimeout(int seconds) {
    DWORD timeout = seconds * 1000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
}

bool ClientHandler::readRequest(std::string& request) {
    char buffer[4096];
    std::string data;
    while (true) {
        int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            if (bytes == 0) Logger::instance().log("Client closed connection");
            else {
                int err = WSAGetLastError();
                if (err != WSAETIMEDOUT) Logger::instance().log("Recv error: " + std::to_string(err));
            }

            return false;
        }

        buffer[bytes] = '\0';
        data += buffer;
        if (data.find("\r\n\r\n") != std::string::npos) {
            request = data;
            return true;
        }

        if (data.size() > 8192) return false;
    }
}

void ClientHandler::sendResponse(const std::string& response) {
    size_t total = 0;
    while (total < response.size()) {
        int sent = send(clientSocket, response.c_str() + total, response.size() - total, 0);
        if (sent <= 0) break;
        total += sent;
    }
}

void ClientHandler::handle() {
    bool keepAlive = true;
    int requestCount = 0;
    const int MAX_REQUESTS = 100;
    const int TIMEOUT_SECONDS = 5;

    while (keepAlive && requestCount < MAX_REQUESTS) {
        if (requestCount > 0) {
            setSocketTimeout(TIMEOUT_SECONDS);
        }

        std::string rawRequest;
        if (!readRequest(rawRequest)) {
            break;
        }

        HttpRequest request;
        if (!RequestParser::parse(rawRequest, request)) {
            std::string response = ResponseBuilder::buildError(400, "Bad Request", false);
            sendResponse(response);
            break;
        }

        bool wantKeepAlive = (rawRequest.find("Connection: close") == std::string::npos);

        const std::string baseDir = "www";
        std::string relativePath = request.path;
        if (relativePath == "/") relativePath = "/index.html";
        std::string fullPath = FileReader::sanitizePath(baseDir, relativePath);

        std::vector<char> fileData;
        bool fileExists = !fullPath.empty() && FileReader::exists(fullPath);
        if (fileExists) {
            std::string mime = FileReader::getMimeType(fullPath);
            if (request.method == "GET") FileReader::readFile(fullPath, fileData);
            std::string response = ResponseBuilder::buildSuccess(mime, fileData, wantKeepAlive, request.method == "HEAD");
            sendResponse(response);
            Logger::instance().log("200 " + request.path + " (" + request.method + ")");
            keepAlive = wantKeepAlive;
        }
        else {
            std::string response = ResponseBuilder::buildError(404, "Not Found", false);
            sendResponse(response);
            Logger::instance().log("404 " + request.path);
            break;
        }

        requestCount++;
    }

    shutdown(clientSocket, SD_SEND);
    closesocket(clientSocket);
}