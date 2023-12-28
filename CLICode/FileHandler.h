#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>

class FileHandler {
public:
    static std::string readFile(const std::string& filename);
    static void writeFile(const std::string& filename, const std::string& content);
    static std::string calculateHash(const std::string& content);
};

#endif // FILE_HANDLER_H
