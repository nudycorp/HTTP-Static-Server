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
        if (requestCount > 0) setSocketTimeout(TIMEOUT_SECONDS);

        std::string rawRequest;
        if (!readRequest(rawRequest)) break;

        HttpRequest request;
        if (!RequestParser::parse(rawRequest, request)) {
            std::string response = ResponseBuilder::buildError(400, "Bad Request", false);
            sendResponse(response);
            break;
        }

        bool wantKeepAlive = (rawRequest.find("Connection: close") == std::string::npos);

        const std::string baseDir = "www";
        std::string relativePath = request.path;

        if (request.path == "/files" && request.method == "GET") {
            std::string json = FileReader::listFilesAsJSON(baseDir);
            std::vector<char> data(json.begin(), json.end());
            std::string response = ResponseBuilder::buildSuccess("application/json", data, wantKeepAlive);
            sendResponse(response);
            Logger::instance().log("200 /files (GET)");
            keepAlive = wantKeepAlive;
            continue;
        }
        else if (request.path == "/upload" && request.method == "POST") {
            if (request.body.empty()) {
                std::string response = ResponseBuilder::buildError(400, "Empty body", false);
                sendResponse(response);
                Logger::instance().log("400 /upload");
                break;
            }

            std::vector<char> fileData(request.body.begin(), request.body.end());
            std::string filename = "upload_" + std::to_string(std::time(nullptr)) + ".dat";
            std::string uploadDir = baseDir + "/uploads";

            if (FileReader::saveFile(uploadDir, filename, fileData)) {
                std::string json = R"({"status":"ok","file":")" + filename + R"("})";
                std::vector<char> respData(json.begin(), json.end());
                std::string response = ResponseBuilder::buildSuccess("application/json", respData, false);
                sendResponse(response);
                Logger::instance().log("200 /upload => " + filename);
            }
            else {
                std::string response = ResponseBuilder::buildError(500, "Save failed", false);
                sendResponse(response);
                Logger::instance().log("500 /upload");
            }

            break;
        }
        else if (request.path == "/delete" && request.method == "DELETE") {
            auto it = request.queryParams.find("file");
            if (it == request.queryParams.end()) {
                std::string response = ResponseBuilder::buildError(400, "Missing file param", false);
                sendResponse(response);
                Logger::instance().log("400 /delete");
                break;
            }

            std::string fileParam = it->second;
            if (!FileReader::isPathSafe(baseDir, fileParam)) {
                std::string response = ResponseBuilder::buildError(400, "Forbidden", false);
                sendResponse(response);
                break;
            }

            std::string fullPath = baseDir + "/" + fileParam;
            if (FileReader::deleteFile(fullPath)) {
                std::string json = R"({"status":"deleted","file":")" + fileParam + R"("})";
                std::vector<char> data(json.begin(), json.end());
                std::string response = ResponseBuilder::buildSuccess("application/json", data, false);
                sendResponse(response);
                Logger::instance().log("200 DELETE " + fileParam);
            }
            else {
                std::string response = ResponseBuilder::buildError(404, "File not found", false);
                sendResponse(response);
                Logger::instance().log("404 DELETE " + fileParam);
            }
            break;
        }
        else {
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
        }

        requestCount++;
    }

    shutdown(clientSocket, SD_SEND);
    closesocket(clientSocket);
}