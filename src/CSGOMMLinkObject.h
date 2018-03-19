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
    std::string playername;

    uint32 rank_id=0;
    std::string rank_str;
    uint32 rank_wins=0;
    float  rank_change;

    uint32 cmd_friendly=0;
    uint32 cmd_teaching=0;
    uint32 cmd_leader=0;

    uint32 player_level = 0;
    std::string player_level_str;
    uint32 player_cur_xp;

    uint32 medals_arms;
    uint32 medals_combat;
    uint32 medals_global;
    uint32 medals_team;
    uint32 medals_weapon;

    uint32 penalty_reason;
    uint32 penalty_seconds;

    uint32 vac_banned = 0;

    std::vector<CSGOMatchData> matches;
};

#endif // CSGOMMLinkObject_h__
