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
    std::cout << "DS: Logging in: " << username << std::endl;
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
    profiles.push_back(Profile(username));
    return LOGGED_IN;
}


Status DataStore::Logout(const char* username) {
    std::cout << "DS: Logging out: " << username << std::endl;
    int index = FindIndexOf(username);
    if (index != -1) {
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
        if (strcmp(profiles.at(i).GetUsername(), username) == 0) {
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


bool DataStore::Enqueue(const char* username, const BYTE* msg, int len) {
    int index = FindIndexOf(username);

    if (index == -1) {
        return false;
    } 

    profiles.at(index).EnqueueBody(BuildByteBody(msg, len));
    return true;
}


ByteBody* DataStore::Dequeue(const char* username) {
    int index = FindIndexOf(username);

    if (index == -1) {
        return NULL;
    } 

    return profiles.at(index).DequeueBody();
}


void DataStore::EnqueueAllExcept(const char* username, const BYTE* msg, int len) {
    for (int i = 0; i < profiles.size(); i++) {
        if (strcmp(profiles.at(i).GetUsername(), username) != 0) {
            profiles.at(i).EnqueueBody(BuildByteBody(msg, len));
        }
    }
}


std::vector<std::string> DataStore::GetUsersWithMsgs() {
    std::vector<std::string> usernames;
    for (int i = 0; i < profiles.size(); i++) {
        if (profiles.at(i).HasMessages()) {
            std::string username(profiles.at(i).GetUsername());
            usernames.push_back(username);
        }       
    }
    return usernames;
}


ByteBody* DataStore::BuildByteBody(const BYTE* msg, int len) {
    BYTE* message = new BYTE[len];
    for (int i = 0; i < len; i++) {
        message[i] = msg[i];
    }

    ByteBody* body = new ByteBody(message, len);

    return body;
}