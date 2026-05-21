#include "ResponseBuilder.h"
#include <sstream>

std::string ResponseBuilder::buildSuccess(const std::string& mimeType, const std::vector<char>& data) {
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: " << mimeType << "\r\n";
	response << "Content-Length: " << data.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response.write(data.data(), data.size());
	return response.str();
}

std::string ResponseBuilder::buildError(int code, const std::string& message) {
	std::string body = "<!DOCTYPE html><html><body><h1>" + std::to_string(code) + " " + message + "</h1></body></html>";
	std::ostringstream response;
	response << "HTML/1.1 " << code << " " << message << "\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n" << body;
	return response.str();
}