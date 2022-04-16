#include "script_reader.hpp"
#include <string>
#include <sstream>
#define MIN_N 1
#define MAX_N 9999

/**
 * Get list of commands for from file.
 */
std::vector<CommandData*> ScriptReader::getCommands() {
    if (!file.is_open()) {
        log->Error("Failed to open file.");
        exit(EXIT_FAILURE);
    }

    std::vector<CommandData*> commands;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string command;

        if(!(iss >> command)) {
            continue;
        }

        if (command.compare("REGISTER") == 0) { 
            commands.push_back(readRegister(iss)); 

        } else if (command.compare("LOGIN") == 0) {  
            commands.push_back(readLogin(iss));

        } else if (command.compare("LOGOUT") == 0) {
            commands.push_back(readLogout());

        } else if (command.compare("SEND") == 0) {
            commands.push_back(readSend(iss, false));

        } else if (command.compare("SEND2") == 0) {
            commands.push_back(readSendTo(iss, false));

        } else if (command.compare("SENDA") == 0) {
            commands.push_back(readSend(iss, true));

        } else if (command.compare("SENDA2") == 0) {
            commands.push_back(readSendTo(iss, true));

        } else if (command.compare("SENDF") == 0) {
            commands.push_back(readSendFile(iss));

        } else if (command.compare("SENDF2") == 0) {
            commands.push_back(readSendFileTo(iss));

        } else if (command.compare("LIST") == 0) {
            commands.push_back(readList());

        } else if (command.compare("DELAY") == 0) {
            commands.push_back(readDelay(iss));

        } else {
            log->Error("Invalid Command: %s", command.c_str());
            exit(EXIT_FAILURE);
        }
    }
    return commands;
}


/**
 * Handle extracting and building REGISTER command data stucture
 */
CommandData* ScriptReader::readRegister(std::istringstream& iss) {
    const char* FUN_ID = "ScriptReader.readRegister()";
    /**
     * finish reading line
     */
    std::string _username, _password;

    if (!(iss >> _username >> _password)) {
        log->Error("%s - expected a username and password.", FUN_ID);
        exit(EXIT_FAILURE);
    }
    
    /**
     * Perform format checking
     */
    if (!isUsernameValid(_username)) {
        std::cout << "Invalid username. See log for more details." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!isPwdValid(_password)) {
        std::cout << "Invalid password. See log for more details." << std::endl;
        exit(EXIT_FAILURE);
    }

    /**
     * Build command data structure
     */
    char* username = new char[_username.length()+1];
    char* password = new char[_password.length()+1];

    strcpy(username, _username.c_str());
    strcpy(password, _password.c_str());

    char** args = new char*[2];
    args[0] = username;
    args[1] = password;

    return new CommandData(REGISTER, args, 2);
}


/**
 * Handle extracting and building a LOGIN command data structure
 */
CommandData* ScriptReader::readLogin(std::istringstream& iss) {
    const char* FUN_ID = "ScriptReader.readLogin()";
    /**
     * finish reading line
     */
    std::string _username, _password;

    if (!(iss >> _username >> _password)) {
        log->Error("%s - expected a username and password.", FUN_ID);
        exit(EXIT_FAILURE);
    }

    /**
     * Perform format checking
     */
    if (!isUsernameValid(_username)) {
        std::cout << "Invalid username. See log for more details." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!isPwdValid(_password)) {
        std::cout << "Invalid password. See log for more details." << std::endl;
        exit(EXIT_FAILURE);
    }

    /**
     * Build command data structure
     */
    char* username = new char[_username.length()+1];
    char* password = new char[_password.length()+1];

    strcpy(username, _username.c_str());
    strcpy(password, _password.c_str());

    char** args = new char*[2];
    args[0] = username;
    args[1] = password;

    return new CommandData(LOGIN, args, 2);
}


/**
 * Build LOGOUT command data structure
 */
CommandData* ScriptReader::readLogout() {
    return new CommandData(LOGOUT, NULL, 0);
}


/**
 * Extract and build SEND command data structure
 */
CommandData* ScriptReader::readSend(std::istringstream&iss, bool isAnon) {
    const char* FUN_ID = "ScriptReader.readSend()";
    /**
     * finish reading line
     */
    std::string _msg = readTrailingMsg(iss);

    if (_msg.length() == 0) {
        log->Error("%s - expected a non empty message.", FUN_ID);
        exit(EXIT_FAILURE);
    }

    /**
     * Build command data structure
     */
    char* msg = new char[_msg.length()+1];

    strcpy(msg, _msg.c_str());

    char** args = new char*[1];
    args[0] = msg;

    return new CommandData((isAnon ? SEND_ANON : SEND), args, 1);
}


/**
 * Extract and build SENT_TO command data structure
 */
CommandData* ScriptReader::readSendTo(std::istringstream& iss, bool isAnon) {
    const char* FUN_ID = "ScriptReader.readSendTo()";
    /**
     * finish reading line
     */
    std::string _username, _msg;

    if (!(iss >> _username)) {
        log->Error("%s - expected a username", FUN_ID);
        exit(EXIT_FAILURE);
    }

    _msg = readTrailingMsg(iss);

    /**
     * Perform format checking
     */
    if (!isMessageValid(_msg)) {
        std::cout << "Invalid message. See log for more details." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!isUsernameValid(_username)) {
        std::cout << "Invalid username. See log for more details." << std::endl;
        exit(EXIT_FAILURE);
    }

    /**
     * Build command data structure
     */
    char* username = new char[_username.length()+1];
    char* msg = new char[_msg.length()+1];

    strcpy(username, _username.c_str());
    strcpy(msg, _msg.c_str());

    char** args = new char*[2];
    args[0] = username;
    args[1] = msg;

    return new CommandData((isAnon ? SEND_TO_ANON : SEND_TO), args, 2);
}


