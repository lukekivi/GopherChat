#include "client.hpp"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include <fcntl.h>

Client::Client(Log* log) { 
	SetLog(log); 
	sockMsgr = new SocketMessenger(log);
}

Client::~Client() {
	delete log;
	delete sockMsgr;
}

void Client::StartClient(const char* serverIp, int port, std::vector<CommandData*> commands) {
	log->Info("There are %d commands to execute.", commands.size());
	int commandIndex = 0;

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((unsigned short) port);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);

	std::cout << "Welcome to GopherChat!" << std:: endl;

	while (1) {	

		if (nConns <= MAX_CONNS && commandIndex < commands.size()) { 
			int i = BuildConn(serverAddr);
			activeCmds[i] = commands.at(commandIndex);
			commandIndex++;

			std::cout << "Send command: " << activeCmds[i]->getCommand() << std::endl;
			std::cout << "\t-" << activeCmds[i]->getArgs()[0] << std::endl;

			int len;
			BYTE* body = sockMsgr->CommandDataToByte(activeCmds[i], &len);
			sockMsgr->BuildSendMsg(&sStats[i], body, len);
			SendMessage(i);
		}

		int r = poll(peers, nConns, -1);	
		if (r < 0) {
			log->Error("Invalid poll() return value.");
		}			

		// Read in UI
		// CheckUi();

		for (int i = 0; i < nConns; i++) {
			if (peers[i].revents & (POLLRDNORM | POLLERR | POLLHUP)) {	
				RecvMessage(i);
				continue;
			}
			if (peers[i].revents & POLLWRNORM) {
				SendMessage(i);
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
				exit(EXIT_FAILURE);
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


int Client::BuildConn(struct sockaddr_in serverAddr) {
	if (nConns > MAX_CONNS) {
		log->Error("Too many conns.");
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
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
				std::cout << rStats[i].bodyStat.msg << std::endl;
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
}


void Client::SetLog(Log* log) {
    this->log = new Log(log);
}


void Client::HandleResponse(BYTE* body, int len) {
	char* msg = sockMsgr->ByteToChar(body, len);

	if (strcmp(msg, OK_MSG) == 0) {
		std::cout << "Succesfully registered user." << std::endl;
	} else {
		std::cout <<"User already registered." << std::endl;
	}
}


bool Client::IsUiOrFileConn(int i) {
	return false;
}