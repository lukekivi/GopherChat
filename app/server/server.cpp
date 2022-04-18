// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

#include "server.hpp"
#include <algorithm>
#include <vector>
#include <string>

#define SUMMARY_LEN 100
#define ONBOARDING_MESSAGE "UI connection established"
#define ANON_USERNAME "Anon"
#define SERVER_USERNAME "Server"

Server::Server(Log* log) { 
	this->log = new Log(log);
	this->sockMsgr = new SocketMessenger(log);
	this->ds = new DataStore(REGISTRAR_FILE_PATH);
}


Server::~Server() {
	delete log;
	delete sockMsgr;
	delete ds;
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
	std::cout << "GopherChat server is running." << std::endl;
	
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
			// Just continue on
			break;
		case ERROR:
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
	const char* FAILURE_MSG = "Logged out a user because their file or UI connection failed.";
	close(peers[i].fd);	
	delete[] rStat[i].sizeStat.msg;
	delete[] rStat[i].bodyStat.msg;
	delete[] sStat[i].msg;
	
	if (IsUiConn(i)) {
		log->Info("Removed UI Connection for: %s", connData[i].GetUsername());
	} else if (IsFileConn(i)) {
		log->Info("Removed FIL Connection for: %s", connData[i].GetUsername());
	} else{
		log->Info("Removed REG Connection.");
	}

	if ((IsUiConn(i) || IsFileConn(i)) && ds->IsLoggedIn(connData[i].GetUsername())) {
		ds->Logout(connData[i].GetUsername());
		log->Out("Log Out", connData[i].GetUsername(), NULL, FAILURE_MSG);
		log->Info("%s", FAILURE_MSG);
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
			HandleSendTo(i, commandData);
			break;
		case SEND_TO_ANON:
			HandleSendToAnon(i, commandData);
			break;
		case SEND_FILE:
			HandleSendFile(i, commandData);
			break;
		case SEND_FILE_TO:
			HandleSendFileTo(i, commandData);
			break;
		case LIST:
			HandleList(i, commandData);
			break;
		case DELAY:
			break;
		case UI_CONN:
			SetUiConn(i, commandData);
			break;
		case FILE_CONN:
			SetFileConn(i, commandData);
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

	Status status = ds->Register(username, password);
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

	log->Out("Register", username, NULL, msg);

	SendResponse(i, new ResponseData(status, msg, username));
}


void Server::HandleLogin(int i, CommandData* commandData) {
	const char* username = commandData->getArgs()[0];
	const char* password = commandData->getArgs()[1];
	Status status = ds->Login(username, password);

	char* message;
	const char* msg;

	switch(status) {
		case OK:
			if (GetUiConn(username) != -1) {	// Is logged in but no UI connection (process never made UI connections)
				msg = "Already logged in somewhere else.";
			} else {
				ds->Logout(username);		// relics of old user
				ShedConnections(username);  // need to be removed
				ds->Login(username, password);
				msg = "Succesfully logged in.";
			}
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

	log->Out("Log In", username, NULL, msg);
	log->Info("Log in: %s\n\t- %s", username, msg);

	SendResponse(i, new ResponseData(status, msg, username));
}


void Server::HandleLogout(int i, CommandData* commandData) {
	const char* username = commandData->getUsername();

	Status status = ds->Logout(username);
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

	log->Info("Log out: %s\n\t- %s", username, msg);
	log->Out("Log Out", username, NULL, msg);

	SendResponse(i, new ResponseData(status, msg, username));
}


void Server::SendResponse(int i, ResponseData* responseData) {
	// prepare message
	ByteBody* byteBody = sockMsgr->ResponseDataToByteBody(responseData);
	sockMsgr->InitSendStat(&sStat[i]);
	sockMsgr->BuildSendMsg(&sStat[i], byteBody);

	SendMessage(i);

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

	ByteBody* byteBody = sockMsgr->CharToByteBody(message);
	sockMsgr->InitSendStat(&sStat[index]);
	sockMsgr->BuildSendMsg(&sStat[index], byteBody);

	delete byteBody;

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

	const char* username = commandData->getUsername();

	char* body = new char[strlen(ONBOARDING_MESSAGE) + 1];
	strcpy(body, ONBOARDING_MESSAGE);

	char* fromUsername = new char[strlen(SERVER_USERNAME) + 1];
	strcpy(fromUsername, SERVER_USERNAME);

	MsgData* msgData = new MsgData(UI_MSG, fromUsername, body);

	ByteBody* byteBody = sockMsgr->MsgDataToByteBody(msgData);

	if (byteBody == NULL) {
		delete msgData;
		ExitGracefully();
	}

	ds->Enqueue(username, byteBody);
	delete msgData;
	delete byteBody;
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

	char* message;
	const char* msg;
	Status status;

	if (ds->IsLoggedIn(username)) {
		if (StartMessageToAllUsers(username, commandData, false) == -1) {
			msg = "No users online to receive your message.";
		} else {
			msg = "Successfully sent public message.";
		}
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

	char* visibleUsername = new char[strlen(ANON_USERNAME) + 1];
	strcpy(visibleUsername, ANON_USERNAME); 

	commandData->setUsername(visibleUsername);

	char* message;
	const char* msg;
	Status status;

	if (ds->IsLoggedIn(username)) {
		if (StartMessageToAllUsers(username, commandData, true) == -1) {
			msg = "No users online to receive your message.";
		} else {
			msg = "Successfully sent anonymous public message.";
		}
		status = OK;

	} else {
		msg = "Not logged in. Failed to send anonymous public message.";
		status = FAILURE;
	}

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, username)); // respond to sender
}


/**
 * @param username of the user who issued the command, is deleted elsewhere
 * @param commandData contains the username that will be visible to the client (as well as message data)
 */
int Server::StartMessageToAllUsers(char* username, CommandData* commandData, bool isAnon) {
	char* msg = new char[strlen(commandData->getArgs()[0]) + 1];
	strcpy(msg, commandData->getArgs()[0]);

	char* usr = new char[strlen(commandData->getUsername()) + 1];
	strcpy(usr, commandData->getUsername());

	const char* message;
	if (isAnon) {
		message = "Sending Anonymous Public Message";
	} else {
		message = "Sending Public Message";
	}

	log->Out(message, username, ds->GetSignedInUsers(), commandData->getArgs()[0]);

	MsgData* msgData = new MsgData(UI_MSG, usr, msg);

	ByteBody* byteBody = sockMsgr->MsgDataToByteBody(msgData);
	
	if (byteBody == NULL) {
		delete msgData;
		ExitGracefully();
	}

	int results = ds->EnqueueAllExcept(username, byteBody);

	delete msgData;
	delete byteBody;
	return results;
}


void Server::CheckMessageDeques() {
	for (int i = 0; i <= nConns; i++) {
		if (IsUiConn(i) && !connData[i].IsActive()) {
			ByteBody* byteBody = ds->Dequeue(connData[i].GetUsername());

			if (byteBody != NULL) {
				sockMsgr->InitSendStat(&sStat[i]);
				sockMsgr->BuildSendMsg(&sStat[i], byteBody);
				connData[i].Activate();
				SendMessage(i);
				delete byteBody;
			}
		}
	}
}



void Server::HandleSendTo(int i, CommandData* commandData) {
	char* username = new char[strlen(commandData->getUsername())+1];
	strcpy(username, commandData->getUsername());

	char* message;
	const char* msg;
	Status status = FAILURE;

	if (ds->IsLoggedIn(username)) {
		int results = StartMessageToUser(username, commandData, false);   // distribute message

		if (results == -2) {
			msg = "User is not registered, cannot send anonymous message.";
		} else if (results == -1) {
			msg = "User was not signed in, cannot send anonymous message.";
		} else {
			msg = "Successfully sent anonymous message.";
			status = OK;
		}
		
	} else {
		msg = "Not logged in. Failed to send anonymous public message.";
	}

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, username)); // respond to sender
}



