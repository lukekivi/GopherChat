#include "server.hpp"
#include <algorithm>
#include <vector>
#include <string>

const char* ONBOARDING_MESSAGE = "UI connection established";

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

		CheckMessageDeques();

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
				SetConn(fd);
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
			commandData = sockMsgr->ByteToCommandData(rStat[i].bodyStat.msg); // convert
			sockMsgr->InitRecvStat(&rStat[i]);		// reset receiving data structure
			HandleReceivedCommand(i, commandData); 	// handle command
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
			if (IsFileConn(i) || IsUiConn(i)) {
				sockMsgr->RefreshSendStat(&sStat[i]); // UI/FILE conn needs to be refreshed
				connData[i].Deactivate();
			} else {
				RemoveConnection(i);				  // Remove transient conn

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

	if ((IsUiConn(i) || IsFileConn(i)) && ds.IsLoggedIn(connData[i].GetUsername())) {
		ds.Logout(connData[i].GetUsername());
		log->Info("Logged out a user because their file or UI connection failed.");
		std::cout << "Logged out a user because their file or UI connection failed." << std::endl;
	} else {
		std::cout << "RemoveConnection: Didn't log anybody out" << std::endl;
	}
	
	if (IsUiConn(i)) {
		std::cout << "Removed UI Connection for: " << connData[i].GetUsername() << std::endl;
	} else if (IsFileConn(i)) {
		std::cout << "Removed FIL Connection for: " << connData[i].GetUsername() << std::endl;
	} else{
		std::cout << "Removed REG Connection" << std::endl;
	}

	if (i < nConns) {	
		memmove(peers + i, peers + i + 1, (nConns-i) * sizeof(struct pollfd));
		memmove(rStat + i, rStat + i + 1, (nConns-i) * sizeof(struct RecvStat));
		memmove(sStat + i, sStat + i + 1, (nConns-i) * sizeof(struct SendStat));
		memmove(connData + i, connData + i + 1, (nConns-i) * sizeof(ConnData));
	}
	nConns--;
}


void Server::HandleReceivedCommand(int i, CommandData* commandData) {
	// std::cout << "Server.HandleReceivedCommand(): " << commandData->getCommand() << std::endl;
	// for (int i = 0; i < commandData->getNumArgs(); i++) {
	// 	std::cout << "\t-" << commandData->getArgs()[i] << std::endl;
	// }

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
			HandleSend(i, commandData);
			break;
		case SEND_ANON:
			HandleSendAnon(i, commandData);
			break;
		case SEND_TO:
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
		case UI_CONN:
			SetUiConn(i, commandData);
			// SendMessageToUi(commandData->getUsername(), "here is a ui message");
			break;
		case FILE_CONN:
			break;
		default:
			log->Error("Invalid COMMAND: %d", commandData->getCommand());
			delete commandData;
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
			break;
		case FAILURE:
			msg = "Registration failed.";
			break;
		default:
			log->Error("Invalid STATUS.");
			exit(EXIT_FAILURE);
			break;
	}	

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, username));
}


void Server::HandleLogin(int i, CommandData* commandData) {
	const char* username = commandData->getArgs()[0];
	const char* password = commandData->getArgs()[1];
	Status status = ds.Login(username, password);
	log->Info("Logging in: %s", username);
	std::cout << "Logging in: " << username << std::endl;
	char* message;
	const char* msg;

	switch(status) {
		case OK:
			msg = "Already logged in somewhere else.";
			break;
		case FAILURE:
			msg = "This user doesn't exist--cannot log in.";
			break;
		case LOGGED_IN:
			msg = "Successfully logged in.";
			break;
		case LOGGED_OUT:
			msg = "Incorrect password.";
			break;
		default:
			log->Error("Invalid STATUS.");
			exit(EXIT_FAILURE);
	}	

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	log->Info("Log in: %s\n\t- %s", username, message);

	SendResponse(i, new ResponseData(status, message, username));
}


