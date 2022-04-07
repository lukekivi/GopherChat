#ifndef MSG_DATA_H
#define MSG_DATA_H

/**
 * Data class to represent message command data.
 */
class MsgData {
  public:
    MsgData(char* rcvrName_, char* msg_, bool anon_) {
        rcvrName = rcvrName_;
        msg = msg_;
        anon = anon_;
    }

    ~MsgData() {
        delete msg;
        delete rcvrName;
    }

    /**
     * Is the message supposed to be public?
     */
    bool isPublic() {
        return rcvrName == nullptr;
    }

    char* getMsg() {
        return msg;
    }

    char* getRcvrName() {
        return rcvrName;
    }

    bool isAnon() {
        return anon;
    }

  private:
    char* rcvrName;
    char* msg;
    bool anon;
};

#endif