void Server::HandleSendToAnon(int i, CommandData* commandData) {
	char* username = new char[strlen(commandData->getUsername())+1];
	strcpy(username, commandData->getUsername());

	char* visibleUsername = new char[strlen(ANON_USERNAME) + 1];
	strcpy(visibleUsername, ANON_USERNAME); 

	commandData->setUsername(visibleUsername);

	char* message;
	const char* msg;
	Status status = FAILURE;

	if (ds->IsLoggedIn(username)) {
		int results = StartMessageToUser(username, commandData, true);   // distribute message

		if (results == -2) {
			msg = "User is not registered, cannot send anonymous message.";
		} else if (results == -1) {
			msg = "User was not signed in, cannot send anonymous message.";
		} else {
			msg = "Successfully sent anonymous message.";
			status = OK;
		}

	} else {
		msg = "Not logged in. Failed to send anonymous public message.";
		status = FAILURE;
	}

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, username)); // respond to sender
}


int Server::StartMessageToUser(char* username, CommandData* commandData, bool isAnon) {
	char* msg = new char[strlen(commandData->getArgs()[1]) + 1];
	strcpy(msg, commandData->getArgs()[1]);

	char* usr = new char[strlen(commandData->getUsername()) + 1];
	strcpy(usr, commandData->getUsername());

	const char* message;
	if (isAnon) {
		message = "Sending Anonymous Message";
	} else {
		message = "Sending Message";
	}

	log->Out(message, username, commandData->getArgs()[0], commandData->getArgs()[1]);

	MsgData* msgData = new MsgData(UI_MSG, usr, msg);

	ByteBody* byteBody = sockMsgr->MsgDataToByteBody(msgData);

	if (byteBody == NULL) {
		delete msgData;
		ExitGracefully();
	}

	int results = ds->Enqueue(commandData->getArgs()[0], byteBody);

	delete msgData;
	delete byteBody;
	return results;
}


