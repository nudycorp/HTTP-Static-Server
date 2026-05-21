#pragma once
#include <string>
#include <vector>

class FileReader {
public:
	static bool exists(const std::string& path);
	static bool readFile(const std::string& path, std::vector<char>& outData);
	static std::string getMimeType(const std::string& path);
	static std::string sanitizePath(const std::string& baseDir, const std::string& requestPath);
};