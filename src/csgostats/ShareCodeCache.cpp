// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ShareCodeCache.h"

#include <algorithm>
#include <fmt/format.h>
#include <string>
#include <vector>

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
        fmt::print(" Cached Sharecodes: {} \n", sharecodeCache.size());
        for (auto const & sharecode : sharecodeCache) {
            fmt::print(" \"{}\" \n", sharecode);
        }
        fmt::print("\n");
    }

    // clear cache
    if (sharecodeCache.size() >= 50) {
        std::ofstream ofs(csvFile, std::ios::out | std::ios::trunc);
        ofs.close();
    }
}

bool ShareCodeCache::find(std::string const & sharecode)
{
    return std::ranges::any_of(sharecodeCache, [&sharecode](auto const & sharecodeFromCache) {
        return sharecode == sharecodeFromCache;
    });
}

bool ShareCodeCache::insert(std::string const & sharecode)
{
    matchDbFile.open(csvFile, std::ios::out | std::ios::app);
    if (matchDbFile.good()) {
        matchDbFile << sharecode << '\n';
        matchDbFile.close();
        return true;
    }
    return false;
}

std::vector<std::string> ShareCodeCache::read(std::istream& input)
{
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(input, token) && !token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

ShareCodeCache::~ShareCodeCache()
{ matchDbFile.close(); }