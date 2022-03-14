#include "server.hpp"
#include <algorithm>

const char* GREETING = "Welcome to GopherChat!";

Server::Server(Log* log) { 
	this->log = new Log(log);
	this->sockMsgr = new NonblockingSocketMessenger(log);
}

Server::~Server() {
	delete log;
	delete sockMsgr;
}

void Server::SendGreeting(int i) {
	int msgSize = strlen(GREETING);

	BuildSendMsg(i, sockMsgr->CharToByte(GREETING), msgSize);

    if (connStat[i].send.msgSize != 0) {
		if (sockMsgr->SendNonBlocking(&connStat[i], &peers[i]) < 0) {
	        RemoveConnection(i);
		}
	}
}

void Server::SetNonBlockIO(int fd) {
	int val = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0) {
		log->Error("Cannot set nonblocking I/O.");
	}
}

void Server::RemoveConnection(int i) {
	close(peers[i].fd);	
	if (i < nConns) {	
		memmove(peers + i, peers + i + 1, (nConns-i) * sizeof(struct pollfd));
		memmove(connStat + i, connStat + i + 1, (nConns-i) * sizeof(struct CONN_STAT));
	}
	nConns--;
}

void Server::BuildSendMsg(int i, const BYTE* msg, int msgSize) {
	delete connStat[i].send.msg;
	connStat[i].send.msg = new BYTE[msgSize + INT_BYTES];
	connStat[i].send.msgSize = msgSize + INT_BYTES;

	BYTE* arr = sockMsgr->IntToByte(msgSize);

	std::copy(msg, msg + msgSize, std::copy(arr, arr + INT_BYTES, connStat[i].send.msg));


	delete arr;
}

void Server::StartServer(int port) {
	
	int listenFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFD < 0) {
		log->Error("Cannot create listening socket.");
	}
	SetNonBlockIO(listenFD);
	
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((unsigned short) port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int optval = 1;
	int r = setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (r != 0) {
		log->Error("Cannot enable SO_REUSEADDR option.");
	}
	signal(SIGPIPE, SIG_IGN);

	if (bind(listenFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
		log->Error("Cannot bind to port %d.", port);
	}
	
	if (listen(listenFD, 16) != 0) {
		log->Error("Cannot listen to port %d.", port);
	}
	
	nConns = 0;	
	memset(peers, 0, sizeof(peers));	
	peers[0].fd = listenFD;
	peers[0].events = POLLRDNORM;	
	memset(connStat, 0, sizeof(connStat));
	
	int connID = 0;
	while (1) {	//the main loop		
		//monitor the listening sock and data socks, nConn+1 in total
		r = poll(peers, nConns + 1, -1);	
		if (r < 0) {
			log->Error("Invalid poll() return value.");
		}			
			
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);	
		
		//new incoming connection
		if ((peers[0].revents & POLLRDNORM) && (nConns < MAX_CONCURRENCY_LIMIT)) {					
			int fd = accept(listenFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
			if (fd != -1) {
				SetNonBlockIO(fd);
				nConns++;
				peers[nConns].fd = fd;
				peers[nConns].events = POLLRDNORM;
				peers[nConns].revents = 0;
				
				memset(&connStat[nConns], 0, sizeof(struct CONN_STAT));
				connStat[nConns].recv.msg = new BYTE[4];
				connStat[nConns].recv.haveSize = false;
				connStat[nConns].recv.nRecv = 0;

				SendGreeting(nConns);
			}
		}
		
		for (int i=1; i<=nConns; i++) {
			if (peers[i].revents & (POLLRDNORM | POLLERR | POLLHUP)) {
				int fd = peers[i].fd;
				
				// recv request
			// 	if (connStat[i].recv.nRecv < 4) {
											
			// 		if (sockMsgr->RecvNonBlocking(i, buf, 4) < 0) {
			// 			RemoveConnection(i);
			// 			continue;
			// 		}
					
			// 		if (connStat[i].nRecv == 4) {
			// 			connStat[i].size = ntohl(connStat[i].size);
			// 			int size = connStat[i].size;
			// 			if (size <= 0 || size > MAX_REQUEST_SIZE) {
			// 				log->Error("Invalid size: %d.", size);
			// 			} 
			// 			log->Info("Transaction %d: %d bytes", ++connID, size);
			// 		}
			// 	}
			}
			
			// a previously blocked data socket becomes writable
			if (peers[i].revents & POLLWRNORM) {
				if (sockMsgr->SendNonBlocking(&connStat[i], &peers[i]) < 0 || connStat[i].send.nSent == connStat[i].send.msgSize) {
					RemoveConnection(i);
					continue;
				}
			}
		}
	}	
}