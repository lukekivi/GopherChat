#ifndef UTILS_HPP
#define UTILS_HPP

#define INT_BYTES 4

typedef unsigned char BYTE;

struct RECV_STAT {
	BYTE* msg;
	bool haveSize;
	int sizeToRecv;		// 4 if unknown yet
	int nRecv;
};

struct SEND_STAT {
	BYTE* msg;
	int msgSize;
	int nSent;
};

struct CONN_STAT {
	struct RECV_STAT recv;
	struct SEND_STAT send;
};

#endif