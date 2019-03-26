#ifndef VersionAndConstants_h__
#define VersionAndConstants_h__

// Application Name
#define CSGO_CLI_BINARYNAME "csgo_cli"

// Application Version
#define CSGO_CLI_VERSION_MAJOR    "1"
#define CSGO_CLI_VERSION_MINOR    "0"
#define CSGO_CLI_VERSION_REVISION "2"
#define CSGO_CLI_VERSION CSGO_CLI_VERSION_MAJOR "." CSGO_CLI_VERSION_MINOR "." CSGO_CLI_VERSION_REVISION

//STEAM API connection timeouts in milliseconds
#define CSGO_CLI_STEAM_CALLBACK_INTERVAL			50     // time between callback requests in the loop
#define CSGO_CLI_STEAM_CMSG_TIMEOUT					15000  // time to wait for answer from steam
#define CSGO_CLI_STEAM_HELLO_DELAY					2000   // wait before requesting matchmaking hello (user infos)
#define CSGO_CLI_STEAM_MATCHLIST_DELAY				2000   // wait before requesting matchlist

struct CSGO_CLI_TimeoutException { };

#endif // VersionAndConstants_h__
