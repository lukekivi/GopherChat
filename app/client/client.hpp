#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <unistd.h>
#include <poll.h>
#include <vector>
#include "../data/command_data.hpp"
#include "../utils/socket/socket_messenger.hpp"
#include "../utils/utils.hpp"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include <fcntl.h>

#define MAX_CONNS 50
#define RES_CONNS 1

class Client {

    public:
        Client(Log* log);
        ~Client();

        void StartClient(const char* serverIp, int port, std::vector<CommandData*> commands_);

    private:
        void CheckUi();
        void SetLog(Log* log);
        void SetNonBlockIO(int fd);
        int BuildConn();
        void RecvMessage(int i);
        void SendMessage(int i);
        void RemoveConnection(int i);
        BYTE* CommandToByte(CommandData* command);
        void HandleResponse(BYTE* body, int len);
        bool IsUiOrFileConn(int i);
        void PrintToUi(int i);
        void PrintResponse(ResponseData* responseData);
        void StartCommand(CommandData* command);
        void SetupSession();
        CommandData* BuildUiCommand();
        void PrepareMessage(CommandData* commandData, int i);

        void ExitGracefully();

        std::vector<CommandData*> commands;

    	struct sockaddr_in serverAddr;
        int nConns = 0;
        pollfd peers[MAX_CONNS];
        SendStat sStats[MAX_CONNS];
        RecvStat rStats[MAX_CONNS];
        Log* log;
        SocketMessenger* sockMsgr;

        char* loggedInUser = NULL;
        int uiConn = -1;
        std::vector<int> fileConns;
};

#endif