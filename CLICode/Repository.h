#ifndef REPOSITORY_H
#define REPOSITORY_H
#define MAX_FILENAME 256

#include <string>
#include <vector>
#include "minizip/zip.h"

// Structure to represent a file record in the repository
struct FileRecord {
    std::string filename;
    std::string oldHash;
    std::string newHash;
};

class Repository {
public:
    Repository();
    Repository(const std::string& repoPath);
    void initializeRepository(bool how);
    bool update();
    void trackFile(const std::string& filename);
    void trackFolder(const std::string& foldername);
    void untrackFile(const std::string& filename);
    void updateCommit();
    std::vector<bool> showStatus();
    void rollbackToVersion(int versionNumber);
    std::vector<std::string> getFiles();
    int getVersion();
private:
    std::string baseRepoPath; // Base path of the repository
    std::string csvFilePath; // Path to the CSV file within the repository
    std::vector<FileRecord> records; // In-memory storage of file records
    int version = 0;
    std::string versionFilePath;
    void decompressFiles(const std::string& zipPath, const std::string& destDir);
    void compressFiles(const std::vector<std::string>& files, const std::string& outputPath);
    void loadRecords(); // Load records from the CSV file
    void saveRecords(); // Save records to the CSV file
    std::string calculateFileHash(const std::string& filepath); // Calculate hash of a file
    std::string calculateFolderHash(const std::string& foldername);
    void addFileToZip(const std::string& filePath, zipFile& zf, const std::string& baseFolderPath);

};

#endif // REPOSITORY_H
