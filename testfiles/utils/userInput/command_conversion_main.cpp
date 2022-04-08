#include <iostream>
#include "../../../app/utils/userInput/script_reader.hpp"
#include "../../../app/utils/log.hpp"
#include "../../../app/utils/socket/socket_messenger.hpp"

const char* LOG_PATH = "log/log_userInput.txt";

int TestScriptRead();
int TestCommandData();

int main() {
    int numFailed = 0;

    numFailed += TestScriptRead();

    if (numFailed == 0) {
        std::cout << "ok" << std::endl;
    } else {
        std::cout << numFailed << " tests failed" << std::endl;
    }

    return 0;
}

int TestScriptRead() {
    int numFailed = 0;

    Log log(LOG_PATH);
    ScriptReader scriptReader("testcases/userInputTest.txt", &log);
    SocketMessenger sockMsgr(&log);

    std::vector<CommandData*> commands = scriptReader.getCommands();

    /** REGISTER **/
    CommandData* commandDataOne = sockMsgr.ByteToCommandData(sockMsgr.CommandDataToByte(commands.at(0)));
    Command commandOne = commandDataOne->getCommand();
    Command expectedCommandOne = REGISTER;
    if (commandOne != expectedCommandOne) {
        std::cout << "expectedCommandOne: " << commandOne << "was not " << expectedCommandOne << std::endl;
    }

    int numArgsOne = commandDataOne->getNumArgs();
    if (numArgsOne != 2) { 
        std::cout << "NumArgsOne: " << numArgsOne << " was not 2" << std::endl;
        numFailed++;
    }

    char* usernameOne = commandDataOne->getArgs()[0];
    std::string expectedUserNameOne = "mario";
    if (strcmp(usernameOne, expectedUserNameOne.c_str()) != 0) {
        std::cout << "usernameOne: " << usernameOne << " was not " << expectedUserNameOne << std::endl;
        numFailed++;
    }

    char* passwordOne = commandDataOne->getArgs()[1];
    std::string expectedPasswordOne = "foobar00";
    if (strcmp(passwordOne, expectedPasswordOne.c_str()) != 0) {
        std::cout << "passwordOne: " << passwordOne << " was not " << expectedPasswordOne << std::endl;
        numFailed++;
    }

    /** LOGIN AND NULL USER **/
    CommandData* commandDataTwo = sockMsgr.ByteToCommandData(sockMsgr.CommandDataToByte(commands.at(1)));
    Command commandTwo = commandDataTwo->getCommand();
    Command expectedCommandTwo = LOGIN;
    if (commandTwo != expectedCommandTwo) {
        std::cout << "expectedCommandTwo: " << commandTwo << "was not " << expectedCommandTwo << std::endl;
    }

    int numArgsTwo = commandDataTwo->getNumArgs();
    if (numArgsTwo != 2) { 
        std::cout << "NumArgsTwo: " << numArgsTwo << " was not 2" << std::endl;
        numFailed++;
    }

    char* usernameTwo = commandDataTwo->getArgs()[0];
    std::string expectedUserNameTwo = "mario";
    if (strcmp(usernameTwo, expectedUserNameTwo.c_str()) != 0) {
        std::cout << "usernameTwo: " << usernameTwo << " was not " << expectedUserNameTwo << std::endl;
        numFailed++;
    }

    char* passwordTwo = commandDataTwo->getArgs()[1];
    std::string expectedPasswordTwo = "foobar00";
    if (strcmp(passwordTwo, expectedPasswordTwo.c_str()) != 0) {
        std::cout << "passwordTwo: " << passwordTwo << " was not " << expectedPasswordTwo << std::endl;
        numFailed++;
    }
    
    if (commandDataTwo->getUsername() != NULL) {
        std::cout << "CommandDataTwo.username should have been NULL" << std::endl;
        numFailed++;
    }

    /** LOGOUT WITH USER **/
    CommandData* commandDataThree = sockMsgr.ByteToCommandData(sockMsgr.CommandDataToByte(commands.at(2)));
    commandDataThree->setUsername("kivix");
    Command commandThree = commandDataThree->getCommand();
    Command expectedCommandThree = LOGOUT;
    if (commandThree != expectedCommandThree) {
        std::cout << "commandThree: " << commandThree << "was not " << expectedCommandThree << std::endl;
    }

    int numArgsThree = commandDataThree->getNumArgs();
    if (numArgsThree != 0) { 
        std::cout << "NumArgsThree: " << numArgsThree << " was not 2" << std::endl;
        numFailed++;
    }

    char** argsThree = commandDataThree->getArgs();
    if (argsThree != NULL) {
        std::cout << "argsThree was not null" << std::endl;
        numFailed++;
    }

    char* sendingUserNameThree = commandDataThree->getUsername();
    std::string expectedSendingUserNameThree = "kivix";
    if (strcmp(sendingUserNameThree, expectedSendingUserNameThree.c_str()) != 0) {
        std::cout << "sendingUserNameThree: " <<  sendingUserNameThree << " was not " << expectedSendingUserNameThree << std::endl;
        numFailed++;
    }

    // /** SEND **/
    // Command commandFour = commands.at(3)->getCommand();
    // Command expectedCommandFour = SEND;
    // if (commandFour != expectedCommandFour) {
    //     std::cout << "expectedCommandFour: " << commandFour << " was not " << expectedCommandFour << std::endl;
    //     numFailed++;
    // }

    // int numArgsFour = commands.at(3)->getNumArgs();
    // if (numArgsFour != 1) { 
    //     std::cout << "NumArgsFour: " << numArgsFour << " was not 1" << std::endl;
    //     numFailed++;
    // }

    // char* msgFour = commands.at(3)->getArgs()[0];
    // std::string expectedMsgFour = "hello there, how art thou?";
    // if (strcmp(msgFour, expectedMsgFour.c_str()) != 0) {
    //     std::cout << "msgFour: \"" << msgFour << "\" was not \"" << expectedMsgFour << "\"" <<  std::endl;
    //     numFailed++;
    // }


    // /** SEND_TO **/
    // Command commandFive = commands.at(4)->getCommand();
    // Command expectedCommandFive = SEND_TO;
    // if (commandFive != expectedCommandFive) {
    //     std::cout << "expectedCommandFive: " << commandFive << " was not " << expectedCommandFive << std::endl;
    //     numFailed++;
    // }

    // int numArgsFive = commands.at(4)->getNumArgs();
    // if (numArgsFive != 2) { 
    //     std::cout << "NumArgsFive: " << numArgsFive << " was not 2" << std::endl;
    //     numFailed++;
    // }

    // char* usernameFive = commands.at(4)->getArgs()[0];
    // std::string expectedUsernameFive = "lucas";
    // if (strcmp(usernameFive, expectedUsernameFive.c_str()) != 0) {
    //     std::cout << "usernameFive: " << usernameFive << " was not " << expectedUsernameFive << std::endl;
    //     numFailed++;
    // }

    // char* msgFive = commands.at(4)->getArgs()[1];
    // std::string expectedMsgFive = "I would like to say hello.";
    // if (strcmp(msgFive, expectedMsgFive.c_str()) != 0) {
    //     std::cout << "msgFive: \"" << msgFive << "\" was not \"" << expectedMsgFive << "\"" << std::endl;
    //     numFailed++;
    // }


    // /** SEND_ANON **/
    // Command commandSix = commands.at(5)->getCommand();
    // Command expectedCommandSix = SEND_ANON;
    // if (commandSix != expectedCommandSix) {
    //     std::cout << "expectedCommandSix: " << commandSix << " was not " << expectedCommandSix << std::endl;
    //     numFailed++;
    // }

    // int numArgsSix = commands.at(5)->getNumArgs();
    // if (numArgsSix != 1) { 
    //     std::cout << "NumArgsSix: " << numArgsSix << " was not 1" << std::endl;
    //     numFailed++;
    // }

    // char* msgSix = commands.at(5)->getArgs()[0];
    // std::string expectedMsgSix = "this is an anonymous message.";
    // if (strcmp(msgSix, expectedMsgSix.c_str()) != 0) {
    //     std::cout << "msgSix: \"" << msgSix << "\" was not \"" << expectedMsgSix << "\"" <<  std::endl;
    //     numFailed++;
    // }


    // /** SEND_TO_ANON **/
    // Command commandSeven = commands.at(6)->getCommand();
    // Command expectedCommandSeven = SEND_TO_ANON;
    // if (commandSeven != expectedCommandSeven) {
    //     std::cout << "expectedCommandSeven: " << commandSeven << " was not " << expectedCommandSeven << std::endl;
    //     numFailed++;
    // }

    // int numArgsSeven = commands.at(6)->getNumArgs();
    // if (numArgsSeven != 2) { 
    //     std::cout << "NumArgsSeven: " << numArgsSeven << " was not 2" << std::endl;
    //     numFailed++;
    // }

    // char* usernameSeven = commands.at(6)->getArgs()[0];
    // std::string expectedUsernameSeven = "steph";
    // if (strcmp(usernameSeven, expectedUsernameSeven.c_str()) != 0) {
    //     std::cout << "usernameSeven: " << usernameSeven << " was not " << expectedUsernameSeven << std::endl;
    //     numFailed++;
    // }

    // char* msgSeven = commands.at(6)->getArgs()[1];
    // std::string expectedMsgSeven = "hey mom, you smell bad.";
    // if (strcmp(msgSeven, expectedMsgSeven.c_str()) != 0) {
    //     std::cout << "msgSeven: \"" << msgSeven << "\" was not \"" << expectedMsgSeven << "\"" << std::endl;
    //     numFailed++;
    // }


    // /** SEND_FILE **/
    // Command commandEight = commands.at(7)->getCommand();
    // Command expectedCommandEight = SEND_FILE;
    // if (commandEight != expectedCommandEight) {
    //     std::cout << "expectedCommandEight: " << commandEight << " was not " << expectedCommandEight << std::endl;
    //     numFailed++;
    // }

    // int numArgsEight = commands.at(7)->getNumArgs();
    // if (numArgsEight != 1) { 
    //     std::cout << "NumArgsEight: " << numArgsEight << " was not 1" << std::endl;
    //     numFailed++;
    // }

    // char* fileEight = commands.at(7)->getArgs()[0];
    // std::string expectedFileEight = "userInputTestFileOne.txt";
    // if (strcmp(fileEight, expectedFileEight.c_str()) != 0) {
    //     std::cout << "fileEight: \"" << fileEight << "\" was not \"" << expectedFileEight << "\"" <<  std::endl;
    //     numFailed++;
    // }


    // /** SEND_FILE_TO **/
    // Command commandNine = commands.at(8)->getCommand();
    // Command expectedCommandNine = SEND_FILE_TO;
    // if (commandNine != expectedCommandNine) {
    //     std::cout << "expectedCommandNine: " << commandNine << " was not " << expectedCommandNine << std::endl;
    //     numFailed++;
    // }

    // int numArgsNine = commands.at(8)->getNumArgs();
    // if (numArgsNine != 2) { 
    //     std::cout << "NumArgsNine: " << numArgsNine << " was not 2" << std::endl;
    //     numFailed++;
    // }

    // char* usernameNine = commands.at(8)->getArgs()[0];
    // std::string expectedUsernameNine = "evan";
    // if (strcmp(usernameNine, expectedUsernameNine.c_str()) != 0) {
    //     std::cout << "usernameNine: \"" << usernameNine << "\" was not \"" << expectedUsernameNine << "\"" <<  std::endl;
    //     numFailed++;
    // }

    // char* fileNine = commands.at(8)->getArgs()[1];
    // std::string expectedFileNine = "userInputTestFileTwo.txt";
    // if (strcmp(fileNine, expectedFileNine.c_str()) != 0) {
    //     std::cout << "fileNine: \"" << fileNine << "\" was not \"" << expectedFileNine << "\"" <<  std::endl;
    //     numFailed++;
    // }


    // /** LIST **/
    // Command commandTen = commands.at(9)->getCommand();
    // Command expectedCommandTen = LIST;
    // if (commandTen != expectedCommandTen) {
    //     std::cout << "expectedCommandTen: " << commandTen << "was not " << expectedCommandTen << std::endl;
    // }

    // int numArgsTen = commands.at(9)->getNumArgs();
    // if (numArgsTen != 0) { 
    //     std::cout << "NumArgsTen: " << numArgsTen << " was not 2" << std::endl;
    //     numFailed++;
    // }

    // char** argsTen = commands.at(9)->getArgs();
    // if (argsTen != NULL) {
    //     std::cout << "argsTen was not null" << std::endl;
    //     numFailed++;
    // }


    // /** DELAY **/
    // Command commandEleven = commands.at(10)->getCommand();
    // Command expectedCommandEleven = DELAY;
    // if (commandEleven != expectedCommandEleven) {
    //     std::cout << "expectedCommandEleven: " << commandEleven << " was not " << expectedCommandEleven << std::endl;
    //     numFailed++;
    // }

    // int numArgsEleven = commands.at(10)->getNumArgs();
    // if (numArgsEleven != 1) { 
    //     std::cout << "NumArgsEleven: " << numArgsEleven << " was not 1" << std::endl;
    //     numFailed++;
    // }

    // char* numEleven = commands.at(10)->getArgs()[0];
    // std::string expectedNumEleven = "3";
    // if (strcmp(numEleven, expectedNumEleven.c_str()) != 0) {
    //     std::cout << "numEleven: \"" << numEleven << "\" was not \"" << expectedNumEleven << "\"" <<  std::endl;
    //     numFailed++;
    // }


    return numFailed;
}