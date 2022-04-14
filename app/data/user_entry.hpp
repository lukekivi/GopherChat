#ifndef USER_ENTRY_H
#define USER_ENTRY_H

#include <cstring>
#include "../utils/utils.hpp"


class UserEntry {
  public:
    UserEntry() {};

    UserEntry operator=(UserEntry entry) {
        strcpy(password, entry.password);
        return entry;
    }

    UserEntry(const char* password_) {
        strcpy(password, password_);
    }

    bool ComparePassword(const char* guess) {
        if (strcmp(guess, password) == 0) {
            return true;
        } else {
            return false;
        }
    }

  private:
    char password[MAX_PWD_SIZE+1];
};

#endif