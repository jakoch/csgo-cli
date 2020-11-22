#ifndef CSGORankUpdate_H
#define CSGORankUpdate_H

#include "CSGOClient.h"

#include <condition_variable>

class CSGORankUpdate
{
public:
    CSGORankUpdate();
    ~CSGORankUpdate();

    void GetWingmanRank();
    void GetDangerZoneRank();

    void RefreshWaitWingmanRank();
    void RefreshWaitDangerZoneRank();

    std::vector<CMsgGCCStrike15_v2_ClientGCRankUpdate> data;

private:
    void OnRankUpdate(const CMsgGCCStrike15_v2_ClientGCRankUpdate &msg);

private:
    bool m_updateComplete = false;
    std::condition_variable m_updateCv;
    mutable std::mutex m_rankUpdateMutex;
    GCMsgHandler<CMsgGCCStrike15_v2_ClientGCRankUpdate> m_rankUpdateHandler;
};

#endif