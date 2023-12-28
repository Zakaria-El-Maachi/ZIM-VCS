#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <string>
#include <vector>

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
    void updateCommit();
    std::vector<bool> showStatus();
    std::vector<std::string> getFiles();
private:
    std::string baseRepoPath; // Base path of the repository
    std::string csvFilePath; // Path to the CSV file within the repository
    std::vector<FileRecord> records; // In-memory storage of file records

    void loadRecords(); // Load records from the CSV file
    void saveRecords(); // Save records to the CSV file
    std::string calculateFileHash(const std::string& filepath); // Calculate hash of a file
};

#endif // REPOSITORY_H
