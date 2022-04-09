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
    for (int i = 0; i < profiles.size(); i++) {
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


int DataStore::SetUiConn(const char* username, int i) {
    std::cout << "DataStore.SetUiConn(): set connection for " << username << std::endl;
    int index = FindIndexOf(username);
    if (index != -1) {
        profiles.at(index)->SetUiConn(i);
        return 0;
    }
    return -1;
}


int DataStore::GetUiConn(const char* username) {
    int index = FindIndexOf(username);
    if (index != -1) {
        return profiles.at(index)->GetUiConn();
    }
    return -1;
}


int DataStore::AttemptLogoutConnLoss(int i) {
    for (int j = 0; j < profiles.size(); j++) {
        if (profiles.at(j)->IsInConns(i))  {
            delete profiles.at(j);
            profiles.erase(profiles.begin() + j);
            return 1;
        }
    }
    return 0;
}