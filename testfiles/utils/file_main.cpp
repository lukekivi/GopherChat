#include "../../app/utils/file_transporter.hpp"
#include "../../app/utils/log.hpp"
#include "../../app/utils/socket/socket_messenger.hpp"
#include <string>
#include <cstring>

const char* LOG_PATH = "log/log_file.txt";

const char* TEST_FILE = "testfiles/test_input_files/testFile.txt";
const char* NEW_FILE_LOCATION = "testfiles/test_output_files/newTestFile.txt";

const char* GIANT_TEST_FILE = "testfiles/test_input_files/giantFile.txt";
const char* GIANT_NEW_FILE_LOCATION = "testfiles/test_output_files/newGiantTestFile.txt";

int main () {
    int numFailed = 0;
    FileTransporter ft;
    Log log(LOG_PATH);
    SocketMessenger sockMsgr(&log);

    std::string username = "lucas";
    char* u1 = new char[strlen(username.c_str()) + 1];
    strcpy(u1, username.c_str());

    char* fn1 = new char[strlen(TEST_FILE) + 1];
    strcpy(fn1, TEST_FILE);

    char* contents1 = ft.fileToChar(TEST_FILE);
    MsgData* msgData = new MsgData(FILE_MSG, u1, contents1, fn1);
    ByteBody* bb1 = sockMsgr.MsgDataFileToByteBody(msgData);
    MsgData* msgDataActual = sockMsgr.ByteToMsgDataFile(bb1->GetBody());

    MsgType mt1Actual = msgDataActual->GetMsgType();
    const char* u1Actual = msgDataActual->GetUsername();
    const char* fn1Actual = msgDataActual->GetFileName();
    const char* fc1Actual = msgDataActual->GetMsg();

    if (mt1Actual != FILE_MSG) {
        std::cout << "mt1Actual was not FILE_MSG." << std::endl;
        numFailed++;
    }

    if (strcmp(u1Actual, u1) != 0) {
        std::cout << u1Actual << " was not " << username << std::endl;
        numFailed++;
    }

    if (strcmp(fn1Actual, fn1) != 0) {
        std::cout << fn1Actual << " was not " << fn1 << std::endl;
        numFailed++;
    }

    if (strcmp(fc1Actual, contents1) != 0) {
        std::cout << "fc1Actual was not correct." << std::endl;
        numFailed++;
    }

    bool resultsOne = ft.charToFile(NEW_FILE_LOCATION,fc1Actual);

    if (!resultsOne) {
        std::cout << "Failed. Make sure the file: \"" << NEW_FILE_LOCATION << "\" doesn't exist." << std::endl;
        std::cout << "If it isn't there then something went wrong." << std::endl;
        numFailed++;
    }

    delete msgData;
    delete bb1;
    delete msgDataActual;


    /**
     * 10 MB input file
     */
    char* u2 = new char[strlen(username.c_str()) + 1];
    strcpy(u2, username.c_str());

    char* fn2 = new char[strlen(GIANT_TEST_FILE) + 1];
    strcpy(fn2, GIANT_TEST_FILE);

    char* contents2 = ft.fileToChar(GIANT_TEST_FILE);
    msgData = new MsgData(FILE_MSG, u2, contents2, fn2);
    ByteBody* bb2 = sockMsgr.MsgDataFileToByteBody(msgData);
    msgDataActual = sockMsgr.ByteToMsgDataFile(bb2->GetBody());

    MsgType mt2Actual = msgDataActual->GetMsgType();
    const char* u2Actual = msgDataActual->GetUsername();
    const char* fn2Actual = msgDataActual->GetFileName();
    const char* fc2Actual = msgDataActual->GetMsg();

    if (mt2Actual != FILE_MSG) {
        std::cout << "mt2Actual was not FILE_MSG." << std::endl;
        numFailed++;
    }

    if (strcmp(u2Actual, u2) != 0) {
        std::cout << u2Actual << " was not " << username << std::endl;
        numFailed++;
    }

    if (strcmp(fn2Actual, fn2) != 0) {
        std::cout << fn2Actual << " was not " << fn2 << std::endl;
        numFailed++;
    }

    if (strcmp(fc2Actual, contents2) != 0) {
        std::cout << "fc2Actual was not correct." << std::endl;
        numFailed++;
    }

    bool resultsTwo = ft.charToFile(GIANT_NEW_FILE_LOCATION,fc2Actual);

    if (!resultsTwo) {
        std::cout << "Failed. Make sure the file: \"" << GIANT_NEW_FILE_LOCATION << "\" doesn't exist." << std::endl;
        std::cout << "If it isn't there then something went wrong." << std::endl;
        numFailed++;
    }

    delete msgData;
    delete bb2;
    delete msgDataActual;

    if (numFailed == 0) {
        std::cout << "ok" << std::endl;
    }
}