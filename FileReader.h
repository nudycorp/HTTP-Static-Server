#pragma once
#include <string>
#include <vector>

class FileReader {
public:
	static bool exists(const std::string& path);
	static bool readFile(const std::string& path, std::vector<char>& outData);
	static std::string getMimeType(const std::string& path);
	static std::string sanitizePath(const std::string& baseDir, const std::string& requestPath);
	static std::string listFilesAsJSON(const std::string& baseDir);
	static bool saveFile(const std::string& dir, const std::string& filename, const std::vector<char>& data);
	static bool deleteFile(const std::string& path);
	static bool isPathSafe(const std::string& baseDir, const std::string& requestPath);
};