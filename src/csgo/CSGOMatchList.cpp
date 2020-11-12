#include "CSGOMatchList.h"
#include "../ExceptionHandler.h"
#include "../VersionAndConstants.h"

#include <algorithm>
#include <iostream>
#include <thread>

/*
  request_recent_user_games
*/

CSGOMatchList::CSGOMatchList() : m_matchListHandler(this, &CSGOMatchList::OnMatchList)
{
    CSGOClient::GetInstance()->RegisterHandler(k_EMsgGCCStrike15_v2_MatchList, &m_matchListHandler);
}

CSGOMatchList::~CSGOMatchList()
{
    CMsgGCCStrike15_v2_MatchList matchlist;

    for (auto &m : m_matches) {
        matchlist.add_matches()->CopyFrom(m);
    }

    CSGOClient::GetInstance()->RemoveHandler(k_EMsgGCCStrike15_v2_MatchList, &m_matchListHandler);
}

void CSGOMatchList::OnMatchList(const CMsgGCCStrike15_v2_MatchList &msg)
{
    std::unique_lock<std::mutex> lock(m_matchMutex);

    for (auto it = msg.matches().rbegin(); it != msg.matches().rend(); ++it) {
        m_matches.push_back(*it);
    }

    m_updateComplete = true;
    lock.unlock();
    m_updateCv.notify_all();
}

void CSGOMatchList::Refresh()
{
    uint32 accountId = SteamUser()->GetSteamID().GetAccountID();

    CMsgGCCStrike15_v2_MatchListRequestRecentUserGames request;
    request.set_accountid(accountId);

    if (CSGOClient::GetInstance()->SendGCMessage(k_EMsgGCCStrike15_v2_MatchListRequestRecentUserGames, &request) !=
        k_EGCResultOK) {
        throw ExceptionHandler("Failed to send EMsgGCCStrike15_v2_MatchListRequestRecentUserGames");
    }
}

void CSGOMatchList::RefreshWait()
{
    m_updateComplete = false;
    Refresh();
    std::unique_lock<std::mutex> lock(m_matchMutex);

    m_updateCv.wait_for(lock, std::chrono::milliseconds(CSGO_CLI_STEAM_CMSG_TIMEOUT));

    if (!m_updateComplete) { throw CSGO_CLI_TimeoutException(); }
}

const std::vector<CDataGCCStrike15_v2_MatchInfo> &CSGOMatchList::Matches() const
{
    std::lock_guard<std::mutex> lock(m_matchMutex);
    return m_matches;
}

int CSGOMatchList::getOwnIndex(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const
{
    uint32 accountId = SteamUser()->GetSteamID().GetAccountID();

    for (int i = 0; i < roundStats.reservation().account_ids().size(); ++i) {
        if (roundStats.reservation().account_ids(i) == accountId) { return i; }
    }

    throw ExceptionHandler("Unable to find own AccountID in matchinfo.");
}

int CSGOMatchList::getPlayerIndex(
    uint32 accountId, const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const
{
    for (int i = 0; i < roundStats.reservation().account_ids().size(); ++i) {
        if (roundStats.reservation().account_ids(i) == accountId) { return i; }
    }

    throw ExceptionHandler("Unable to find specified AccountID in matchinfo.");
}

std::string CSGOMatchList::getMatchResult(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const
{
    const int num = getMatchResultNum(roundStats);
    if (num == 0) return "TIE";
    if (num == 1) return "WIN";
    return "LOSS";
}

int CSGOMatchList::getMatchResultNum(const CMsgGCCStrike15_v2_MatchmakingServerRoundStats &roundStats) const
{
    const auto ownIndex = getOwnIndex(roundStats);
    if (roundStats.match_result() == 0) {
        return 0; // tie
    }
    if (roundStats.match_result() == 1 && ownIndex <= 4) {
        return 1; // win
    }
    if (roundStats.match_result() == 2 && ownIndex >= 5) {
        return 1; // win
    }
    return 2; // loss
}
