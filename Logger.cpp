#define _CRT_SECURE_NO_WARNINGS

#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

Logger& Logger::instance() {
	static Logger logger;
	return logger;
}

Logger::Logger() {
	logFile.open("server.log", std::ios::app);
	if (!logFile.is_open()) std::cerr << "Warning: can not open log file\n";
}

Logger::~Logger() {
	if (logFile.is_open()) logFile.close();
}

void Logger::log(const std::string& message) {
	std::lock_guard<std::mutex> lock(mtx);
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
	std::cout << message << std::endl;
}

void Logger::log(const std::string& ip, const std::string& request, int status) {
	std::lock_guard<std::mutex> lock(mtx);
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << " [" << ip << "]  \"" << request << "\" " << status << std::endl;
}