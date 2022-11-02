/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <functional>
#include <vector>
#include <Bentley/BeAssert.h>
#include <Bentley/BeThread.h>

BEGIN_BENTLEY_NAMESPACE

typedef std::function<void()> cancel_callback_type;

struct BeEventScope final {
private:
    std::vector<cancel_callback_type> m_cancelCbs;
public:
    BeEventScope() {}
    BeEventScope(const BeEventScope&) = delete;
    BeEventScope& operator = (const BeEventScope&) = delete;
    void Add(cancel_callback_type cb) {
        m_cancelCbs.push_back(cb);
    }
    void CancelAll() {
        for (auto& cancelDb : m_cancelCbs) {
            if (cancelDb != nullptr)
                cancelDb();
        }
        m_cancelCbs.clear();
    }
    ~BeEventScope() {
        CancelAll();
    }
};

template<typename... Args>
struct BeEvent final {
    typedef std::function<void(Args...)> listener_type;

private:
    struct EventContext {
    private:
        BeEvent::listener_type m_listener;
        bool m_once;
        uint32_t m_id;
    public:
        EventContext(listener_type listener, bool once, uint32_t id):
            m_listener(listener), m_once(once), m_id(id) {
        }
        listener_type Listener() {
            return m_listener;
        }
        bool Once() const {
            return m_once;
        }
        uint32_t Id() const {
            return m_id;
        }
    };
    std::vector<std::unique_ptr<EventContext>> m_listeners;
    mutable BeMutex m_mutex;
    uint32_t m_listenerIds;
public:
    explicit BeEvent() : m_listenerIds(0) {}
    BeEvent(const BeEvent&) = delete;
    BeEvent& operator = (const BeEvent&) = delete;
    BeMutex& GetMutex() const {return m_mutex;}
    size_t Count() const {
        return m_listeners.size();
    }
    void RemoveAll() {
        BeMutexHolder lock(m_mutex);
        m_listeners.clear();
    }
    cancel_callback_type AddListener(listener_type listener, uint32_t* eventId = nullptr) {
        BeMutexHolder lock(m_mutex);
        m_listeners.push_back(std::make_unique<EventContext>(listener, false, ++m_listenerIds));
        uint32_t id = m_listeners.back()->Id();
        if (eventId) *eventId = id;
        return [=]() { RemoveListener(id); };
    }
    void AddListener(BeEventScope& scope, listener_type listener) {
        uint32_t id;
        auto cancel = AddListener(listener, &id);
        scope.Add([=]() { RemoveListener(id); });
    }
    cancel_callback_type AddOnce(listener_type listener, uint32_t* eventId = nullptr) {
        BeMutexHolder lock(m_mutex);
        m_listeners.push_back(std::make_unique<EventContext>(listener, true,  ++m_listenerIds));
        uint32_t id = m_listeners.back()->Id();
        if (eventId) *eventId = id;
        return [=]() { RemoveListener(id); };
    }
    void AddOnce(BeEventScope& scope, listener_type listener) {
        uint32_t id;
        auto cancel = AddOnce(listener, &id);
        scope.Add([=]() { RemoveListener(id); });
    }
    void RemoveListener(uint32_t eventId) {
        BeMutexHolder lock(m_mutex);
        for (auto it=m_listeners.begin(); it != m_listeners.end(); ++it) {
            if ((*it)->Id() == eventId) {
                m_listeners.erase(it);
                return;
            }
        }
    }
    void RaiseEvent(Args ... args) {
        BeMutexHolder lock(m_mutex);
        for (auto it = m_listeners.begin(); it != m_listeners.end();) {
            EventContext* ctx=  (*it).get();
            ctx->Listener()(std::forward<Args>(args)...);
            if (ctx->Once()) {
                it = m_listeners.erase(it);
            } else {
                ++it;
            }
        }
    }
    // void RaiseEvent(Args&& ... args) {
    //     std::unique_lock<std::recursive_mutex> lock(m_mutex);
    //     for (auto it = m_listeners.begin(); it != m_listeners.end();) {
    //         EventContext* ctx=  (*it).get();
    //         ctx->Listener()(std::forward<Args>(args)...);
    //         if (ctx->Once()) {
    //             it = m_listeners.erase(it);
    //         } else {
    //             ++it;
    //         }
    //     }
    //}
};

END_BENTLEY_NAMESPACE
