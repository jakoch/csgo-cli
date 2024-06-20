// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ShareCodeCache.h"

ShareCodeCache::ShareCodeCache(bool verboseMode)
{
    // create file, if not exists
    if (!std::filesystem::exists(csvFile)) {
        matchDbFile.open(csvFile, std::ios::out | std::ios::app);
        matchDbFile.close();
    }

    matchDbFile.open(csvFile, std::ios::in);
    sharecodeCache = read(matchDbFile);
    matchDbFile.close();

    if (verboseMode) {
        // debug print sharecode cache
        printf(" Cached Sharecodes: %zd \n", sharecodeCache.size());
        for (auto const & sharecode : sharecodeCache) {
            printf(" \"%s\" \n", sharecode.c_str());
        }
        printf("\n");
    }

    // clear cache
    if (sharecodeCache.size() >= 50) {
        std::ofstream ofs(csvFile, std::ios::out | std::ios::trunc);
        ofs.close();
    }
}

bool ShareCodeCache::find(std::string sharecode)
{
    for (auto const & sharecodeFromCache : sharecodeCache) {
        if (sharecode.compare(sharecodeFromCache.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

bool ShareCodeCache::insert(std::string sharecode)
{
    matchDbFile.open(csvFile, std::ios::out | std::ios::app);
    if (matchDbFile.good()) {
        matchDbFile << sharecode << std::endl;
        matchDbFile.close();
        return true;
    }
    return false;
}

std::vector<std::string> ShareCodeCache::read(std::istream& is)
{
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(is, token) && !token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

ShareCodeCache::~ShareCodeCache()
{
    matchDbFile.close();
}