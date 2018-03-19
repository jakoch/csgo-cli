#ifndef CSGOMatchPlayerScore_h__
#define CSGOMatchPlayerScore_h__

#include <steamtypes.h>

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

#endif // CSGOMatchPlayerScore_h__
