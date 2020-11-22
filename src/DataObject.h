#ifndef DataObject_H
#define DataObject_H

#include "DateTimeUtils.h"
#include "csgo/CSGOMatchData.h" // for vector<MatchData> matches
#include "cstrike15_gcmessages.pb.h"

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

    const std::vector<std::string> penalty_reasons_long = {
        "Temporary Matchmaking Cooldown (No reason)",
        "You have been kicked from your last matchmaking game.",
        "You killed too many teammates.",
        "You killed a teammate at round start.",
        "You failed to reconnect to your last match.",
        "You abandoned your last match.",
        "You dealt too much damage to your teammates.",
        "You dealt too much damage to your teammates at round start.",
        "Your account is permanently untrusted. (Illegal Angles)",
        "You were kicked from too many recent matches.",
        "Convicted by Overwatch: Majorly Disruptive",
        "Convicted by Overwatch: Minorly Disruptive",
        "Resolving Matchmaking state for your account.",
        "Resolving Matchmaking state for your last match.",
        "Your account is permanently untrusted. (VAC)",
        "Permanent Matchmaking Cooldown (No reason)",
        "You failed to connect by match start.",
        "You kicked too many teammates in recent matches.",
        "Your account is under skill placement calibration.",
        "A server using your game server token has been banned."};

    const std::vector<std::string> penalty_reasons_short = {
        "CooldownNone"
        "Kicked"
        "KilledMate"
        "RoundStartKill"
        "FailReconnect"
        "Abandon"
        "DamagedMate"
        "DamagedMateStart"
        "UntrustedAngles"
        "KickedTooMuch"
        "MajorlyDisruptive"
        "MinorlyDisruptive"
        "ResolveState"
        "ResolveStateLastMatch"
        "UntrustedVac"
        "PermanentCooldownNone"
        "FailConnect"
        "KickedMates"
        "NewbieCooldown"
        "GameServerBanned"};

    const std::string steam_profile_url_base = "https://steamcommunity.com/profiles/";

    // TODO
    const std::string tpl_url_wingman_replays = "https://steamcommunity.com/id/{}/gcpd/730/?tab=matchhistorywingman";

    const int calcPlayerXpBase() const;

public:
    std::string getSteamId();
    std::string getSteamProfileUrl();
    std::string getPlayerLevel();
    std::string getPlayerRank(int rank_type_id);
    std::string getPlayerXp();
    const float getPlayerXpPercentage();
    std::string getVacStatus();
    std::string getLevelName(int i);
    std::string getRankName(int i);
    std::string getRankType(int i);
    std::string getDangerzoneRankName(int i);
    std::string getCanDoOverwatch();
    std::string getAverageSearchTime();
    std::string getPenaltyReasonShort(int i);
    std::string getPenaltyReasonLong(int i);

    // SteamUser
    uint32 account_id      = 0;
    uint64 steam_id        = 0;
    int steam_player_level = 0;
    std::string steam_profile_url;

    // SteamFriends -> playername, clan_name, clan_tag
    std::string playername;
    std::string clan_name;
    std::string clan_tag;

    // ranking
    struct RankingInfo
    {
        uint32 id   = 0;
        uint32 type = 0;
        uint32 wins = 0;
        float change;
    };
    std::vector<RankingInfo> rankings;

    // commendation
    uint32 cmd_friendly = 0;
    uint32 cmd_teaching = 0;
    uint32 cmd_leader   = 0;

    // player level
    int32 player_level  = 0;
    int32 player_cur_xp = 0; // starts at 327680000 (level % = (player_cur_xp - 327680000) / 5000)
    int32 player_xp_bonus_flags = 0; // TODO

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

    // matches
    bool has_matches_played = 0;
    int num_matches_played  = 0;
    std::vector<CSGOMatchData> matches;

    // statistics
    struct GameTypeStats
    {
        uint32 game_type         = 0;
        uint32 players_searching = 0;
        uint32 search_time_avg   = 0;
    };
    struct GlobalStats
    {
        uint32 players_online    = 0;
        uint32 servers_online    = 0;
        uint32 players_searching = 0;
        uint32 servers_available = 0;
        uint32 ongoing_matches   = 0;
        uint32 search_time_avg   = 0;
        std::vector<GameTypeStats> gameTypeStats;
    };
    GlobalStats global_stats;
};
#endif
