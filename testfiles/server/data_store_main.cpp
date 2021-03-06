#include "../../app/server/data_store.hpp"
#include "../../app/utils/log.hpp"
#include "../../app/utils/socket/socket_messenger.hpp"
#include "../../app/data/byte_body.hpp"
#include <string>
#include <vector>
#include <iostream>

const char* LOG_PATH = "log/log_data_store.txt";
const char* REGISTRAR_FILE_PATH = "testfiles/server/registrar.txt";

int BasicLoginLogout();
int MixedLoginLogout();
int DequeTests();
int GroupEnqueueTests();

int main () {
    int numFails = 0;

    numFails += BasicLoginLogout();
    numFails += MixedLoginLogout();
    numFails += DequeTests();
    numFails += GroupEnqueueTests();

    if (numFails != 0) {
        std::cout << "Failed " << numFails << " tests." << std::endl;
    } else std::cout << "ok" << std::endl;    
}


int BasicLoginLogout() {
    DataStore ds(REGISTRAR_FILE_PATH);
    int numFails = 0;
    /**
     * Register one user
     */
    std::string u1 = "Lucas";
    std::string p1 = "1234";
    std::string p11 = "12345";
    if (ds.Register(u1.c_str(), p1.c_str()) != OK) {
        std::cout << "Failed to register e1 !!! MAKE SURE YOU DELETED \"testfiles/server/registrar.txt\" prior to running the test" << std::endl;
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

    if (ds.Login(u1.c_str(), p1.c_str()) != LOGGED_IN) {
        std::cout << "Failed to login e1" << std::endl;
        numFails++;
    }

    if (!ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged in" << std::endl;
        numFails++;
    }

    // /**
    //  * Logout user one
    //  */
    if (ds.Logout(u1.c_str()) != LOGGED_OUT) {
        std::cout << "Failed to logout e1" << std::endl;
        numFails++;
    }

    if (ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged out" << std::endl;
        numFails++;
    }


    /**
     * Login user two
     */
    if (ds.IsLoggedIn(u2.c_str())) {
        std::cout << "Failed to see that e2 was not logged in" << std::endl;
        numFails++;
    }

    if (ds.Login(u2.c_str(), p2.c_str()) != LOGGED_IN) {
        std::cout << "Failed to login e2" << std::endl;
        numFails++;
    }

    if (!ds.IsLoggedIn(u2.c_str())) {
        std::cout << "Failed to see that e2 was logged in" << std::endl;
        numFails++;
    }



    /**
     * Logout user two
     */
    if (ds.Logout(u2.c_str()) != LOGGED_OUT) {
        std::cout << "Failed to logout e2" << std::endl;
        numFails++;
    }

    if (ds.IsLoggedIn(u2.c_str())) {
        std::cout << "Failed to see that e2 was logged out" << std::endl;
        numFails++;
    }

    return numFails;
}


int MixedLoginLogout() {
    DataStore ds(REGISTRAR_FILE_PATH);
    int numFails = 0;
    /*
     * Register one user
     */
    std::string u1 = "rissa";
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
    std::string u2 = "evan";
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

    if (ds.Login(u1.c_str(), p1.c_str()) != LOGGED_IN) {
        std::cout << "Failed to login e1" << std::endl;
        numFails++;
    }

    if (!ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged in" << std::endl;
        numFails++;
    }

    /**
     * Login user two
     */
    if (ds.IsLoggedIn(u2.c_str())) {
        std::cout << "Failed to see that e2 was not logged in" << std::endl;
        numFails++;
    }

    if (ds.Login(u2.c_str(), p2.c_str()) != LOGGED_IN) {
        std::cout << "Failed to login e2" << std::endl;
        numFails++;
    }

    if (!ds.IsLoggedIn(u2.c_str())) {
        std::cout << "Failed to see that e2 was logged in" << std::endl;
        numFails++;
    }

    /**
     * Logout user two
     */
    if (ds.Logout(u2.c_str()) != LOGGED_OUT) {
        std::cout << "Failed to logout e2" << std::endl;
        numFails++;
    }

    if (ds.IsLoggedIn(u2.c_str())) {
        std::cout << "Failed to see that e2 was logged out" << std::endl;
        numFails++;
    }


    // /**
    //  * Logout user one
    //  */
    if (ds.Logout(u1.c_str()) != LOGGED_OUT) {
        std::cout << "Failed to logout e1" << std::endl;
        numFails++;
    }

    if (ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged out" << std::endl;
        numFails++;
    }

    return numFails;
}


int DequeTests() {
    DataStore ds(REGISTRAR_FILE_PATH);
    int numFails = 0;
    Log log(LOG_PATH);
    SocketMessenger sockMsgr(&log);
    
    std::string u1 = "steph";
    std::string p1 = "1234";

    ds.Register(u1.c_str(), p1.c_str());

    ds.Login(u1.c_str(), p1.c_str());

    if (!ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged in" << std::endl;
        numFails++;
    }

    std::string m1 = "Hello how goes it?";
    std::string m2 = "Hey, I am doing well.";

    ByteBody* bm1 = sockMsgr.CharToByteBody(m1.c_str());
    ByteBody* bm2 = sockMsgr.CharToByteBody(m2.c_str());



    if (!ds.Enqueue(u1.c_str(), bm1)) {
        std::cout << "Failed to enqueue bm1" << std::endl;
        numFails++;
    }

    if (!ds.Enqueue(u1.c_str(), bm2)) {
        std::cout << "Failed to enqueue bm2" << std::endl;
        numFails++;
    }

    ByteBody* actual_bm1 = ds.Dequeue(u1.c_str());
    ByteBody* actual_bm2 = ds.Dequeue(u1.c_str());

    char* actual_m1 = sockMsgr.ByteBodyToChar(actual_bm1);
    char* actual_m2 = sockMsgr.ByteBodyToChar(actual_bm2);

    if (strcmp(actual_m1, m1.c_str()) != 0) {
        std::cout << "Actual_m1 was not m1" << std::endl;
        numFails++;
    }

    if (strcmp(actual_m2, m2.c_str()) != 0) {
        std::cout << "Actual_m2 was not m2" << std::endl;
        numFails++;
    }

    delete[] actual_m1;
    delete[] actual_m2;
    delete bm1;
    delete bm2;
    delete actual_bm1;
    delete actual_bm2;

    return numFails;
}


int GroupEnqueueTests() {
    DataStore ds(REGISTRAR_FILE_PATH);
    int numFails = 0;
    Log log(LOG_PATH);
    SocketMessenger sockMsgr(&log);
    
    std::string u1 = "steve";
    std::string p1 = "1234";

    std::string u2 = "shawn";
    std::string p2 = "3455";

    std::string u3 = "koda";
    std::string p3 = "6782";

    ds.Register(u1.c_str(), p1.c_str());
    ds.Login(u1.c_str(), p1.c_str());

    ds.Register(u2.c_str(), p2.c_str());
    ds.Login(u2.c_str(), p2.c_str());

    ds.Register(u3.c_str(), p3.c_str());
    ds.Login(u3.c_str(), p3.c_str());

    if (!ds.IsLoggedIn(u1.c_str())) {
        std::cout << "Failed to see that e1 was logged in" << std::endl;
        numFails++;
    }

    if (!ds.IsLoggedIn(u2.c_str())) {
        std::cout << "Failed to see that e1 was logged in" << std::endl;
        numFails++;
    }

    if (!ds.GetUsersWithMsgs().empty()) {
        std::cout << "Users had messages which is impossible" << std::endl;
        numFails++;
    }

    std::string m1 = "Hello how goes it?";
    std::string m2 = "Hey, I am doing well.";

    ByteBody* bm1 = sockMsgr.CharToByteBody(m1.c_str());
    ByteBody* bm2 = sockMsgr.CharToByteBody(m2.c_str());
    
    ds.EnqueueAllExcept(u3.c_str(), bm1);
    ds.EnqueueAllExcept(u3.c_str(), bm2);

    std::vector<std::string> usernames = ds.GetUsersWithMsgs();

    if (usernames.at(0).compare(u2) == 0) {
        std::cout << usernames.at(0) << " wasn't " << u1 << std::endl;
        numFails++;
    }

    if (usernames.at(1).compare(u1) == 0) {
        std::cout << usernames.at(1) << " wasn't " << u2 << std::endl;
        numFails++;
    }

    ByteBody* actual_bm1 = ds.Dequeue(u1.c_str());
    ByteBody* actual_bm2 = ds.Dequeue(u1.c_str());

    char* actual_m1 = sockMsgr.ByteBodyToChar(actual_bm1);
    char* actual_m2 = sockMsgr.ByteBodyToChar(actual_bm2);

    if (strcmp(actual_m1, m1.c_str()) != 0) {
        std::cout << "Actual_m1 was not m1" << std::endl;
        numFails++;
    }

    if (strcmp(actual_m2, m2.c_str()) != 0) {
        std::cout << "Actual_m2 was not m2" << std::endl;
        numFails++;
    }

    delete actual_bm1;
    delete actual_bm2;
    delete[] actual_m1;
    delete[] actual_m2;
    

    ByteBody* actual_bm11 = ds.Dequeue(u2.c_str());
    ByteBody* actual_bm22 = ds.Dequeue(u2.c_str());

    char* actual_m11 = sockMsgr.ByteBodyToChar(actual_bm11);
    char* actual_m22 = sockMsgr.ByteBodyToChar(actual_bm22);

    if (strcmp(actual_m11, m1.c_str()) != 0) {
        std::cout << "Actual_m11 was not m1" << std::endl;
        numFails++;
    }

    if (strcmp(actual_m22, m2.c_str()) != 0) {
        std::cout << "Actual_m22 was not m2" << std::endl;
        numFails++;
    }

    delete actual_bm11;
    delete actual_bm22;
    delete[] actual_m11;
    delete[] actual_m22;


    ByteBody* actual_bm111 = ds.Dequeue(u3.c_str());

    if (actual_bm111 != NULL) {
        std::cout << "Actual_m11 was not m1" << std::endl;
        numFails++;
    }

    delete bm1;
    delete bm2;


    return numFails;
}