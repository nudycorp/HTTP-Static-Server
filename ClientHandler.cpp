#include "ClientHandler.h"
#include "Logger.h"
#include "RequestParser.h"
#include "ResponseBuilder.h"
#include "FileReader.h"
#include <vector>
#include <string>

ClientHandler::ClientHandler(SOCKET clientSocket) : clientSocket(clientSocket) {}

void ClientHandler::handle() {
    // Читаем запрос
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';
    std::string requestStr(buffer);

    // Парсим
    HttpRequest request;
    if (!RequestParser::parse(requestStr, request)) {
        std::string response = ResponseBuilder::buildError(400, "Bad Request");
        send(clientSocket, response.c_str(), response.size(), 0);
        shutdown(clientSocket, SD_SEND);
        closesocket(clientSocket);
        return;
    }

    // Формируем путь к файлу
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
        std::string response = ResponseBuilder::buildError(404, "Not Found");
        send(clientSocket, response.c_str(), response.size(), 0);
        Logger::instance().log("404 " + request.path);
    }

    shutdown(clientSocket, SD_SEND);
    closesocket(clientSocket);
}