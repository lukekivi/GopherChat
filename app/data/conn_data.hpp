// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

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
        isActive = false;
    }

    ConnData(const ConnData& connData) {
        conn = connData.conn;
        SetUsername(connData.username);
        isActive = connData.isActive;
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

    bool IsActive() {
        return isActive;
    }

    void Activate() {
        isActive = true;
    }

    void Deactivate() {
        isActive = false;
    }

 private: 
    bool isActive;
    ConnType conn;
    char username[MAX_USRNAME_SIZE+1];
};

#endif