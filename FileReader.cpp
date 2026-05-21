#include "FileReader.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace fs = std::filesystem;

bool FileReader::exists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

bool FileReader::readFile(const std::string& path, std::vector<char>& outData) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    outData.resize(size);
    return file.read(outData.data(), size).good();
}

std::string FileReader::getMimeType(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".txt") return "text/plain";
    return "application/octet-stream";
}

std::string FileReader::sanitizePath(const std::string& baseDir, const std::string& requestPath) {
    if (requestPath.find("..") != std::string::npos) return "";
    std::string clean = requestPath;
    if (!clean.empty() && clean[0] == '/') clean.erase(0, 1);
    if (clean.empty()) clean = "index.html";
    std::replace(clean.begin(), clean.end(), '/', '\\');
    std::string full = baseDir + "\\" + clean;
    return full;
}