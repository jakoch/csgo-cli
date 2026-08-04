#include <steamapi.h>

std::string STEAM_APIKEY = "76561197960493477";

REQUEST URL: http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v2/?key=79CFF2A7AF1A1BD2DE9071D05306B9D5&steamids=76561197960493477

RESPONSE:

{
    "response":
    {
        "players" : [{
            "steamid": "76561197960493477",
            "communityvisibilitystate": 3,
            "profilestate": 1,
            "personaname": "knd.stuиt",
            "commentpermission": 2,
            "profileurl": "https://steamcommunity.com/id/johnvain/",
            "avatar": "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/avatars/91/"
                      "91b7abe2eebec821dc4b4de5e801de7989a7f0b5.jpg",
            "avatarmedium": "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/avatars/91/"
                            "91b7abe2eebec821dc4b4de5e801de7989a7f0b5_medium.jpg",
            "avatarfull": "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/avatars/91/"
                          "91b7abe2eebec821dc4b4de5e801de7989a7f0b5_full.jpg",
            "avatarhash": "91b7abe2eebec821dc4b4de5e801de7989a7f0b5",
            "personastate": 0,
            "realname": "JK",
            "primaryclanid": "103582791429569405",
            "timecreated": 1063456587,
            "personastateflags": 0
        }]
    }
}