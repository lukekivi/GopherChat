#include "server.hpp"
#include <fstream>
#include <stdio.h>

const std::string LOG_FILE = "log/server_log.txt";

int main(int argc, char** argv) {	
    Log log(LOG_FILE);

	if (argc != 2) {
		log.Error("Usage: %s [server Port]", argv[0]);
		return -1;
	}
	if (strcmp(argv[1], "reset") == 0) { 
		std::cout << "Reset registration file." << std::endl;
		remove(REGISTRAR_FILE_PATH);
		exit(0);
	}

    Server server(&log);
	
	int port = atoi(argv[1]);
	server.StartServer(port);
	
	return 0;
}