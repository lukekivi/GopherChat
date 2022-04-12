#include <iostream>
#include "../../../app/utils/log.hpp"
#include "../../../app/utils/socket/socket_messenger.hpp"
#include "../../../app/data/msg_data.hpp"
#include <string>

const char* LOG_PATH = "log/log_msg_conversion.txt";

int TestConversion();
int TestAnonConversion();

int main() {
    int numFailed = 0;

    numFailed += TestConversion();
    numFailed += TestAnonConversion();

    if (numFailed == 0) {
        std::cout << "ok" << std::endl;
    } else {
        std::cout << numFailed << " tests failed" << std::endl;
    }

    return 0;
}

int TestConversion() {
    int len;
    int numFailed = 0;

    Log log(LOG_PATH);
    SocketMessenger sockMsgr(&log);

    std::string username ="lucas";
    std::string msg = "hi how is it going?";

    char* u1 = new char[username.length() + 1];
    char* m1 = new char[msg.length() + 1];

    strcpy(u1, username.c_str());
    strcpy(m1, msg.c_str());

    MsgType msgType = UI_MSG;
    MsgData* msgData = new MsgData(msgType, u1, m1);

    ByteBody* body = sockMsgr.MsgDataToByteBody(msgData);

    MsgData* actualMsgData = sockMsgr.ByteToMsgData(body->GetBody());
    const char* actualUsername = actualMsgData->GetUsername();
    const char* actualMsg = actualMsgData->GetMsg();
    MsgType actualMsgType = actualMsgData->GetMsgType();

    if (strcmp(m1, actualMsg) != 0) {
        std::cout << "Msgs didn't match" << std::endl;
        numFailed++;
    }

    if (strcmp(u1, actualUsername) != 0) {
        std::cout << "Usernames didn't match" << std::endl;
        numFailed++;
    }

    if (msgType != actualMsgType) {
        std::cout << "Status' didn't match" << std::endl;
        numFailed++;
    }

    delete actualMsgData;
    delete msgData;
    delete body;
    return numFailed;
}


int TestAnonConversion() {
    int len;
    int numFailed = 0;

    Log log(LOG_PATH);
    SocketMessenger sockMsgr(&log);

    std::string msg = "hi how is it going?";

    char* u1 = NULL;
    char* m1 = new char[msg.length() + 1];
    strcpy(m1, msg.c_str());

    MsgType msgType = UI_MSG;
    MsgData* msgData = new MsgData(msgType, u1, m1);

    ByteBody* body = sockMsgr.MsgDataToByteBody(msgData);

    MsgData* actualMsgData = sockMsgr.ByteToMsgData(body->GetBody());
    const char* actualUsername = actualMsgData->GetUsername();
    const char* actualMsg = actualMsgData->GetMsg();
    MsgType actualMsgType = actualMsgData->GetMsgType();

    if (strcmp(m1, actualMsg) != 0) {
        std::cout << "Msgs didn't match" << std::endl;
        numFailed++;
    }

    if (actualUsername != NULL) {
        std::cout << "Usernames didn't match" << std::endl;
        numFailed++;
    }

    if (msgType != actualMsgType) {
        std::cout << "Status' didn't match" << std::endl;
        numFailed++;
    }

    delete actualMsgData;
    delete msgData;
    delete body;

    return numFailed;
}