#ifndef COMMANDS_DATA_H
#define COMMANDS_DATA_H

#include "../utils/utils.hpp"

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

    ~CommandData() {
        delete[] args;
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

  private:
    Command command;
    char** args;
    int numArgs;
};

#endif