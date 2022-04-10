#include "data_store.hpp"
#include <iostream> 

Status DataStore::Register(const char* username, const char* password) {
    std::string usr(username);

    if (IsInMap(usr)) {
        return FAILURE;
    } else {
        m[usr] = UserEntry(password);
        return OK;
    }
}


Status DataStore::Login(const char* username, const char* password) {
    std::string usr(username);

    if (!IsInMap(usr)) {
        return FAILURE;
    } 

    if (!m[usr].ComparePassword(password)) {
        return LOGGED_OUT;
    }

    if (IsLoggedIn(username)) {
        return OK;
    }

    // actually perform log in
    profiles.push_back(new Profile(username));
    std::cout << "LOGGING IN" << std::endl;
    FindIndexOf(usr);
    return LOGGED_IN;
}


Status DataStore::Logout(const char* username) {
    int index = FindIndexOf(username);
    if (index != -1) {
        delete profiles.at(index);
        profiles.erase(profiles.begin() + index);
    } else {
        return OK;
    }
    return LOGGED_OUT;
}


bool DataStore::IsLoggedIn(const char* username) {
    return FindIndexOf(username) != -1;
}


int DataStore::FindIndexOf(const char* username) {
    std::cout << "Find index of : " << username << std::endl;
    for (int i = 0; i < profiles.size(); i++) {
        std::cout << "\t- " << profiles.at(i)->GetUsername() << std::endl;
        if (strcmp(profiles.at(i)->GetUsername(), username) == 0) {
            return i;
        }
    }
    return -1;
}


bool DataStore::IsInMap(std::string username) {
     for (auto e : m) {
        if (e.first.compare(username) == 0) {
            return true;
        }
    }
    return false;
}