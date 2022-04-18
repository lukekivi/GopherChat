// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019


#include "../utils/log.hpp"
#include "../utils/userInput/script_reader.hpp"
#include "client.hpp"
#include <stdlib.h>
#include <time.h>
#define MAX_PATH_LENGTH 32;

const std::string LOG_FILE_PREFIX = "log/client_log_";

Log* StartLog();

int main(int argc, char **argv) {

    Log* log = StartLog();

    if (argc != 4) {
        log->Info("Commmand arguments should be: [server ip address] [server port number] [script txt file]");
        exit(EXIT_SUCCESS);
    }

    const char* serverIp = argv[1];
    int port = atoi(argv[2]);
    char* scriptFileName = argv[3];

    ScriptReader reader(scriptFileName, log);
    Client client(log);

    client.StartClient(serverIp, port, reader.getCommands());

    delete log;

    return 0;
}

Log* StartLog() {
    srand(time(NULL));
    std::string semiUniqueId = std::to_string(rand() % 10000);
    std::cout << "Log id: " << semiUniqueId << std::endl;
    return new Log(LOG_FILE_PREFIX + semiUniqueId + ".txt");
}