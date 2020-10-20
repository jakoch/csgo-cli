#include "VersionAndConstants.h"
#include "ShareCodeUpload.h"

#include <thread>
#include <iostream>

ShareCodeUpload::ShareCodeUpload(bool verboseMode)
{
    verbose    = verboseMode;
    curl = initCurlConnection();

    struct curl_slist *host = NULL;
    host = curl_slist_append(NULL, "csgostats.gg:80:104.18.76.107");
    host = curl_slist_append(host, "csgostats.gg:443:104.18.76.107");
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
    }
    catch (std::bad_alloc &e) {
        // cast to void (formerly self-assign) to avoid unused/unreferenced variable e
        static_cast<void>(e);
        //handle memory problem
        return 0;
    }

    std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);

    return size * nmemb;
}

/*
Perform an inital GET request to csgostats.gg to get a connection and COOKIES.
*/
CURL* ShareCodeUpload::initCurlConnection()
{
    CURL *curl = curl_easy_init();

    if (curl) {

        CURLcode res;
        char errorBuffer[CURL_ERROR_SIZE];

        // set the error buffer as empty before performing a request
        errorBuffer[0] = 0;

        // provide a buffer for storing errors
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

        curl_easy_setopt(curl, CURLOPT_RESOLVE, host);

        // 1. URL that is about to receive our GET request
        curl_easy_setopt(curl, CURLOPT_URL, "https://csgostats.gg/");

        // 2. we dont' want the body content (just the headers, including cookies)
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

        // 3. follow+timeout
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        // 4. SSL
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); //only for https

        // 5. enable verbose mode
        if (verbose) {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        }

        // prepare user-agent identifier
        char ua_ident[100];
        sprintf(ua_ident, "User-Agent: Mozilla/5.0 (compatible; %s)", CSGO_CLI_USERAGENT_ID);

        // 4. set headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: */*");
        headers = curl_slist_append(headers, "Accept-Language: en-US;q=0.8,en;q=0.7");
        headers = curl_slist_append(headers, ua_ident);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 5. start the cookie engine
        struct curl_slist *my_cookies;
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
        curl_easy_setopt(curl, CURLOPT_COOKIELIST, &my_cookies);

        // perform the request
        res = curl_easy_perform(curl);

        // show error infos
        if (res != CURLE_OK) {
            // if the request did not complete correctly, show the error information
            size_t len = strlen(errorBuffer);
            fprintf(stderr, "\nlibcurl: (%d) ", res);
            if (len) {
                fprintf(stderr, "%s%s", errorBuffer, ((errorBuffer[len - 1] != '\n') ? "\n" : ""));
            }
            else {
                // if no detailed error information was written to errorBuffer,
                // show the more generic information from curl_easy_strerror instead
                fprintf(stderr, "%s\n", curl_easy_strerror(res));
            }

            // cleanup
            curl_easy_cleanup(curl);
            // free the custom headers
            curl_slist_free_all(headers);

            // when CURL is NOT OK, exit
            exit(1);
        }

        // RESET
        // keeps these info in the curl handle:
        // live connections, Session ID cache, DNS cache, cookies and shares.
        curl_easy_reset(curl);

        // --------------------

        // WAIT 1sec
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else
    {   // something's gone wrong with curl at the beginning
        fprintf(stderr, "Curl init failed!\n");

        exit(1);
    }

    return curl;
}

