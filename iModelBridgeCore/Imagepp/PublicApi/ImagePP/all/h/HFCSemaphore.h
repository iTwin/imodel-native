//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSemaphore.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCSemaphore
//-----------------------------------------------------------------------------
// Creates or gievs access to a semaphore.  Used for multithread/multiprocess
// synchronization purposes.
//-----------------------------------------------------------------------------
// Note: actually, works only on Win95 and WinNT
//-----------------------------------------------------------------------------

#pragma once

#include "HFCSynchro.h"

BEGIN_IMAGEPP_NAMESPACE
/**

    This class is used to create and access @t{semaphores}.  These objects are
    used for synchronization purposes in multithread and multi-process
    environments.  Usage of a semaphore is simple: it accumulates "signals"
    (by calling its method @k{Signal}) and any thread can query it to know if it
    is "signaled" (if the semaphore accumulated at least one signal).  Each
    time a thread queries a semaphore (by calling its method
    @k{WaitUntilSignaled}), a signal is removed from that semaphore.  If the
    semaphore is not signaled, the thread pauses until that signal arrives
    or until a certain delay elapsed.

    A semaphore may be identified by a name.  In this case it may be
    "opened" by threads and processes that cannot have access to the
    original object.  This class can be used to open existent semaphore by
    name by using the static method @k{Open} provided for this situation, that
    allocates an instance mapping to an existent semaphore identified by a
    given name.

    @see HFCSynchro

*/

class HFCSemaphore : public HFCSynchro
    {
public:

    //:> Construction/Destruction

    IMAGEPP_EXPORT                 HFCSemaphore(int32_t       pi_InitCount   = 0,
                                        int32_t       pi_MaxCount    = LONG_MAX);
    IMAGEPP_EXPORT                 HFCSemaphore(const WString& pi_rName,
                                        int32_t       pi_InitCount   = 0,
                                        int32_t       pi_MaxCount    = LONG_MAX);
    IMAGEPP_EXPORT virtual         ~HFCSemaphore();


    //:> Open a named semaphore - must have been
    //:> previously created with the named
    //:> contructor

    static HFCSemaphore*
    Open(const WString& pi_rName);


    //:> Synchronization

    void            Signal();


protected:

    virtual HFCHandle
    GetHandle() const;


private:

    //:> Methods not implemented

    HFCSemaphore(const HFCSemaphore& pi_rObj);
    HFCSemaphore& operator=(const HFCSemaphore&);


    //:> Members

    //:> Used to create the actual semaphore when GetHandle() is invoked.
    const WString   m_Name;
    const int32_t  m_InitCount;
    const int32_t  m_MaxCount;

    //:> Handle to the actual object
    HAutoPtr<HFCHandle>
    m_pHandle;


    //:> Used to synchronize the creation of the actual object in GetHandle()
    //:Ignore
#ifdef _WIN32
    static struct InternalKey
        {
        InternalKey();
        ~InternalKey();
        void ClaimKey();
        void ReleaseKey();
        CRITICAL_SECTION m_Key;
        }* s_pInternalKey;
    //:End Ignore

    friend struct EventKeyCreator;
#endif
    };

END_IMAGEPP_NAMESPACE

#include "HFCSemaphore.hpp"
