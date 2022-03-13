#include "log.hpp"

const std::string ERROR_STRING = "ERROR: ";
const int ERROR_MSG_SIZE = 4096;
const int LOG_MSG_SIZE = 2048;


/**
 * @brief Print message to stderr.
 * 
 * @param format printing format that can include conversion specifiers
 * @param ...    args to be converted into the format
 */
void Log::Error(const char * format, ...) {
    char msg[ERROR_MSG_SIZE];
    va_list argptr;

	va_start(argptr, format);
	vsprintf(msg, format, argptr);
	va_end(argptr);

    std::cerr << ERROR_STRING << msg << std::endl;
}


/**
 * @brief Log a message. If a file is set it will be logged there.
 * 
 * @param format printing format that can include conversion specifiers
 * @param ...    args to be converted into the format
 */
void Log::Info(const char * format, ...) {
    char msg[LOG_MSG_SIZE];
	va_list argptr;

	va_start(argptr, format);
	vsprintf(msg, format, argptr);
	va_end(argptr);

    if (file.is_open()) {
        file << msg << std::endl;
    } else {
        std::cout << msg << std::endl;
    }
}


/**
 * @brief Set the info logging file.
 * @param path 
 */
void Log::SetLogFile(std::string path) {
    if (file.is_open()) {
        file.close();
        file.clear();
    }

    file.open(path, std::ios::out | std::ios::trunc);
    filePath = path;
}

/**
 * @brief If the rhs Log has an open file descriptor the lhs
 * opens a new file descriptor to the same file.
 * 
 * @param log 
 * @return Newly set log 
 */
Log* Log::operator=(const Log& log) {
    if (log.file.is_open()) {
        this->CopyLogFile(log.filePath);
    }
    return this;
}

/**
 * @brief Set the info logging file. Writes will append.
 * @param path 
 */
void Log::CopyLogFile(std::string path) {
    if (file.is_open()) {
        file.close();
        file.clear();
    }

    file.open(path, std::ios::out | std::ios::app);
    filePath = path;
}