#pragma once

#include <experimental/filesystem> // C++-standard header file name
#include <filesystem> // Microsoft-specific implementation header file name

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>

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

    std::vector<std::string> read(std::istream& input);
};
