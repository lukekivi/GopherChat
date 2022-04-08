#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <signal.h>
#include "../utils/socket/socket_messenger.hpp"
#include "../data/command_data.hpp"

#define MAX_REQUEST_SIZE 10000000
#define MAX_CONCURRENCY_LIMIT 64

class Server {

  public:
    Server(Log* log);
    ~Server();

    void StartServer(int port);

  private:
    void RecvMessage(int i);
    void SendMessage(int i);
    void SendGreeting(int i);
    void SetNonBlockIO(int fd);
    void RemoveConnection(int i);
    
    Log* log;
    SocketMessenger* sockMsgr;

    int nConns;	                                     // total # of data sockets
    struct pollfd peers[MAX_CONCURRENCY_LIMIT+1];	   // sockets to be monitored by poll()
    struct SendStat sStat[MAX_CONCURRENCY_LIMIT+1]; // send stats
    struct RecvStat rStat[MAX_CONCURRENCY_LIMIT+1]; // recv stats
};

#endif