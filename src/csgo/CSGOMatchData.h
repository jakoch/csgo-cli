// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGO_CSGOMATCHDATA_H_
#define SRC_CSGO_CSGOMATCHDATA_H_

#include <fmt/color.h>
#include <fmt/format.h>
#include <steam/steamtypes.h>

#include "CSGOMatchPlayerScore.h"

#include <string>
#include <vector>

struct CSGOMatchData
{
private:
    // TODO(jakoch): find out, if this is correct?
    // the idea is that "lastRound.reservation.game_type" is the "map_name"
    enum GameTypes
    {
        de_dust2    = 520,
        de_train    = 1032,
        de_inferno  = 4104,
        de_nuke     = 8200,
        de_vertigo  = 16392,
        cs_office   = 65544,
        de_mirage   = 32776,
        de_cache    = 1048584,
        de_workout  = 67108872,
        de_zoo      = 33554440,
        cs_agency   = 134217736,
        de_overpass = 268435464
    };

    std::string getGameTypeStr(/*GameTypes game_types*/ int32 game_type) const
    {
        switch (game_type) {
        case GameTypes::de_train:
            return "de_train";
        case GameTypes::de_dust2:
            return "de_dust2";
        case GameTypes::de_inferno:
            return "de_inferno";
        case GameTypes::de_nuke:
            return "de_nuke";
        case GameTypes::de_vertigo:
            return "de_vertigo";
        case GameTypes::cs_office:
            return "cs_office";
        case GameTypes::de_mirage:
            return "de_mirage";
        case GameTypes::de_cache:
            return "de_cache";
        case GameTypes::de_zoo:
            return "de_zoo";
        case GameTypes::cs_agency:
            return "cs_agency";
        case GameTypes::de_overpass:
            return "de_overpass";
        case GameTypes::de_workout:
            return "de_workout";
            // omit default case to trigger compiler warning for missing cases
        }
        return std::to_string(static_cast<std::uint32_t>(game_type));
    };

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
    std::string replaylink; /* roundstats.map */

    std::string map;      /* watchablematchinfo.game_map */
    std::string mapgroup; /* watchablematchinfo.game_mapgroup */
    uint32 game_type;     /* roundstatsall.reservation.game_type | watchablematchinfo.game_type */

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

    // @todo(jakoch): how to get mapname?
    std::string getMapname() const
    {
        return (map.empty() ? "? " : map);
    }

    std::string getMatchResult() const
    {
        if (result_str == "LOSS") {
            return fmt::format(fmt::fg(fmt::color::red), "LOSS");
        } else if (result_str == "WIN") {
            return fmt::format(fmt::fg(fmt::color::green), "WIN");
        } else { // result_str == "TIE"
            return fmt::format(fmt::fg(fmt::color::yellow), "TIE");
        }
    }

    std::string getGameType() const
    {
        return getGameTypeStr(game_type);
    }
};

#endif  // SRC_CSGO_CSGOMATCHDATA_H_
