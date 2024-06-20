// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGOSTATS_SHARECODECACHE_H_
#define SRC_CSGOSTATS_SHARECODECACHE_H_

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

class ShareCodeCache
{
public:
    explicit ShareCodeCache(bool verboseMode);
    ~ShareCodeCache();

    bool find(std::string sharecode);
    bool insert(std::string sharecode);

private:
    std::string const csvFile = "sharecode.db";
    std::fstream matchDbFile;

    std::vector<std::string> sharecodeCache;

    std::vector<std::string> read(std::istream& input);
};

#endif // SRC_CSGOSTATS_SHARECODECACHE_H_