void Server::HandleLogout(int i, CommandData* commandData) {
	const char* username = commandData->getUsername();
	log->Info("Logging out: %s", username);
	std::cout << "Logging out: " << username << std::endl;

	Status status = ds.Logout(username);
	char* message;
	const char* msg;

	switch(status) {
		case OK:
			// should never happen
			msg = "This user is not logged in. This shouldn't be possible.";
			break;
		case LOGGED_OUT:
			msg = "Successfully logged out.";
			break;
		default:
			log->Error("Invalid STATUS.");
			exit(EXIT_FAILURE);
	}	

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	log->Info("Log out: %s\n\t- %s", username, message);

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


bool Server::IsUiConn(int i) {
	return connData[i].GetConnType() == UI;
}

bool Server::IsFileConn(int i) {
	return connData[i].GetConnType() == FIL;
}


void Server::SendMessageToUi(const char* username, const char* message) {
	int index = GetUiConn(username);

	if (index == -1) {
		log->Error("Didn't find a match for \"%s\" in logged in users", username);
		ExitGracefully();
	}

	int len = strlen(message);
	BYTE* body = sockMsgr->CharToByte(message);
	sockMsgr->InitSendStat(&sStat[index]);
	sockMsgr->BuildSendMsg(&sStat[index], body, len);

	SendMessage(index);
}


void Server::ExitGracefully() {
	delete log;
	delete sockMsgr;

	exit(EXIT_SUCCESS);
}


void Server::SetConn(int fd) {
	SetNonBlockIO(fd);
	nConns++;
	peers[nConns].fd = fd;
	peers[nConns].events = POLLRDNORM;
	peers[nConns].revents = 0;

	memset(&sStat[nConns], 0, sizeof(struct SendStat));
	memset(&rStat[nConns], 0, sizeof(struct RecvStat));
	connData[nConns].SetConnType(REG);
	sockMsgr->InitRecvStat(&rStat[nConns]);
}


void Server::SetUiConn(int i, CommandData* commandData) {
	connData[i].SetUsername(commandData->getUsername());
	connData[i].SetConnType(UI);

	const BYTE* body = sockMsgr->CharToByte(ONBOARDING_MESSAGE);
	const char* username = commandData->getUsername();

	ds.Enqueue(username, body, strlen(ONBOARDING_MESSAGE));
	delete[] body;
}

int Server::GetUiConn(const char* username) {
	for (int i = 1; i <= nConns; i++) {
		if (strcmp(connData[i].GetUsername(), username) == 0) {
			return i;
		}
	}
	return -1;
}


void Server::HandleSend(int i, CommandData* commandData) {
	char* username = new char[strlen(commandData->getUsername())+1];
	strcpy(username, commandData->getUsername());

	log->Info("%s: sending public message.", username);

	char* message;
	const char* msg;
	Status status;

	if (ds.IsLoggedIn(username)) {
		StartMessageToAllUsers(username, commandData);  // distribute message
		msg = "Successfully sent public message.";
		status = OK;
	} else {
		msg = "Not logged in. Failed to send public message.";
		status = FAILURE;
	}

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, username)); // respond to sender
}


void Server::HandleSendAnon(int i, CommandData* commandData) {
	char* username = new char[strlen(commandData->getUsername())+1];
	strcpy(username, commandData->getUsername());
	commandData->setUsername(NULL);
	log->Info("%s: sending anonymous public message.", username);

	char* message;
	const char* msg;
	Status status;

	if (ds.IsLoggedIn(username)) {
		StartMessageToAllUsers(username, commandData);   // distribute message
		msg = "Successfully sent anonymous public message.";
		status = OK;
	} else {
		msg = "Not logged in. Failed to send anonymous public message.";
		status = FAILURE;
	}

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, username)); // respond to sender
}


void Server::StartMessageToAllUsers(char* username, CommandData* commandData) {
	int len = 0;
	const BYTE* body = sockMsgr->CommandDataToByte(commandData, &len);

	const char * message = sockMsgr->ByteToChar(body, len);
	log->Info("Starting message for all from %s", username);
	log->Info("Body: %s", message);
	log->Info("Body size: %d", len);

	ds.EnqueueAllExcept(username, body, len);

	delete[] body;
}


void Server::CheckMessageDeques() {
	for (int i = 0; i <= nConns; i++) {
		if (IsUiConn(i) && !connData[i].IsActive()) {
			ByteBody* body = ds.Dequeue(connData[i].GetUsername());

			if (body != NULL) {
				const char * message = sockMsgr->ByteToChar(body->GetBody(), body->GetLen());

				log->Info("Dequeueing and sending message for %s", connData[i].GetUsername());
				log->Info("Body: %s", message);
				log->Info("Body size: %d", body->GetLen());

				delete[] message;

				sockMsgr->InitSendStat(&sStat[i]);
				sockMsgr->BuildSendMsg(&sStat[i], body->GetBody(), body->GetLen());
				connData[i].Activate();
				SendMessage(i);
				delete body;
			}
		}
	}
}