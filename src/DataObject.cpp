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
/*std::string DataObject::getDemoFilename(const CDataGCCStrike15_v2_MatchInfo& match, const CMsgGCCStrike15_v2_MatchmakingServerRoundStats& roundstats) {
	std::ostringstream out;
	out << "match730_";
	out << std::setfill('0') << std::setw(21) << roundstats.reservationid();
	out << "_" << std::setw(10) << match.watchablematchinfo().tv_port();
	out << "_" << match.watchablematchinfo().server_ip() << ".dem";
	return out.str();
}*/
