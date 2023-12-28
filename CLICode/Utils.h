#ifndef UTILS_H
#define UTILS_H

#include <string>

class Utils {
public:
    // Hashes content using a robust mechanism
    static std::string hashContent(const std::string& content);

    // Placeholder for a file monitoring function
    static void monitorDirectoryChanges(const std::string& directoryPath);
};

#endif // UTILS_H
