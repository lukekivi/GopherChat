#ifndef SOCKET_MESSENGER_HPP
#define SOCKET_MESSENGER_HPP

#include "../utils.hpp"
#include "../log.hpp"
#include "../../data/command_data.hpp"

class SocketMessenger {
  public:
    SocketMessenger(Log* log) { SetLog(log); }
    ~SocketMessenger() { delete log; };
    
    BYTE* IntToByte(int value);
    int ByteToInt(BYTE* arr);
    BYTE* CharToByte(const char* str);

    NbStatus SendMsgNB(struct SendStat* sStat, struct pollfd* pPeer);
    void BuildSendMsg(struct SendStat* sStat, const BYTE* msg);
    void InitSendStat(struct SendStat* sStat);

    NbStatus RecvMsgNB(struct RecvStat* rStat, struct pollfd* pPeer);
    void InitRecvStat(struct RecvStat* rStat); 

    BYTE* CommandDataToByte(CommandData* command);
    CommandData* ByteToCommandData(BYTE* body);
    
  private:
    NbStatus RecvNB(struct MsgStat* mStat, int sockfd);
    void SetRecvStatWithSize(struct RecvStat* rStat);

    // helpers for ByteToCommandData
    Command ReadCommand(BYTE* body);
    char* ReadUsername(BYTE* body);
    char** ReadArgs(BYTE* body, int numArgs);

    void SetLog(Log* log);
    Log* log;
};

#endif