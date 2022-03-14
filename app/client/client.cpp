#include "client.hpp"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include<thread>
#include <poll.h>

#define MAX_REQUEST_SIZE 10000000

Client::Client(Log* log) { 
	SetLog(log); 
	server.fd = -1;
}

Client::~Client() {
	if (server.fd != -1) {
		close(server.fd); 
	}

	delete log;
}

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
	server.fd = socket(AF_INET, SOCK_STREAM, 0);
		
    if (server.fd == -1) {
		log->Error("Cannot create socket.");		
		exit(EXIT_FAILURE);
	}
		
	//Connect to server
	if (connect(server.fd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0) {
		log->Error("Cannot connect to server %s:%d.", serverIp, port);
		exit(EXIT_FAILURE);
    } else {
        log->Info("Client: connected to %s:%d", serverIp, port);
		std::thread listeningThread(ListenToServer);
		listeningThread.join();
    }
}

void Client::ListenToServer() {
	while (1) {
		if (server.revents & (POLLRDNORM | POLLERR | POLLHUP)) {
				
			//recv request
			if (connStat[i].nRecv < 4) {
											
				if (RecvNonBlocking(i, buf, 4) < 0) {
					RemoveConnection(i);
					continue;
				}
					
				if (connStat[i].nRecv == 4) {
					connStat[i].size = ntohl(connStat[i].size);
					int size = connStat[i].size;
					if (size <= 0 || size > MAX_REQUEST_SIZE) {
						log->Error("Invalid size: %d.", size);
					} 
					log->Info("Transaction %d: %d bytes", ++connID, size);
				}
			}		
			
			// a previously blocked data socket becomes writable
			if (peers[i].revents & POLLWRNORM) {
				int size = connStat[i].size;
				if (SendNonBlocking(peers[i].fd, buf, size, &connStat[i], &peers[i]) < 0 || connStat[i].nSent == size) {
					RemoveConnection(i);
					continue;
				}
			}
		}
	}
}


int Client::RecvBlocking() {
	int nRecv = 0;
	while (nRecv < MAX_REQUEST_SIZE) {
		int n = recv(sockFD, data + nRecv, len - nRecv, 0);
		if (n > 0) {
			nRecv += n;
		} else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
			Log("Connection closed.");
			close(sockFD);
			return -1;
		} else {
			Error("Unexpected error %d: %s.", errno, strerror(errno));
		}
	}
	return 0;
}

void Client::SetLog(Log* log) {
    this->log = new Log(log);
}