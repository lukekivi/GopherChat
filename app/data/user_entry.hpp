// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

#ifndef USER_ENTRY_H
#define USER_ENTRY_H

#include <cstring>
#include "../utils/utils.hpp"


class UserEntry {
  public:
    UserEntry(const char* username_, const char* password_) {
        strcpy(username, username_);
        strcpy(password, password_);
    }

    UserEntry operator=(UserEntry entry) {
        strcpy(username, entry.username);
        strcpy(password, entry.password);
        return entry;
    }

    bool ComparePassword(const char* guess) {
        if (strcmp(guess, password) == 0) {
            return true;
        } else {
            return false;
        }
    }

    const char* GetUsername() {
        return username;
    }

    const char* GetPassword() {
        return password;
    }

  private:
    char username[MAX_USRNAME_SIZE+1];
    char password[MAX_PWD_SIZE+1];
};

#endif