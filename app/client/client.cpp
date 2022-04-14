#include "client.hpp"
#include <unistd.h>

Client::Client(Log* log) { 
	SetLog(log); 
	sockMsgr = new SocketMessenger(log);
	unconfirmedLogout = false;
    unconfirmedLogin = false;
	loggedInUser = NULL;
	nConns = 0;
}

Client::~Client() {
	ExitGracefully();
}

void Client::StartClient(const char* serverIp, int port, std::vector<CommandData*> commands_) {
	commands = commands_;
	log->Info("There are %d commands to execute.", commands.size());

	int commandIndex = 0;

	memset(&serverAddr, 0, sizeof(serverAddr));	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((unsigned short) port);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);

	signal(SIGPIPE, SIG_IGN);

	std::cout << "Welcome to GopherChat!" << std:: endl;

	while (1) {	
		if (nConns <= MAX_CONNS && commandIndex < commands.size()) { 
			if (StartCommand(commands.at(commandIndex++)) == 1) {
				continue;
			}
		}

		int r = poll(peers, nConns, -1);	
		if (r < 0) {
			log->Error("Invalid poll() return value.");
		}			

		for (int i = 0; i < nConns; i++) {
			if (peers[i].revents & POLLWRNORM) {
				SendMessage(i);
			}

			if (peers[i].revents & (POLLRDNORM | POLLERR | POLLHUP)) {	
				RecvMessage(i);
			}
		}
	}
}


void Client::SetNonBlockIO(int fd) {
	int val = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0) {
		log->Error("Cannot set nonblocking I/O.");
		ExitGracefully();
	}
}


