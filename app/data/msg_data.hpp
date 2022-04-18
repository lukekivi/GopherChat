// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

#ifndef MSG_DATA_H
#define MSG_DATA_H

#include "../utils/utils.hpp"

class MsgData {
  public:
    MsgData(MsgType msgType_, const char* username_, const char* msg_, const char* fileName_ = NULL) {
        msgType = msgType_;
        username = username_;
        msg = msg_;
        fileName = fileName_;
    }

    ~MsgData() {
        delete[] username;
        delete[] msg;
        delete[] fileName;  
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

    const char* GetFileName() {
        return fileName;
    }

  private:
    const char* username;
    const char* msg;
    const char* fileName;
    MsgType msgType;
};

#endif