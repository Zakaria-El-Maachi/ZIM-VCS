#include "Repository.h"
#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

// Constructor
Repository::Repository(const std::string& repoPath)
    : baseRepoPath(repoPath), csvFilePath(repoPath + "/version_control.csv") {
    initializeRepository(true);
}

// Initialize the repository by creating a CSV file
void Repository::initializeRepository(bool how) {
    std::cerr << "Initializing repository at " << baseRepoPath << std::endl;

    // Check if the CSV file already exists
    if(how){
        if (std::filesystem::exists(csvFilePath)) {
            std::cout << "CSV file already exists. Loading records." << std::endl;
            loadRecords();
        }
    }
    else {
        // If the CSV file doesn't exist, create it
        std::ofstream csvFile(csvFilePath);
        if (!csvFile.is_open()) {
            std::cerr << "Failed to create or open .csv file at: " << csvFilePath << std::endl;
            throw std::runtime_error("Failed to create .csv file in repository.");
        }

        // Write CSV header
        csvFile << "filename,oldHash,newHash\n";
        csvFile.close();

        std::cout << "Repository initialized with .csv file at " << csvFilePath << std::endl;
    }
}


std::vector<std::string> Repository::getFiles(){
    std::vector<std::string> files;
    for(auto& record: records){
        files.push_back(record.filename);
    }
    return files;
}


void Repository::trackFile(const std::string& filename) {
    // Check if the filename already exists in records
    auto it = std::find_if(records.begin(), records.end(), [filename](const auto& record) {
        return record.filename == filename;
    });

    if (it != records.end()) {
        // Filename already exists, throw an exception with a precise message
        throw std::runtime_error("File '" + filename + "' already tracked in the repository.");
    }

    // Filename doesn't exist, proceed to add it
    std::cerr << "Filename is " << filename << std::endl;
    std::string fileHash = calculateFileHash(filename);
    records.push_back({filename, fileHash, fileHash});  // store the relative filename
    saveRecords();
}

bool Repository::update() {
    bool hasChanged = false;
    for (auto& record : records) {
        std::string newHash = calculateFileHash(record.filename);
        if(newHash != record.oldHash){
            hasChanged = true;
        }
        record.newHash = newHash;
    }
    if(hasChanged) saveRecords();
    return hasChanged;
}

void Repository::updateCommit() {
    bool hasChanged = update();
    if(!hasChanged) throw std::runtime_error("At least one file must be modified before committing.");
    for (auto& record : records) {
        std::string currentHash = calculateFileHash(record.filename);
        if (record.oldHash != currentHash) { // Check if the file has been modified
            record.oldHash = currentHash; // Update the old hash to match the new one
        }
    }
    saveRecords();
}

// Show the status of files in the repository
std::vector<bool> Repository::showStatus() {
    std::vector<bool> modified = std::vector<bool>();
    for (const auto& record : records) {
        if (record.oldHash != record.newHash) {
            std::cout << record.filename << " has been modified." << std::endl;
        }
        modified.push_back((record.oldHash != record.newHash));
    }
    return modified;
}

// Load records from the CSV file into memory
void Repository::loadRecords() {
    std::ifstream csvFile(csvFilePath);
    if (!csvFile.is_open()) {
        throw std::runtime_error("Failed to open .csv file in repository.");
    }

    std::string line, filename, oldHash, newHash;
    records.clear();
    // Skip the header line
    std::getline(csvFile, line);
    while (std::getline(csvFile, line)) {
        std::stringstream linestream(line);
        std::getline(linestream, filename, ',');
        std::getline(linestream, oldHash, ',');
        std::getline(linestream, newHash);
        records.push_back({filename, oldHash, newHash});
    }
}

// Save records from memory to the CSV file
void Repository::saveRecords() {
    std::ofstream csvFile(csvFilePath);
    // std::ofstream csvFile(csvFilePath);
    if (!csvFile.is_open()) {
        throw std::runtime_error("Failed to open .csv file for writing.");
    }
    csvFile << "filename,oldHash,newHash\n"; // CSV Header
    for (const auto& record : records) {
        csvFile << record.filename << "," << record.oldHash << "," << record.newHash << "\n";
    }
    csvFile.close();
}


void Repository::untrackFile(const std::string& filename) {

    // Find and remove the record with the given filename
    auto it = std::find_if(records.begin(), records.end(),
                           [&filename](const FileRecord& record) { return record.filename == filename; });

    if (it != records.end()) {
        records.erase(it);
        saveRecords();  // Save the updated records back to the CSV file
    } else {
        throw std::runtime_error("The file you selected is not staged.");
    }
}


// Helper function to calculate the hash of a file's content
std::string Repository::calculateFileHash(const std::string& filepath) {
    std::string content = FileHandler::readFile(filepath);
    return FileHandler::calculateHash(content);
}
