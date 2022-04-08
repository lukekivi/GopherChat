#include "data_store.hpp"
#include <iostream> 

Status DataStore::Register(const char* username, const char* password) {
    // std::cout << "DataStore.Register()\n\t-user: " <<  username << std::endl;
    // std::cout << "\t-pwd: " << password << std::endl;

    // std::cout << "Current keys: " << std::endl;
    // for (auto const& element : m) {
    //     std::cout << "\t-\"" << element.first << "\"" << std::endl;
    // }

    std::string usr(username);

    if (m.find(usr) != m.end()) {
        std::cout << "DataStore.Register() username existed." << std::endl;
        return FAILURE;
    } else {
        std::cout << "DataStore.Register() username didn't exist." << std::endl;
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

