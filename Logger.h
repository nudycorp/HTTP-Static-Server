#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
	static Logger& instance();
	void log(const std::string& message);
	void log(const std::string& ip, const std::string& request, int status);
private:
	Logger();
	~Logger();
	std::ofstream logFile;
	std::mutex mtx;
};