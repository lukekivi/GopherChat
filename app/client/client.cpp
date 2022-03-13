#include "client.hpp"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>

/**
 * @brief Setup a client
 * 
 * @param serverIp 
 * @param port 
 */
void Client::StartClient(const char* serverIp, int port) {
    struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((unsigned short) port);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);
		
	signal(SIGPIPE, SIG_IGN); //ignore the SIGPIPE signal that may crash the program in some corner cases	
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
		
    if (sockFd == -1) {
		log.Error("Cannot create socket.");		
		exit(EXIT_FAILURE);
	}
		
	//Connect to server
	if (connect(sockFd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0) {
		log.Error("Cannot connect to server %s:%d.", serverIp, port);
		exit(EXIT_FAILURE);
    } else {
        log.Info("Client: connected to %s:%d", serverIp, port);
    }
}

void Client::SetLog(Log& log) {
    this->log = log;
}