#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <stdarg.h>
#include <fstream>
#include <string.h>

class Log {
    
    public:
        Log() {}
        Log(std::string filePath) { SetLogFile(filePath); }
        Log(Log* log) { CopyLogFile(log->filePath); }
        ~Log() {}

        void Error(const char* format, ...);
        void Info(const char* format, ...);
        void SetLogFile(std::string path);
        Log* operator=(const Log& log);

    private:
        void CopyLogFile(std::string path);

        std::fstream file;
        std::string filePath;
};

#endif