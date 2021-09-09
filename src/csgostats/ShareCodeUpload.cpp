#include "ShareCodeUpload.h"
#include "../VersionAndConstants.h"

#include <iostream>
#include <thread>

ShareCodeUpload::ShareCodeUpload(bool verboseMode)
{
    verbose = verboseMode;

    curl = curl_easy_init();

    host = curl_slist_append(NULL, "csgostats.gg:80:104.16.222.55");
    host = curl_slist_append(host, "csgostats.gg:443:104.16.222.55");
}

ShareCodeUpload::~ShareCodeUpload()
{
    curl_slist_free_all(host);

    curl_easy_cleanup(curl);
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    size_t oldLength = s->size();

    try {
        s->resize(oldLength + newLength);
    } catch (std::bad_alloc &e) {
        // cast to void (formerly self-assign) to avoid unused/unreferenced variable e
        static_cast<void>(e);
        // handle memory problem
        return 0;
    }

    std::copy((char *)contents, (char *)contents + newLength, s->begin() + oldLength);

    return size * nmemb;
}

/*
  POST the CSGO Demo Sharecode to csgostats.gg
*/
int ShareCodeUpload::uploadShareCode(std::string shareCode, std::string &responseContent)
{
    CURLcode res;

    char errorBuffer[CURL_ERROR_SIZE];

    // set the error buffer as empty before performing a request
    errorBuffer[0] = 0;

    // provide a buffer for storing errors
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

    curl_easy_setopt(curl, CURLOPT_RESOLVE, host);

    // 1. URL that is about to receive our POST data
    curl_easy_setopt(curl, CURLOPT_URL, "https://csgostats.gg/match/upload/ajax");

    // 2. build data to POST
    std::string postData("sharecode=");
    postData.append(shareCode); // this is the raw sharecode, not an URL. escaping not needed.
    postData.append("&index=0");

    // 3. set data to POST
    const char *data = postData.c_str();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(data));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    // prepare user-agent identifier
    char ua_ident[100];
    sprintf(ua_ident, "User-Agent: Mozilla/5.0 (compatible; %s)", CSGO_CLI_USERAGENT_ID);

    // 4. set headers
    struct curl_slist *headers = NULL;
    headers                    = curl_slist_append(headers, "Accept: application/json, text/javascript, */*; q=0.01");
    headers                    = curl_slist_append(headers, "Accept-Language: en-US;q=0.8,en;q=0.7");
    headers                    = curl_slist_append(headers, ua_ident);
    headers                    = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 5. follow+timeout
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    // 6. SSL
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // only for https

    // 7. setup method to handle the response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseContent);

    // 8. enable verbose mode
    if (verbose) { curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); }

    // perform the request
    res = curl_easy_perform(curl);

    // show error infos
    if (res != CURLE_OK) {
        // if the request did not complete correctly, show the error information
        size_t len = strlen(errorBuffer);
        fprintf(stderr, "\nlibcurl: (%d) ", res);
        if (len) {
            fprintf(stderr, "%s%s", errorBuffer, ((errorBuffer[len - 1] != '\n') ? "\n" : ""));
        } else {
            // if no detailed error information was written to errorBuffer,
            // show the more generic information from curl_easy_strerror instead
            fprintf(stderr, "%s\n", curl_easy_strerror(res));
        }

        // cleanup
        curl_easy_cleanup(curl);
        // free the custom headers
        curl_slist_free_all(headers);

        // when CURL is NOT OK, return false
        return 1;
    }

    // output response
    if (verbose) {
        std::cout << "\n\n[LOG] [UploadShareCode] [POST Request] ResponseContent:\n";
        std::cout << "---\n" << responseContent << "\n---\n";
    }

    curl_easy_reset(curl);

    // when CURL is OK, return 0
    return 0;
}

/**
 * return codes:
 * 0. should never happen :) "and you may ask yourself: how did i get here?"
 * 1. Error: response empty or response content HTML, instead of JSON
 * 2. Error: JSON parsing failed
 * 3. Server-Side: Error
 * 4. Server-Side: Match queued / processing demo file
 * 5. Server-Side: complete
 */
int ShareCodeUpload::processJsonResponse(std::string &jsonResponse)
{
    // response empty?
    if (jsonResponse.empty()) {
        printError("Error", "The response content is empty.\n");
        return 1;
    }

    // make sure response is JSON and not HTML
    if (jsonResponse.rfind("<!doctype html>", 0) == 0 || jsonResponse.rfind("<!DOCTYPE html>", 0) == 0) {
        // if HTML, check if we hit the Cloudflare Captcha page
        if (jsonResponse.find("Cloudflare") != std::string::npos) {
            printError("Error", "The response content is not JSON, but HTML (Cloudflare Captcha!).\n");
            return 1;
        }
        printError("Error", "The response content is not JSON, but HTML.\n");
        return 1;
    }

    // parse response as json, catch parsing errors
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(jsonResponse);
    } catch (nlohmann::json::parse_error &e) {
        const auto msg = fmt::format(
            "Message: {}\n Exception Id: {}\n Byte position of parsing error: {}\n", e.what(), e.id, e.byte);
        printError("JsonParsingError", msg.c_str());
        return 2;
    }

    // ensure that the keys "status" and "data" are present
    if (!json.contains("status") && !json.contains("data")) {
        fmt::print("Response: {}", jsonResponse);
        printError("Error", "Json Response does not contain the keys \"status\" and \"data\".");
        return 2;
    }
    const auto status = json["status"].get<std::string>();
    const auto data   = json["data"];

    /*
       csgostats.gg has 4 json responses to a sharecode POST request: error, queued, retrying, complete.
    */

    if (status == "error") {
        const std::string msg = data["msg"].get<std::string>();

        const auto result = fmt::format(" Result: {} -> {}. \n", status, msg);
        printRed(result);

        return 3;
    }

    if (status == "queued" || status == "retrying") {
        const std::string msg = data["msg"].get<std::string>();

        // msg contains HTML crap, let's cut that out
        std::string msgHtml = msg;
        std::string newMsg(" ");

        // get the "in queue #X" part (start of value (char 0) to char "<"span)
        const std::string queuedString = msgHtml.substr(0, msgHtml.find("<"));
        newMsg.append(queuedString);

        // get the "time remaining part (start of value (char "~" + 1) to end)
        const std::string timeString = msgHtml.substr(msgHtml.find("~") + 1, -1);
        newMsg.append(timeString);

        const std::string url = data["url"].get<std::string>();

        const auto result = fmt::format(" Result: {} -> {} | {} \n", status, url, newMsg);
        printDarkOrange(result);

        return 4;
    }

    if (status == "complete") {
        const std::string url = data["url"].get<std::string>();

        const auto result = fmt::format(" Result: {} -> {} \n", status, url);
        printGreen(result);

        return 5;
    }

    return 0;
}