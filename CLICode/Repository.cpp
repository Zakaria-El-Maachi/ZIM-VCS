#include "Repository.h"
#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <minizip/zip.h>
#include <minizip/unzip.h>
#include "FileHandler.h"

// Constructor
Repository::Repository(const std::string& repoPath)
    : baseRepoPath(repoPath), csvFilePath(repoPath + "/version_control.csv"), versionFilePath(repoPath + "/version.txt") {
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
        if(std::filesystem::exists(versionFilePath)){
            std::ifstream versionFile(versionFilePath);
            if (versionFile.is_open()) {
                versionFile >> version;
                versionFile.close();
            }
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


        version = 0; // Default to version 0 if file does not exist
        std::ofstream outVersionFile(versionFilePath);
        outVersionFile << version;
        outVersionFile.close();

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


int Repository::getVersion(){
    return version;
}


void Repository::trackFile(const std::string& filename) {
    // Check if the filename already exists in records
    for (const auto& record : records) {
        if (std::filesystem::path(filename).string().find(record.filename) == 0) {
            throw std::runtime_error("File is part of an already tracked folder: " + record.filename);
        }
    }

    // Filename doesn't exist, proceed to add it
    std::cerr << "Filename is " << filename << std::endl;
    std::string fileHash = calculateFileHash(filename);
    records.push_back({filename, fileHash, fileHash});  // store the relative filename
    saveRecords();
}



void Repository::trackFolder(const std::string& foldername) {
    // Check if the foldername already exists in records
    for (const auto& record : records) {
        if (std::filesystem::path(foldername).string().find(record.filename) == 0 ||
            std::filesystem::path(record.filename).string().find(foldername) == 0) {
            throw std::runtime_error("Folder conflicts with an already tracked file or folder: " + record.filename);
        }
    }

    // Foldername doesn't exist, proceed to add it
    std::cerr << "Foldername is " << foldername << std::endl;

    // Create a hash for the folder based on its contents
    std::string folderHash = calculateFolderHash(foldername);
    records.push_back({foldername, folderHash, folderHash});  // store the relative foldername
    saveRecords();
}

std::string Repository::calculateFolderHash(const std::string& foldername) {
    std::string combinedHashes;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(foldername)) {
        if (entry.is_regular_file()) {
            std::string fileHash = calculateFileHash(entry.path().string());
            combinedHashes += fileHash; // Concatenate all file hashes (or combine them in a more sophisticated way)
        }
    }
    return FileHandler::calculateHash(combinedHashes); // Use the same hash function to hash the combined hashes
}



bool Repository::update() {
    bool hasChanged = false;

    for (auto& record : records) {
        std::string newHash;

        // Check if the record is a file or a directory
        if (std::filesystem::is_directory(record.filename)) {
            newHash = calculateFolderHash(record.filename);  // Use the folder hash function
        } else {
            newHash = calculateFileHash(record.filename);  // Use the file hash function
        }

        // Check if the hash has changed
        if (newHash != record.oldHash) {
            hasChanged = true;
        }
        record.newHash = newHash;  // Update the new hash
    }

    if (hasChanged) {
        saveRecords();
    }
    return hasChanged;
}

// Refresh record Statuses and return whether a file has been modified
void Repository::updateCommit() {
    bool hasChanged = update();
    std::cout << hasChanged << std::endl;
    if(!hasChanged) throw std::runtime_error("At least one file must be modified before committing.");
    for (auto& record : records) {
        record.oldHash = record.newHash;
    }

    std::string historyPath = baseRepoPath + "/history";
    if (!std::filesystem::exists(historyPath)) {
        std::filesystem::create_directory(historyPath);
    }
    // Code for compressing files
    std::string versionedArchiveName = "commit_" + std::to_string(version) + ".zip";
    compressFiles(getFiles(), historyPath + "/" + versionedArchiveName);
    version++;

    std::ofstream versionFile(versionFilePath);
    if (!versionFile.is_open()) {
        throw std::runtime_error("Failed to open or create version file.");
    }
    versionFile << version;
    versionFile.close();

    saveRecords();
}


void Repository::compressFiles(const std::vector<std::string>& paths, const std::string& outputPath) {
    namespace fs = std::filesystem;
    zipFile zf = zipOpen(outputPath.c_str(), APPEND_STATUS_CREATE);
    if (!zf) {
        throw std::runtime_error("Failed to open output zip file.");
    }

    std::string baseFolderPath = baseRepoPath; // Get base folder path

    for (const auto& path : paths) {
        if (fs::is_directory(path)) {
            // Recursively add files from directory
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    addFileToZip(entry.path().string(), zf, baseFolderPath);
                }
            }
        } else if (fs::is_regular_file(path)) {
            // Add the file to the zip
            addFileToZip(path, zf, baseFolderPath);
        } else {
            std::cerr << "Skipping non-regular file or directory: " << path << std::endl;
        }
    }

    zipClose(zf, NULL /* global comment */);
}



