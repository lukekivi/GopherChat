#ifndef SCRIPT_READER_H
#define SCRIIPT_READER_H

#include <fstream>
#include "../utils.hpp"
#include "../../data/command_data.hpp"
#include "../log.hpp"
#include <vector>

/**
 * Read in command script.
 */
class ScriptReader {
  public:
    ScriptReader(const char* filePath, Log* log) { 
        file.open(filePath); 
        this->log = new Log(log);
    };
    ~ScriptReader() { 
        file.close(); 
        delete log;
    }

    std::vector<CommandData*> getCommands();

    CommandData* readRegister(std::istringstream& iss);
    CommandData* readLogin(std::istringstream& iss);
    CommandData* readLogout();
    CommandData* readSend(std::istringstream& iss, bool isAnon);
    CommandData* readSendTo(std::istringstream& iss, bool isAnon);
    CommandData* readSendFile(std::istringstream& iss);
    CommandData* readSendFileTo(std::istringstream& iss);
    CommandData* readList();
    CommandData* readDelay(std::istringstream& iss);
  
    bool isUsernameValid(std::string username);
    bool isPwdValid(std::string pwd);
    bool isAlphaNumeric(std::string str);
    bool isMessageValid(std::string msg);
    std::string readTrailingMsg(std::istringstream& iss);
    
  private:
    std::ifstream file;  
    Log* log; 
};

#endif