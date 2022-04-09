#include "data_store.hpp"
#include <iostream> 

Status DataStore::Register(const char* username, const char* password) {
    std::string usr(username);

    for (auto e : m) {
        std::cout << e.first << std::endl;
    }

    if (m.find(usr) != m.end()) {
        return FAILURE;
    } else {
        m[usr] = UserEntry(password);
        return OK;
    }
}


Status DataStore::Login(const char* username, const char* password) {
    std::string usr(username);

    if (m.find(usr) == m.end() || !m[usr].ComparePassword(password)) {
        return FAILURE;
    } 

    if (IsLoggedIn(username)) {
        return FAILURE;
    }

    // actually perform log in
    profiles.push_back(new Profile(username));
    return OK;
}


Status DataStore::Logout(const char* username) {
    int index = FindIndexOf(username);
    if (index != -1) {
        delete profiles.at(index);
        profiles.erase(profiles.begin() + index);
    } 
    return OK;
}


bool DataStore::IsLoggedIn(const char* username) {
    return FindIndexOf(username) != -1;
}


int DataStore::FindIndexOf(const char* username) {
    for (int i = 0; i < profiles.size(); i++) {
        if (strcmp(profiles.at(i)->GetUsername(), username) == 0) {
            return i;
        }
    }
    return -1;
}
