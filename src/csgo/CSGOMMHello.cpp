// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSGOMMHello.h"
#include "../ExceptionHandler.h"
#include "../VersionAndConstants.h"

#include <iostream>

CSGOMMHello::CSGOMMHello() : m_mmhelloHandler(this, &CSGOMMHello::OnMMHello)
{
    CSGOClient::GetInstance()->RegisterHandler(k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello, &m_mmhelloHandler);
}

CSGOMMHello::~CSGOMMHello()
{
    CSGOClient::GetInstance()->RemoveHandler(k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello, &m_mmhelloHandler);
}

void CSGOMMHello::OnMMHello(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello const & msg)
{
    std::unique_lock<std::mutex> lock(m_mmhelloMutex);
    data             = msg;
    m_updateComplete = true;
    lock.unlock();
    m_updateCv.notify_all();
}

void CSGOMMHello::Refresh()
{
    CMsgGCCStrike15_v2_MatchmakingClient2GCHello request;
    if (CSGOClient::GetInstance()->SendGCMessage(k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello, &request) !=
        k_EGCResultOK) {
        throw ExceptionHandler("Failed to send EMsgGCCStrike15_v2_MatchmakingClient2GCHello");
    }
}

void CSGOMMHello::RefreshWait()
{
    m_updateComplete = false;
    Refresh();
    std::unique_lock<std::mutex> lock(m_mmhelloMutex);

    m_updateCv.wait_for(lock, std::chrono::milliseconds(CSGO_CLI_STEAM_CMSG_TIMEOUT));

    if (!m_updateComplete) {
        throw CSGO_CLI_TimeoutException();
    }
}
