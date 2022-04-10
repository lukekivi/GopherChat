#ifndef CONN_DATA_H
#define CONN_DATA_H

#include "../utils/utils.hpp"
#include <cstring>
#include <string>

class ConnData {
  public:
    ConnData() {
        conn = REG;
        strcpy(username, "");
    }

    ConnData(const ConnData& connData) {
        conn = connData.conn;
        SetUsername(connData.username);
    }

    void SetUsername(const char* username_) {
        strcpy(username, username_);
    }

    void SetConnType(ConnType connType) {
        conn = connType;
    }  

    const char* GetUsername() {
        return username;
    }

    ConnType GetConnType() {
        return conn;
    }

 private: 
    ConnType conn;
    char username[MAX_USRNAME_SIZE+1];
};

#endif