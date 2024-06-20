// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DataObject.h"

std::string DataObject::getPlayerRank(int rank_type_id)
{
    int rank_id;
    if (rank_type_id == 6) {
        rank_id = rankings.at(0).id;
    }
    if (rank_type_id == 7) {
        rank_id = rankings.at(1).id;
    }
    if (rank_type_id == 10) {
        rank_id = rankings.at(2).id;
    }

    int rank = (rank_id < 0) ? rank_id - 1 : rank_id;

    return getRankName(rank);
}

std::string DataObject::getPlayerLevel()
{
    int level = (player_level < 0) ? player_level - 1 : player_level;

    return getLevelName(level);
}

int const DataObject::calcPlayerXpBase() const
{
    return player_cur_xp - 327680000; // xp minus base value, gives normalized player xp
}

std::string DataObject::getPlayerXp()
{
    return std::to_string(calcPlayerXpBase());
}

float const DataObject::getPlayerXpPercentage()
{
    return (static_cast<float>(calcPlayerXpBase()) / 5000) * 100;
}

std::string DataObject::getVacStatus()
{
    return (vac_banned == 1) ? "banned" : "ok";
}

std::string DataObject::getLevelName(int i)
{
    return levels[i];
}

std::string DataObject::getRankName(int i)
{
    return ranks[i];
}

std::string DataObject::getRankType(int i)
{
    if (i == 6) {
        return "MatchMaking";
    }
    if (i == 7) {
        return "Wingman";
    }
    if (i == 10) {
        return "DangerZone";
    }
    return "Unknown RankType";
}

std::string DataObject::getDangerzoneRankName(int i)
{
    return dangerzone_ranks[i];
}

std::string DataObject::getSteamId()
{
    return std::to_string(steam_id);
}

std::string DataObject::getSteamProfileUrl()
{
    return steam_profile_url_base + getSteamId();
}
std::string DataObject::getCanDoOverwatch()
{
    RankingInfo matchmaking_rank = rankings.at(0);
    int rank_id                  = matchmaking_rank.id;
    int rank_wins                = matchmaking_rank.wins;

    if (rank_id < 7) {
        return "Your rank is too low. " + getRankName(7) + " required.";
    }
    if (rank_wins < 150) {
        return "You don't have enough wins: 150 required.";
    }
    return "Qualified to request replays.";
}
std::string DataObject::getAverageSearchTime()
{
    return format_duration_get_minutes(global_stats.search_time_avg);
}

std::string DataObject::getPenaltyReasonShort(int i)
{
    return penalty_reasons_short[i];
}

std::string DataObject::getPenaltyReasonLong(int i)
{
    return penalty_reasons_long[i];
}

/*std::string DataObject::getDemoFilename(const CDataGCCStrike15_v2_MatchInfo&
match, const CMsgGCCStrike15_v2_MatchmakingServerRoundStats& roundstats) {
    std::ostringstream out;
    out << "match730_";
    out << std::setfill('0') << std::setw(21) << roundstats.reservationid();
    out << "_" << std::setw(10) << match.watchablematchinfo().tv_port();
    out << "_" << match.watchablematchinfo().server_ip() << ".dem";
    return out.str();
}*/
