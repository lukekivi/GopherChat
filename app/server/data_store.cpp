#include "data_store.hpp"
#include <iostream> 

Status DataStore::Register(const char* username, const char* password) {
    std::string usr(username);

    if (m.find(usr) != m.end()) {
        return FAILURE;
    } else {
        m[usr] = new UserEntry(password);
        return OK;
    }
}


Status DataStore::Login(const char* username, const char* password) {
    std::string usr(username);

    if (m.find(usr) == m.end() || !m[usr].ComparePassword(password)) {
        return FAILURE;
    } else {
        m[usr].isLoggedIn = true;
        return OK;
    }
}


Status DataStore::Logout(const char* username) {
    std::string usr(username);

    if (m.find(usr) == m.end()) {
        return FAILURE;
    } else {
        m[usr].isLoggedIn = false; 
        return OK;
    }
}


bool DataStore::IsLoggedIn(const char* username) {
    std::string usr(username);

    if (m.find(usr) == m.end() || !m[usr].isLoggedIn) {
        return false;
    } else {
        return true;
    }
}

