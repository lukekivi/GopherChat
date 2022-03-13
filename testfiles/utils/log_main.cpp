#include "../../app/utils/log.hpp"

std::string LOG_OUT_PATH = "log/log_out.txt";
std::string SECOND_LOG_OUT_PATH = "log/second_log_out.txt";


int main() {

    Log* log = new Log(LOG_OUT_PATH);

    log->Error("%s", "errors are a-happening");

    log->Error("%s", "errors are still a-happening");

    log->Info("%d, %d, %d, %d, %s", 1, 2, 3, 4, "I declare a thumb war");

    Log* logTwo = new Log(log);

    logTwo->Error("%s", "error are a-happening in a copy");

    logTwo->Info("%d, %d, %d, %d, %s", 5, 6, 7, 8, "You're the one I really hate");

    delete log;    

    logTwo->Info("%s", "Just kidding you the best");

    logTwo->Error("%s", "error are a-happening in a copy after deleting the original");

    logTwo->SetLogFile(SECOND_LOG_OUT_PATH);

    logTwo->Info("%s %d", "Log file ", 2);
    
    delete logTwo;

    return EXIT_SUCCESS;
}