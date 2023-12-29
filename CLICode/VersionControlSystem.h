#ifndef VERSION_CONTROL_SYSTEM_H
#define VERSION_CONTROL_SYSTEM_H

#include "Repository.h"
#include <string>

class VersionControlSystem {
public:
    VersionControlSystem(const std::string& repoPath);
    std::vector<std::string> getFiles();
    void init();
    void add(const std::string& filename);
    void addDirectory(const std::string& foldername);
    void commit();
    void refresh();
    std::vector<bool> status();
    int getVersion();
    void rollback(int version);
private:
    Repository repo;
};

#endif // VERSION_CONTROL_SYSTEM_H
