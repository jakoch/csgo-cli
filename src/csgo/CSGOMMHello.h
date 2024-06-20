// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGO_CSGOMMHELLO_H_
#define SRC_CSGO_CSGOMMHELLO_H_

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
    void OnMMHello(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello const & msg);

private:
    bool m_updateComplete = false;
    std::condition_variable m_updateCv;
    mutable std::mutex m_mmhelloMutex;
    GCMsgHandler<CMsgGCCStrike15_v2_MatchmakingGC2ClientHello> m_mmhelloHandler;
};

#endif  // SRC_CSGO_CSGOMMHELLO_H_
