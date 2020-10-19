#include "VersionAndConstants.h"
#include "CSGOClient.h"
#include "ExceptionHandler.h"

static const uint32_t ProtobufFlag = (1 << 31);
CSGOClient* CSGOClient::m_instance = nullptr;

CSGOClient::CSGOClient() :
    m_welcomeHandler(this, &CSGOClient::OnClientWelcome),
    m_availableCb(this, &CSGOClient::OnMessageAvailable),
    m_failedCb(this, &CSGOClient::OnMessageFailed)
{
    m_gameCoordinator =
      (ISteamGameCoordinator *)SteamClient()->GetISteamGenericInterface(
          SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(),
          STEAMGAMECOORDINATOR_INTERFACE_VERSION);

    RegisterHandler(k_EMsgGCClientWelcome, &m_welcomeHandler);

    CMsgClientHello hello;
    hello.set_client_session_need(1);

    if (SendGCMessage(k_EMsgGCClientHello, &hello) != k_EGCResultOK) {
        throw ExceptionHandler("failed to send GCClientHello");
    }

}

EGCResults CSGOClient::SendGCMessage(uint32_t uMsgType, google::protobuf::Message* msg)
{
    std::lock_guard<std::mutex> lock(m_sendMutex);

    if (m_msgBuffer.size() < msg->ByteSize() + 2 * sizeof(uint32_t)) {
        m_msgBuffer.resize(msg->ByteSize() + 2 * sizeof(uint32_t));
    }

    uMsgType |= ProtobufFlag;

    ((uint32_t*)m_msgBuffer.data())[0] = uMsgType;
    ((uint32_t*)m_msgBuffer.data())[1] = 0;

    msg->SerializeToArray(
        m_msgBuffer.data() + 2 * sizeof(uint32_t),
        m_msgBuffer.size() - 2 * sizeof(uint32_t)
    );

    return m_gameCoordinator->SendMessage(
        uMsgType,
        m_msgBuffer.data(),
        msg->ByteSize() + 2 * sizeof(uint32_t)
    );
}

void CSGOClient::OnMessageAvailable(GCMessageAvailable_t* msg)
{
    std::lock_guard<std::mutex> lock(m_recvMutex);
    std::lock_guard<std::mutex> lock2(m_handlerMutex);

    if (m_recvBuffer.size() < msg->m_nMessageSize) {
        m_recvBuffer.resize(msg->m_nMessageSize);
    }

    uint32_t msgType;
    uint32_t msgSize;

    // TODO res unsued?
    auto res = m_gameCoordinator->RetrieveMessage(
        &msgType,
        m_recvBuffer.data(),
        m_recvBuffer.size(),
        &msgSize
    );

    if (msgType & ProtobufFlag)
    {
        auto handler = m_msgHandler.equal_range(msgType & (~ProtobufFlag));

        for (auto it = handler.first; it != handler.second; ++it) {
            it->second->Handle(
                m_recvBuffer.data() + 2 * sizeof(uint32_t),
                msgSize - 2 * sizeof(uint32_t)
            );
        }
    }
}

void CSGOClient::OnMessageFailed(GCMessageFailed_t* msg)
{
    throw ExceptionHandler("Failed to deliver GC message");
}

void CSGOClient::RegisterHandler(uint32 msgId, IGCMsgHandler* handler)
{
    std::lock_guard<std::mutex> lock(m_handlerMutex);
    m_msgHandler.insert({msgId, handler});
}

void CSGOClient::RemoveHandler(uint32 msgId, IGCMsgHandler* handler)
{
    std::lock_guard<std::mutex> lock(m_handlerMutex);

    auto h = m_msgHandler.equal_range(msgId);

    for (auto it = h.first; it != h.second; ++it)
    {
        if (it->second == handler)
        {
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
    if (m_connectedToGameClient) {
        return;
    }

    std::unique_lock<std::mutex> lock(m_connectedMutex);
    // if this takes longer than 10 seconds we are already connected to the gc
    m_connectedCV.wait_for(lock, std::chrono::seconds(10));
    m_connectedToGameClient = true;
}

void CSGOClient::OnClientWelcome(const CMsgClientWelcome& msg)
{
    //printf("Received welcome CS:GO Game Coordinator version %s (Connected to %s).",
    //  std::to_string(msg.version()).c_str(), msg.location().country().c_str());

    m_connectedToGameClient = true;
    m_connectedCV.notify_all();
}
