#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>
// For simplicity, using a standard library for hashing; in a real scenario, you'd likely use a cryptographic library
#include <functional>

// Reads the entire content of a file
std::string FileHandler::readFile(const std::string& filename) {
    std::ifstream file(filename);
    std::cerr << filename << std::endl;
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


// Writes content to a file
void FileHandler::writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    file << content;
    file.close();
}

// Calculates a simple hash of the content (for illustration purposes, using std::hash)
std::string FileHandler::calculateHash(const std::string& content) {
    // In a real-world application, replace this with a proper hashing algorithm like SHA-256
    std::hash<std::string> hasher;
    auto hashed = hasher(content); // This produces a size_t hash value
    std::stringstream ss;
    ss << hashed;
    return ss.str();
}
