#include <iostream>
#include "../../../app/utils/log.hpp"
#include "../../../app/utils/socket/socket_messenger.hpp"

const char* LOG_PATH = "log/log_response_conversion.txt";

int TestConversion();

int main() {
    int numFailed = 0;

    numFailed += TestConversion();

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

    char* username = new char[6];
    strcpy(username, "lucas");

    char* msg = new char[20];
    strcpy(msg, "hi how is it going?");
    Status status = OK;
    ResponseData* response = new ResponseData(status, msg, username);


    BYTE* body = sockMsgr.ResponseDataToByte(response, &len);



    ResponseData* actualResponse = sockMsgr.ByteToResponseData(body);
    const char* actualUsername = actualResponse->getUsername();
    const char* actualMsg = actualResponse->getMsg();
    Status actualStatus = actualResponse->GetStatus();

    if (strcmp(msg, actualMsg) != 0) {
        std::cout << "Msgs didn't match" << std::endl;
        numFailed++;
    }

    if (strcmp(username, actualUsername) != 0) {
        std::cout << "Usernames didn't match" << std::endl;
        numFailed++;
    }

    if (status != actualStatus) {
        std::cout << "Status' didn't match" << std::endl;
        numFailed++;
    }

    delete actualResponse;
    delete response;

    return numFailed;
}