#include "DataObject.h"

std::string DataObject::getPlayerRank()
{
    int rank = (rank_id < 0) ? rank_id - 1 : rank_id;

    return getRankName(rank);
}

std::string DataObject::getPlayerLevel()
{
    int level = (player_level < 0) ? player_level - 1 : player_level;

    return getLevelName(level);
}

const int DataObject::calcPlayerXpBase() const
{
    return player_cur_xp - 327680000; // xp minus base value, gives normalized player xp
}

std::string DataObject::getPlayerXp()
{
    return std::to_string(calcPlayerXpBase());
}

const float DataObject::getPlayerXpPercentage()
{
    return ((float)calcPlayerXpBase() / 5000) * 100;
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

std::string DataObject::getDangerzoneRankName(int i)
{
    return dangerzone_ranks[i];
}

std::string DataObject::getSteamId() {
  return std::to_string(steam_id);
}

std::string DataObject::getSteamProfileUrl() {
  return steam_profile_url_base + getSteamId();
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

std::string DataObject::getGameType(GameTypes &game_types)
{
    switch (game_types) {
        case GameTypes::de_train    : return "de_train";
        case GameTypes::de_dust2    : return "de_dust2";
        case GameTypes::de_inferno  : return "de_inferno";
        case GameTypes::de_nuke     : return "de_nuke";
        case GameTypes::de_vertigo  : return "de_vertigo";
        case GameTypes::cs_office   : return "cs_office";
        case GameTypes::de_mirage   : return "de_mirage";
        case GameTypes::de_cache    : return "de_cache";
        case GameTypes::de_zoo      : return "de_zoo";
        case GameTypes::cs_agency   : return "cs_agency";
        case GameTypes::de_overpass : return "de_overpass";
        case GameTypes::de_workout  : return "de_workout";
        // omit default case to trigger compiler warning for missing cases
    };
    return std::to_string(static_cast<std::uint32_t>(game_types));
};