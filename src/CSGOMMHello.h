#ifndef CSGOMMHello_h__
#define CSGOMMHello_h__

#include "CSGOClient.h"
#include <condition_variable>


class CSGOMMHello
{
public:

    CSGOMMHello();
    ~CSGOMMHello();

    void Refresh();
    void RefreshWait();

    CMsgGCCStrike15_v2_MatchmakingGC2ClientHello data;

private:
    void OnMMHello(const CMsgGCCStrike15_v2_MatchmakingGC2ClientHello& msg);

private:
    bool m_updateComplete = false;
    std::condition_variable m_updateCv;
    mutable std::mutex m_mmhelloMutex;
    GCMsgHandler<CMsgGCCStrike15_v2_MatchmakingGC2ClientHello> m_mmhelloHandler;
};

#endif // CSGOMMHello_h__
