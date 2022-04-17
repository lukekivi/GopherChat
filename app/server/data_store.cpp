#include "data_store.hpp"

DataStore::DataStore(const char* registrarFilePath) {
    reg = new Registrar(registrarFilePath);
}

Status DataStore::Register(const char* username, const char* password) {
    UserEntry userEntry(username, password);
    
    if (reg->Register(userEntry)) {
        return OK;
    } else {
        return FAILURE;
    }
}


Status DataStore::Login(const char* username, const char* password) {
    UserEntry* userEntry = reg->GetUser(username);

    if (userEntry == NULL) {
        return FAILURE;
    } 

    if (!userEntry->ComparePassword(password)) {
        delete userEntry;
        return LOGGED_OUT;
    }

    if (IsLoggedIn(username)) {
        delete userEntry;
        return OK;
    }

    // actually perform log in
    profiles.push_back(Profile(username));
    delete userEntry;
    return LOGGED_IN;
}


Status DataStore::Logout(const char* username) {
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


int DataStore::Enqueue(const char* username, ByteBody* byteBody) {
    int index = FindIndexOf(username);

    if (index == -1) {
        if (reg->GetUser(username) == NULL) {
            return -2;
        } else {
            return -1;
        }
    } 

    profiles.at(index).EnqueueBody(new ByteBody(byteBody));
    return 0;
}


ByteBody* DataStore::Dequeue(const char* username) {
    int index = FindIndexOf(username);

    if (index == -1) {
        return NULL;
    } 

    return profiles.at(index).DequeueBody();
}


int DataStore::EnqueueAllExcept(const char* username, ByteBody* byteBody) {
    if (profiles.size() == 0) {
        return -1;
    }
    for (int i = 0; i < profiles.size(); i++) {
        if (strcmp(profiles.at(i).GetUsername(), username) != 0) {
            profiles.at(i).EnqueueBody(new ByteBody(byteBody));
        }
    }
    return 0;
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


std::vector<std::string> DataStore::GetSignedInUsers() {
    std::vector<std::string> usernames;
    for (int i = 0; i < profiles.size(); i++) {
        std::string username(profiles.at(i).GetUsername());
        usernames.push_back(username);
    }
    return usernames;
}

bool DataStore::IsUserRegistered(const char* username) {
    return reg->IsIn(username);
}