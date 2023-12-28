#include "VersionControlSystem.h"
#include <iostream>

// Constructor
VersionControlSystem::VersionControlSystem(const std::string& repoPath)
    : repo(repoPath) {  // Initialize Repository with the given path
    // Any other initialization as necessary
}

// Initialize the repository (init command)
void VersionControlSystem::init() {
    try {
        repo.initializeRepository(false);
        std::cout << "Repository initialized." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
    }
}

std::vector<std::string> VersionControlSystem::getFiles(){
    return repo.getFiles();
}

void VersionControlSystem::refresh(){
    repo.update();
}

// Add a file to the repository (add command)
void VersionControlSystem::add(const std::string& filename) {
    repo.trackFile(filename);
    std::cout << filename << " added to the repository." << std::endl;
}

// Commit changes to the repository (commit command)
void VersionControlSystem::commit() {
    repo.updateCommit();
    std::cout << "Changes committed to the repository." << std::endl;
}

// Display status of the repository (status command)
std::vector<bool> VersionControlSystem::status() {
    try {
        return repo.showStatus();
    } catch (const std::exception& e) {
        std::cerr << "Failed to retrieve status: " << e.what() << std::endl;
    }
}
