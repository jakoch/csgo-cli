#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

#include "../src/csgostats/ShareCodeUpload.h"

#include <iterator>

TEST_CASE("[ShareCodeUpload] Response to POST request can be parsed:", "[testProcessJsonResponse]") {

  ShareCodeUpload *shareCodeUpload = new ShareCodeUpload(false);

  SECTION("if response is empty, return 1") {
    std::string response = "";
    int r = shareCodeUpload->processJsonResponse(response);
    REQUIRE(r == 1);
  }

  SECTION("if response content is not JSON, but HTML, return 1") {
    std::string response = "<!DOCTYPE html><head></html>";
    int r = shareCodeUpload->processJsonResponse(response);
    REQUIRE(r == 1);
  }

  SECTION("if response content is not JSON, but HTML and a Cloudflare Captcha, return 1") {
    std::string response = "<!DOCTYPE html> ... Cloudflare</title>";
    int r = shareCodeUpload->processJsonResponse(response);
    REQUIRE(r == 1);
  }

  /*SECTION("if response content is JSON, but syntax - invalid JSON, return 2") {
    auto response = R"(
        {
          "status:
        }
      )"_json;
    int r = shareCodeUpload->processJsonResponse(response.dump());
    REQUIRE(r == 2);
  }*/

  SECTION("if response content is JSON, with status error, return 3") {

    auto response = R"(
    {
      "status": "error",
      "data" : {
          "msg": "Failed to add to the queue, please verify sharecode",
          "index" : null,
          "sharecode" : null
      },
      "error" : 1
    }
  )"_json;

    std::string response_string = response.dump();
    int r = shareCodeUpload->processJsonResponse(response_string);
    REQUIRE(r == 3);
  }

  SECTION("if response content is JSON, with status queued, return 4") {
    auto response = R"(
      {
        "status": "queued",
        "data" : {
            "msg": "in Queue #9  <span style=\"margin-left:8px;\"><\/span> ~ Time Remaining 4m 50s",
            "queue_id" : 1827459,
            "queue_pos" : 9,
            "in_queue" : 1,
            "demo_id" : 0,
            "url" : "https:\/\/csgostats.gg\/match\/processing\/1827459\/",
            "demo_url" : null,
            "start" : false,
            "index" : 0,
            "sharecode" : "CSGO-R7CCX-WRquC-xFxQO-hvFHt-uMOcF"
        },
        "error" : 0
      }
  )"_json;

    std::string response_string = response.dump();
    int r = shareCodeUpload->processJsonResponse(response_string);
    REQUIRE(r == 4);
  }

  SECTION("if response content is JSON, with status complete, return 5") {
    auto response = R"(
        {
          "status": "complete",
          "data" : {
              "msg": "Complete - <a href=\"\/match\/1731725\">View<\/a>",
              "index" : 0,
              "sharecode" : "CSGO-c5vYt-KEmjy-j6FYV-tz8Rt-JrzUO",
              "queue_id" : 1765382,
              "demo_id" : 1731725,
              "url" : "https:\/\/csgostats.gg\/match\/1731725"
          },
          "error" : 0
        }
      )"_json;
    std::string response_string = response.dump();
    int r = shareCodeUpload->processJsonResponse(response_string);
    REQUIRE(r == 5);
  }

};
