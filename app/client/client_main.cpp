
#include "../utils/log.hpp"
#include "client.hpp"
#include <stdlib.h>
#include <time.h>
#define MAX_PATH_LENGTH 32;

const std::string LOG_FILE_PREFIX = "log/client_log_";

Log* StartLog();

int main(int argc, char **argv) {

    Log* log = StartLog();

    if (argc != 3) {
        log->Info("Commmand arguments should be: [server ip address] [server port number] [*optional* script txt file]");
        exit(EXIT_SUCCESS);
    }

    const char* serverIp = argv[1];
    int port = atoi(argv[2]);
    char* scriptFd = (argc == 4) ? argv[3] : nullptr;

    Client client(log);

    client.StartClient(serverIp, port);

    delete log;

    return 0;
}

Log* StartLog() {
    srand(time(NULL));
    std::string semiUniqueId = std::to_string(rand() % 10000);
    return new Log(LOG_FILE_PREFIX + semiUniqueId + ".txt");
}