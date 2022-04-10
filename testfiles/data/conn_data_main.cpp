#include "../../app/data/conn_data.hpp"
#include <iostream>

#define CONN_DATA_SIZE 4

int main() {
    int numFails = 0;

    ConnData connData[CONN_DATA_SIZE];
    std::string username = "Lucas";
    // for (int i = 0; i < CONN_DATA_SIZE; i++) {
    //     memset(&connData, REG, sizeof(ConnData));
    // }

    for (int i = 0; i < CONN_DATA_SIZE; i++) {
        if (connData[i].GetConnType() != REG) {
            std::cout << "connData[" << i << "] was not REG" << std::endl;
            numFails++;
        }
        if (strcmp(connData[i].GetUsername(), "") != 0) {
            std::cout << "connData[" << i << "] was not an empty string" << std::endl;
            numFails++;
        }
    }

    connData[2].SetConnType(FIL);
    connData[2].SetUsername(username.c_str());

    if (connData[2].GetConnType() != FIL) {
        std::cout << "connData[" << 2 << "] was not FIL" << std::endl;
        numFails++;
    }
    if (strcmp(connData[2].GetUsername(), username.c_str()) != 0) {
        std::cout << "connData[" << 2 << "] was not " << username << std::endl;
        numFails++;
    }

    memmove(connData + 1, connData + 1 + 1, (CONN_DATA_SIZE - 1) * sizeof(ConnData));

    if (connData[1].GetConnType() != FIL) {
        std::cout << "connData[" << 1 << "] was not FIL" << std::endl;
        numFails++;
    }
    if (strcmp(connData[1].GetUsername(), username.c_str()) != 0) {
        std::cout << "connData[" << 1 << "] was not " << username << std::endl;
        numFails++;
    }

    if (numFails != 0) {
        std::cout << "Failed " << numFails << " tests." << std::endl;
    } else std::cout << "ok" << std::endl;

    return 0;
}