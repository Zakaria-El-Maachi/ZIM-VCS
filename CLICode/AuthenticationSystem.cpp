// AuthenticationSystem.cpp
#include "AuthenticationSystem.h"
#include "openssl/evp.h" // Corrected include path for OpenSSL SHA library
#include <fstream>
#include <sstream>
#include <iomanip> // for std::setw and std::setfill
#include "QString"
#include "QCoreApplication"
#include <iostream>

// Function to compute SHA256 hash of a string using OpenSSL
std::string sha256(const std::string& str) {
    unsigned char hash[EVP_MAX_MD_SIZE]; // An array to store the hash
    unsigned int lengthOfHash = 0;

    EVP_MD_CTX* evp_md_ctx = EVP_MD_CTX_new(); // Create a digest context
    if (!evp_md_ctx) {
        // Handle errors appropriately in production code
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(evp_md_ctx, EVP_sha256(), NULL) != 1 || // Initialise the Digest
        EVP_DigestUpdate(evp_md_ctx, str.c_str(), str.size()) != 1 || // Hash the data
        EVP_DigestFinal_ex(evp_md_ctx, hash, &lengthOfHash) != 1) { // Finalize and get the hash
        EVP_MD_CTX_free(evp_md_ctx); // Always free resources
        // Handle errors appropriately in production code
        throw std::runtime_error("Hash computation failed");
    }

    EVP_MD_CTX_free(evp_md_ctx); // Free the context

    std::stringstream ss;
    for (unsigned int i = 0; i < lengthOfHash; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// Constructor loads users upon object creation
AuthenticationSystem::AuthenticationSystem() {
    users = loadUsers();
}

// Private function to load users and their hashed passwords into a map
std::unordered_map<std::string, std::string> AuthenticationSystem::loadUsers() {
    std::unordered_map<std::string, std::string> loadedUsers;
    QString appDirPath = QCoreApplication::applicationDirPath();
    std::ifstream file(appDirPath.toStdString() + "/users.csv");
    std::string line, username, hashedPassword;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::getline(ss, username, ',');
        std::getline(ss, hashedPassword, ',');
        loadedUsers[username] = hashedPassword;
    }

    return loadedUsers;
}

// Function to authenticate a user
bool AuthenticationSystem::authenticateUser(const std::string &username, const std::string &password) {
    auto it = users.find(username);
    if (it != users.end()) {
        // Hash the input password and compare
        std::string hashedInputPassword = sha256(password);
        return hashedInputPassword == it->second;
    }
    return false;
}
