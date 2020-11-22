#ifndef VersionAndConstants_H
#define VersionAndConstants_H

// Application Name
#define CSGO_CLI_BINARYNAME "csgo_cli"

// Application Version
#define CSGO_CLI_VERSION_MAJOR    "1"
#define CSGO_CLI_VERSION_MINOR    "3"
#define CSGO_CLI_VERSION_REVISION "0"

#define CSGO_CLI_VERSION CSGO_CLI_VERSION_MAJOR "." CSGO_CLI_VERSION_MINOR "." CSGO_CLI_VERSION_REVISION

// CURL user-agent identifier
#define CSGO_CLI_WEBSITE      "https://github.com/jakoch/csgo-cli"
#define CSGO_CLI_USERAGENT_ID CSGO_CLI_BINARYNAME "/" CSGO_CLI_VERSION "; " CSGO_CLI_WEBSITE

// STEAM API connection timeouts in milliseconds
#define CSGO_CLI_STEAM_CALLBACK_INTERVAL 50    // time between callback requests in the loop
#define CSGO_CLI_STEAM_CMSG_TIMEOUT      10000 // time to wait for answer from steam
#define CSGO_CLI_STEAM_HELLO_DELAY       500   // wait before requesting matchmaking hello
#define CSGO_CLI_STEAM_MATCHLIST_DELAY   500   // wait before requesting matchlist

struct CSGO_CLI_TimeoutException
{
};

#endif