/**
 * Extract and build SEND_FILE command data structure
 */
CommandData* ScriptReader::readSendFile(std::istringstream& iss) {
    const char* FUN_ID = "ScriptReader.readSendFile()";
    /**
     * finish reading line
     */
    std::string _fileName;

    if (!(iss >> _fileName)) {
        log->Error("%s - expected a file name", FUN_ID);
        exit(EXIT_FAILURE);
    }

    /**
     * Build command data structure 
     */
    char* fileName = new char[_fileName.length()+1];

    strcpy(fileName, _fileName.c_str());

    char** args = new char*[2];
    args[0] = fileName;
    args[1] = NULL;

    return new CommandData(SEND_FILE, args, 2);
}


/**
 * Extract and build SEND_FILE_TO command data structure
 */
CommandData* ScriptReader::readSendFileTo(std::istringstream& iss) {
    const char* FUN_ID = "ScriptReader.readSendFileTo()";
    /**
     * finish reading line
     */
    std::string _username, _fileName;

    if (!(iss >> _username >> _fileName)) {
        log->Error("%s - expected a username and a file name", FUN_ID);
        exit(EXIT_FAILURE);
    }

    /**
     * Perform format checking
     */
    if (!isUsernameValid(_username)) {
        std::cout << "Invalid username. See log for more details." << std::endl;
        exit(EXIT_FAILURE);
    }

    /**
     * Build command data structure
     */
    char* username = new char[_username.length()+1];
    char* fileName = new char[_fileName.length()+1];

    strcpy(username, _username.c_str());
    strcpy(fileName, _fileName.c_str());

    char** args = new char*[3];
    args[0] = username;
    args[1] = fileName;
    args[2] = NULL;

    return new CommandData(SEND_FILE_TO, args, 3);
}


/**
 * Build LIST command data structure
 */
CommandData* ScriptReader::readList() {
    return new CommandData(LIST, NULL, 0);
}


/**
 * Build DELAY command data structure
 */
CommandData* ScriptReader::readDelay(std::istringstream& iss) {
    const char* FUN_ID = "ScriptReader.readDelay()";
    /**
     * finish reading line
     */
    int _num;

    if (!(iss >> _num)) {
        log->Error("%s - expected an integer to delay length", FUN_ID);
        exit(EXIT_FAILURE);
    }

    if (_num < MIN_N || _num > MAX_N) {
        log->Error("%s - expected delay between %d and %d", FUN_ID, MIN_N-1, MAX_N+1);
        exit(EXIT_FAILURE);
    }

    std::string tempNum = std::to_string(_num);

    char* num = new char[tempNum.length()+1];
    strcpy(num, tempNum.c_str());

    char** args = new char*[1];
    args[0] = num;

    return new CommandData(DELAY, args, 1);
}


/**
 * Helper function to see if string is composed exclusively of chars and digits
 */
bool ScriptReader::isAlphaNumeric(std::string str) {
    for (int i=0; i < str.length(); i++) {
        if (!std::isalnum(str[i])) {
            return false;
        }
    }
    return true;
}


/**
 * Check to see if username is valid
 */
bool ScriptReader::isUsernameValid(std::string username) {
    const char* FUN_ID = "ScriptReader.isUserNameValid()";
    int usrnameLen = username.length();
    if (usrnameLen < MIN_USRNAME_SIZE || usrnameLen > MAX_USRNAME_SIZE) {
        log->Error("%s - username length should be %d-%d characters", FUN_ID, MIN_USRNAME_SIZE, MAX_USRNAME_SIZE);
        return false;
    }

    if (!isAlphaNumeric(username)) {
        log->Error("%s - username must be composed of numbers anc characters: %s", FUN_ID, username.c_str());
        return false;
    }
    return true;
}

/**
 * Check to see if password is valid
 */
bool ScriptReader::isPwdValid(std::string pwd) {
    const char* FUN_ID = "ScriptReader.isPwdValid()";
    int pwdLen = pwd.length();
    if (pwdLen < MIN_PWD_SIZE || pwdLen > MAX_PWD_SIZE) {
        log->Error("%s - password length should be %d-%d characters", FUN_ID, MIN_PWD_SIZE, MAX_PWD_SIZE);
        return false;
    }

    if (!isAlphaNumeric(pwd)) {
        log->Error("%s - password must be composed of numbers anc characters: %s", FUN_ID, pwd.c_str());
        return false;
    }
    return true;
}


/**
 * Check to see if message is valid
 */
bool ScriptReader::isMessageValid(std::string msg) {
    std::string FUN_ID = "ScriptReader.isMessagValid()";
    int msgLen = msg.length();
    if (msgLen <= 0) {
        log->Error("%s - message should not be empty.", FUN_ID.c_str());
        return false;
    }

    if (msgLen > MAX_MSG_SIZE) {
        log->Error("%s - message should not be longer than %d bytes", MAX_MSG_SIZE);
        return false;
    }
    return true;
}


/**
 * Helper to read in the rest of a line (trailing message)
 */
std::string ScriptReader::readTrailingMsg(std::istringstream& iss) {
    std::string msg;
    std::getline(iss >> std::ws, msg);
    return msg;
}