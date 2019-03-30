#include "DataObject.h"

std::string DataObject::getPlayerLevel()
{
	return getLevelName(player_level - 1);
}

std::string DataObject::getPlayerRank()
{
	return getRankName(rank_id - 1);
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
