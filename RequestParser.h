#pragma once
#include <string>
#include <unordered_map>

struct HttpRequest {
	std::string method;
	std::string path;
	std::string version;
	std::string body;
	std::unordered_map<std::string, std::string> queryParams;
};

class RequestParser {
public:
	static bool parse(const std::string& rawRequest, HttpRequest& outRequest);
};