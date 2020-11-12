#ifndef DataObject_H
#define DataObject_H

#include "csgo/CSGOMatchData.h" // for vector<MatchData> matches

#include <ostream>
#include <steam/steamtypes.h>
#include <vector>

class DataObject
{
private:
    const std::vector<std::string> ranks = {
        "Not Ranked",
        "Silver 1",
        "Silver 2",
        "Silver 3",
        "Silver 4",
        "Silver Elite",
        "Silver Elite Master",
        "Gold Nova 1",
        "Gold Nova 2",
        "Gold Nova 3",
        "Gold Nova 4",
        "Master Guardian 1",             // single AK
        "Master Guardian 2",             // AK with wings
        "Master Guardian Elite",         // double AK
        "Distinguished Master Guardian", // star
        "Legendary Eagle",               // eagle
        "Legendary Eagle Master",        // eagle with wings
        "Supreme Master First Class",    // small globe
        "Global Elite",                  // big globe
    };

    const std::vector<std::string> levels = {
        "Not Recruited",      "Recruit",           "Private I",          "Private II",
        "Private III",        "Corporal I",        "Corporal II",        "Corporal III",
        "Corporal IV",        "Sergeant I",        "Sergeant II",        "Sergeant III",
        "Sergeant IV",        "Master Sergeant I", "Master Sergeant II", "Master Sergeant III",
        "Master Sergeant IV", "Sergeant Major I",  "Sergeant Major II",  "Sergeant Major III",
        "Sergeant Major IV",  "Lieutenant I",      "Lieutenant II",      "Lieutenant III",
        "Lieutenant IV",      "Captain I",         "Captain II",         "Captain III",
        "Captain IV",         "Major I",           "Major II",           "Major III",
        "Major IV",           "Colonel I",         "Colonel II",         "Colonel III",
        "Brigadier General",  "Major General",     "Lieutenant General", "General",
        "Global General"};

    const std::vector<std::string> dangerzone_ranks = {
        "Hidden",
        "Lab Rat I",
        "Lab Rat II",
        "Sprinting Hare I",
        "Sprinting Hare II",
        "Wild Scout I",
        "Wild Scout II",
        "Wild Scout Elite",
        "Hunter Fox I",
        "Hunter Fox II",
        "Hunter Fox II",
        "Hunter Fox Elite",
        "Timber Wolf",
        "Ember Wolf",
        "Wildfire Wolf",
        "The Howling Alpha"};

    enum GameTypes : uint32_t
    {
        de_train    = 1032,
        de_dust2    = 520,
        de_inferno  = 4104,
        de_nuke     = 8200,
        de_vertigo  = 16392,
        cs_office   = 65544,
        de_mirage   = 32776,
        de_cache    = 1048584,
        de_zoo      = 33554440,
        cs_agency   = 134217736,
        de_overpass = 268435464,
        de_workout  = 67108872
    };

    const std::string steam_profile_url_base = "https://steamcommunity.com/profiles/";

    const int calcPlayerXpBase() const;

public:
    std::string getSteamId();
    std::string getSteamProfileUrl();
    std::string getPlayerLevel();
    std::string getPlayerRank();
    std::string getPlayerXp();
    const float getPlayerXpPercentage();
    std::string getVacStatus();
    std::string getLevelName(int i);
    std::string getRankName(int i);
    std::string getDangerzoneRankName(int i);
    std::string getGameType(GameTypes &game_types);
    // std::string getDemoFilename(const CDataGCCStrike15_v2_MatchInfo& match, const
    // CMsgGCCStrike15_v2_MatchmakingServerRoundStats& roundstats);

    // SteamUser
    uint32 account_id;
    uint64 steam_id;
    int steam_player_level = 0;
    std::string steam_profile_url;

    // SteamFriends -> playername, clan_name, clan_tag
    std::string playername;
    std::string clan_name;
    std::string clan_tag;

    // ranking
    uint32 rank_id   = 0;
    uint32 rank_wins = 0;
    float rank_change;

    // commendation
    uint32 cmd_friendly = 0;
    uint32 cmd_teaching = 0;
    uint32 cmd_leader   = 0;

    // player level
    int32 player_level = 0;
    // current XP, starts at 327680000 (level % = (player_cur_xp - 327680000) / 5000)
    int32 player_cur_xp = 0;

    // medals
    uint32 medals_arms   = 0;
    uint32 medals_combat = 0;
    uint32 medals_global = 0;
    uint32 medals_team   = 0;
    uint32 medals_weapon = 0;

    // vac status
    int32 vac_banned       = 0;
    uint32 penalty_reason  = 0;
    uint32 penalty_seconds = 0;

    /*uint32 global_stats_players_online = 0;
    uint32 global_stats_servers_online = 0;
    uint32 global_stats_players_searching = 0;
    uint32 global_stats_servers_available = 0;
    uint32 global_stats_ongoing_matches = 0;*/

    bool has_matches_played = 0;
    int num_matches_played  = 0;
    std::vector<CSGOMatchData> matches;
};
#endif
