#ifndef CSGOMatchData_H
#define CSGOMatchData_H

#include "CSGOMatchPlayerScore.h"
#include <steam/steamtypes.h>
#include <vector>
#include <string>
#include <fmt/format.h>
#include <fmt/color.h>

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
        std::string replaylink;   /* roundstats.map */

        std::string map;        /* watchablematchinfo.game_map */
        std::string mapgroup;   /* watchablematchinfo.game_mapgroup */
        uint32 gametype;        /* roundstatsall.reservation.game_type | watchablematchinfo.game_type */

        uint32 spectators;

        std::vector<CSGOMatchPlayerScore> scoreboard;

        int result;
        std::string result_str;

        int score_ally;
        int score_enemy;

        std::string getScore() const
        {
          return fmt::format("{:02} : {:02}", score_ally, score_enemy);
        }

        // TODO how to get mapname?
        std::string getMapname() const
        {
          return (map.empty() ? "? " : map);
        }

        std::string getMatchResult() const
        {
          if(result_str == "LOSS") {
            return fmt::format(fmt::fg(fmt::color::red), "LOSS");
          }
          else if(result_str == "WIN") {
            return fmt::format(fmt::fg(fmt::color::green), "WIN");
          }
          else { // result_str == "TIE"
            return fmt::format(fmt::fg(fmt::color::yellow), "TIE");
          }
        }
};

#endif