int Client::BuildConn(ConnType connType) {
	if (nConns > MAX_CONNS) {
		log->Error("Too many conns.");
		ExitGracefully();
	}

	connTypes[nConns] = connType;

	memset(&rStats[nConns], 0, sizeof(RecvStat));
	memset(&sStats[nConns], 0, sizeof(SendStat));
	sockMsgr->InitRecvStat(&rStats[nConns]);
	sockMsgr->InitSendStat(&sStats[nConns]);

	memset(&peers[nConns], 0, sizeof(pollfd));	
	peers[nConns].events = POLLRDNORM;	
	peers[nConns].revents = 0;

	//Create the socket
	peers[nConns].fd = socket(AF_INET, SOCK_STREAM, 0);
	if (peers[nConns].fd == -1) {
		log->Error("Cannot create socket.");		
	}

	//Connect to server
	if (connect(peers[nConns].fd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0) {
		log->Error("Cannot connect to server %s:%d.", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
		ExitGracefully();
	}

	SetNonBlockIO(peers[nConns].fd);

	nConns++;
	return nConns-1;
}


/**
 * Receive a message from a TCP connection. The poll has deemed
 * it readable
 */
void Client::RecvMessage(int i) {
	NbStatus status = sockMsgr->RecvMsgNB(&rStats[i], &peers[i]);

	switch(status) {
		case OKAY:
			// successfully read from socket
			if (IsUiConn(i)) {
				HandleMsg(i);
				sockMsgr->InitRecvStat(&rStats[i]);
			} else if(IsFileConn(i)) {
				HandleFile(i);
				sockMsgr->InitRecvStat(&rStats[i]);
			} else {
				HandleResponse(i);
			}
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
void Client::SendMessage(int i) {
	NbStatus status = sockMsgr->SendMsgNB(&sStats[i], &peers[i]);

	switch(status) {
		case OKAY:
			// Reset sStat
			sockMsgr->RefreshSendStat(&sStats[i]);
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
 * remove a connection and cleanup its data
 */
void Client::RemoveConnection(int i) {
	close(peers[i].fd);	
	delete[] rStats[i].sizeStat.msg;
	delete[] rStats[i].bodyStat.msg;
	delete[] sStats[i].msg;

	if (i < nConns) {	
		memmove(peers + i, peers + i + 1, (nConns-i) * sizeof(struct pollfd));
		memmove(rStats + i, rStats + i + 1, (nConns-i) * sizeof(struct RecvStat));
		memmove(sStats + i, sStats + i + 1, (nConns-i) * sizeof(struct SendStat));
		memmove(connTypes + i, connTypes + i + 1, (nConns-i) * sizeof(ConnType));
	}
	nConns--;
}


void Client::SetLog(Log* log) {
    this->log = new Log(log);
}


void Client::HandleResponse(int i) {
	ResponseData* responseData = sockMsgr->ByteToResponseData(rStats[i].bodyStat.msg);
	Status status = responseData->GetStatus();
	const char* username;

	switch(status) {
		case OK:
			log->Info("Response OK\n\t- user: %s\n\t- msg: ", responseData->getUsername(), responseData->getMsg());
			break;
		case FAILURE:
		 	log->Info("Response FAILURE\n\t- user: %s\n\t- msg: ", responseData->getUsername(), responseData->getMsg());
			break;
		case LOGGED_IN:
			log->Info("Confirmed that %s was logged in", responseData->getUsername());
			unconfirmedLogin = false;	// login is confirmed
			username = responseData->getUsername();
			loggedInUser = new char[strlen(username)+1];
			strcpy(loggedInUser, username);
			SetupSession();
			break;
		case LOGGED_OUT:
			log->Info("Confirmed that %s was logged out", loggedInUser);
			unconfirmedLogout = false; 	// logout is confirmed
			delete[] loggedInUser;
			loggedInUser = NULL;
			break;
		default:
			log->Error("HandleResponse: hit nonexisteant Status.");
			ExitGracefully();
	}

	if (status == LOGGED_OUT) {
		DisconnectFromServer();
	} else {
		RemoveConnection(i);
	}

	PrintResponse(responseData);

	delete responseData;
}


bool Client::IsUiConn(int i) {
	return connTypes[i] == UI;
}

bool Client::IsFileConn(int i) {
	return connTypes[i] == FIL;
}


void Client::PrintToUi(MsgData* msgData) {
	std::cout << msgData->GetUsername() << ": " << msgData->GetMsg() << std::endl;
}


void Client::PrintResponse(ResponseData* responseData) {
	const char* msg = responseData->getMsg();
	const char* username = responseData->getUsername();

	std::cout << username << ": " << msg << std::endl;
}


int Client::StartCommand(CommandData* commandData) {
	char* fileContents;
	switch(commandData->getCommand()) {
		case LOGIN:
			if (StartLogin() == 1) {
				return 1;
			}
			break;
		case LOGOUT:
			if (StartLogout() == 1) {
				return 1;
			}
			commandData->setUsername(loggedInUser);
			break;
		case DELAY:
			Pause(atoi(commandData->getArgs()[0]));
			return 1;
		case REGISTER:
			break;
		case SEND:
		case SEND_ANON:
		case SEND_TO:
		case SEND_TO_ANON:
			if (loggedInUser == NULL) {
				std::cout << "You cannot send a message until you are logged in!" << std::endl;
				return 1;
			}
			commandData->setUsername(loggedInUser);
			break;
		case SEND_FILE:
			if (loggedInUser == NULL) {
				std::cout << "You cannot send a file until you are logged in!" << std::endl;
				return 1;
			}
			fileContents = fileTrans.fileToChar(commandData->getArgs()[0]);
			if (fileContents == NULL) {
				log->Error("Failed to read from file: %s", commandData->getArgs()[0]);
				return 1;
			}
			commandData->setUsername(loggedInUser);
			commandData->setFileContents(fileContents);
			break;
		case SEND_FILE_TO:
			if (loggedInUser == NULL) {
				std::cout << "You cannot send a file until you are logged in!" << std::endl;
				return 1;
			}
			fileContents = fileTrans.fileToChar(commandData->getArgs()[1]);
			if (fileContents == NULL) {
				log->Error("Failed to read from file: %s", commandData->getArgs()[1]);
				return 1;
			}
			commandData->setUsername(loggedInUser);
			commandData->setFileContents(fileContents);

			std::cout << "sender" << commandData->getUsername() << std::endl;
	std::cout << "numArgs" << commandData->getNumArgs() << std::endl;
	std::cout << "recip" << commandData->getArgs()[0] << std::endl;
	std::cout << "fileName" << commandData->getArgs()[1] << std::endl;
		std::cout << "fileContents" << commandData->getArgs()[2] << std::endl;
			break;
		default:
			if (loggedInUser == NULL) {
				std::cout << "You cannot contact GopherChat until you are logged in!" << std::endl;
				return 1;
			}
	}

	int i = BuildConn(REG);
	PrepareMessage(commandData, i);
	SendMessage(i);
	return 0;
}


int Client::StartLogin() {
	if (loggedInUser != NULL) {
		if (unconfirmedLogout) {
			std::cout << "Slow down, please. Still waiting to get a logout confirmation for: " << loggedInUser << std::endl;
		} else {
			std::cout << "You are already logged in as: " << loggedInUser << std::endl;
		}
		return 1;
	} else if (unconfirmedLogin) {
		std::cout << "Slow down, please. Still waiting to get a login confirmation."<< std::endl;
		return 1;
	} 
	log->Info("Begin login attempt");
	unconfirmedLogin = true;  // needs to be confirmed by server
	return 0;
}
    

int Client::StartLogout() {
	if (loggedInUser == NULL) {
		if (unconfirmedLogin) {
			std::cout << "Slow down, please. Still waiting to get a login confirmation" << std::endl;
		} else {
			std::cout << "You cannot logout if you are not yet logged in!" << std::endl;
		}
		return 1;
	}
	log->Info("Begin logout attempt");

	unconfirmedLogout = true; // needs to be confirmed by server
	return 0;
}


void Client::ExitGracefully() {
	for (int i = 0; i < commands.size(); i++) {
		delete commands.at(i);
	}
	commands.clear();
	delete log;
	delete sockMsgr;
	delete loggedInUser;
	exit(EXIT_SUCCESS);
}


void Client::SetupSession() {
	// setup UI connection
	int index = BuildConn(UI);
	CommandData* uiCommandData = BuildSetupCommand(UI_CONN);
	PrepareMessage(uiCommandData, index);
	delete uiCommandData;
	log->Info("Started UI connection.");
	SendMessage(index);

	// setup FILE connections
	CommandData* fileCommandData = BuildSetupCommand(FILE_CONN);
	for (int i = 0; i < FILE_CONNS; i++) {
		index = BuildConn(FIL);
		PrepareMessage(fileCommandData, index);
		log->Info("Started FILE connection.");
		SendMessage(index);
	}
	delete fileCommandData;
}


CommandData* Client::BuildSetupCommand(Command command) {
	CommandData* commandData = new CommandData(command, NULL, 0);
	char* username = new char[strlen(loggedInUser)+1];
	strcpy(username, loggedInUser);
	commandData->setUsername(username);
	return commandData;
}


void Client::PrepareMessage(CommandData* commandData, int i) {
	int len;
	BYTE* body = sockMsgr->CommandDataToByte(commandData, &len);
	ByteBody byteBody(body, len);
	sockMsgr->BuildSendMsg(&sStats[i], &byteBody);
}


void Client::DisconnectFromServer() {
	for (int i = 0; i < nConns; i++) {
		if (IsFileConn(i)) {
			RemoveConnection(i);
			log->Info("Disconnected a File Connection");
		} else if (IsUiConn(i)) {
			RemoveConnection(i);
			log->Info("Disconnected UI Connection");
		}
	}

	log->Info("Disconnected from server");
}


void Client::Pause(int secs) {
	log->Info("Paused for %d", secs);
	std::cout << "Paused for " << secs << "secs" << std::endl;
	usleep(secs * 1000000);
}


void Client::HandleMsg(int i) {
	MsgData* msgData = sockMsgr->ByteToMsgData(rStats[i].bodyStat.msg);

	MsgType msgType = msgData->GetMsgType();
	if (msgType == UI_MSG) {
		PrintToUi(msgData);
	} else {
		log->Error("MsgType did not match UI_MSG(51), %d", msgType);
		delete msgData;
		ExitGracefully();
	}

	delete msgData;
}


void Client::HandleFile(int i) {
	MsgData* msgData = sockMsgr->ByteToMsgDataFile(rStats[i].bodyStat.msg);

	MsgType msgType = msgData->GetMsgType();

	if (msgType == FILE_MSG) {
		if (fileTrans.charToFile(msgData->GetFileName(), msgData->GetMsg())) {
			std::cout << msgData->GetUsername() << ": sent you the file \"" << msgData->GetFileName() << "\"" << std::endl;
		} else {
			std::cout << msgData->GetUsername() << ":sent you the file \"" << msgData->GetFileName() << "\". However, a file by the same name already existed so it failed to download." << std::endl;
		}
	} else {
		log->Error("MsgType did not match FILE_MSG(50), %d", msgType);
		delete msgData;
		ExitGracefully();
	}

	delete msgData;
}