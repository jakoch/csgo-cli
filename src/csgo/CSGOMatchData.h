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
    // The "game_type" field in the reservation message is actually a map ID,
    // not a game mode (e.g. casual, competitive). The Steam API does not
    // provide the map name string for recent matches, so we use this numeric
    // ID from roundstats.reservation.game_type to look up the map name.
    // Reference: https://github.com/jakoch/csgo-cli/issues/3
    enum class MapIds
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

    std::string getMapNameFromId(uint32 map_id) const
    {
        switch (map_id) {
        case static_cast<uint32>(MapIds::de_train):
            return "de_train";
        case static_cast<uint32>(MapIds::de_dust2):
            return "de_dust2";
        case static_cast<uint32>(MapIds::de_inferno):
            return "de_inferno";
        case static_cast<uint32>(MapIds::de_nuke):
            return "de_nuke";
        case static_cast<uint32>(MapIds::de_vertigo):
            return "de_vertigo";
        case static_cast<uint32>(MapIds::cs_office):
            return "cs_office";
        case static_cast<uint32>(MapIds::de_mirage):
            return "de_mirage";
        case static_cast<uint32>(MapIds::de_cache):
            return "de_cache";
        case static_cast<uint32>(MapIds::de_zoo):
            return "de_zoo";
        case static_cast<uint32>(MapIds::cs_agency):
            return "cs_agency";
        case static_cast<uint32>(MapIds::de_overpass):
            return "de_overpass";
        case static_cast<uint32>(MapIds::de_workout):
            return "de_workout";
            // omit default case to trigger compiler warning for missing cases
        }
        return std::to_string(map_id);
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
    uint32 map_id;        /* roundstatsall.reservation.game_type | watchablematchinfo.game_type */

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

    std::string getMapname() const
    {
        if (!map.empty()) {
            return map;
        }
        if (map_id != 0) {
            return getMapNameFromId(map_id);
        }
        return "?";
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
};

#endif // SRC_CSGO_CSGOMATCHDATA_H_
