#ifndef DataObject_h__
#define DataObject_h__
#include <vector>

#include <steamtypes.h>

#include "CSGOMatchData.h"

class DataObject
{
public:
	std::string getPlayerLevel();
	std::string getPlayerRank();
	std::string getVacStatus();
	std::string getLevelName(int i);
	std::string getRankName(int i);
	//std::string getDemoFilename(const CDataGCCStrike15_v2_MatchInfo& match, const CMsgGCCStrike15_v2_MatchmakingServerRoundStats& roundstats);

	uint32 account_id;
	uint64 steam_id;
	int steam_player_level = 0;
	const wchar_t* playername; /* playernames might contain UTF-8 symbols */

	// ranking
	uint32 rank_id = 0;
	std::string rank_str;
	uint32 rank_wins = 0;
	float  rank_change;

	// commendation
	uint32 cmd_friendly = 0;
	uint32 cmd_teaching = 0;
	uint32 cmd_leader = 0;

	// player level
	int32 player_level = 0;
	std::string player_level_str;
	int32 player_cur_xp = 0;

	// medals
	uint32 medals_arms = 0;
	uint32 medals_combat = 0;
	uint32 medals_global = 0;
	uint32 medals_team = 0;
	uint32 medals_weapon = 0;

	// vac status
	int32 vac_banned = 0;
	uint32 penalty_reason = 0;
	uint32 penalty_seconds = 0;

	/*uint32 global_stats_players_online = 0;
	uint32 global_stats_servers_online = 0;
	uint32 global_stats_players_searching = 0;
	uint32 global_stats_servers_available = 0;
	uint32 global_stats_ongoing_matches = 0;*/

	bool has_matches_played = 0;
	int num_matches_played = 0;
	std::vector<CSGOMatchData> matches;

private:	
	const std::vector<std::string> ranks = {
		"-unranked-",
		"Silver 1",
		"Silver 2",
		"Silver 3",
		"Silver 4",
		"Silver Elite",
		"Silver Elite Master"
		"Gold Nova 1",
		"Gold Nova 2",
		"Gold Nova 3",
		"Gold Nova 4",
		"Master Guardian 1",              // single AK
		"Master Guardian 2",              // AK with wings
		"Master Guardian Elite",          // double AK
		"Distinguished Master Guardian",  // star
		"Legendary Eagle",                // eagle
		"Legendary Eagle Master",         // eagle with wings
		"Supreme Master First Class",     // small globe
		"Global Elite",                   // big globe
	};

	const std::vector<std::string> levels =
	{
		"Not Recruited",
		"Recruit",
		"Private I",
		"Private II",
		"Private III",
		"Corporal I",
		"Corporal II",
		"Corporal III",
		"Corporal IV",
		"Sergeant I",
		"Sergeant II",
		"Sergeant III",
		"Sergeant IV",
		"Master Sergeant I",
		"Master Sergeant II",
		"Master Sergeant III",
		"Master Sergeant IV",
		"Sergeant Major I",
		"Sergeant Major II",
		"Sergeant Major III",
		"Sergeant Major IV",
		"Lieutenant I",
		"Lieutenant II",
		"Lieutenant III",
		"Lieutenant IV",
		"Captain I",
		"Captain II",
		"Captain III",
		"Captain IV",
		"Major I",
		"Major II",
		"Major III",
		"Major IV",
		"Colonel I",
		"Colonel II",
		"Colonel III",
		"Brigadier General",
		"Major General",
		"Lieutenant General",
		"General",
		"Global General"
	};
};

#endif // DataObject_h__
