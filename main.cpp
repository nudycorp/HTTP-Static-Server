#include "Server.h"
#include "Logger.h"
#include <iostream>

int main() {
	Logger::instance().log("HTTP Server starting...");

	Server server(8080);
	if (!server.start()) {
		Logger::instance().log("Failed to start server");
		return 1;
	}

	std::cout << "Server will run on http://localhost:8080\n";
	std::cout << "Press any key to exit...\n";

	std::thread serverThread([&server]() {
		server.run();
	});

	std::cin.get();
	server.stop();

	if (serverThread.joinable()) serverThread.join();

	Logger::instance().log("HTTP Server stopped");
	return 0;
}