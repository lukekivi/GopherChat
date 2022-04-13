#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <stdarg.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <string>

class Log {
    
    public:
        Log(std::string filePath) { SetLogFile(filePath); }
        Log(Log* log) { CopyLogFile(log->filePath); }
        ~Log() {}

        void Error(const char* format, ...);
        void Info(const char* format, ...);
        void Out(const char* description, const char* user, const char* recipient = NULL, const char* msg = NULL);
        void Out(const char* description, const char* user, std::vector<std::string> recipients, const char* msg = NULL);
        void Out(const char* description, const char* user, const char* recipient, const char* fileName, const char* msg, int size);
        void Out(const char* description, const char* user, std::vector<std::string> recipients, const char* fileName, const char* msg, int size);
        void SetLogFile(std::string path);
        Log* operator=(const Log& log);

    private:
        void CopyLogFile(std::string path);

        std::fstream file;
        std::string filePath;
};

#endif