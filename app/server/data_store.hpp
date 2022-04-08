#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "../utils/utils.hpp"
#include "../data/user_enty.hpp"
#include <unordered_map>
#include <string>

class DataStore {
  public:
    Status Register(const char* username, const char* password);
    Status Login(const char* username, const char* password);
    Status Logout(const char* username);
    bool IsLoggedIn(const char* username);

  private:
    std::unordered_map<std::string, UserEntry> m;

};

#endif