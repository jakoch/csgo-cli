#ifndef CSGOMatchPlayerScore_H
#define CSGOMatchPlayerScore_H

#include "CSGOClient.h"

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
        int32 mvps=0;
        int32 score=0;
};

#endif
