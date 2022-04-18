// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

#ifndef REGISTRAR_H
#define REGISTRAR_H

#include "../data/user_entry.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

#define STARTING_POS 0

class Registrar {
  public:
    Registrar(const char* filePath) {
        file.open(filePath, std::ios::out | std::ios::app | std::ios::in);
    }


    ~Registrar() { file.close(); }


    UserEntry* GetUser(const char* username) {
        UserEntry* results = NULL;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string user, pwd;

            if (!(iss >> user)) {                // read in username
                continue;                        // line is empty

            } else if (user.compare(username) == 0) {
                if (!(iss >> pwd)) {            // read in password
                    std::cerr << "ERROR: Invalid Entry in registrar file. Missing a password." << std::endl;
                    exit(EXIT_FAILURE);
                }
                results = new UserEntry(username, pwd.c_str());
                break;
            }
        }
        ResetFile();
        return results;
    }


    bool Register(UserEntry userEntry) {
        if (IsIn(userEntry.GetUsername())) {
            return false;
        } else {
            file << userEntry.GetUsername() << " " << userEntry.GetPassword() << std::endl;
            ResetFile();
        }
        return true;
    }

    
    bool IsIn(const char* username) {
        bool results = false;
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string user, pwd;

            if (!(iss >> user)) {                // read in username
                continue;                        // line is empty
            }
            if (user.compare(username) == 0) {   // username already registered
                results = true;
                break;
            }
        }
        ResetFile();
        return results;
    }


  private:
    void ResetFile() {
        file.clear();
        file.seekg(STARTING_POS, std::ios::beg);
    }

    std::fstream file;
};

#endif