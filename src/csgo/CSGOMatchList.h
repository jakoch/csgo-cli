// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGO_CSGOMATCHLIST_H_
#define SRC_CSGO_CSGOMATCHLIST_H_

#include "CSGOClient.h"

#include <condition_variable>
#include <string>
#include <vector>

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

    std::vector<CDataGCCStrike15_v2_MatchInfo> const & Matches() const;

    int getOwnIndex(CMsgGCCStrike15_v2_MatchmakingServerRoundStats const & roundStats) const;
    int getPlayerIndex(uint32 accountId, CMsgGCCStrike15_v2_MatchmakingServerRoundStats const & roundStats) const;
    std::string getMatchResult(CMsgGCCStrike15_v2_MatchmakingServerRoundStats const & roundStats) const;
    int getMatchResultNum(CMsgGCCStrike15_v2_MatchmakingServerRoundStats const & roundStats) const;

    CMsgGCCStrike15_v2_MatchList matchList;

private:
    void OnMatchList(CMsgGCCStrike15_v2_MatchList const & msg);

private:
    bool m_updateComplete = false;
    std::condition_variable m_updateCv;
    mutable std::mutex m_matchMutex;
    std::vector<CDataGCCStrike15_v2_MatchInfo> m_matches;
    GCMsgHandler<CMsgGCCStrike15_v2_MatchList> m_matchListHandler;
};

#endif // SRC_CSGO_CSGOMATCHLIST_H_
