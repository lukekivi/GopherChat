#ifndef BYTE_BODY_H
#define BYTE_BODY_H

#include "../utils/utils.hpp"

class ByteBody {
  public:
    ByteBody(BYTE* body_, int len_) {
        len = len_;
        body = body_;
    }

    ByteBody(ByteBody* byteBody) {
        len = byteBody->len;
        body = new BYTE[len];

        for (int i = 0; i < len; i++) {
            body[i] = (BYTE) byteBody->body[i];
        }
    }

    ~ByteBody() {
        delete[] body;
    }


    BYTE* GetBody() {
        return body;
    }

    int GetLen() {
        return len;
    }

  private:
    BYTE* body;
    int len;
};

#endif