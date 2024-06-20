// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CSGO_GCMSGHANDLER_H_
#define SRC_CSGO_GCMSGHANDLER_H_

#include <functional>

struct IGCMsgHandler
{
    virtual void Handle(void* buf, size_t len) const = 0;
};

template <typename M>
/**
 * Handler for Protobuf Messages
 */
class GCMsgHandler : public IGCMsgHandler
{
public:
    using CallbackThread = std::function<void(M const & msg)>;

    template <typename C>
    /**
     * Construct from class handler
     */
    GCMsgHandler(C* instance, void (C::*handler)(M const &))
    {
        m_handler = std::bind(std::mem_fn(handler), instance, std::placeholders::_1);
    }

    template <typename F>
    /**
     * Construct from functor
     */
    explicit GCMsgHandler(F const & handler) : m_handler(handler)
    {
    }

    /**
     * Try parsing msg from buffer and call handler
     */
    /*virtual*/ void Handle(void* buf, size_t len) const final
    {
        M msg;
        msg.ParseFromArray(buf, len);
        m_handler(msg);
    }

private:
    CallbackThread m_handler;
};

#endif // SRC_CSGO_GCMSGHANDLER_H_
