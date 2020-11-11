#ifndef ShareCodeUpload_H
#define ShareCodeUpload_H

#include <curl/curl.h>
#include <curl/easy.h>
#include <nlohmann/json.hpp>

#include "../platform/windows/WinCliColors.h"
#include "../ErrorHandler.h"

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace WinCliColors;

class ShareCodeUpload
{
public:
    ShareCodeUpload(bool verboseMode);
    ~ShareCodeUpload();

    int uploadShareCode(std::string shareCode, std::string &responseContent);
    int processJsonResponse(std::string &jsonResponse);

    //int testProcessJsonResponse();

  private:
    CURL *curl = nullptr;
    bool verbose = false;

    struct curl_slist *host = NULL;

    CURL* initCurlConnection();
};

#endif
