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

std::string FileReader::listFilesAsJSON(const std::string& baseDir) {
    std::ostringstream json;
    json << "[";
    bool first = true;
    for (const auto& entry : fs::recursive_directory_iterator(baseDir)) {
        if (entry.is_regular_file()) {
            if (!first) json << ",";
            first = false;
            std::string rel = fs::relative(entry.path(), baseDir).string();
            std::replace(rel.begin(), rel.end(), '\\', '/');
            json << R"({"name":")" << rel << R"(","size":)" << entry.file_size() << "}";
        }
    }

    json << "]";
    return json.str();
}

bool FileReader::saveFile(const std::string& dir, const std::string& filename, const std::vector<char>& data) {
    fs::create_directories(dir);
    fs::path full = fs::path(dir) / filename;
    std::ofstream out(full, std::ios::binary);
    if (!out) return false;
    out.write(data.data(), data.size());
    return out.good();
}

bool FileReader::deleteFile(const std::string& path) { return fs::remove(path); }

bool FileReader::isPathSafe(const std::string& baseDir, const std::string& requestPath) {
    fs::path full = fs::weakly_canonical(fs::path(baseDir) / requestPath);
    std::string fullStr = full.string();
    std::string baseStr = fs::canonical(baseDir).string();
    return fullStr.find(baseStr) == 0;
}