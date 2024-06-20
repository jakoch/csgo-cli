// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGOSTATS_SHARECODEUPLOAD_H_
#define SRC_CSGOSTATS_SHARECODEUPLOAD_H_

#include <curl/curl.h>
#include <curl/easy.h>
#include <nlohmann/json.hpp>

#include "../ErrorHandler.h"
#include "../platform/windows/WinCliColors.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>

using WinCliColors;

class ShareCodeUpload
{
public:
    explicit ShareCodeUpload(bool verboseMode);
    ~ShareCodeUpload();

    int uploadShareCode(std::string shareCode, std::string& responseContent);
    int processJsonResponse(std::string& jsonResponse);

    // int testProcessJsonResponse();

private:
    CURL* curl   = nullptr;
    bool verbose = false;

    struct curl_slist* host = NULL;

    CURL* initCurlConnection();
};

#endif  // SRC_CSGOSTATS_SHARECODEUPLOAD_H_
