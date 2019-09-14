#pragma once

#include "dependencies\curl\include\curl\curl.h"
#include "dependencies\rapidjson\include\rapidjson\document.h" // rapidjson DOM api 
#include "dependencies\rapidjson\include\rapidjson\error\en.h" // parse error messages
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>

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

    CURL* initCurlConnection();
};

