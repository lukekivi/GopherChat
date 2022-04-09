#include "server.hpp"
#include <algorithm>

Server::Server(Log* log) { 
	this->log = new Log(log);
	this->sockMsgr = new SocketMessenger(log);
}


Server::~Server() {
	delete log;
	delete sockMsgr;
}


/**
 * Start the GopherChat server. Connect clients and route messages.
 */
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
		exit(EXIT_FAILURE);
	}
	signal(SIGPIPE, SIG_IGN);

	if (bind(listenFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
		log->Error("Cannot bind to port %d.", port);
		exit(EXIT_FAILURE);
	}
	
	if (listen(listenFD, 16) != 0) {
		log->Error("Cannot listen to port %d.", port);
		exit(EXIT_FAILURE);
	}

	log->Info("Listening to port %d.", port);
	
	nConns = 0;	
	memset(peers, 0, sizeof(peers));	
	peers[0].fd = listenFD;
	peers[0].events = POLLRDNORM;	
	memset(rStat, 0, sizeof(RecvStat));
	memset(sStat, 0, sizeof(SendStat));
	
	int connID = 0;
	while (1) {	//the main loop		
		//monitor the listening sock and data socks, nConn+1 in total
		r = poll(peers, nConns + 1, -1);	
		if (r < 0) {
			log->Error("Invalid poll() return value.");
			exit(EXIT_FAILURE);
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

				memset(&sStat[nConns], 0, sizeof(struct SendStat));
				memset(&rStat[nConns], 0, sizeof(struct RecvStat));
				sockMsgr->InitRecvStat(&rStat[nConns]);
			}
		}
		
		for (int i=1; i<=nConns; i++) {
			if (peers[i].revents & (POLLRDNORM | POLLERR | POLLHUP)) {
				RecvMessage(i);
			}
			
			// a previously blocked data socket becomes writable
			if (peers[i].revents & POLLWRNORM) {
				SendMessage(i);
			}
		}
	}	
}


/**
 * Receive a message from a TCP connection. The poll has deemed
 * it readable
 */
void Server::RecvMessage(int i) {
	
	NbStatus status = sockMsgr->RecvMsgNB(&rStat[i], &peers[i]);
	CommandData* commandData = NULL;

	switch(status) {
		case OKAY:
			// successfully read from socket
			std::cout << "RecvMessage: OK" << std::endl;
			commandData = sockMsgr->ByteToCommandData(rStat[i].bodyStat.msg);
			sockMsgr->InitRecvStat(&rStat[i]);
			HandleReceivedCommand(i, commandData);
			delete commandData;
			break;
		case BLOCKED:
			std::cout << "RecvMessage: BLOCKED" << std::endl;
			// Just continue on
			break;
		case ERROR:
			std::cout << "RecvMessage: ERROR - index " << i << std::endl;
			// Remove connection
			RemoveConnection(i);
			break;
	}
}


/**
 * Send a non-blocking message to a client.
 */
void Server::SendMessage(int i) {
	NbStatus status = sockMsgr->SendMsgNB(&sStat[i], &peers[i]);

	switch(status) {
		case OKAY:
			// Reset sStat
			if (!IsUiOrFileConn(i)) {
				RemoveConnection(i);
			} else {
				sockMsgr->RefreshSendStat(&sStat[i]);
			}
			break;
		case BLOCKED:
			// Just continue on
			break;
		case ERROR:
			RemoveConnection(i);
			break;
	}
}


/**
 * Set socket to be non-blocking
 */
void Server::SetNonBlockIO(int fd) {
	int val = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0) {
		log->Error("Cannot set nonblocking I/O.");
		exit(EXIT_FAILURE);
	}
}


/**
 * remove a connection and cleanup its data
 */
void Server::RemoveConnection(int i) {
	close(peers[i].fd);	
	delete[] rStat[i].sizeStat.msg;
	delete[] rStat[i].bodyStat.msg;
	delete[] sStat[i].msg;

	if (i < nConns) {	
		memmove(peers + i, peers + i + 1, (nConns-i) * sizeof(struct pollfd));
		memmove(rStat + i, rStat + i + 1, (nConns-i) * sizeof(struct RecvStat));
		memmove(sStat + i, sStat + i + 1, (nConns-i) * sizeof(struct SendStat));
	}
	nConns--;
}


