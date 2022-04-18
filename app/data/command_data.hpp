// Author Name: Lucas Kivi
// Date: Mon Apr 18, 2022
// x500 UN: kivix019

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
        username = NULL;
    }

    CommandData(const CommandData& commandData) {
        command = commandData.command;
        args = commandData.args;
        numArgs = commandData.numArgs;
        username = commandData.username;
    }

    ~CommandData() {
        for (int i = 0; i < numArgs; i++) {
            delete[] args[i];
        }
        delete[] args;
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

    void setFileContents(char* fileContents) {
        args[numArgs-1] = fileContents;
    }

  private:
    Command command;
    char** args;
    int numArgs;
    char* username;
};

#endif