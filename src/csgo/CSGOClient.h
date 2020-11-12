#ifndef CSGOClient_H
#define CSGOClient_H

// Steamworks SDK
#pragma warning(disable : 4996)
#include <steam/isteamgamecoordinator.h>
#include <steam/steam_api.h>

// CSGO Protobufs
#include "cstrike15_gcmessages.pb.h"
#include "engine_gcmessages.pb.h"
#include "gcsdk_gcmessages.pb.h"
#include "gcsystemmsgs.pb.h"

#include "../ExceptionHandler.h"
#include "../VersionAndConstants.h"
#include "GCMsgHandler.h"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
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
    static CSGOClient *GetInstance();

    /**
     * Destroys the csgo client
     */
    static void Destroy();

    /**
     * Sends a gc protobuf message
     */
    EGCResults SendGCMessage(uint32 uMsgType, google::protobuf::Message *msg);

    /**
     * Registers a gc protobuf msg handler
     */
    void RegisterHandler(uint32 msgId, IGCMsgHandler *handler);

    /**
     * Removes a gc probouf msg handler
     */
    void RemoveHandler(uint32 msgId, IGCMsgHandler *handler);

    /**
     * Blocks until we are connected to the GameClient
     */
    void WaitForGameClientConnect();

private:
    /**
     * Sends client mm hello
     */
    CSGOClient();
    CSGOClient(const CSGOClient &) = delete;

    /**
     * Steam callback for gc messages
     */
    void OnMessageAvailable(GCMessageAvailable_t *msg);

    /**
     * Steam callback for failed gc messages
     */
    void OnMessageFailed(GCMessageFailed_t *msg);

    /**
     * Handles the gc welcome msg
     */
    void OnClientWelcome(const CMsgClientWelcome &msg);

private:
    static CSGOClient *m_instance;

    GCMsgHandler<CMsgClientWelcome> m_welcomeHandler;
    std::condition_variable m_connectedCV;
    std::mutex m_connectedMutex;
    bool m_connectedToGameClient = false;

    ISteamGameCoordinator *m_gameCoordinator;

    CCallback<CSGOClient, GCMessageAvailable_t, false> m_availableCb;
    CCallback<CSGOClient, GCMessageFailed_t, false> m_failedCb;
    std::vector<char> m_recvBuffer;
    std::vector<char> m_msgBuffer;
    std::mutex m_sendMutex;
    std::mutex m_recvMutex;
    std::mutex m_handlerMutex;
    std::multimap<uint32, IGCMsgHandler *> m_msgHandler;
};

#endif
