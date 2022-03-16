#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <unistd.h>
#include <poll.h>
#include "../utils/socket/socket_messenger.hpp"

class Client {

    public:
        Client(Log* log);
        ~Client();

        void StartClient(const char* serverIp, int port);

    private:
        void SendMessage(struct SEND_STAT* sStat, struct pollfd* server);

        void SetLog(Log* log);
        void SetNonBlockIO(int fd);

        struct RECV_STAT mainRStat;
        struct pollfd server;
        Log* log;
        SocketMessenger* sockMsgr;
};

#endif