// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGO_CSGORANKUPDATE_H_
#define SRC_CSGO_CSGORANKUPDATE_H_

#include "CSGOClient.h"

#include <condition_variable>
#include <vector>

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
    void OnRankUpdate(CMsgGCCStrike15_v2_ClientGCRankUpdate const & msg);

private:
    bool m_updateComplete = false;
    std::condition_variable m_updateCv;
    mutable std::mutex m_rankUpdateMutex;
    GCMsgHandler<CMsgGCCStrike15_v2_ClientGCRankUpdate> m_rankUpdateHandler;
};

#endif  // SRC_CSGO_CSGORANKUPDATE_H_