/*
  POST the CSGO Demo Sharecode to csgostats.gg
*/
int ShareCodeUpload::uploadShareCode(std::string shareCode, std::string& responseContent)
{
    CURLcode res;
    char errorBuffer[CURL_ERROR_SIZE];

    // set the error buffer as empty before performing a request
    errorBuffer[0] = 0;

    curl_easy_setopt(curl, CURLOPT_RESOLVE, host);

    // 1. URL that is about to receive our POST data
    curl_easy_setopt(curl, CURLOPT_URL, "https://csgostats.gg/match/upload/ajax");

    // 2. build data to POST
    std::string postData("sharecode=");
    postData.append(shareCode); // this is the raw sharecode, not an URL. escaping not needed.
    postData.append("&index=0");
    //std::cout << postData << std::endl;

    // 3. set data to POST
    const char *data = postData.c_str();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(data));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    // prepare user-agent identifier
    char ua_ident[100];
    sprintf(ua_ident, "User-Agent: Mozilla/5.0 (compatible; %s)", CSGO_CLI_USERAGENT_ID);

    // 4. set headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: */*"); // application/json?
    headers = curl_slist_append(headers, "Accept-Language: en-US;q=0.8,en;q=0.7");
    headers = curl_slist_append(headers, ua_ident);
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 5. follow+timeout
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    // 6. SSL
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); //only for https

    // 7. setup method to handle the response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseContent);

    // 8. enable verbose mode
    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }

    // perform the request
    res = curl_easy_perform(curl);

    // show error infos
    if (res != CURLE_OK) {
        // if the request did not complete correctly, show the error information
        size_t len = strlen(errorBuffer);
        fprintf(stderr, "\nlibcurl: (%d) ", res);
        if (len) {
            fprintf(stderr, "%s%s", errorBuffer, ((errorBuffer[len - 1] != '\n') ? "\n" : ""));
        }
        else {
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
 * 0. ---
 * 1. Error: we got an HTML response instead of JSON
 * 2. Error: JSON parsing failed
 * 3. Server-Side: Error
 * 4. Server-Side: Match queued / processing demo file
 * 5. Server-Side: complete
 */
int ShareCodeUpload::processJsonResponse(std::string& jsonResponse)
{
    // Error: we got an HTML response instead of JSON
    if (jsonResponse.rfind("<!doctype html>", 0) == 0) {
        fprintf(stderr, "\nError: Response content is not JSON, but HTML.\n");
        return 1;
    }

    using namespace rapidjson;

    Document document;
    document.Parse(jsonResponse.c_str());

    // Error: JSON parsing failed
    if (document.HasParseError()) {
        fprintf(stderr, "\nError(offset %u): %s\n%s\n",
            (unsigned)document.GetErrorOffset(),
            GetParseError_En(document.GetParseError()),
            jsonResponse.substr(0, (int)document.GetErrorOffset() + 1).c_str()
        );
        return 2;
    }

    Value::MemberIterator status = document.FindMember("status");
    Value::MemberIterator data = document.FindMember("data");

    /*
       csgostats.gg has the following 3 response cases for a sharecode POST request
       the keys status and data are always present. rest is changing.
       see testProcessJsonResponse() below.
    */

    if (strcmp("error", status->value.GetString()) == 0) {
        Value::MemberIterator msg = data->value.FindMember("msg");

        printf(" %s    | %s. \n", status->value.GetString(), msg->value.GetString());

        return 3;
    }

    else if (strcmp("queued", status->value.GetString()) == 0) {
        Value::MemberIterator msg = data->value.FindMember("msg");

        // msg contains HTML crap, let's cut that out
        std::string msgHtml = msg->value.GetString();
        std::string newMsg(" ");

        // get the "in queue #X" part (start of value (char 0) to char "<"span)
        std::string queuedString = msgHtml.substr(0, msgHtml.find("<"));
        newMsg.append(queuedString);

        // get the "time remaining part (start of value (char "~" + 1) to end)
        std::string timeString = msgHtml.substr(msgHtml.find("~") + 1, -1);
        newMsg.append(timeString);

        Value::MemberIterator queue_id = data->value.FindMember("queue_id");
        Value::MemberIterator url = data->value.FindMember("url");

        printf(" %s   | %i | %s | %s \n", status->value.GetString(), queue_id->value.GetInt(), url->value.GetString(), newMsg.c_str());

        return 4;
    }

    else if (strcmp("complete", status->value.GetString()) == 0) {
        Value::MemberIterator queue_id = data->value.FindMember("queue_id");
        Value::MemberIterator url = data->value.FindMember("url");

        printf(" %s | %i | %s \n", status->value.GetString(), queue_id->value.GetInt(), url->value.GetString());

        return 5;
    }

    return 0;
}

/*
int ShareCodeUpload::testProcessJsonResponse()
{
    // csgostats.gg has the following 3 response cases for a sharecode POST request:

    static const char *JSON_ERROR_STRING =
    "{"
        "\"status\": \"error\","
        "\"data\" : {"
            "\"msg\": \"Failed to add to the queue, please verify sharecode\","
            "\"index\" : null,"
            "\"sharecode\" : null"
        "},"
        "\"error\" : 1"
    "}";

    static const char *JSON_QUEUED_STRING =
    "{"
        "\"status\": \"queued\","
        "\"data\" : {"
            "\"msg\": \"in Queue #9  <span style=\\\"margin-left:8px;\\\"><\/span> ~ Time Remaining 4m 50s\","
            "\"queue_id\" : 1827459,"
            "\"queue_pos\" : 9,"
            "\"in_queue\" : 1,"
            "\"demo_id\" : 0,"
            "\"url\" : \"https:\/\/csgostats.gg\/match\/processing\/1827459\","
            "\"demo_url\" : null,"
            "\"start\" : false,"
            "\"index\" : 0,"
            "\"sharecode\" : \"CSGO-R7CCX-WRquC-xFxQO-hvFHt-uMOcF\""
        "},"
        "\"error\" : 0"
    "}";

    static const char *JSON_COMPLETE_STRING =
    "{"
        "\"status\": \"complete\","
        "\"data\" : {"
            "\"msg\": \"Complete - <a href=\\\"\/match\/1731725\\\">View<\/a>\","
            "\"index\" : 0,"
            "\"sharecode\" : \"CSGO-c5vYt-KEmjy-j6FYV-tz8Rt-JrzUO\","
            "\"queue_id\" : 1765382,"
            "\"demo_id\" : 1731725,"
            "\"url\" : \"https:\/\/csgostats.gg\/match\/1731725\""
        "},"
        "\"error\" : 0"
    "}";

    // in-case the response content is not JSON, but HTML
    static const char *HTML_ERROR_STRING = "<!doctype html><html></html>";

    std::string html(HTML_ERROR_STRING);
    std::string error(JSON_ERROR_STRING);
    std::string queued(JSON_ERROR_STRING);
    std::string complete(JSON_COMPLETE_STRING);

    processJsonResponse(html);
    processJsonResponse(error);
    processJsonResponse(queued);
    processJsonResponse(complete);

    return 0;
}*/
