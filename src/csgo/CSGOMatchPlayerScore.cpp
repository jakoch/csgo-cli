#include "CSGOMatchPlayerScore.h"

std::string CSGOMatchPlayerScore::getKillDeathRatio() const
{
    return fmt::format("{}", kills / deaths);
}

/*std::string CSGOMatchPlayerScore::getHSRatio() const
{
    return fmt::format("{} ({}%)", headshot, headshot_percentage = (headshots / kills) * 100));
}*/

std::string CSGOMatchPlayerScore::getKills() const
{
    return std::to_string(kills);
}
std::string CSGOMatchPlayerScore::getAssists() const
{
    return std::to_string(assists);
}
std::string CSGOMatchPlayerScore::getDeaths() const
{
    return std::to_string(deaths);
}
std::string CSGOMatchPlayerScore::getMVPs() const
{
    return std::to_string(mvps);
}
std::string CSGOMatchPlayerScore::getScore() const
{
    return std::to_string(score);
}
