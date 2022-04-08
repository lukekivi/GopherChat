#include "data_store.hpp"

Status DataStore::Register(const char* username, const char* password) {
    if (m.find(username) != m.end()) {
        return FAILURE;
    } else {
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

