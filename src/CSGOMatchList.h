#ifndef CSGOMatchList_h__
#define CSGOMatchList_h__

#include <condition_variable>
#include "CSGOClient.h"

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

    const std::vector<CDataGCCStrike15_v2_MatchInfo>& Matches() const;

    int getOwnIndex(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats & roundStats);
    int getPlayerIndex(uint32 accid, const CMsgGCCStrike15_v2_MatchmakingServerRoundStats & roundStats);
    std::string getMatchResult(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats & roundStats);
    int getMatchResultNum(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats & roundStats);
    
    CMsgGCCStrike15_v2_MatchList matchList;

private:
    //void OnClientWelcome(const CMsgClientWelcome& msg);
    void OnMatchList(const CMsgGCCStrike15_v2_MatchList& msg);
    //void LoadOldMatchList();
    //void SaveMatchList();

private:
    bool m_updateComplete = false;
    std::condition_variable m_updateCv;
    mutable std::mutex m_matchMutex;
    std::vector<CDataGCCStrike15_v2_MatchInfo> m_matches;
    GCMsgHandler<CMsgGCCStrike15_v2_MatchList> m_matchListHandler;
};

#endif // CSGOMatchList_h__
