#ifndef CSGOMatchData_h__
#define CSGOMatchData_h__
#include <vector>
#include <string>

#include <steamtypes.h>

#include "CSGOMatchPlayerScore.h"

struct CSGOMatchData
{

    public:
        uint64 matchid;

        time_t matchtime;
        std::string matchtime_str;

        time_t match_duration;
        std::string match_duration_str;

        uint32 server_ip;
        uint32 tv_port;
        uint64 reservation_id;

        std::string sharecode;
        std::string demolink;   /* roundstats.map */

		std::string map;        /* watchablematchinfo.game_map */
		std::string mapgroup;   /* watchablematchinfo.game_mapgroup */
		std::string gametype;   /* watchablematchinfo.game_type */

        uint32 spectators;

        std::vector<CSGOMatchPlayerScore> scoreboard;

        int result;
        std::string result_str;

        int score_ally;
        int score_enemy;
};


#endif // CSGOMatchData_h__
