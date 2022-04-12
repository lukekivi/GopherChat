#ifndef PROFILE_H
#define PROFILE_H

#include <deque>
#include "../utils/utils.hpp"
#include <iostream>
#include <cstring>
#include "../data/byte_body.hpp"

class Profile {
  public:
    Profile(const char* username_) {
      strcpy(username, username_);
    }

    const char* GetUsername() {
      return username;
    }

    bool HasMessages() {
      return !messages.empty();
    }

    ByteBody* DequeueBody() {
      ByteBody* results = NULL;
      if (messages.size() > 0) {
        results = messages.front();
        messages.pop_front();
      } 
      return results;
    }

    void EnqueueBody(ByteBody* body) {
      messages.push_back(body);
    }

  private:
    char username[MAX_USRNAME_SIZE];
    std::deque<ByteBody*> messages;
};

#endif