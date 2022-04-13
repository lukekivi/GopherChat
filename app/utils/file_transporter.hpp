#ifndef FILE_TRANSPORTER_H
#define FILE_TRANSPORTER_H

#include "../data/byte_body.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <iostream>
#include <fstream>

class FileTransporter {
  public:
    char* fileToChar(const char* filePath) {
        FILE * file = fopen(filePath, "r+");

        if (file == NULL) {
            std::cout << "Couldn't read from file" << std::endl;
            return NULL;
        }

        fseek(file, 0, SEEK_END);
        long int size = ftell(file);

        fclose(file);

        if (size > MAX_FILE_SIZE) {
            std::cout << "Exceeded max file size" << std::endl;
            return NULL;
        }

        file = fopen(filePath, "r+");
        char* body = new char[size+1];
        body[size] = '\0';
        int len = fread(body, sizeof(char), size, file);
        fclose(file);

        return body;
    }

    bool charToFile(const char* filePath, char* contents) {
        if (DoesFileExist(filePath)) {
            return false;
        }

        std::ofstream file;
        file.open(filePath);
        file << contents;
        file.close();

        return true;
    }

  private:
    bool DoesFileExist(const char* filePath) {
        int res = access(filePath, R_OK);
        if (res < 0) {
            if (errno == ENOENT) {
                // file does not exist
                return false;
            } else {
                return true;
            }
        }
        return true;
    }
};

#endif