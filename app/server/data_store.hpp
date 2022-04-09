#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "../utils/utils.hpp"
#include "profile.hpp"
#include "../data/user_entry.hpp"
#include <unordered_map>
#include <string>
#include <vector>

class DataStore {
  public:
    Status Register(const char* username, const char* password);
    Status Login(const char* username, const char* password);
    Status Logout(const char* username);
    bool IsLoggedIn(const char* username);

    int SetUiConn(const char* username, int i);
    int GetUiConn(const char* username);
    int AttemptLogoutConnLoss(int i);
  private:
    int FindIndexOf(const char* username);
    bool IsInMap(std::string username);

    std::unordered_map<std::string, UserEntry> m;
    std::vector<Profile*> profiles;
};

#endif