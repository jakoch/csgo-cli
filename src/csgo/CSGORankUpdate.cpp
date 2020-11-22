#include "CSGORankUpdate.h"
#include "../ExceptionHandler.h"
#include "../VersionAndConstants.h"

#include <iostream>

CSGORankUpdate::CSGORankUpdate() : m_rankUpdateHandler(this, &CSGORankUpdate::OnRankUpdate)
{
    CSGOClient::GetInstance()->RegisterHandler(k_EMsgGCCStrike15_v2_ClientGCRankUpdate, &m_rankUpdateHandler);
}

CSGORankUpdate::~CSGORankUpdate()
{
    CSGOClient::GetInstance()->RemoveHandler(k_EMsgGCCStrike15_v2_ClientGCRankUpdate, &m_rankUpdateHandler);
}

void CSGORankUpdate::OnRankUpdate(const CMsgGCCStrike15_v2_ClientGCRankUpdate &msg)
{
    std::unique_lock<std::mutex> lock(m_rankUpdateMutex);
    data.push_back(msg);
    m_updateComplete = true;
    lock.unlock();
    m_updateCv.notify_all();
}

void CSGORankUpdate::GetWingmanRank()
{
    CMsgGCCStrike15_v2_ClientGCRankUpdate request;
    request.add_rankings()->set_rank_type_id(7);

    if (CSGOClient::GetInstance()->SendGCMessage(k_EMsgGCCStrike15_v2_ClientGCRankUpdate, &request) != k_EGCResultOK) {
        throw ExceptionHandler("Failed to send EMsgGCCStrike15_v2_ClientGCRankUpdate");
    }
}

void CSGORankUpdate::GetDangerZoneRank()
{
    CMsgGCCStrike15_v2_ClientGCRankUpdate request;
    request.add_rankings()->set_rank_type_id(10);

    if (CSGOClient::GetInstance()->SendGCMessage(k_EMsgGCCStrike15_v2_ClientGCRankUpdate, &request) != k_EGCResultOK) {
        throw ExceptionHandler("Failed to send EMsgGCCStrike15_v2_ClientGCRankUpdate");
    }
}

void CSGORankUpdate::RefreshWaitWingmanRank()
{
    m_updateComplete = false;
    GetWingmanRank();
    std::unique_lock<std::mutex> lock(m_rankUpdateMutex);

    m_updateCv.wait_for(lock, std::chrono::milliseconds(CSGO_CLI_STEAM_CMSG_TIMEOUT + 10000));

    if (!m_updateComplete) { throw CSGO_CLI_TimeoutException(); }
}

void CSGORankUpdate::RefreshWaitDangerZoneRank()
{
    m_updateComplete = false;
    GetDangerZoneRank();
    std::unique_lock<std::mutex> lock(m_rankUpdateMutex);

    m_updateCv.wait_for(lock, std::chrono::milliseconds(CSGO_CLI_STEAM_CMSG_TIMEOUT + 10000));

    if (!m_updateComplete) { throw CSGO_CLI_TimeoutException(); }
}
