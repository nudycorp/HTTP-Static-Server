#include "Logger.h"
#include <iostream>

int main() {
	Logger::instance().log("HTTP Server starting...");
	std::cout << "Server will run on http://localhost:8080\n";
	std::cout << "Press any key to exit...\n";
	std::cin.get();
	Logger::instance().log("HTTP Server stopped");
	return 0;
}