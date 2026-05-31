#include "RequestParser.h"
#include <sstream>

bool RequestParser::parse(const std::string& rawRequest, HttpRequest& outRequest) {
	std::istringstream stream(rawRequest);
	std::string line;
	if (!std::getline(stream, line)) return false;

	std::istringstream lineStream(line);
	lineStream >> outRequest.method >> outRequest.path >> outRequest.version;

	if (outRequest.method != "GET" && outRequest.method != "HEAD" && outRequest.method != "POST" && outRequest.method != "DELETE") return false;

	size_t qpos = outRequest.path.find('?');
	if (qpos != std::string::npos) {
        std::string query = outRequest.path.substr(qpos + 1);
        outRequest.path = outRequest.path.substr(0, qpos);
        size_t start = 0;
        while (start < query.size()) {
            size_t eq = query.find('=', start);
            if (eq == std::string::npos) break;
            size_t amp = query.find('&', eq);
            if (amp == std::string::npos) amp = query.size();
            std::string key = query.substr(start, eq - start);
            std::string val = query.substr(eq + 1, amp - eq - 1);
            outRequest.queryParams[key] = val;
            start = amp + 1;
        }
	}

    size_t bodyStart = rawRequest.find("\r\n\r\n");
    if (bodyStart != std::string::npos) outRequest.body = rawRequest.substr(bodyStart + 4);
	return true;
}