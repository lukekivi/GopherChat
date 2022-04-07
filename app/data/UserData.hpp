#ifndef USER_DATA_H
#define USER_DATA_H

/**
 * Data class to represent user data.
 */
class UserData {
  public:
    UserData(char* username_, char* password_) {
        username = username_;
        password = password_;
    }

    ~UserData() {
        delete username;
        delete password;
    }

    char* password() {
        return password;
    }

    char* getUsername() {
        return username;
    }

  private:
    char* username;
    char* password;
};

#endif