#pragma once
#include <string>

struct HttpRequest {
	std::string method;
	std::string path;
	std::string version;
};

class RequestParser {
public:
	static bool parse(const std::string& rawRequest, HttpRequest& outRequest);
};