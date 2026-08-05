// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CSGOClient.h"

#include <algorithm>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

#ifdef SendMessage
#undef SendMessage
#endif

static uint32_t const ProtobufFlag = (1U << 31);
static uint32_t const Steam730ClientVersion = 2000880;
static auto const GameClientConnectRetryInterval = std::chrono::milliseconds(500);
CSGOClient* CSGOClient::m_instance = nullptr;

CSGOClient::CSGOClient() :
    m_welcomeHandler(this, &CSGOClient::OnClientWelcome),
    m_mmHelloHandler(this, &CSGOClient::OnMatchmakingHello),
    m_availableCb(this, &CSGOClient::OnMessageAvailable),
    m_failedCb(this, &CSGOClient::OnMessageFailed)
{
    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace(
            "Initializing GameCoordinator interface: pipe={}, user={}, version='{}'",
            SteamAPI_GetHSteamPipe(),
            SteamAPI_GetHSteamUser(),
            STEAMGAMECOORDINATOR_INTERFACE_VERSION);
    }

    m_gameCoordinator = reinterpret_cast<ISteamGameCoordinator*>(SteamClient()->GetISteamGenericInterface(
        SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), STEAMGAMECOORDINATOR_INTERFACE_VERSION));

    if (!m_gameCoordinator) {
        throw ExceptionHandler("failed to acquire ISteamGameCoordinator interface");
    }

    RegisterHandler(k_EMsgGCClientWelcome, &m_welcomeHandler);
    RegisterHandler(k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello, &m_mmHelloHandler);

    SendConnectionHelloMessages();
}

void CSGOClient::SendConnectionHelloMessages()
{
    CMsgClientHello hello;
    hello.set_version(Steam730ClientVersion);

    auto const sendResult = SendGCMessage(k_EMsgGCClientHello, &hello);
    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace(
            "Sent GCClientHello: version={}, result={}",
            Steam730ClientVersion,
            static_cast<int>(sendResult));
    }

    if (sendResult != k_EGCResultOK) {
        throw ExceptionHandler("failed to send GCClientHello");
    }

    CMsgGCCStrike15_v2_MatchmakingClient2GCHello mmHello;
    auto const mmHelloResult = SendGCMessage(k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello, &mmHello);
    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace("Sent MatchmakingClient2GCHello: result={}", static_cast<int>(mmHelloResult));
    }

    if (mmHelloResult != k_EGCResultOK) {
        throw ExceptionHandler("failed to send MatchmakingClient2GCHello");
    }
}

EGCResults CSGOClient::SendGCMessage(uint32_t uMsgType, google::protobuf::Message* msg)
{
    std::lock_guard<std::mutex> lock(m_sendMutex);

    auto const originalMsgType = uMsgType;

#if GOOGLE_PROTOBUF_VERSION > 3009002
    int body_size = google::protobuf::internal::ToIntSize(msg->ByteSizeLong());
#else
    int body_size = msg->ByteSize();
#endif

    auto size = body_size + 2 * sizeof(uint32_t);

    if (m_msgBuffer.size() < size) {
        m_msgBuffer.resize(size);
    }

    uMsgType |= ProtobufFlag;

    reinterpret_cast<uint32_t*>(m_msgBuffer.data())[0] = uMsgType;
    reinterpret_cast<uint32_t*>(m_msgBuffer.data())[1] = 0;

    msg->SerializeToArray(m_msgBuffer.data() + 2 * sizeof(uint32_t), m_msgBuffer.size() - 2 * sizeof(uint32_t));

    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace(
            "GC send: msg_type={}, payload_size={}, wire_size={}",
            originalMsgType,
            body_size,
            size);
    }

    return m_gameCoordinator->SendMessage(uMsgType, m_msgBuffer.data(), size);
}

