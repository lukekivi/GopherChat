#ifndef FILE_DATA_H
#define FILE_DATA_H

/**
 * Data class to represent file command data.
 */
class FileData {
  public:
    FileData(char* rcvrName_, char* fileName_) {
        rcvrName = rcvrName_;
        fileName = fileName_;
    }

    ~FileData() {
        delete fileName;
        delete rcvrName;
    }

    /**
     * Is the file message supposed to be public?
     */
    bool isPublic() {
        return rcvrName == nullptr;
    }

    char* getFileName() {
        return fileName;
    }

    char* getRcvrName() {
        return rcvrName;
    }

  private:
    char* rcvrName;
    char* fileName;

};

#endif