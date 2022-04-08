#ifndef USER_ENTY_H
#define USER_ENTRY_H

#include <vector>
#include <cstring>


class UserEntry {
  public:
    UserEntry() {};

    UserEntry* operator=(UserEntry* entry) {
        password = new char[strlen(entry->password) + 1];
        strcpy(password, entry->password);
        for (int i = 0; i < entry->fileConnIndices.size(); i++) {
            fileConnIndices.push_back(entry->fileConnIndices.at(i));
        }
        uiConnIndex = entry->uiConnIndex;
        return entry;
    }

    UserEntry(const char* password_) {
        int len = strlen(password_);
        password = new char[len+1];
        strcpy(password, password_);
    }

    ~UserEntry() {
        delete password;
    }

    bool ComparePassword(const char* guess) {
        if (strcmp(guess, password) == 0) {
            return true;
        } else {
            return false;
        }
    }

    bool isLoggedIn = false;

  private:
    char* password;
    std::vector<int> fileConnIndices;
    int uiConnIndex;
};

#endif