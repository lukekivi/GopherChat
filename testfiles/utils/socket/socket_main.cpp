#include <iostream>
#include "../../../app/utils/socket/socket_messenger.hpp"
#include "../../../app/utils/log.hpp"
#include <string>
#include <vector>

const int NUM_TESTS = 6;
const char* LOG_PATH = "log/log_socket.txt";

int TestUsernamesConversion();

int main() {
    Log log(LOG_PATH);
    SocketMessenger sockMsgr(&log);

    int tests[] = {0, 4, 255, 256, 10000000, 2147483647};

    for (int i=0; i < NUM_TESTS; i++) {
        BYTE* arr = sockMsgr.IntToByte(tests[i]);
        int value = sockMsgr.ByteToInt(arr);
        bool truthValue = (tests[i] == value);
        delete[] arr;

        if (!truthValue) {
            std::cout << "socket messenger tests failed" << std::endl;
            return -1;
        }
    }

    if (TestUsernamesConversion() != 0) {
        return -1;
    }

    std::cout << "ok" << std::endl;
    return 0;
}


int TestUsernamesConversion() {
    Log log(LOG_PATH);
    SocketMessenger sockMsgr(&log);

    std::vector<std::string> usernames;
    std::string u1("Lucas");
    std::string u2("Steph");
    std::string u3("Evan");

    usernames.push_back(u1);
    usernames.push_back(u2);
    usernames.push_back(u3);

    const char* msg = sockMsgr.ListOfUsersToChar(usernames);

    std::cout << msg << std::endl;

    std::vector<std::string> usernames2 = sockMsgr.BuildListOfUsers(msg);

    for (std::string username : usernames2) {
        std::cout << username << std::endl;
    }


    usernames.clear();

    const char* msg2 = sockMsgr.ListOfUsersToChar(usernames);

    std::cout << msg2 << std::endl;

    std::vector<std::string> usernames3 = sockMsgr.BuildListOfUsers(msg2);

    std::cout << usernames3.size() << std::endl;

    delete[] msg;    

    return 0;
}