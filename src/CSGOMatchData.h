#ifndef CSGOMatchData_H
#define CSGOMatchData_H

#include "CSGOMatchPlayerScore.h"
#include <steam/steamtypes.h>
#include <vector>
#include <string>

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
        uint32 gametype;   /* roundstatsall.reservation.game_type | watchablematchinfo.game_type */

        uint32 spectators;

        std::vector<CSGOMatchPlayerScore> scoreboard;

        int result;
        std::string result_str;

        int score_ally;
        int score_enemy;
};

#endif
