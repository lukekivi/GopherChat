#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <unistd.h>

#include "../utils/log.hpp"

class Client {

    public:
        ~Client() { if (sockFd != -1) close(sockFd); }

        void StartClient(const char* serverIp, int port);
        void SetLog(Log& log);
        // void ProcessRequets();

    private:
        int sockFd = -1;
        Log log;

};

#endif CLIENT_HPP