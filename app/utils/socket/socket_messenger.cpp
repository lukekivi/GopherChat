#include "socket_messenger.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <poll.h>

const int NUM_BYTES = 4;
const int MAX_VALUE = 256;

BYTE* SocketMessenger::IntToByte(int value) {
    BYTE* arr = new BYTE[NUM_BYTES];

    for (int i = 0; i < NUM_BYTES; i++) {
        if (value < MAX_VALUE) {
            arr[i] = (BYTE) value;
            value = 0;
        } else {
            arr[i] = (BYTE) (value % MAX_VALUE);
            value /= MAX_VALUE;
        }
    }
    return arr;
}


int SocketMessenger::ByteToInt(BYTE* arr) {
    int value = 0;
    int multiplier = 1;

    for (int i = 0; i < NUM_BYTES; i++) {
        value += ((int) arr[i]) * multiplier;

        multiplier *= MAX_VALUE;
    }

    return value;
}


BYTE* SocketMessenger::CharToByte(const char* str) {
    return reinterpret_cast<BYTE*>(const_cast<char*>(str));
}


NbStatus SocketMessenger::SendMsgNB(struct SEND_STAT* sStat, struct pollfd* pPeer) {	

	std::cout << "Sending size: " << sStat->size << std::endl;

	while (sStat->nSent < sStat->size) {
		//connStat keeps tracks of how many bytes have been sent, allowing us to "resume" 
		//when a previously non-writable socket becomes writable. 
		int n = send(pPeer->fd, sStat->msg + sStat->nSent, sStat->size - sStat->nSent, 0);
		if (n >= 0) {
			sStat->nSent += n;
		} else if (n < 0 && (errno == ECONNRESET || errno == EPIPE)) {
			log->Info("Connection closed.");
			close(pPeer->fd);
			return ERROR;
		} else if (n < 0 && (errno == EWOULDBLOCK)) {
			//The socket becomes non-writable. Exit now to prevent blocking. 
			//OS will notify us when we can write
			pPeer->events |= POLLWRNORM; 
			return BLOCKED; 
		} else {
			log->Error("Unexpected send error %d: %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	std::cout << "Message sent." << std::endl;
	pPeer->events &= ~POLLWRNORM;
	return OKAY;
}


void SocketMessenger::BuildSendMsg(struct SEND_STAT* sStat, const BYTE* body) {
	int bodySize = strlen((char*) body);

	// msg size as a byte array for sending across socket
	BYTE* arr = IntToByte(bodySize);

	delete sStat->msg;
	sStat->msg = new BYTE[bodySize + INT_BYTES];
	sStat->size = bodySize + INT_BYTES;
	sStat->nSent = 0;

	// prepend size onto body and add it to sStat.msg
	std::copy(body, body + bodySize, std::copy(arr, arr + INT_BYTES, sStat->msg));

	delete arr;
}


void SocketMessenger::InitSendStat(struct SEND_STAT* sStat) {
	delete sStat->msg;
	sStat->msg = nullptr;
	sStat->nSent = 0;
	sStat->size = -1;
}


NbStatus SocketMessenger::RecvMsgNB(struct RECV_STAT* rStat, struct pollfd* pPeer) {
	
	/**
	 * Recieve size of body. An integer represented by 4 BYTEs.
	 */
	while (!rStat->sizeIsSet) { 
		NbStatus status = RecvNB(&rStat->sizeStat, pPeer->fd);

		if (status == OKAY) {
			if (rStat->sizeStat.nRecv == rStat->sizeStat.size) {
				// size read completely
				SetRecvStatWithSize(rStat);
				std::cout << "Finished reading size: " 
					<< rStat->sizeStat.size << std::endl;
			} else if (rStat->sizeStat.nRecv > rStat->sizeStat.size) {
				log->Error("Unexpected error occurred where rStat sizeStat.nRecv is greater than its exepected size");
				exit(EXIT_FAILURE);
			}
		} else {
			return status; // either blocked or error
		}
	}

	while (rStat->sizeIsSet) {
		NbStatus status = RecvNB(&rStat->bodyStat, pPeer->fd);

		if (status == OKAY) {
			if (rStat->bodyStat.nRecv == rStat->bodyStat.size) {
				std::cout << "Finished reading body: " 
					<< rStat->bodyStat.size << std::endl;
				return status;
			} else if (rStat->sizeStat.nRecv > rStat->sizeStat.size) {
				log->Error("Unexpected error occurred where rStat sizeStat.nRecv is greater than its exepected size");
				exit(EXIT_FAILURE);
			}
		} else {
			return status; // either blocked or error
		}
	}

	// Impossible to get here
	return ERROR;
}


void SocketMessenger::InitRecvStat(struct RECV_STAT* rStat) {
	// prep size struct
	rStat->sizeIsSet = false;
	rStat->sizeStat.size = INT_BYTES;
	rStat->sizeStat.nRecv = 0;
	delete rStat->sizeStat.msg;
	rStat->sizeStat.msg = new BYTE[INT_BYTES];

	// prep body struct
	rStat->bodyStat.nRecv = 0;
	delete rStat->bodyStat.msg;
	rStat->bodyStat.msg = nullptr;
	rStat->bodyStat.size = -1;
}


NbStatus SocketMessenger::RecvNB(struct MSG_STAT* mStat, int sockfd) {
	int n = recv(sockfd, mStat->msg + mStat->nRecv, mStat->size - mStat->nRecv, 0);	

	if (n > 0) {
		mStat->nRecv += n;
		return OKAY;
	} else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
		log->Info("Connection closed.");
		close(sockfd);
		return ERROR;
	} else if (n < 0 && (errno == EWOULDBLOCK)) { 
		//The socket becomes non-readable. Exit now to prevent blocking. 
		//OS will notify us when we can read
		return BLOCKED; 
	} else {
		log->Error("Unexpected recv error %d: %s.", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}


void SocketMessenger::SetRecvStatWithSize(struct RECV_STAT* rStat) {
	rStat->sizeIsSet = true;
	rStat->bodyStat.size = ByteToInt(rStat->sizeStat.msg);
	rStat->bodyStat.msg = new BYTE[rStat->bodyStat.size];
}


void SocketMessenger::SetLog(Log* log) {
    this->log = new Log(log);
}