#ifndef RESPONSE_DATA_H
#define RESPONSE_DATA_H

#include "../utils/utils.hpp"

/**
 * Data class to hold raw data about responses.
 */
class ResponseData {
  public:
    ResponseData(Status status_, char* msg_, char* username_) {
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

    char* getMsg() {
        return msg;
    }

    char* getUsername() {
        return username;
    }

  private:
    Status status;
    char* msg;
    char* username;
};

#endif