void Server::SetFileConn(int i, CommandData* commandData) {
	connData[i].SetUsername(commandData->getUsername());
	connData[i].SetConnType(FIL);
}
    

int Server::GetFileConn(const char* username) {
	for (int i = 1; i <= nConns; i++) {
		if (strcmp(connData[i].GetUsername(), username) == 0 && connData[i].GetConnType() == FIL && !connData[i].IsActive()) {
			return i;
		}
	}
	return -1;
}


void Server::HandleSendFile(int i, CommandData* commandData) {
	char* message;
	const char* msg;
	Status status;

	if (ds->IsLoggedIn(commandData->getUsername())) {
		if(StartFileToAllUsers(commandData) == -1) {
			msg = "There are no users online to send the file to.";
		} else {
			msg = "Successfully sent public file.";
		}
		status = OK;
	} else {
		msg = "Not logged in. Failed send to public file.";
		status = FAILURE;
	}

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, commandData->getUsername())); // respond to sender
}


void Server::HandleSendFileTo(int i, CommandData* commandData) {
	char* message;
	const char* msg;
	Status status = FAILURE;

	if (ds->IsLoggedIn(commandData->getUsername())) {
		int results = SendFileToUser(commandData);

		if (results == 0) {
			msg = "Successfully sent file.";
			status = OK;
		} else if (results == -1) {
			msg = "Cannot send file, user not fully logged in yet. Missing file connections.";
		} else if (results == -2) {
			msg = "Cannot send file, user not logged in.";
		} else {
			msg = "Cannot send file, user is not registered.";
		}

	} else {
		msg = "Not logged in. Failed to send file.";
	}

	message = new char[strlen(msg) + 1];
	strcpy(message, msg);

	SendResponse(i, new ResponseData(status, message, commandData->getUsername())); // respond to sender
}


