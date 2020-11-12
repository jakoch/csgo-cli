#ifndef CSGOMatchList_H
#define CSGOMatchList_H

#include "CSGOClient.h"

#include <condition_variable>

/**
 * utility class to store and update the match list
 */
class CSGOMatchList
{
public:
    CSGOMatchList();
    ~CSGOMatchList();

    void Refresh();
    void RefreshWait();

    const std::vector<CDataGCCStrike15_v2_MatchInfo> &Matches() const;

    int getOwnIndex(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const;
    int getPlayerIndex(uint32 accid, const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const;
    std::string getMatchResult(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const;
    int getMatchResultNum(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const;

    CMsgGCCStrike15_v2_MatchList matchList;

private:
    void OnMatchList(const CMsgGCCStrike15_v2_MatchList &msg);

private:
    bool m_updateComplete = false;
    std::condition_variable m_updateCv;
    mutable std::mutex m_matchMutex;
    std::vector<CDataGCCStrike15_v2_MatchInfo> m_matches;
    GCMsgHandler<CMsgGCCStrike15_v2_MatchList> m_matchListHandler;
};

#endif
