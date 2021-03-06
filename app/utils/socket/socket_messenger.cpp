// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

#include "socket_messenger.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <poll.h>

const int MAX_VALUE = 256;


/**
 * @brief Convert an int to a byte array of size 4
 * 
 * @param value 
 * @return malloc'd BYTE* array
 */
BYTE* SocketMessenger::IntToByte(int value) {
    BYTE* arr = new BYTE[INT_BYTES];

    for (int i = 0; i < INT_BYTES; i++) {
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


/**
 * @brief Convert a size 4 byte array into an int
 * 
 * @param arr 
 * @return int representation
 */
int SocketMessenger::ByteToInt(const BYTE* arr) {
    int value = 0;
    int multiplier = 1;

    for (int i = 0; i < INT_BYTES; i++) {
        value += ((int) arr[i]) * multiplier;

        multiplier *= MAX_VALUE;
    }

    return value;
}


/**
 * @brief Convert a character array into a byte array.
 * 
 * @param str 
 * @return BYTE* representation
 */
ByteBody* SocketMessenger::CharToByteBody(const char* str) {
	int len = strlen(str);
	BYTE* body = new BYTE[len];

	for (int i = 0; i < len; i++) {
		body[i] = (BYTE) str[i];
	}

	return new ByteBody(body, len);
}


/**
 * @brief Send a message over a socket
 * 
 * @param sStat msg data container
 * @param pPeer socket data
 * @return status
 */
NbStatus SocketMessenger::SendMsgNB(struct SendStat* sStat, struct pollfd* pPeer) {	

	while (sStat->nSent < sStat->size) {
		//connStat keeps tracks of how many bytes have been sent, allowing us to "resume" 
		//when a previously non-writable socket becomes writable. 
		int n = send(pPeer->fd, sStat->msg + sStat->nSent, sStat->size - sStat->nSent, 0);
		if (n >= 0) {
			sStat->nSent += n;
		} else if (n < 0 && (errno == ECONNRESET || errno == EPIPE)) {
			log->Info("SendNB: Connection closed.");
			close(pPeer->fd);
			return ERROR;
		} else if (n < 0 && (errno == EWOULDBLOCK)) {
			//The socket becomes non-writable. Exit now to prevent blocking. 
			//OS will notify us when we can write
			pPeer->events |= POLLWRNORM; 
			return BLOCKED; 
		} else {
			log->Error("SendNB: Unexpected send error %d: %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	pPeer->events &= ~POLLWRNORM;
	return OKAY;
}


/**
 * Build SEND_STAT object based off of a bytebody message
 */
void SocketMessenger::BuildSendMsg(struct SendStat* sStat, ByteBody* byteBody) {
	// msg size as a byte array for sending across socket
	int len = byteBody->GetLen();
	BYTE* body = byteBody->GetBody();
	BYTE* arr = IntToByte(len);
	delete[] sStat->msg;

	sStat->msg = new BYTE[len + INT_BYTES];
	sStat->size = len + INT_BYTES;
	sStat->nSent = 0;

	// prepend size onto body and add it to sStat.msg
	std::copy(body, body + len, std::copy(arr, arr + INT_BYTES, sStat->msg));

	delete[] arr;
}


/**
 * @brief initialize SEND_STAT
 */
void SocketMessenger::InitSendStat(struct SendStat* sStat) {
	sStat->msg = NULL;
	sStat->nSent = 0;
	sStat->size = -1;
}


void SocketMessenger::RefreshSendStat(struct SendStat* sStat) {
	delete[] sStat->msg;
	sStat->msg = NULL;
	sStat->nSent = 0;
	sStat->size = -1;
}


/**
 * @brief Receive a message from a socket. Expects body to be
 * prepended with a 4 byte size
 * 
 * @param rStat container for received message data
 * @param pPeer socket information
 * @return status
 */
NbStatus SocketMessenger::RecvMsgNB(struct RecvStat* rStat, struct pollfd* pPeer) {

	/**
	 * Recieve size of body. An integer represented by 4 BYTEs.
	 */
	while (!rStat->sizeIsSet) { 
		NbStatus status = RecvNB(&rStat->sizeStat, pPeer->fd);

		if (status == OKAY) {
			if (rStat->sizeStat.nRecv == rStat->sizeStat.size) {
				// size read completely
				SetRecvStatWithSize(rStat);
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


/**
 * @brief initialize RECV_STAT to wait for a message
 */
void SocketMessenger::InitRecvStat(struct RecvStat* rStat) {
	// prep size struct
	rStat->sizeIsSet = false;
	rStat->sizeStat.size = INT_BYTES;
	rStat->sizeStat.nRecv = 0;
	delete[] rStat->sizeStat.msg;
	rStat->sizeStat.msg = new BYTE[INT_BYTES];

	// prep body struct
	rStat->bodyStat.nRecv = 0;
	delete[] rStat->bodyStat.msg;
	rStat->bodyStat.msg = nullptr;
	rStat->bodyStat.size = -1;
}


/**
 * @brief Generic receive.
 * 
 * @param mStat container for received data.
 * @param sockfd
 * @return status
 */
NbStatus SocketMessenger::RecvNB(struct MsgStat* mStat, int sockfd) {
	int n = recv(sockfd, mStat->msg + mStat->nRecv, mStat->size - mStat->nRecv, 0);	

	if (n > 0) {
		mStat->nRecv += n;
		return OKAY;
	} else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
		log->Info("RecvNB: Connection closed.");
		close(sockfd);
		return ERROR;
	} else if (n < 0 && (errno == EWOULDBLOCK)) { 
		//The socket becomes non-readable. Exit now to prevent blocking. 
		//OS will notify us when we can read
		return BLOCKED; 
	} else {
		log->Error("RecvNB: Unexpected recv error %d: %s.", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}


/**
 * @brief Set the body portion of RECV_STAT with information
 * from the size portion.
 */
void SocketMessenger::SetRecvStatWithSize(struct RecvStat* rStat) {
	rStat->sizeIsSet = true;
	rStat->bodyStat.size = ByteToInt(rStat->sizeStat.msg);
	rStat->bodyStat.msg = new BYTE[rStat->bodyStat.size];
}


void SocketMessenger::SetLog(Log* log) {
    this->log = new Log(log);
}


BYTE* SocketMessenger::CommandDataToByte(CommandData* command, int* len) {
	int numArgs = command->getNumArgs();
	char* username = command->getUsername();

	char** args = command->getArgs();
	int lens[numArgs];

	/**
	 * total length includes
	 * - integer representation of command enum
	 * - optional username field of length 8
	 * - integer preceding each arg to represent arg length
	 * - length of each arg
	 */
	*len = (numArgs + 1) * INT_BYTES + MAX_USRNAME_SIZE;
	for (int i = 0; i < numArgs; i++) {
		lens[i] = strlen(args[i]);
		*len += lens[i];
	}

	BYTE* body = new BYTE[*len];
	BYTE* cmd = IntToByte(command->getCommand());
	
	int i = 0;
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = cmd[j];
		i++;
	}

	for (int j = 0; j < MAX_USRNAME_SIZE; j++) {
		if (username == NULL || j > strlen(username) - 1) {
			body[i] = '-';
		} else {
			body[i] = username[j];
		}
		i++;
	}

	for (int j = 0; j < numArgs; j++) {
		BYTE* argLen = IntToByte(lens[j]);
		ByteBody* byteBody = CharToByteBody(args[j]);
		BYTE* arg = byteBody->GetBody();
		for (int k = 0; k < INT_BYTES; k++) {
			body[i] = argLen[k];
			i++;
		}

		for (int k = 0; k < lens[j]; k++) {
			body[i] =  arg[k];
			i++;
		}
		delete[] argLen;
		delete byteBody;
	}

	delete[] cmd;
	return body;
}

CommandData* SocketMessenger::ByteToCommandData(BYTE* body) {
	CommandData* commandData = NULL;
	Command command = ReadCommand(body);

	char* username = ReadUsername(body);
	int numArgs = 0;
	switch(command) {
		case SEND_FILE_TO:
			numArgs = 3;
			break;
		case REGISTER:
		case LOGIN:
		case SEND_TO:
		case SEND_TO_ANON:
		case SEND_FILE:
			numArgs = 2;
			break;
		case SEND:
		case SEND_ANON:
		case DELAY:
			numArgs = 1;
			break;
		case LOGOUT:
		case LIST:
		case UI_CONN:
		case FILE_CONN:
			break;
		default:
			log->Error("Invalid COMMAND.");
			exit(EXIT_FAILURE);
	}

	commandData = new CommandData(command, ReadArgs(body, numArgs), numArgs);
	commandData->setUsername(username);

	return commandData;
}


Command SocketMessenger::ReadCommand(BYTE* body) {
	BYTE cmd[4];

	int i = 0;
	for (int j = 0; j < 4; j++) {
		cmd[j] = body[i];
		i++;
	}

	return static_cast<Command>(ByteToInt(cmd));
}


/**
 * Username always comes after the Command enum which is of size INT_BYTES
 */
char* SocketMessenger::ReadUsername(const BYTE* body) {

	if (body[INT_BYTES] == '-') {
		return NULL;
	}

	std::string temp;

	for (int i = INT_BYTES; i < INT_BYTES + MAX_USRNAME_SIZE; i++) {
		char letter = (char) body[i];
		if (letter == '-') {
			break;
		} else {
			temp.push_back(letter);
		}
	}

	char* username = new char[temp.length()+1];
	strcpy(username, temp.c_str());

	return username;
}

char** SocketMessenger::ReadArgs(BYTE* body, int numArgs) {
	if (numArgs == 0) {
		return NULL;
	}

	int offset = INT_BYTES + MAX_USRNAME_SIZE;
	int sizes[numArgs];
	char** args = new char*[numArgs];

	for (int i = 0; i < numArgs; i++) {
		BYTE* num = body + offset;
		sizes[i] = ByteToInt(num);
		offset += INT_BYTES;

		char* arg = new char[sizes[i]+1];
		arg[sizes[i]] = '\0';

		for (int j = 0; j < sizes[i]; j++) {
			arg[j] = (char) body[offset];
			offset++;
		}
		args[i] = arg;
	}

	return args;
}


char* SocketMessenger::ByteBodyToChar(ByteBody* byteBody) {
	char* msg = new char[byteBody->GetLen()+1];
	BYTE* body = byteBody->GetBody();
	for (int i = 0; i < byteBody->GetLen(); i++) {
		msg[i] = body[i];
	}
	msg[byteBody->GetLen()] = '\0';
	return msg;
}


ByteBody*  SocketMessenger::ResponseDataToByteBody(ResponseData* response) {
	const char* username = response->getUsername();
	const char* msg = response->getMsg();
	int msgLen = strlen(msg);

	/**
	 * total length includes
	 * - integer representation of status enum
	 * - username field of length 8
	 * - integer preceding msg to represent msg length
	 * - length of each msg
	 */
	int totalLen = 2 * INT_BYTES + MAX_USRNAME_SIZE + msgLen;

	BYTE* body = new BYTE[totalLen];
	BYTE* status = IntToByte(response->GetStatus());
	
	int i = 0;
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = status[j];
		i++;
	}

	for (int j = 0; j < MAX_USRNAME_SIZE; j++) {
		if (username == NULL || j > strlen(username) - 1) {
			body[i] = '-';
		} else {
			body[i] = username[j];
		}
		i++;
	}

	BYTE* argLen = IntToByte(msgLen);
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = argLen[j];
		i++;
	}

	// convert the msg
	ByteBody* byteBody = CharToByteBody(msg);
	BYTE* arg = byteBody->GetBody();
	for (int j = 0; j < msgLen; j++) {
		body[i] = arg[j];
		i++;
	}

	delete[] argLen;
	delete[] status;
	delete byteBody;

	ByteBody* returnBody = new ByteBody(body, totalLen);

	return returnBody;
}


ResponseData* SocketMessenger::ByteToResponseData(BYTE* body) {
	Status status = ReadStatus(body);
	char* username = ReadUsername(body);
	char* msg = ReadMsg(body);
	ResponseData* responseData = new ResponseData(status, msg, username);
	delete[] username;
	delete[] msg;

	return responseData;
}


Status SocketMessenger::ReadStatus(BYTE* body) {
	BYTE cmd[4];

	int i = 0;
	for (int j = 0; j < 4; j++) {
		cmd[j] = body[i];
		i++;
	}

	return static_cast<Status>(ByteToInt(cmd));
}


char* SocketMessenger::ReadMsg(const BYTE* body) {
	int offset = INT_BYTES + MAX_USRNAME_SIZE;

	const BYTE* num = body + offset;
	int size = ByteToInt(num);

	char* msg = new char[size+1];
	msg[size] = '\0';

	offset += INT_BYTES;

	for (int i = 0; i < size; i++) {
		msg[i] = (char) body[offset];
		offset++;
	}
	
	return msg;
}


ByteBody* SocketMessenger::MsgDataToByteBody(MsgData* msgData) {
	if (msgData->GetMsgType() != UI_MSG) {
		log->Error("SocketMessenger.MsgDataToByteBody() - receivied FILE_MSG. Should only receive UI_MSGs.");
		return NULL;
	}
	const char* username = msgData->GetUsername();
	const char* msg = msgData->GetMsg();
	int msgLen = strlen(msg);
	int len = 0;

	/**
	 * total length includes
	 * - integer representation of MsgType enum
	 * - username field of length 8
	 * - integer preceding msg to represent msg length
	 * - length of each msg
	 */
	len = 2 * INT_BYTES + MAX_USRNAME_SIZE + msgLen;

	BYTE* body = new BYTE[len];
	BYTE* msgType = IntToByte(msgData->GetMsgType());	
	int i = 0;
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = msgType[j];
		i++;
	}

	for (int j = 0; j < MAX_USRNAME_SIZE; j++) {
		if (username == NULL || j > strlen(username) - 1) {
			body[i] = '-';
		} else {
			body[i] = username[j];
		}
		i++;
	}

	BYTE* argLen = IntToByte(msgLen);
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = argLen[j];
		i++;
	}

	// convert the msg
	ByteBody* byteBody = CharToByteBody(msg);
	BYTE* arg = byteBody->GetBody();
	for (int j = 0; j < msgLen; j++) {
		body[i] = arg[j];
		i++;
	}

	delete[] argLen;
	delete[] msgType;
	delete byteBody;
	return new ByteBody(body, len);
}


MsgData* SocketMessenger::ByteToMsgData(const BYTE* body) {
	MsgType msgType = ReadMsgType(body);
	char* username = ReadUsername(body);
	return new MsgData(msgType, username, ReadMsg(body));
}


MsgType SocketMessenger::ReadMsgType(const BYTE* body) {
	BYTE cmd[4];

	int i = 0;
	for (int j = 0; j < 4; j++) {
		cmd[j] = body[i];
		i++;
	}

	return static_cast<MsgType>(ByteToInt(cmd));
}


ByteBody* SocketMessenger::MsgDataFileToByteBody(MsgData* msgData) {
	if (msgData->GetMsgType() != FILE_MSG) {
		log->Error("SocketMessenger.MsgDataFileToByteBody() - receivied UI_MSG. Should only receive FILE_MSGs.");
		return NULL;
	}

	const char* username = msgData->GetUsername();
	const char* fileName = msgData->GetFileName();
	const char* fileContents = msgData->GetMsg();
	int fileNameLen = strlen(fileName);
	int fileContentsLen = strlen(fileContents);
	int len = 0;

	/**
	 * total length includes
	 * - integer representation of MsgType enum
	 * - username field of length 8
	 * - integer preceding msg to represent msg length
	 * - length of each msg
	 */
	len = 3 * INT_BYTES + MAX_USRNAME_SIZE + fileNameLen + fileContentsLen;

	// allocate space for entire body
	BYTE* body = new BYTE[len];

	// embed the msgtype
	BYTE* msgType = IntToByte(msgData->GetMsgType());	
	int i = 0;
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = msgType[j];
		i++;
	}

	// embed the username
	for (int j = 0; j < MAX_USRNAME_SIZE; j++) {
		if (username == NULL || j > strlen(username) - 1) {
			body[i] = '-';
		} else {
			body[i] = username[j];
		}
		i++;
	}

	// embed the filename length
	BYTE* argOneLen = IntToByte(fileNameLen);
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = argOneLen[j];
		i++;
	}

	// embed the filename
	ByteBody* byteBodyOne = CharToByteBody(fileName);
	BYTE* argOne = byteBodyOne->GetBody();
	for (int j = 0; j < fileNameLen; j++) {
		body[i] = argOne[j];
		i++;
	}

	// embed the fileContents len
	BYTE* argTwoLen = IntToByte(fileContentsLen);
	for (int j = 0; j < INT_BYTES; j++) {
		body[i] = argTwoLen[j];
		i++;
	}

	// embed the fileContents
	ByteBody* byteBodyTwo = CharToByteBody(fileContents);
	BYTE* argTwo = byteBodyTwo->GetBody();
	for (int j = 0; j < fileContentsLen; j++) {
		body[i] = argTwo[j];
		i++;
	}

	delete byteBodyOne;
	delete byteBodyTwo;
	delete[] argOneLen;
	delete[] argTwoLen;
	delete[] msgType;
	return new ByteBody(body, len);
}


MsgData* SocketMessenger::ByteToMsgDataFile(BYTE* body) {
	MsgType msgType = ReadMsgType(body);
	char* username = ReadUsername(body);
	char** args = ReadArgs(body, 2);
	char* fileName = args[0];
	char* fileConents = args[1];
	args[0] = NULL;
	args[1] = NULL;
	delete[] args;
	return new MsgData(msgType, username, fileConents, fileName);
}


const char* SocketMessenger::ListOfUsersToChar(std::vector<std::string> usernames) {
	int numUsers = usernames.size();
	if (numUsers == 0) {
		return EMPTY_USERNAME_STR;
	}

	int totalLen = 0;

	for (std::string username : usernames) {
		totalLen += username.length();
	}

	int len = (numUsers - 1) + totalLen;

	char* msg = new char[len + 1];
	msg[len] = '\0';

	int index = 0;
	for (int i = 0; i < usernames.size(); i++) {
		for (int j = 0; j < usernames.at(i).length(); j++) {
			msg[index] = usernames.at(i)[j];
			index++;
		}
		if (i != usernames.size() - 1) {
			msg[index] = '-';
			index++;
		}
	}

	return msg;
}


std::vector<std::string> SocketMessenger::BuildListOfUsers(const char* msg) {
	std::vector<std::string> usernames;

	if (msg == NULL || strlen(msg) < 3 || msg[0] == '!') {
		return usernames;
	}

	std::string username;
	for (int i = 0; i < strlen(msg); i++) {
		if (msg[i] == '-') {
			usernames.push_back(username);
			username.clear();
		} else {
			username.push_back(msg[i]);
		}
	}
	usernames.push_back(username);
	return usernames;
}