#pragma once
#include <string>
#include <vector>

class ResponseBuilder {
public:
	static std::string buildSuccess(const std::string& mimeType, const std::vector<char>& data);
	static std::string buildError(int code, const std::string& message);
};