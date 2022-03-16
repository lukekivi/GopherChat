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

#define MAX_REQUEST_SIZE 10000000
#define MAX_CONCURRENCY_LIMIT 64

class Server {

  public:
    Server(Log* log);
    ~Server();

    /**
     * @brief Start the GopherChat server. Connect clients and route messages.
     * 
     * @param port 
     */
    void StartServer(int port);

  private:
    /**
     * @brief Send a non-blocking message to a client.
     * 
     * @param i index of the client
     */
    void SendMessage(int i);
    /**
     * @brief Send a welcome message to new clients.
     * 
     * @param i index of the client.
     */
    void SendGreeting(int i);
    /**
     * @brief Set socket to be non-blocking
     */
    void SetNonBlockIO(int fd);
    /**
     * @brief remove a connection and cleanup its data
     * 
     * @param i index of the connection
     */
    void RemoveConnection(int i);
    
    Log* log;
    SocketMessenger* sockMsgr;

    int nConns;	                                     // total # of data sockets
    struct pollfd peers[MAX_CONCURRENCY_LIMIT+1];	   // sockets to be monitored by poll()
    struct SEND_STAT sStat[MAX_CONCURRENCY_LIMIT+1]; // send stats
    struct RECV_STAT rStat[MAX_CONCURRENCY_LIMIT+1]; // recv stats
};

#endif