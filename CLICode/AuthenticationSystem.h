// AuthenticationSystem.h
#ifndef AUTHENTICATIONSYSTEM_H
#define AUTHENTICATIONSYSTEM_H

#include <string>
#include <unordered_map>

class AuthenticationSystem {
public:
    AuthenticationSystem();
    bool authenticateUser(const std::string &username, const std::string &password);
private:
    std::unordered_map<std::string, std::string> loadUsers();
    std::unordered_map<std::string, std::string> users;
};

#endif // AUTHENTICATIONSYSTEM_H
