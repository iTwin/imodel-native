//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCTimer.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCTimer
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCExclusiveKey.h>

BEGIN_IMAGEPP_NAMESPACE
class HFCTimer;

class HFCTimerListener
    {
public:

    HFCTimerListener();
    virtual ~HFCTimerListener();

protected:

    friend class HFCTimer;

    virtual void Trigger(HFCTimer* pi_pTimer) = 0;

private:

    HFCTimerListener(const HFCTimerListener&);
    HFCTimerListener& operator=(const HFCTimerListener&);
    };

//----------------------------------------------------------------------------

class HFCTimer
    {
public:

    HFCTimer();
    IMAGEPP_EXPORT virtual      ~HFCTimer();

    void                Start(uint32_t pi_Interval = 0, bool pi_IsPeriodic = true);
    void                Stop();

    size_t              GetInterval() const;
    bool               IsPeriodic() const;
    size_t              CountTicks() const;

    void                AddListener(HFCTimerListener* pi_pTimerListener);
    void                RemoveListener(HFCTimerListener* pi_pTimerListener);

protected:

    void                TriggerInternal();
    IMAGEPP_EXPORT virtual void Trigger();

private:

    HFCTimer(const HFCTimer&);
    HFCTimer& operator=(const HFCTimer&);

    typedef list<HFCTimerListener*> ListenerList;

    ListenerList        m_Listeners;
    size_t              m_Interval;
    bool                m_IsPeriodic;
    size_t              m_Ticks;
    HFCExclusiveKey     m_Key;

    uint32_t             m_TimerID;  // The timer ID returned by windows

#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)    // Windows timer callback
    IMAGEPP_EXPORT static void CALLBACK TimerCallback(uint32_t uID, uint32_t uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
#endif
    };

//----------------------------------------------------------------------------

template <class T>
class HFCMethodCallbackTimer : public HFCTimer
    {
public:

    typedef void (T::*TriggerMethodPtr)();

    HFCMethodCallbackTimer(T* pi_pObj, TriggerMethodPtr pi_pMethod);
    virtual             ~HFCMethodCallbackTimer();

protected:

    virtual void        Trigger();

private:

    T*                  m_pObj;
    TriggerMethodPtr    m_pMethod;
    };

//----------------------------------------------------------------------------

template<class T>
class HFCMethodCallbackListener : public HFCTimerListener
    {
public:

    typedef void (T::*TriggerMethodPtr)();

    HFCMethodCallbackListener(T* pi_pObj, TriggerMethodPtr pi_pMethod);
    virtual             ~HFCMethodCallbackListener();

    uint32_t            CountTicks() const;
    void                ResetCount();

protected:

    virtual void Trigger(HFCTimer* pi_pTimer) = 0;

private:

    uint32_t            m_Ticks;
    T*                  m_pObj;
    TriggerMethodPtr    m_pMethod;
    };

END_IMAGEPP_NAMESPACE
#include "HFCTimer.hpp"

