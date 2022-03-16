#include "server.hpp"

const std::string LOG_FILE = "log/server_log.txt";

int main(int argc, char** argv) {	
    Log log(LOG_FILE);

	if (argc != 2) {
		log.Error("Usage: %s [server Port]", argv[0]);
		return -1;
	}

    Server server(&log);
	
	int port = atoi(argv[1]);
	server.StartServer(port);
	
	return 0;
}