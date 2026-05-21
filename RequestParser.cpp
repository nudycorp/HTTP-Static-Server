#include "RequestParser.h"
#include <sstream>

bool RequestParser::parse(const std::string& rawRequest, HttpRequest& outRequest) {
	std::istringstream stream(rawRequest);
	std::string line;
	if (!std::getline(stream, line)) return false;
	std::istringstream lineStream(line);
	lineStream >> outRequest.method >> outRequest.path >> outRequest.version;

	if (outRequest.method != "GET") return false;

	size_t qpos = outRequest.path.find('?');
	if (qpos != std::string::npos) outRequest.path = outRequest.path.substr(0, qpos);
	return true;
}