#ifndef ShareCodeCache_H
#define ShareCodeCache_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

class ShareCodeCache
{
public:
    ShareCodeCache(bool verboseMode);
    ~ShareCodeCache();

    bool find(std::string sharecode);
    bool insert(std::string sharecode);

private:
    const std::string csvFile = "sharecode.db";
    std::fstream matchDbFile;

    std::vector<std::string> sharecodeCache;

    std::vector<std::string> read(std::istream &input);
};

#endif