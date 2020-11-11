#ifndef CSGOMatchPlayerScore_H
#define CSGOMatchPlayerScore_H

#include "CSGOClient.h"
#include <fmt/format.h>

/**
 * This struct represents the PlayerScoreboard.
 *
 * Usage: std::vector<CSGOMatchPlayerScore> scoreboard;
 */
struct CSGOMatchPlayerScore
{
    public:
        int index=0;
        uint32 account_id=0;
        uint64 steam_id=0;
        int32 kills=0;
        int32 assists=0;
        int32 deaths=0;
        int32 kdr=0;
        int32 mvps=0;
        int32 score=0;

        std::string getKills() const;
        std::string getAssists() const;
        std::string getDeaths() const;
        std::string getKillDeathRatio() const;
        //std::string getHSRatio() const;
        std::string getMVPs() const;
        std::string getScore() const;
};

#endif
