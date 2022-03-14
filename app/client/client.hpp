#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <unistd.h>

#include "../utils/log.hpp"

class Client {

    public:
        Client(Log* log);
        ~Client();

        void StartClient(const char* serverIp, int port);

    private:
        void SetLog(Log* log);
        void ListenToServer();
        int RecvBlocking();

        struct pollfd server;
        Log* log;

};

#endif