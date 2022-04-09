#include "client.hpp"

Client::Client(Log* log) { 
	SetLog(log); 
	sockMsgr = new SocketMessenger(log);
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

	std::cout << "Welcome to GopherChat!" << std:: endl;

	while (1) {	

		if (nConns <= MAX_CONNS && commandIndex < commands.size()) { 
			StartCommand(commands.at(commandIndex));
			commandIndex++;
		}

		int r = poll(peers, nConns, -1);	
		if (r < 0) {
			log->Error("Invalid poll() return value.");
		}			

		// Read in UI
		// CheckUi();

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


void Client::CheckUi() {
	if (peers[0].revents & (POLLRDNORM | POLLERR | POLLHUP)) {	
		// recv request
		NbStatus status = sockMsgr->RecvMsgNB(&rStats[0], &peers[0]);

		switch(status) {
			case OKAY:
				// successfully read from socket
				std::cout << "UI: " << rStats[0].bodyStat.msg << std::endl;
				sockMsgr->InitRecvStat(&rStats[0]);
				break;
			case BLOCKED:
				// Just continue on
				break;
			case ERROR:
			ExitGracefully();
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


int Client::BuildConn() {
	if (nConns > MAX_CONNS) {
		log->Error("Too many conns.");
		ExitGracefully();
	}

	memset(&rStats[nConns], 0, sizeof(RecvStat));
	memset(&sStats[nConns], 0, sizeof(SendStat));
	sockMsgr->InitRecvStat(&rStats[nConns]);
	sockMsgr->InitSendStat(&sStats[nConns]);

	signal(SIGPIPE, SIG_IGN);

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

	log->Info("Connected to server %s:%d.", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

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
			if (!IsUiOrFileConn(i)) {
				HandleResponse(rStats[i].bodyStat.msg, rStats[i].bodyStat.size);
				RemoveConnection(i);
			} else {
				PrintToUi(i);
				sockMsgr->InitRecvStat(&rStats[i]);
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
	}
	nConns--;

	if (uiConn == i) {
		uiConn = -1;
	} else {
		for (int j = 0; j < fileConns.size(); j++) {
			if (fileConns.at(j) == i) {
				fileConns.erase(fileConns.begin() + j);
			}
		}
	}
}


void Client::SetLog(Log* log) {
    this->log = new Log(log);
}


void Client::HandleResponse(BYTE* body, int len) {
	ResponseData* responseData = sockMsgr->ByteToResponseData(body);
	Status status = responseData->GetStatus();
	const char* username;

	switch(status) {
		case OK:
			break;
		case FAILURE:
			break;
		case LOGGED_IN:
			username = responseData->getUsername();
			loggedInUser = new char[strlen(username)+1];
			strcpy(loggedInUser, username);
			SetupSession();
			break;
		case LOGGED_OUT:
			delete[] loggedInUser;
			loggedInUser = NULL;
			break;
		default:
			log->Error("HandleResponse: hit nonexisteant Status.");
			ExitGracefully();
	}
	PrintResponse(responseData);
	delete responseData;
}


bool Client::IsUiOrFileConn(int i) {
	return false;
}


void Client::PrintToUi(int i) {
	int size = rStats[i].bodyStat.size;
	char body[size + 1];
	body[size] = '\0';

	for (int i = 0; i < size; i++) {
		body[i] = (char) rStats[i].bodyStat.msg[i];
	}

	std::cout << body << std::endl;
}


void Client::PrintResponse(ResponseData* responseData) {
	const char* msg = responseData->getMsg();
	const char* username = responseData->getUsername();

	std::cout << username << ": " << msg << std::endl;
}


void Client::StartCommand(CommandData* commandData) {
	switch(commandData->getCommand()) {
		case LOGIN:
			if (loggedInUser != NULL) {
				std::cout << "You are already logged in as: " << loggedInUser << std::endl;
				return;
			}
			break;
		case LOGOUT:
			if (loggedInUser == NULL) {
				std::cout << "You cannot logout if you are not yet logged in!" << std::endl;
				return;
			}
			commandData->setUsername(loggedInUser);
			break;
		case DELAY:
		case REGISTER:
			break;
		default:
			if (loggedInUser == NULL) {
				std::cout << "You cannot contact GopherChat until you are logged in!" << std::endl;
				return;
			}
	}

	int i = BuildConn();
	PrepareMessage(commandData, i);
	SendMessage(i);
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
	int i = BuildConn();
	uiConn = i;
	CommandData* uiCommandData = BuildUiCommand();
	PrepareMessage(uiCommandData, i);

	delete uiCommandData;  // generalize deletion after BYTE CONVERSION

	SendMessage(i);
}


CommandData* Client::BuildUiCommand() {
	CommandData* commandData = new CommandData(UI_CONN, NULL, 0);
	char* username = new char[strlen(loggedInUser)+1];
	strcpy(username, loggedInUser);
	commandData->setUsername(username);
	return commandData;
}


void Client::PrepareMessage(CommandData* commandData, int i) {
	int len;
	BYTE* body = sockMsgr->CommandDataToByte(commandData, &len);
	sockMsgr->BuildSendMsg(&sStats[i], body, len);
}