//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCTimer.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HFCTimer
//----------------------------------------------------------------------------

#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
#include <wtypes.h>     // Windows timer system
#include <mmsystem.h>    // Windows timer system
#endif

BEGIN_IMAGEPP_NAMESPACE

inline HFCTimerListener::HFCTimerListener()
    {
    // Nothing to do!
    }

//----------------------------------------------------------------------------
inline HFCTimer::HFCTimer()
    {
    }

inline size_t HFCTimer::GetInterval() const
    {
    return m_Interval;
    }

inline bool HFCTimer::IsPeriodic() const
    {
    return m_IsPeriodic;
    }

inline size_t HFCTimer::CountTicks() const
    {
    return m_Ticks;
    }

inline void HFCTimer::AddListener(HFCTimerListener* pi_pTimerListener)
    {
    m_Key.ClaimKey();
    m_Listeners.push_back(pi_pTimerListener);
    m_Key.ReleaseKey();
    }

inline void HFCTimer::RemoveListener(HFCTimerListener* pi_pTimerListener)
    {
    m_Key.ClaimKey();
    ListenerList::iterator itr = find(m_Listeners.begin(), m_Listeners.end(), pi_pTimerListener);
    HASSERT(itr != m_Listeners.end());
    m_Listeners.erase(itr);
    m_Key.ReleaseKey();
    }

inline void HFCTimer::Stop()  // placed before Start because that calls it (inlining)
    {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    // Cancel the timer
    timeKillEvent(m_TimerID);
    m_TimerID = 0;
    m_Ticks = 0;
#endif
    }

inline void HFCTimer::Start(uint32_t pi_Interval, bool pi_IsPeriodic)
    {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    if (m_TimerID != 0)
        Stop();
    m_Interval = pi_Interval;
    m_IsPeriodic = pi_IsPeriodic;
    m_Ticks = 0;
    if (m_IsPeriodic)
        m_TimerID = timeSetEvent((uint32_t)m_Interval, 0, HFCTimer::TimerCallback, (INT_PTR)this, TIME_PERIODIC);
    else
        m_TimerID = timeSetEvent((uint32_t)m_Interval, 0, HFCTimer::TimerCallback, (INT_PTR)this, TIME_ONESHOT);
#endif
    }

inline void HFCTimer::TriggerInternal()
    {
    ++m_Ticks;
    Trigger();
    }

//----------------------------------------------------------------------------
template <class T>
inline HFCMethodCallbackTimer<T>::HFCMethodCallbackTimer
(T* pi_pObj,
 typename HFCMethodCallbackTimer::TriggerMethodPtr pi_pMethod)
    : HFCTimer(), m_pObj(pi_pObj), m_pMethod(pi_pMethod)
    {
    }

template<class T>
HFCMethodCallbackTimer<T>::~HFCMethodCallbackTimer()
    {
    // Nothing to do
    }

template<class T>
void HFCMethodCallbackTimer<T>::Trigger()
    {
    (m_pObj->*m_pMethod)();
    }


//----------------------------------------------------------------------------
template<class T>
inline HFCMethodCallbackListener<T>::HFCMethodCallbackListener
(T* pi_pObj,
 typename HFCMethodCallbackListener::TriggerMethodPtr pi_pMethod)
    : HFCTimer(), m_pObj(pi_pObj), m_pMethod(pi_pMethod), m_Ticks(0)
    {
    }

template<class T>
HFCMethodCallbackListener<T>::~HFCMethodCallbackListener()
    {
    // Nothing to do
    }

template<class T>
inline uint32_t HFCMethodCallbackListener<T>::CountTicks() const
    {
    return m_Ticks;
    }

template<class T>
inline void HFCMethodCallbackListener<T>::ResetCount()
    {
    m_Ticks = 0;
    }

template<class T>
void HFCMethodCallbackListener<T>::Trigger(HFCTimer* pi_pTimer)
    {
    ++m_Ticks;
    (m_pObj->*m_pMethod)();
    }

END_IMAGEPP_NAMESPACE