#include "data_store.hpp"
#include <iostream> 

Status DataStore::Register(const char* username, const char* password) {
    std::cout << "DataStore.Register()\n\t-user: " <<  username << std::endl;
    std::cout << "\t-pwd: " << password << std::endl;

    std::cout << "Current keys: " << std::endl;
    for (auto const& element : m) {
        std::cout << "\t-\"" << element.first << "\"" << std::endl;
    }

    if (m.find(username) != m.end()) {
        std::cout << "DataStore.Register() username existed." << std::endl;
        return FAILURE;
    } else {
        std::cout << "DataStore.Register() username didn't exist." << std::endl;
        m[username] = new UserEntry(password);
        return OK;
    }
}


Status DataStore::Login(const char* username, const char* password) {
    if (m.find(username) == m.end() || !m[username].ComparePassword(password)) {
        return FAILURE;
    } else {
        m[username].isLoggedIn = true;
        return OK;
    }
}


Status DataStore::Logout(const char* username) {
    if (m.find(username) == m.end()) {
        return FAILURE;
    } else {
        m[username].isLoggedIn = false; 
        return OK;
    }
}


bool DataStore::IsLoggedIn(const char* username) {
    if (m.find(username) == m.end() || !m[username].isLoggedIn) {
        return false;
    } else {
        return true;
    }
}