int Server::StartFileToAllUsers(CommandData* commandData) {
	std::vector<std::string> recipients = ds->GetSignedInUsers();

	char* sender = commandData->getUsername();
	char* sndr = new char[strlen(sender) + 1];
	strcpy(sndr, sender);

	char** args = commandData->getArgs();

	char* fileName = args[0];
	char* fName = new char[strlen(fileName) + 1];
	strcpy(fName, fileName);

	char* fileContents = args[1];
	char* msg = new char[strlen(fileContents) + 1];
	strcpy(msg, fileContents);

	char contentSummary[SUMMARY_LEN+4];
	int contentLen = strlen(fileContents);
	if (SUMMARY_LEN < contentLen) {
		contentSummary[SUMMARY_LEN]   = '.';
		contentSummary[SUMMARY_LEN+1] = '.';
		contentSummary[SUMMARY_LEN+2] = '.';
		contentSummary[SUMMARY_LEN+3] = '\0';
		for (int i = 0; i < SUMMARY_LEN; i++) {
			contentSummary[i] = fileContents[i];
		}
	} else {
		strcpy(contentSummary, fileContents);
	}

	log->Out("Sending file.", sndr, recipients, fName, contentSummary, contentLen);

	MsgData* msgData = new MsgData(FILE_MSG, sndr, msg, fName);
	ByteBody* byteBody = sockMsgr->MsgDataFileToByteBody(msgData);
	

	if (byteBody == NULL) {
		delete msgData;
		ExitGracefully();
	}

	
	for (std::string recipient : recipients) {
		if (strcmp(recipient.c_str(), sndr) == 0) {
			continue;
		}

		int index = GetFileConn(recipient.c_str());

		if (index == -1) {
			log->Error("Was unable to find an available file connection for: %s", recipient.c_str());
		} else {
			sockMsgr->InitSendStat(&sStat[index]);
			sockMsgr->BuildSendMsg(&sStat[index], byteBody);	
			SendMessage(index);
		}
	}

	delete msgData;
	delete byteBody;

	if (recipients.size() == 1) {
		return -1;
	} else {
		return 0;
	}
}


int Server::SendFileToUser(CommandData* commandData) {
	char* sender = commandData->getUsername();
	char* sndr = new char[strlen(sender) + 1];
	strcpy(sndr, sender);

	char** args = commandData->getArgs();

	char* recipient = args[0];

	char* fileName = args[1];
	char* fName = new char[strlen(fileName) + 1];
	strcpy(fName, fileName);

	char* fileContents = args[2];
	char* msg = new char[strlen(fileContents) + 1];
	strcpy(msg, fileContents);

	char contentSummary[SUMMARY_LEN+4];
	int contentLen = strlen(fileContents);
	if (SUMMARY_LEN < contentLen) {
		contentSummary[SUMMARY_LEN]   = '.';
		contentSummary[SUMMARY_LEN+1] = '.';
		contentSummary[SUMMARY_LEN+2] = '.';
		contentSummary[SUMMARY_LEN+3] = '\0';
		for (int i = 0; i < SUMMARY_LEN; i++) {
			contentSummary[i] = fileContents[i];
		}
	} else {
		strcpy(contentSummary, fileContents);
	}

	log->Out("Sending file.", sndr, recipient, fName, contentSummary, contentLen);

	MsgData* msgData = new MsgData(FILE_MSG, sndr, msg, fName);
	ByteBody* byteBody = sockMsgr->MsgDataFileToByteBody(msgData);

	if (byteBody == NULL) {
		delete msgData;
		ExitGracefully();
	}

	int results;
	if (ds->IsLoggedIn(recipient)) {
		int index = GetFileConn(recipient);
		if (index == -1) {
			log->Error("HandleSendFileTo(): Was unable to find an available file connection for: %s", recipient);
			results = -1; // no file conn
		} else {
			sockMsgr->InitSendStat(&sStat[index]);
			sockMsgr->BuildSendMsg(&sStat[index], byteBody);
			SendMessage(index);
			results = 0;
		}
	} else if (ds->IsUserRegistered(recipient)) {
		results = -2; // not logged in
	} else {
		results = -3; // not logged in or registered
	}	

	delete msgData;
	delete byteBody;
	return results;
}


void Server::ShedConnections(const char* username) {
	for (int i = 0; i < nConns; i++) {
		if (strcmp(username, connData[i].GetUsername()) == 0) {
			RemoveConnection(i);
		}
	}
}


void Server::HandleList(int i, CommandData* commandData) {
	char* username = commandData->getUsername();
	std::vector<std::string> usernames = ds->GetSignedInUsers();
	const char* msg = sockMsgr->ListOfUsersToChar(usernames);

	log->Out("List", username, NULL, msg);
	log->Info("List: %s\n\t- %s", username, msg);

	SendResponse(i, new ResponseData(LISTT, msg, username));

	delete[] msg;
}