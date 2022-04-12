#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "../utils/utils.hpp"
#include "profile.hpp"
#include "../data/user_entry.hpp"
#include "../data/byte_body.hpp"
#include <unordered_map>
#include <string>
#include <vector>

class DataStore {
  public:
    Status Register(const char* username, const char* password);
    Status Login(const char* username, const char* password);
    Status Logout(const char* username);
    bool IsLoggedIn(const char* username);
    bool Enqueue(const char* username, const BYTE* msg, int len);
    void EnqueueAllExcept(const char* username, const BYTE* msg, int len);
    ByteBody* Dequeue(const char* username);
    std::vector<std::string> GetUsersWithMsgs();

  private:
    ByteBody* BuildByteBody(const BYTE* msg, int len);
    int FindIndexOf(const char* username);
    bool IsInMap(std::string username);

    std::unordered_map<std::string, UserEntry> m;
    std::vector<Profile> profiles;
};

#endif