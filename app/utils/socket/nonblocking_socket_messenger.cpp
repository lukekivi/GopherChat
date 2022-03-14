#include "nonblocking_socket_messenger.hpp"
#include <sys/socket.h>
#include <unistd.h>

int NonblockingSocketMessenger::SendNonBlocking(struct CONN_STAT* pStat, struct pollfd* pPeer) {	
    int len = pStat->send.msgSize;

	while (pStat->send.nSent < len) {
		//pStat keeps tracks of how many bytes have been sent, allowing us to "resume" 
		//when a previously non-writable socket becomes writable. 
		int n = send(pPeer->fd, pStat->send.msg + pStat->send.nSent, len - pStat->send.nSent, 0);
		if (n >= 0) {
			pStat->send.nSent += n;
		} else if (n < 0 && (errno == ECONNRESET || errno == EPIPE)) {
			log->Info("Connection closed.");
			close(pPeer->fd);
			return -1;
		} else if (n < 0 && (errno == EWOULDBLOCK)) {
			//The socket becomes non-writable. Exit now to prevent blocking. 
			//OS will notify us when we can write
			pPeer->events |= POLLWRNORM; 
			return 0; 
		} else {
			log->Error("Unexpected send error %d: %s", errno, strerror(errno));
		}
	}
	pPeer->events &= ~POLLWRNORM;
	return 0;
}

int NonblockingSocketMessenger::RecvNonBlocking(struct CONN_STAT* pStat, struct pollfd* pPeer) {
	int len;

	if (!pStat->recv.haveSize) {
		// still need to finish reading the message size
		len = INT_BYTES;
		while (pStat->recv.nRecv < len) {
			int n = recv(pPeer->fd, pStat->recv.msg + pStat->recv.nRecv, len - pStat->recv.nRecv, 0);
			if (n > 0) {
				pStat->recv.nRecv += n;
			} else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
				log->Info("Connection closed.");
				close(pPeer->fd);
				return -1;
			} else if (n < 0 && (errno == EWOULDBLOCK)) { 
				//The socket becomes non-readable. Exit now to prevent blocking. 
				//OS will notify us when we can read
				return 0; 
			} else {
				log->Error("Unexpected recv error %d: %s.", errno, strerror(errno));
			}

			if (pStat->recv.nRecv == len) {
				pStat->recv.haveSize = true;
				pStat->recv.sizeToRecv = ByteToInt(pStat->recv.msg);
				pStat->recv.nRecv = 0;
				delete pStat->recv.msg;
				pStat->recv.msg = new BYTE[pStat->recv.sizeToRecv];
			}
		}
	} else {
		len = pStat->recv.sizeToRecv;

		while (pStat->recv.nRecv < len) {
			int n = recv(pPeer->fd, pStat->recv.msg + pStat->recv.nRecv, len - pStat->recv.nRecv, 0);
			if (n > 0) {
				pStat->recv.nRecv += n;
			} else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
				log->Info("Connection closed.");
				close(pPeer->fd);
				return -1;
			} else if (n < 0 && (errno == EWOULDBLOCK)) { 
				//The socket becomes non-readable. Exit now to prevent blocking. 
				//OS will notify us when we can read
				return 0; 
			} else {
				log->Error("Unexpected recv error %d: %s.", errno, strerror(errno));
			}
		}
	}
	
	return 0;
}

void NonblockingSocketMessenger::SetLog(Log* log) {
    this->log = new Log(log);
}