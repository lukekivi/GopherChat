#ifndef PROFILE_H
#define PROFILE_H

#include "utils/tcp_conn.hpp"

class Profile {
  public:
    Profile(int port_, char* ip, TcpConn* uiConn_) {
        port = port_;
        ip = ip_;
        uiConn = uiConn_;
    };

    ~Profile() {
        delete uiConn;
    };

    void SetUsername(char* username_) {
        username = username_;
    }

  private:
    char* username = null;
    int port;
    char* ip;
    TcpConn* uiConn;
};

#endif