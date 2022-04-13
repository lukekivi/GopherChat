#include "../../app/utils/file_transporter.hpp"

const char* TEST_FILE = "testfiles/testFile.txt";
const char* NEW_FILE_LOCATION = "testfiles/newTestFile.txt";

const char* GIANT_TEST_FILE = "testfiles/giantFile.txt";
const char* GIANT_NEW_FILE_LOCATION = "testfiles/newGiantTestFile.txt";

int main () {
    FileTransporter ft;

    char* contents = ft.fileToChar(TEST_FILE);
    bool resultsOne = ft.charToFile(NEW_FILE_LOCATION, contents);

    if (!resultsOne) {
        std::cout << "Failed. Make sure the file: \"" << NEW_FILE_LOCATION << "\" doesn't exist." << std::endl;
        std::cout << "If it isn't there then something went wrong." << std::endl;
    }

    delete[] contents;

    contents = ft.fileToChar(GIANT_TEST_FILE);
    bool resultsTwo = ft.charToFile(GIANT_NEW_FILE_LOCATION, contents);

    if (!resultsTwo) {
        std::cout << "Failed. Make sure the file: \"" << GIANT_NEW_FILE_LOCATION << "\" doesn't exist." << std::endl;
        std::cout << "If it isn't there then something went wrong." << std::endl;
    }

    delete[] contents;

    if (resultsOne && resultsTwo) {
        std::cout << "ok" << std::endl;
    }
}