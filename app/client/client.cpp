#include "client.hpp"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include <thread>
#include <poll.h>
#include <fcntl.h>

#define MAX_REQUEST_SIZE 10000000

Client::Client(Log* log) { 
	SetLog(log); 
	server.fd = -1;
	sockMsgr = new SocketMessenger(log);
}

Client::~Client() {
	if (server.fd != -1) {
		close(server.fd); 
	}

	delete log;
	delete sockMsgr;
}


void Client::StartClient(const char* serverIp, int port) {
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((unsigned short) port);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);
	signal(SIGPIPE, SIG_IGN);

	memset(&server, 0, sizeof(server));	
	server.events = POLLRDNORM;	
	memset(&mainRStat, 0, sizeof(struct RECV_STAT));
	sockMsgr->InitRecvStat(&mainRStat);

	//Create the socket
	server.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server.fd == -1) {
		log->Error("Cannot create socket.");		
	}

	//Connect to server
	if (connect(server.fd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0) {
		log->Error("Cannot connect to server %s:%d.", serverIp, port);
		exit(EXIT_FAILURE);
	}

	SetNonBlockIO(server.fd);

	log->Info("Connected to server %s:%d.", serverIp, port);
	
	while (1) {	
		//monitor the listening sock and data socks, nConn+1 in total
		int r = poll(&server, 1, -1);	
		if (r < 0) {
			log->Error("Invalid poll() return value.");
		}			
			
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);	
				
		if (server.revents & (POLLRDNORM | POLLERR | POLLHUP)) {	
			// recv request
			NbStatus status = sockMsgr->RecvMsgNB(&mainRStat, &server);

				switch(status) {
					case OKAY:
						// successfully read from socket
						std::cout << mainRStat.bodyStat.msg << std::endl;
						sockMsgr->InitRecvStat(&mainRStat);
						break;
					case BLOCKED:
						// Just continue on
						break;
					case ERROR:
						exit(EXIT_FAILURE);
				}
		}
	}
}

void Client::SetNonBlockIO(int fd) {
	int val = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0) {
		log->Error("Cannot set nonblocking I/O.");
		exit(EXIT_FAILURE);
	}
}


void Client::SendMessage(struct SEND_STAT* sStat, struct pollfd* server) {
	NbStatus status = sockMsgr->SendMsgNB(sStat, server);

	switch(status) {
		case OKAY:
			// Reset sStat
			sockMsgr->InitSendStat(sStat);
			break;
		case BLOCKED:
			// Just continue on
			break;
		case ERROR:
			exit(EXIT_FAILURE);
	}
}

void Client::SetLog(Log* log) {
    this->log = new Log(log);
}