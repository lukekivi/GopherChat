#include "socket_messenger.hpp"
#include <arpa/inet.h>
#include <iostream>

const int NUM_BYTES = 4;
const int MAX_VALUE = 256;

/**
 * @brief Convert an integer to bytes
 * 
 * @param value to be converted
 * @return BYTE representation
 */
BYTE* SocketMessenger::IntToByte(int value) {

    BYTE* arr = new BYTE[NUM_BYTES];

    for (int i = 0; i < NUM_BYTES; i++) {
        if (value < MAX_VALUE) {
            arr[i] = (BYTE) value;
            value = 0;
        } else {
            arr[i] = (BYTE) (value % MAX_VALUE);
            value /= MAX_VALUE;
        }
    }
    return arr;
}

/**
 * @brief Convert byte array (size 4) to an int
 * 
 * @param arr to be converted
 * @return integer representation
 */
int SocketMessenger::ByteToInt(BYTE* arr) {
    int value = 0;
    int multiplier = 1;

    for (int i = 0; i < NUM_BYTES; i++) {
        value += ((int) arr[i]) * multiplier;

        multiplier *= MAX_VALUE;
    }

    return value;
}

BYTE* SocketMessenger::CharToByte(const char* str) {
    return reinterpret_cast<BYTE*>(const_cast<char*>(str));
}