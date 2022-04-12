#ifndef BYTE_BODY_H
#define BYTE_BODY_H

#include "../utils/utils.hpp"

class ByteBody {
  public:
    ByteBody(const BYTE* body_, int len_) {
        len = len_;
        body = body_;
    }

    ~ByteBody() {
        delete[] body;
    }


    const BYTE* GetBody() {
        return body;
    }

    int GetLen() {
        return len;
    }

  private:
    const BYTE* body;
    int len;
};

#endif