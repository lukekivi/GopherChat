#include <iostream>
#include "../../../app/utils/socket/socket_messenger.hpp"

const int NUM_TESTS = 6;

int main() {
    SocketMessenger sockMsgr;

    int tests[] = {0, 4, 255, 256, 10000000, 2147483647};

    for (int i=0; i < NUM_TESTS; i++) {
        BYTE* arr = sockMsgr.IntToByte(tests[i]);
        int value = sockMsgr.ByteToInt(arr);
        bool truthValue = (tests[i] == value);
        delete arr;

        if (!truthValue) {
            std::cout << "socket messenger tests failed" << std::endl;
        }
    }

    std::cout << "ok" << std::endl;
}