void Server::HandleReceivedCommand(int i, CommandData* commandData) {
	std::cout << "Handle RCV: " << commandData->getCommand() << std::endl;
	for (int i = 0; i < commandData->getNumArgs(); i++) {
		std::cout << "\t-" << commandData->getArgs()[i] << std::endl;
	}

	switch (commandData->getCommand()) {
		case REGISTER:
			HandleRegister(i, commandData);
			break;
		case LOGIN:
			HandleLogin(i, commandData);
			break;
		case LOGOUT:
			HandleLogout(i, commandData);
			break;
		case SEND:
			break;
		case SEND_TO:
			break;
		case SEND_ANON:
			break;
		case SEND_TO_ANON:
			break;
		case SEND_FILE:
			break;
		case SEND_FILE_TO:
			break;
		case LIST:
			break;
		case DELAY:
			break;
		default:
			log->Error("Invalid COMMAND.");
			exit(EXIT_FAILURE);
	}
}


void Server::HandleRegister(int i, CommandData* commandData) {
	const char* username = commandData->getArgs()[0];
	const char* password = commandData->getArgs()[1];

	Status status = ds.Register(username, password);
	char* message;
	const char* msg;

	switch(status) {
		case OK:
			msg = "Registration succeeded.";
			message = new char[strlen(msg) + 1];
			strcpy(message, msg);
			break;
		case FAILURE:
			msg = "Registration failed.";
			message = new char[strlen(msg) + 1];
			strcpy(message, msg);
			break;
		default:
			log->Error("Invalid STATUS.");
			exit(EXIT_FAILURE);
			break;
	}	

	SendResponse(i, new ResponseData(status, message, username));
}


void Server::HandleLogin(int i, CommandData* commandData) {
	const char* username = commandData->getArgs()[0];
	const char* password = commandData->getArgs()[1];

	Status status = ds.Login(username, password);
	char* message;
	const char* msg;

	switch(status) {
		case OK:
			msg = "Already logged in somewhere else.";
			message = new char[strlen(msg) + 1];
			strcpy(message, msg);
			break;
		case FAILURE:
			msg = "This user doesn't exist--cannot log in.";
			message = new char[strlen(msg) + 1];
			strcpy(message, msg);
			break;
		case LOGGED_IN:
			msg = "Successfully logged in.";
			message = new char[strlen(msg) + 1];
			strcpy(message, msg);
			break;
		default:
			log->Error("Invalid STATUS.");
			exit(EXIT_FAILURE);
			break;
	}	

	SendResponse(i, new ResponseData(status, message, username));
}


void Server::HandleLogout(int i, CommandData* commandData) {
	const char* username = commandData->getArgs()[0];

	Status status = ds.Logout(username);
	char* message;
	const char* msg;

	switch(status) {
		case OK:
			// should never happen
			msg = "This user is not logged in. This shouldn't be possible.";
			message = new char[strlen(msg) + 1];
			strcpy(message, msg);
			break;
		case LOGGED_OUT:
			msg = "Successfully logged out.";
			message = new char[strlen(msg) + 1];
			strcpy(message, msg);
			break;
		default:
			log->Error("Invalid STATUS.");
			exit(EXIT_FAILURE);
			break;
	}	

	SendResponse(i, new ResponseData(status, message, username));
}


void Server::SendResponse(int i, ResponseData* responseData) {
	// prepare message
	int len;
	BYTE* body = sockMsgr->ResponseDataToByte(responseData, &len);
	sockMsgr->InitSendStat(&sStat[i]);
	sockMsgr->BuildSendMsg(&sStat[i], body, len);

	SendMessage(i);

	log->Info("Sent an %d!", responseData->GetStatus());
	delete responseData;
}


bool Server::IsUiOrFileConn(int i) {
	// check if conn[i] is a UI/File conn
	return false;
}