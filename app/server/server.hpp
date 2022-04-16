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
#include "data_store.hpp"
#include "../utils/utils.hpp"
#include "../data/response_data.hpp"
#include "../data/conn_data.hpp"
#include "../data/byte_body.hpp"
#include "../data/msg_data.hpp"

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
    void SetNonBlockIO(int fd);
    void RemoveConnection(int i);
    void HandleReceivedCommand(int i, CommandData* commandData);
    void HandleRegister(int i, CommandData* commandData);
    void HandleLogin(int i, CommandData* commandData);
    void HandleLogout(int i, CommandData* commandData);
    void SendResponse(int i, ResponseData* responseData);
    bool IsUiConn(int i);
    bool IsFileConn(int i);
    void SendMessageToUi(const char* username, const char* message);

    void SetConn(int fd);
    void SetUiConn(int i, CommandData* commandData);
    int GetUiConn(const char* username);
    void SetFileConn(int i, CommandData* commandData);
    int GetFileConn(const char* username);

    void HandleSend(int i, CommandData* commandData);
    void HandleSendAnon(int i, CommandData* commandData);
    void HandleSendTo(int i, CommandData* commandData);
    void HandleSendToAnon(int i, CommandData* commandData);
    void StartMessageToAllUsers(char* username, CommandData* commandData, bool isAnon);
    void StartMessageToUser(char* username, CommandData* commandData, bool isAnon);
    void CheckMessageDeques();
    void StartFileToAllUsers(CommandData* commandData);
    void SendFileToUser(CommandData* commandData);

    void HandleSendFile(int i, CommandData* commandData);
    void HandleSendFileTo(int i, CommandData* commandData);
    void ShedConnections(const char* username);

    void HandleList(int i, CommandData* commandData);

    void ExitGracefully();

    Log* log;
    SocketMessenger* sockMsgr;
    DataStore* ds;

    int nConns;	                                     // total # of data sockets
    struct pollfd peers[MAX_CONCURRENCY_LIMIT+1];	   // sockets to be monitored by poll()
    struct SendStat sStat[MAX_CONCURRENCY_LIMIT+1]; // send stats
    struct RecvStat rStat[MAX_CONCURRENCY_LIMIT+1]; // recv stats
    ConnData connData[MAX_CONCURRENCY_LIMIT+1];      // data structure that identifes the conns
};

#endif