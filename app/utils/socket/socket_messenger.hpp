#ifndef SOCKET_MESSENGER_HPP
#define SOCKET_MESSENGER_HPP

#include "../utils.hpp"
#include "../log.hpp"

class SocketMessenger {
  public:
    SocketMessenger(Log* log) { SetLog(log); }
    ~SocketMessenger() { delete log; };
    
    /**
     * @brief Convert an int to a byte array of size 4
     * 
     * @param value 
     * @return malloc'd BYTE* array
     */
    BYTE* IntToByte(int value);

    /**
     * @brief Convert a size 4 byte array into an int
     * 
     * @param arr 
     * @return int representation
     */
    int ByteToInt(BYTE* arr);

    /**
     * @brief Convert a character array into a byte array.
     * 
     * @param str 
     * @return BYTE* representation
     */
    BYTE* CharToByte(const char* str);
    
    /**
     * @brief Send a message over a socket
     * 
     * @param sStat msg data container
     * @param pPeer socket data
     * @return status
     */
    NbStatus SendMsgNB(struct SEND_STAT* sStat, struct pollfd* pPeer);

    /**
     * @brief Build SEND_STAT object based off of a msg
     * 
     * @param sStat container
     * @param msg to be sent
     */
    void BuildSendMsg(struct SEND_STAT* sStat, const BYTE* msg);

    /**
     * @brief initialize SEND_STAT
     */
    void InitSendStat(struct SEND_STAT* sStat);

    /**
     * @brief Receive a message from a socket. Expects body to be
     * prepended with a 4 byte size
     * 
     * @param rStat container for received message data
     * @param pPeer socket information
     * @return status
     */
    NbStatus RecvMsgNB(struct RECV_STAT* rStat, struct pollfd* pPeer);

    /**
     * @brief initialize RECV_STAT to wait for a message
     */
    void InitRecvStat(struct RECV_STAT* rStat);


  private:
    /**
     * @brief Generic receive.
     * 
     * @param mStat container for received data.
     * @param sockfd
     * @return status
     */
    NbStatus RecvNB(struct MSG_STAT* mStat, int sockfd);
    
    /**
     * @brief Set the body portion of RECV_STAT with information
     * from the size portion.
     */
    void SetRecvStatWithSize(struct RECV_STAT* rStat);

    void SetLog(Log* log);
    Log* log;
};

#endif