#ifndef GCMsgHandler_H
#define GCMsgHandler_H

#include <functional>

struct IGCMsgHandler
{
    virtual void Handle(void *buf, size_t len) const = 0;
};

template <typename M>
/**
 * Handler for Protobuf Messages
 */
class GCMsgHandler : public IGCMsgHandler
{
public:
    using CallbackThread = std::function<void(const M &msg)>;

    template <typename C>
    /**
     * Construct from class handler
     */
    GCMsgHandler(C *instance, void (C::*handler)(const M &))
    {
        m_handler = std::bind(std::mem_fn(handler), instance, std::placeholders::_1);
    }

    template <typename F>
    /**
     * Construct from functor
     */
    GCMsgHandler(const F &handler) : m_handler(handler)
    {
    }

    /**
     * Try parsing msg from buffer and call handler
     */
    virtual void Handle(void *buf, size_t len) const final
    {
        M msg;
        msg.ParseFromArray(buf, len);
        m_handler(msg);
    }

private:
    CallbackThread m_handler;
};

#endif
