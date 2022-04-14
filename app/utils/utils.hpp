#ifndef UTILS_HPP
#define UTILS_HPP

#define INT_BYTES 4
#define MAX_FILE_SIZE 10000000
#define MIN_USRNAME_SIZE 4
#define MAX_USRNAME_SIZE 8
#define MIN_PWD_SIZE 4
#define MAX_PWD_SIZE 8
#define MAX_MSG_SIZE 256
#define MAX_TOTAL_SIZE 10000100
#define MAX_INT_SIZE_LENGTH 8
#define FILE_CONNS 10

typedef unsigned char BYTE;

/**
 * A struct to hold incoming message stats
 * - msg: a container for reading in data from socket
 * - nRecv: the amount received thus far
 * - size: the size of msg
 */
struct MsgStat {
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
struct RecvStat {
	struct MsgStat sizeStat;
	bool sizeIsSet;
	struct MsgStat bodyStat;
};

/**
 * A struct to hold outgoing message stats
 * - msg: a container that holds outgoing data
 * - nRecv: the amount sent thus far
 * - size: the size of msg
 */
struct SendStat {
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


/**
 * State to distinguish incoming commands
 */
enum Command {
    REGISTER        = 0,
    LOGIN           = 1,
    LOGOUT          = 2,
	SEND            = 3,
    SEND_TO         = 4,
    SEND_ANON       = 5,
    SEND_TO_ANON    = 6,
    SEND_FILE       = 7,
    SEND_FILE_TO    = 8,
    LIST            = 9,
    DELAY           = 10,
	UI_CONN			= 11,	// reserved for system, not for users
	FILE_CONN 		= 12	// reserved for system, not for users
};


/**
 * Status to ACK client messages.
 * - OK: everything went ok
 * - FAILURE: something went wrong
 * - LOGGED_IN: Use was logged in
 */
enum Status {
	OK			= 30,
	FAILURE     = 31,
	LOGGED_IN   = 32,
	LOGGED_OUT  = 33,
};


/**
 * Message states
 */
enum MsgType {
	FILE_MSG = 50,
	UI_MSG = 51
};



enum ConnType {
    REG = 40,
    UI = 41,
    FIL = 42
};
#endif