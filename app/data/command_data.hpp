#ifndef COMMANDS_DATA_H
#define COMMANDS_DATA_H

#include "../utils/utils.hpp"
#include <iostream>

/**
 * Data class to hold raw data about commands read in.
 */
class CommandData {
  public:
    CommandData(Command command_, char** args_, int numArgs_) {
        command = command_;
        args = args_;    
        numArgs = numArgs_;
    }

    CommandData(const CommandData& commandData) {
        command = commandData.command;
        args = commandData.args;
        numArgs = commandData.numArgs;
        username = commandData.username;
        std::cout << "CALLED COPY CONSTRUCTOR" << std::endl;
    }

    ~CommandData() {
        int index = 1;
        std::cout << "Got here: " << index++ << std::endl;
        for (int i = 0; i < numArgs; i++) {
            std::cout << "Got here: " << index++ << std::endl;
            std::cout << args[i] << std::endl;
            delete[] args[i];
        }
        std::cout << "Got here: " << index++ << std::endl;
        delete[] args;
        std::cout << "Got here: " << index++ << std::endl;
        delete[] username;
    }

    Command getCommand() {
        return command;
    }

    char** getArgs() {
        return args;
    }

    int getNumArgs() {
        return numArgs;
    }

    char* getUsername() {
        return username;
    }

    void setUsername(char* username_) {
        username = username_;
    }

  private:
    Command command;
    char** args;
    int numArgs;
    char* username = NULL;
};

#endif