#include "Utils.h"
#include <functional>
#include <iostream>

// Hashes content using std::hash for demonstration (Replace with a robust hashing mechanism like SHA-256 in production)
std::string Utils::hashContent(const std::string& content) {
    std::hash<std::string> hasher;
    auto hashed = hasher(content); // This produces a size_t hash value
    return std::to_string(hashed); // Convert to string for simplicity
}

// Placeholder function for monitoring directory changes
void Utils::monitorDirectoryChanges(const std::string& directoryPath) {
    // Implement file monitoring using platform-specific APIs like ReadDirectoryChangesW on Windows or inotify on Linux.
    std::cout << "Monitoring changes in directory: " << directoryPath << std::endl;

    // Placeholder code - Replace with actual implementation
    // This is where you'd set up the file monitoring, handle events, etc.
}
