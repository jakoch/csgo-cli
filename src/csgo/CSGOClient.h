// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGO_CSGOCLIENT_H_
#define SRC_CSGO_CSGOCLIENT_H_

// Steamworks SDK
#pragma warning(disable : 4996)
#include <steam/isteamgamecoordinator.h>
#include <steam/steam_api.h>

// CSGO Protobuf's
#include "cstrike15_gcmessages.pb.h"
#include "engine_gcmessages.pb.h"
#include "gcsdk_gcmessages.pb.h"
#include "gcsystemmsgs.pb.h"

#include "../ExceptionHandler.h"
#include "../VersionAndConstants.h"
#include "GCMsgHandler.h"

#include <condition_variable>
#include <optional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/**
 * A minimal CS:GO Game Client
 */
class CSGOClient
{
public:
    /**
     * Retrieves the instance to the current csgo client or creates a new one
     */
    static CSGOClient* GetInstance();

    /**
     * Destroys the csgo client
     */
    static void Destroy();

    /**
     * Sends a gc protobuf message
     */
    EGCResults SendGCMessage(uint32 uMsgType, google::protobuf::Message* msg);

    /**
     * Registers a gc protobuf msg handler
     */
    void RegisterHandler(uint32 msgId, IGCMsgHandler* handler);

    /**
     * Removes a gc protobuf msg handler
     */
    void RemoveHandler(uint32 msgId, IGCMsgHandler* handler);

    /**
     * Blocks until we are connected to the GameClient
     */
    void WaitForGameClientConnect();

    /**
     * Copies the most recent matchmaking hello if one was already received.
     */
    bool TryGetCachedMatchmakingHello(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello& msg);

private:
    /**
     * Sends client mm hello
     */
    CSGOClient();
    CSGOClient(CSGOClient const &) = delete;

    /**
     * Steam callback for gc messages
     */
    void OnMessageAvailable(GCMessageAvailable_t* msg);

    /**
     * Steam callback for failed gc messages
     */
    void OnMessageFailed(GCMessageFailed_t* msg);

    /**
     * Sends the initial hello messages required by the current GC handshake.
     */
    void SendConnectionHelloMessages();

    /**
     * Handles the gc welcome msg
     */
    void OnClientWelcome(CMsgClientWelcome const & msg);

    /**
     * Handles the matchmaking hello used by current clients as a GC-ready signal.
     */
    void OnMatchmakingHello(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello const & msg);

private:
    static CSGOClient* m_instance;

    GCMsgHandler<CMsgClientWelcome> m_welcomeHandler;
    GCMsgHandler<CMsgGCCStrike15_v2_MatchmakingGC2ClientHello> m_mmHelloHandler;
    std::condition_variable m_connectedCV;
    std::mutex m_connectedMutex;
    bool m_connectedToGameClient = false;
    std::optional<std::string> m_connectionFailure;
    bool m_hasCachedMatchmakingHello = false;
    CMsgGCCStrike15_v2_MatchmakingGC2ClientHello m_cachedMatchmakingHello;

    ISteamGameCoordinator* m_gameCoordinator;

    CCallback<CSGOClient, GCMessageAvailable_t, false> m_availableCb;
    CCallback<CSGOClient, GCMessageFailed_t, false> m_failedCb;
    std::vector<char> m_recvBuffer;
    std::vector<char> m_msgBuffer;
    std::mutex m_sendMutex;
    std::mutex m_recvMutex;
    std::mutex m_handlerMutex;
    std::multimap<uint32, IGCMsgHandler*> m_msgHandler;
};

#endif // SRC_CSGO_CSGOCLIENT_H_
