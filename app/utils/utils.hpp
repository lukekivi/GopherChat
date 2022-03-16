#ifndef UTILS_HPP
#define UTILS_HPP

#define INT_BYTES 4

typedef unsigned char BYTE;

/**
 * A struct to hold incoming message stats
 * - msg: a container for reading in data from socket
 * - nRecv: the amount received thus far
 * - size: the size of msg
 */
struct MSG_STAT {
	BYTE* msg;
	int nRecv;
	int size;
};

/**
 * A struct to hold transaction stats
 * - sizeStat: contains information about the body's intended size
 * - sizeIsSet: knows whether sizeStat is set or not
 * - bodyStat: contains the body
 */
struct RECV_STAT {
	struct MSG_STAT sizeStat;
	bool sizeIsSet;
	struct MSG_STAT bodyStat;
};

/**
 * A struct to hold outgoing message stats
 * - msg: a container that holds outgoing data
 * - nRecv: the amount sent thus far
 * - size: the size of msg
 */
struct SEND_STAT {
	BYTE* msg;
	int nSent;
	int size;
};

/**
 * Status enum for non-blocking socket transactions
 * - BLOCKED: blocked and incomplete
 * - ERROR: remove and deallocate associated data
 * - OKAY: Operation succeeded but may not be complete
 */
enum NbStatus {
	BLOCKED,
	ERROR,
	OKAY
};

#endif