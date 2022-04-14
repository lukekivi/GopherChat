#ifndef SOCKET_MESSENGER_HPP
#define SOCKET_MESSENGER_HPP

#include "../utils.hpp"
#include "../log.hpp"
#include "../../data/command_data.hpp"
#include "../../data/response_data.hpp"
#include "../../data/msg_data.hpp"
#include "../../data/byte_body.hpp"

class SocketMessenger {
  public:
    SocketMessenger(Log* log) { SetLog(log); }
    ~SocketMessenger() { delete log; };
    
    BYTE* IntToByte(int value);
    int ByteToInt(const BYTE* arr);
    ByteBody* CharToByteBody(const char* str);

    NbStatus SendMsgNB(struct SendStat* sStat, struct pollfd* pPeer);
    void BuildSendMsg(struct SendStat* sStat, ByteBody* byteBody);
    void InitSendStat(struct SendStat* sStat);
    void RefreshSendStat(struct SendStat* sStat);

    NbStatus RecvMsgNB(struct RecvStat* rStat, struct pollfd* pPeer);
    void InitRecvStat(struct RecvStat* rStat); 

    BYTE* CommandDataToByte(CommandData* command, int* len);
    CommandData* ByteToCommandData(BYTE* body);

    ByteBody* ResponseDataToByteBody(ResponseData* response);
    ResponseData* ByteToResponseData(BYTE* body);

    char* ByteBodyToChar(ByteBody* byteBody);

    ByteBody* MsgDataToByteBody(MsgData* msgData);
    MsgData* ByteToMsgData(const BYTE* body);
    ByteBody* MsgDataFileToByteBody(MsgData* msgData);
    MsgData* ByteToMsgDataFile(BYTE* body);
    
  private:
    NbStatus RecvNB(struct MsgStat* mStat, int sockfd);
    void SetRecvStatWithSize(struct RecvStat* rStat);

    // helpers for ByteToCommandData
    Command ReadCommand(BYTE* body);
    char* ReadUsername(const BYTE* body);
    char** ReadArgs(BYTE* body, int numArgs);

    // helper for ByteToResponseData
    Status ReadStatus(BYTE* body);
    char* ReadMsg(const BYTE* body);

    // helper for ByteToMsgData
    MsgType ReadMsgType(const BYTE* body);

    void SetLog(Log* log);
    Log* log;
};

#endif