void CSGOClient::OnMessageAvailable(GCMessageAvailable_t* msg)
{
    std::lock_guard<std::mutex> lock(m_recvMutex);
    std::lock_guard<std::mutex> lock2(m_handlerMutex);

    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace("GC message available: announced_size={}", msg->m_nMessageSize);
    }

    if (m_recvBuffer.size() < msg->m_nMessageSize) {
        m_recvBuffer.resize(msg->m_nMessageSize);
    }

    uint32_t msgType;
    uint32_t msgSize;

    auto res = m_gameCoordinator->RetrieveMessage(&msgType, m_recvBuffer.data(), m_recvBuffer.size(), &msgSize);

    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace(
            "GC retrieve: result={}, msg_type={}, msg_size={}, protobuf={}",
            static_cast<int>(res),
            msgType,
            msgSize,
            (msgType & ProtobufFlag) != 0);
    }

    if (res == k_EGCResultOK) {
        if (msgType & ProtobufFlag) {
            auto const strippedMsgType = msgType & (~ProtobufFlag);
            auto handler               = m_msgHandler.equal_range(strippedMsgType);

            if (strippedMsgType == k_EMsgGCCStrike15_v2_ClientLogonFatalError) {
                CMsgGCCStrike15_v2_ClientLogonFatalError fatalError;
                if (fatalError.ParseFromArray(m_recvBuffer.data() + 2 * sizeof(uint32_t), msgSize - 2 * sizeof(uint32_t))) {
                    std::string failureMessage = fmt::format(
                        "GC client logon rejected: errorcode={}, country='{}', message='{}'",
                        fatalError.errorcode(),
                        fatalError.country(),
                        fatalError.message());
                    spdlog::error(
                        "{}",
                        failureMessage);

                    {
                        std::lock_guard<std::mutex> connectionLock(m_connectedMutex);
                        m_connectionFailure = std::move(failureMessage);
                    }
                    m_connectedCV.notify_all();
                }
            }

            if (spdlog::should_log(spdlog::level::trace)) {
                auto const handlerCount = std::distance(handler.first, handler.second);
                spdlog::trace("GC dispatch: stripped_msg_type={}, handlers={}", strippedMsgType, handlerCount);
            }

            for (auto it = handler.first; it != handler.second; ++it) {
                it->second->Handle(m_recvBuffer.data() + 2 * sizeof(uint32_t), msgSize - 2 * sizeof(uint32_t));
            }
        }
    }
}

void CSGOClient::OnMessageFailed(GCMessageFailed_t* msg)
{
    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace("GC message delivery failed callback received");
    }
    throw ExceptionHandler("Failed to deliver GC message");
}

void CSGOClient::RegisterHandler(uint32 msgId, IGCMsgHandler* handler)
{
    std::lock_guard<std::mutex> lock(m_handlerMutex);
    m_msgHandler.insert({msgId, handler});
}

void CSGOClient::RemoveHandler(uint32 msgId, IGCMsgHandler const* handler)
{
    std::lock_guard<std::mutex> lock(m_handlerMutex);

    auto h = m_msgHandler.equal_range(msgId);

    for (auto it = h.first; it != h.second; ++it) {
        if (it->second == handler) {
            it = m_msgHandler.erase(it);
            return;
        }
    }
}

CSGOClient* CSGOClient::GetInstance()
{
    if (!m_instance) {
        m_instance = new CSGOClient();
    }

    return m_instance;
}

void CSGOClient::Destroy()
{
    delete m_instance;
    m_instance = nullptr;
}

void CSGOClient::WaitForGameClientConnect()
{
    std::unique_lock<std::mutex> lock(m_connectedMutex);
    if (m_connectedToGameClient) {
        return;
    }

    auto const deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

    while (!m_connectedToGameClient && !m_connectionFailure.has_value()) {
        auto const waitUntil = std::min(deadline, std::chrono::steady_clock::now() + GameClientConnectRetryInterval);

        m_connectedCV.wait_until(lock, waitUntil, [this]() { return m_connectionFailure.has_value(); });

        if (std::chrono::steady_clock::now() >= deadline) {
            break;
        }

        lock.unlock();
        SendConnectionHelloMessages();
        lock.lock();
    }

    if (!m_connectedToGameClient && !m_connectionFailure.has_value()) {
        throw ExceptionHandler("Timed out waiting for GameClient welcome message");
    }

    if (m_connectionFailure.has_value()) {
        throw ExceptionHandler(*m_connectionFailure);
    }
}

bool CSGOClient::TryGetCachedMatchmakingHello(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello& msg)
{
    std::lock_guard<std::mutex> lock(m_connectedMutex);
    if (!m_hasCachedMatchmakingHello) {
        return false;
    }

    msg = m_cachedMatchmakingHello;
    return true;
}

void CSGOClient::OnClientWelcome(CMsgClientWelcome const & msg)
{
    // printf("Received welcome CS:GO Game Coordinator version %s (Connected to %s).",
    //  std::to_string(msg.version()).c_str(), msg.location().country().c_str());

    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace(
            "Received GC welcome: version='{}', location='{}'",
            msg.version(),
            msg.location().has_country() ? msg.location().country() : "");
    }

    std::lock_guard<std::mutex> lock(m_connectedMutex);
    m_connectedToGameClient = true;
    m_connectedCV.notify_all();
}

void CSGOClient::OnMatchmakingHello(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello const & msg)
{
    if (spdlog::should_log(spdlog::level::trace)) {
        spdlog::trace(
            "Received matchmaking hello: account_id={}, player_level={}, has_global_stats={}, penalty_seconds={}, vac_banned={}",
            msg.account_id(),
            msg.player_level(),
            msg.has_global_stats(),
            msg.penalty_seconds(),
            msg.vac_banned());
    }

    std::lock_guard<std::mutex> lock(m_connectedMutex);
    m_cachedMatchmakingHello    = msg;
    m_hasCachedMatchmakingHello = true;
    m_connectedToGameClient = true;
    m_connectedCV.notify_all();
}
