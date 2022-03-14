#ifndef SOCKET_MESSENGER_HPP
#define SOCKET_MESSENGER_HPP

#include "../utils.hpp"

class SocketMessenger {
  public:
    BYTE* IntToByte(int value);
    int ByteToInt(BYTE* arr);
    BYTE* CharToByte(const char* str);
};

#endif