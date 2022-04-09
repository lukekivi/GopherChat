#ifndef PROFILE_H
#define PROFILE_H

#include <deque>
#include "../utils/utils.hpp"
#include <iostream>
#include <cstring>

struct FileConn {
  int index;
  bool isInUse;
};

class Profile {
  public:
    Profile(const char* username_) {
      username = new char[strlen(username_)+1];
      strcpy(username, username_);
      uiConn.index = -1;
      uiConn.isInUse = false;
    }

    ~Profile() {
      delete[] username;
    }

    const char* GetUsername() {
      return username;
    }

    BYTE* dequeueMsg() {
      BYTE* results = NULL;
      if (messages.size() > 0) {
        results = messages.front();
        messages.pop_front();
      } 
      return results;
    }

    void EnqueueMsg(BYTE* msg) {
      messages.push_back(msg);
    }

    void SetUiConn(int i) {
      uiConn.index = i;
      uiConn.isInUse = false;
    }

    int GetUiConn() {
      if (uiConn.isInUse) {
        return -1;
      } else {
        return uiConn.index;
      }
    } 

    void SetFileConn(int i) {
      fileConns[numFileConns].index = i;
      fileConns[numFileConns].isInUse = false;
    }

    int GetFileConn() {
      for (int i = 0; i < FILE_CONNS; i++) {
        if (!fileConns[i].isInUse) {
          fileConns[i].isInUse = true;
          return fileConns[i].index;
        }
      }
      return -1;
    }

    void ResetFileConn(int i) {
      for (int j = 0; j < FILE_CONNS; j++) {
        if (fileConns[j].index == i) {
          fileConns[j].isInUse = false;
        }
      }
    }

    bool IsInConns(int i) {
      if (uiConn.index == i) {
        return true;
      }

      for (int j = 0; j < numFileConns; j++) {
          if (fileConns[j].index == i) {
            return true;
          }
      }

      return false;
    }

  private:
    char* username;
    struct FileConn uiConn;
    std::deque<BYTE*> messages;
    struct FileConn fileConns[FILE_CONNS];
    int numFileConns = 0;
};

#endif