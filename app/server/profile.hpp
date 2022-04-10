#ifndef PROFILE_H
#define PROFILE_H

#include <deque>
#include "../utils/utils.hpp"
#include <iostream>
#include <cstring>

class Profile {
  public:
    Profile(const char* username_) {
      username = new char[strlen(username_)+1];
      strcpy(username, username_);
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

  private:
    char* username;
    std::deque<BYTE*> messages;
};

#endif