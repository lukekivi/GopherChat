#ifndef RESPONSE_DATA_H
#define RESPONSE_DATA_H

#include "../utils/utils.hpp"

/**
 * Data class to hold raw data about responses.
 */
class ResponseData {
  public:
    ResponseData(Status status_, const char* msg_, const char* username_) {
        status = status_;
        msg = msg_;    
        username = username_;
    }

    ~ResponseData() {
        delete[] msg;
        delete[] username;
    }

    Status GetStatus() {
        return status;
    }

    const char* getMsg() {
        return msg;
    }

    const char* getUsername() {
        return username;
    }

  private:
    Status status;
    const char* msg;
    const char* username;
};

#endif