#ifndef VersionAndConstants_h__
#define VersionAndConstants_h__

// Application Name
#define CSGO_MM_LINK_BINARYNAME "CSGO_CLI"

// Application Version
#define CSGO_CLI_VERSION_MAJOR    "1"
#define CSGO_CLI_VERSION_MINOR    "0"
#define CSGO_CLI_VERSION_REVISION "0"
#define CSGO_CLI_VERSION CSGO_CLI_VERSION_MAJOR "." CSGO_CLI_VERSION_MINOR "." CSGO_CLI_VERSION_REVISION

//STEAM API connection timeouts
#define CSGO_CLI_STEAM_GC_CONNECT_MAX		12000
#define CSGO_CLI_STEAM_CMSG_TIMEOUT	        10000
#define CSGO_CLI_STEAM_CALLBACK_INTERVAL	225
#define CSGO_CLI_STEAM_HELLO_DELAY			4000
#define CSGO_CLI_STEAM_MATCHLIST_DELAY		1250

struct CSGO_CLI_TimeoutException { };

#endif // VersionAndConstants_h__