void Repository::addFileToZip(const std::string& filePath, zipFile& zf, const std::string& baseFolderPath) {
    std::ifstream inFile(filePath, std::ios_base::binary);
    if (!inFile) {
        std::cerr << "Could not open " << filePath << " for reading." << std::endl;
        return;
    }

    std::string contents((std::istreambuf_iterator<char>(inFile)),
                         std::istreambuf_iterator<char>());
    inFile.close();  // Ensure the file is closed after reading

    if (contents.empty()) {
        std::cerr << "Warning: " << filePath << " is empty or unreadable." << std::endl;
        return;
    }

    // Calculate the relative path
    std::filesystem::path fsPath(filePath);
    std::filesystem::path baseFolderPath_fs(baseFolderPath);
    std::string relativePath = std::filesystem::relative(fsPath, baseFolderPath_fs).string();

    zip_fileinfo zfi;
    memset(&zfi, 0, sizeof(zfi));

    int err = zipOpenNewFileInZip(zf, relativePath.c_str(), &zfi,
                                  NULL, 0, NULL, 0, NULL,
                                  Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (err != ZIP_OK) {
        std::cerr << "Failed to add file to zip: " << relativePath << " with error: " << err << std::endl;
        return;
    }

    err = zipWriteInFileInZip(zf, contents.data(), contents.size());
    if (err != ZIP_OK) {
        std::cerr << "Failed to write file to zip: " << relativePath << std::endl;
        // Consider removing or not adding the file to the zip if this fails
    }

    zipCloseFileInZip(zf);
}



void Repository::decompressFiles(const std::string& zipPath, const std::string& destDir) {
    unzFile zipfile = unzOpen(zipPath.c_str());
    if (!zipfile) {
        throw std::runtime_error("Could not open zip file for reading.");
    }

    if (unzGoToFirstFile(zipfile) != UNZ_OK) {
        unzClose(zipfile);
        throw std::runtime_error("Could not read first file in zip archive.");
    }

    do {
        char filename[MAX_FILENAME];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipfile, &fileInfo, filename, MAX_FILENAME, NULL, 0, NULL, 0) != UNZ_OK) {
            throw std::runtime_error("Could not read file info in zip archive.");
        }

        if (unzOpenCurrentFile(zipfile) != UNZ_OK) {
            throw std::runtime_error("Could not open file in zip archive.");
        }

        // Construct full path for file/directory
        std::string fullPath = destDir + "/" + filename;

        // Check if it's a directory (ends with '/')
        if (filename[strlen(filename) - 1] == '/') {
            std::filesystem::create_directories(fullPath);
        } else {
            // Ensure the directory exists before writing file
            std::filesystem::create_directories(std::filesystem::path(fullPath).parent_path());

            // Create a buffer to hold file contents and read the file
            char *buffer = new char[fileInfo.uncompressed_size];
            int bytesRead = unzReadCurrentFile(zipfile, buffer, fileInfo.uncompressed_size);

            if (bytesRead > 0) {
                std::ofstream outFile(fullPath, std::ios::binary);
                outFile.write(buffer, bytesRead);
                outFile.close();
            }

            delete[] buffer; // Clean up buffer
        }

        unzCloseCurrentFile(zipfile);
    } while (unzGoToNextFile(zipfile) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipfile);
}



void Repository::rollbackToVersion(int versionNumber) {
    std::string versionedArchiveName = baseRepoPath + "/history/commit_" + std::to_string(versionNumber) + ".zip";

    if (!std::filesystem::exists(versionedArchiveName)) {
        throw std::runtime_error("Specified version does not exist.");
    }

    // Clear existing files and directories or implement your strategy here

    decompressFiles(versionedArchiveName, baseRepoPath);

    version = versionNumber;  // Update the version

    std::ofstream versionFile(versionFilePath);
    if (!versionFile.is_open()) {
        throw std::runtime_error("Failed to open or create version file.");
    }
    versionFile << version;
    versionFile.close();

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
    // In your initializeRepository and saveRecords functions
    csvFile << "filename,oldHash,newHash\n";
     // CSV Header
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
