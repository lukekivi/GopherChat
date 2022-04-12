#ifndef MSG_DATA_H
#define MSG_DATA_H

#include "../utils/utils.hpp"

class MsgData {
  public:
    MsgData(MsgType msgType_, const char* username_, const char* msg_) {
        msgType = msgType_;
        username = username_;
        msg = msg_;
    }

    ~MsgData() {
        delete[] username;
        delete[] msg;
    }

    const char* GetUsername() {
        return username;
    }

    const char* GetMsg() {
        return msg;
    }

    MsgType GetMsgType() {
        return msgType;
    }

  private:
    const char* username;
    const char* msg;
    MsgType msgType;
};

#endif