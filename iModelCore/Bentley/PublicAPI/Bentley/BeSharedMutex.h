/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeSharedMutex.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <mutex>
#include <condition_variable>
#include <chrono>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// Mutexes synchronize multi-thread access to data.
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
typedef std::recursive_mutex      BeRecursiveMutex;

// shared mutex implementation adapted from:
// Copyright Howard Hinnant 2007-2010. Distributed under the Boost
// Software License, Version 1.0. (see http://www.boost.org/LICENSE_1_0.txt)

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BeSharedMutex
{
    typedef std::condition_variable cond_t;
    typedef unsigned                count_t;

    std::mutex m_gateMutex;
    cond_t  m_gate1;
    cond_t  m_gate2;
    count_t m_state;

    static const count_t writeEnteredFlag = 1U << (sizeof(count_t)*CHAR_BIT - 1);
    static const count_t readersMask = ~writeEnteredFlag;

public:
    BeSharedMutex() : m_state(0) {}
    ~BeSharedMutex() {std::lock_guard<std::mutex> _(m_gateMutex);}

    // Exclusive ownership
    void lock()
        {
        std::unique_lock<std::mutex> lk(m_gateMutex);
        while (m_state & writeEnteredFlag)
            m_gate1.wait(lk);
        m_state |= writeEnteredFlag;
        while (m_state & readersMask)
            m_gate2.wait(lk);
        }

    bool try_lock()
        {
        std::unique_lock<std::mutex> lk(m_gateMutex);
        if (m_state == 0)
            {
            m_state = writeEnteredFlag;
            return true;
            }
        return false;
        }

    void unlock()
        {
        std::lock_guard<std::mutex> _(m_gateMutex);
        m_state = 0;
        m_gate1.notify_all();
        }

    template <class Rep, class Period> bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
        {
        return try_lock_until(std::chrono::steady_clock::now() + rel_time);
        }

    template <class Clock, class Duration> bool try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)
        {
        std::unique_lock<std::mutex> lk(m_gateMutex);
        if (m_state & writeEnteredFlag)
            {
            while (true)
                {
                std::cv_status status = m_gate1.wait_until(lk, abs_time);
                if ((m_state & writeEnteredFlag) == 0)
                    break;
                if (status == std::cv_status::timeout)
                    return false;
                }
            }
        m_state |= writeEnteredFlag;
        if (m_state & readersMask)
            {
            while (true)
                {
                std::cv_status status = m_gate2.wait_until(lk, abs_time);
                if ((m_state & readersMask) == 0)
                    break;
                if (status == std::cv_status::timeout)
                    {
                    m_state &= ~writeEnteredFlag;
                    return false;
                    }
                }
            }
        return true;
        }

    // Shared ownership
    void lock_shared()
        {
        std::unique_lock<std::mutex> lk(m_gateMutex);
        while ((m_state & writeEnteredFlag) || (m_state & readersMask) == readersMask)
            m_gate1.wait(lk);
        count_t num_readers = (m_state & readersMask) + 1;
        m_state &= ~readersMask;
        m_state |= num_readers;
        }

    bool try_lock_shared()
        {
        std::unique_lock<std::mutex> lk(m_gateMutex);
        count_t num_readers = m_state & readersMask;
        if (!(m_state & writeEnteredFlag) && num_readers != readersMask)
            {
            ++num_readers;
            m_state &= ~readersMask;
            m_state |= num_readers;
            return true;
            }
        return false;
        }

    template <class Rep, class Period> bool try_lock_shared_for(const std::chrono::duration<Rep, Period>& rel_time)
        {
        return try_lock_shared_until(std::chrono::steady_clock::now() + rel_time);
        }

    template <class Clock, class Duration> bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration>& abs_time)
        {
        std::unique_lock<std::mutex> lk(m_gateMutex);
        if ((m_state & writeEnteredFlag) || (m_state & readersMask) == readersMask)
            {
            while (true)
                {
                std::cv_status status = m_gate1.wait_until(lk, abs_time);
                if ((m_state & writeEnteredFlag) == 0 &&
                                                 (m_state & readersMask) < readersMask)
                    break;
                if (status == std::cv_status::timeout)
                    return false;
                }
            }
        count_t num_readers = (m_state & readersMask) + 1;
        m_state &= ~readersMask;
        m_state |= num_readers;
        return true;
        }

    void unlock_shared()
        {
        std::lock_guard<std::mutex> _(m_gateMutex);
        count_t num_readers = (m_state & readersMask) - 1;
        m_state &= ~readersMask;
        m_state |= num_readers;
        if (m_state & writeEnteredFlag)
            {
            if (num_readers == 0)
                m_gate2.notify_one();
            }
        else
            {
            if (num_readers == readersMask - 1)
                m_gate1.notify_one();
            }
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
template <class Mutex> struct BeSharedMutexLock
{
public:
    typedef Mutex BeMutexype;

private:
    BeMutexype* m_mutex;
    bool        m_owns;

    struct __nat {int _;};

public:
    BeSharedMutexLock() : m_mutex(nullptr), m_owns(false) {}
    explicit BeSharedMutexLock(BeMutexype& m) : m_mutex(&m), m_owns(true) {m_mutex->lock_shared();}
    BeSharedMutexLock(BeMutexype& m, std::defer_lock_t) : m_mutex(&m), m_owns(false) {}
    BeSharedMutexLock(BeMutexype& m, std::try_to_lock_t) : m_mutex(&m), m_owns(m.try_lock_shared()) {}
    BeSharedMutexLock(BeMutexype& m, std::adopt_lock_t) : m_mutex(&m), m_owns(true) {}

    template <class Clock, class Duration> BeSharedMutexLock(BeMutexype& m, const std::chrono::time_point<Clock, Duration>& abs_time)
        : m_mutex(&m), m_owns(m.try_lock_shared_until(abs_time)) {}

    template <class Rep, class Period> BeSharedMutexLock(BeMutexype& m, const std::chrono::duration<Rep, Period>& rel_time)
        : m_mutex(&m), m_owns(m.try_lock_shared_for(rel_time)) {}

    ~BeSharedMutexLock()
        {
        if (m_owns)
            m_mutex->unlock_shared();
        }

    BeSharedMutexLock(BeSharedMutexLock&& sl) : m_mutex(sl.m_mutex), m_owns(sl.m_owns) {sl.m_mutex = nullptr; sl.m_owns = false;}

    BeSharedMutexLock& operator=(BeSharedMutexLock&& sl)
        {
        if (m_owns)
            m_mutex->unlock_shared();
        m_mutex = sl.m_mutex;
        m_owns = sl.m_owns;
        sl.m_mutex = nullptr;
        sl.m_owns = false;
        return *this;
        }

    explicit BeSharedMutexLock(std::unique_lock<BeMutexype>&& ul) : m_mutex(ul.mutex()), m_owns(ul.owns_lock())
        {
        if (m_owns)
            m_mutex->unlock_and_lock_shared();
        ul.release();
        }

    void lock()
        {
        if (m_mutex == nullptr)
            throw std::system_error(std::error_code(EPERM, std::system_category()), "BeSharedMutexLock::lock: references null mutex");
        if (m_owns)
            throw std::system_error(std::error_code(EDEADLK, std::system_category()), "BeSharedMutexLock::lock: already locked");
        m_mutex->lock_shared();
        m_owns = true;
        }

    bool try_lock()
        {
        if (m_mutex == nullptr)
            throw std::system_error(std::error_code(EPERM, std::system_category()), "BeSharedMutexLock::try_lock: references null mutex");
        if (m_owns)
            throw std::system_error(std::error_code(EDEADLK, std::system_category()), "BeSharedMutexLock::try_lock: already locked");
        m_owns = m_mutex->try_lock_shared();
        return m_owns;
        }

    template <class Rep, class Period> bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
        {
        return try_lock_until(std::chrono::steady_clock::now() + rel_time);
        }

    template <class Clock, class Duration> bool try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)
        {
        if (m_mutex == nullptr)
            throw std::system_error(std::error_code(EPERM, std::system_category()), "BeSharedMutexLock::try_lock_until: references null mutex");
        if (m_owns)
            throw std::system_error(std::error_code(EDEADLK, std::system_category()), "BeSharedMutexLock::try_lock_until: already locked");
        m_owns = m_mutex->try_lock_shared_until(abs_time);
        return m_owns;
        }

    void unlock()
        {
        if (!m_owns)
            throw std::system_error(std::error_code(EPERM, std::system_category()), "BeSharedMutexLock::unlock: not locked");
        m_mutex->unlock_shared();
        m_owns = false;
        }

    void swap(BeSharedMutexLock&& u)
        {
        std::swap(m_mutex, u.m_mutex);
        std::swap(m_owns, u.m_owns);
        }

    BeMutexype* release()
        {
        BeMutexype* r = m_mutex;
        m_mutex = nullptr;
        m_owns = false;
        return r;
        }

    bool owns_lock() const {return m_owns;}
    operator int __nat::* () const {return m_owns ? &__nat::_ : 0;}
    BeMutexype* mutex() const {return m_mutex;}
};

template <class Mutex> inline void swap(BeSharedMutexLock<Mutex>& x, BeSharedMutexLock<Mutex>& y) {x.swap(y); }
typedef BeSharedMutexLock<BeSharedMutex> BeSharedMutexHolder;


END_BENTLEY_NAMESPACE
