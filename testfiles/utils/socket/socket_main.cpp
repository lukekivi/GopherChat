#include <iostream>
#include "../../../app/utils/socket/socket_messenger.hpp"
#include "../../../app/utils/log.hpp"

const int NUM_TESTS = 6;
const char* LOG_PATH = "log/log_socket.txt";

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

    std::cout << "ok" << std::endl;
    return 0;
}