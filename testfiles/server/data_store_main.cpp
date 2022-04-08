#include "../../app/server/data_store.hpp"
#include <string>
#include <iostream>

int main () {
    int numFails = 0;
    DataStore ds;

    /**
     * Register one user
     */
    std::string u1 = "Lucas";
    std::string p1 = "1234";
    std::string p11 = "12345";
    if (ds.Register(u1.c_str(), p1.c_str()) != OK) {
        std::cout << "Failed to register e1" << std::endl;
        numFails++;
    }

    if (ds.Register(u1.c_str(), p1.c_str()) != FAILURE) {
        std::cout << "Double registered e1" << std::endl;
        numFails++;
    }

    if (ds.Register(u1.c_str(), p11.c_str()) != FAILURE) {
        std::cout << "Double registered e1 with diff password" << std::endl;
        numFails++;
    }

    /**
     * Register a second user
     */
    std::string u2 = "Oscar";
    std::string p2 = "345";
    if (ds.Register(u2.c_str(), p2.c_str()) != OK) {
        std::cout << "Failed to register e2" << std::endl;
        numFails++;
    }

    if (ds.Register(u2.c_str(), p2.c_str()) != FAILURE) {
        std::cout << "Double registered e2" << std::endl;
        numFails++;
    }

    /**
     * Login user one
     */
    if (ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was not logged in" << std::endl;
        numFails++;
    }

    if (ds.Login(u1.c_str(), p1.c_str()) != OK) {
        std::cout << "Failed to login e1" << std::endl;
        numFails++;
    }

    if (!ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged in" << std::endl;
        numFails++;
    }

    /**
     * Logout user one
     */
    if (ds.Logout(u1.c_str()) != OK) {
        std::cout << "Failed to logout e1" << std::endl;
        numFails++;
    }

    if (ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged out" << std::endl;
        numFails++;
    }


    if (numFails != 0) {
        std::cout << "Failed " << numFails << " tests." << std::endl;
    } else std::cout << "ok" << std::endl;
}