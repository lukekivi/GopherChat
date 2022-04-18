// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "../utils/utils.hpp"
#include "profile.hpp"
#include "../data/user_entry.hpp"
#include "../data/byte_body.hpp"
#include "registrar.hpp"
#include <unordered_map>
#include <string>
#include <vector>

class DataStore {
  public:
    DataStore(const char* registrarFilePath);
    ~DataStore() { delete reg; }
    Status Register(const char* username, const char* password);
    Status Login(const char* username, const char* password);
    Status Logout(const char* username);
    bool IsLoggedIn(const char* username);
    int Enqueue(const char* username, ByteBody* byteBody);
    int EnqueueAllExcept(const char* username, ByteBody* byteBody);
    ByteBody* Dequeue(const char* username);
    std::vector<std::string> GetUsersWithMsgs();
    std::vector<std::string> GetSignedInUsers();
    bool IsUserRegistered(const char* username);

  private:
    int FindIndexOf(const char* username);

    Registrar* reg;
    std::vector<Profile> profiles;
};

#endif