#ifndef CSGOMMLinkObject_h__
#define CSGOMMLinkObject_h__
#include <vector>

#include <steamtypes.h>

#include "CSGOMatchData.h"


class CSGOMMLinkObject
{
public:
    uint32 account_id;
    uint64 steam_id;
	//int steam_player_level;
    const wchar_t* playername; /* playernames might contain UTF-8 symbols */

	// ranking
    uint32 rank_id=0;
    std::string rank_str;
    uint32 rank_wins=0;
    float  rank_change;

	// commendation
    uint32 cmd_friendly=0;
    uint32 cmd_teaching=0;
    uint32 cmd_leader=0;

	// player level
    uint32 player_level = 0;
    std::string player_level_str;
    uint32 player_cur_xp;

    uint32 medals_arms;
    uint32 medals_combat;
    uint32 medals_global;
    uint32 medals_team;
    uint32 medals_weapon;

	// vac status
	uint32 vac_banned = 0;
    uint32 penalty_reason;
    uint32 penalty_seconds;
	   	
	/*uint32 global_stats_players_online = 0;
	uint32 global_stats_servers_online = 0;
	uint32 global_stats_players_searching = 0;
	uint32 global_stats_servers_available = 0;
	uint32 global_stats_ongoing_matches = 0;*/
	
    std::vector<CSGOMatchData> matches;
};

#endif // CSGOMMLinkObject_h__
