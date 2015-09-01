//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCEvent.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCEvent
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCSynchro.h>


BEGIN_IMAGEPP_NAMESPACE

/**

    This class is used to create and access event.  These objects are used
    for synchronization purposes in multithreaded and multi-process
    environments.  An event is used to signal an event in a program.  The
    event has two states: signaled and not signaled.

    An event can be signaled by using the Signal method.  A thread that
    waits for the event to be signaled will be suspended until the it become
    signaled or a given time out expires.  The management of the event state
    may be automatic or manual.

    @list{automatic: the state automatically returns to not signaled once
          WaitUntilSignaled method has returned}

    @list{manual: the state remains signaled once the WaitUntilSignaled method
          has returned.  A call to Reset is needed to change the state to not
          signaled.}
    @end

    An event object may be identified by a name.  Other thread, or even
    process on the same machine, can have access to the same event by
    calling the Open method.  A duplicate of the original object is returned
    if the specified name refers to an existing named event.

    @see HFCSynchro
    @see HFCSemaphore
    @see HFCThread

*/

class HFCEvent : public HFCSynchro
    {
public:

    //:> Construction - Destruction

    IMAGEPP_EXPORT                 HFCEvent(bool          pi_ManualReset = true,
                                    bool          pi_Signaled    = true);
    IMAGEPP_EXPORT                 HFCEvent(const WString& pi_rName,
                                    bool          pi_ManualReset = true,
                                    bool          pi_Signaled    = true);
    IMAGEPP_EXPORT virtual         ~HFCEvent();


    //:> Open a named event - must have been
    //:> previously created with the named
    //:> contructor

    static HFCEvent*
    Open(const WString& pi_rName);


    //:> Synchronization

    void            Signal();
    void            Reset();
    void            Pulse();


protected:

    //:> Creates the actual object or gives the handle
    //:> to it.
    IMAGEPP_EXPORT virtual HFCHandle
    GetHandle() const;


private:

    // Methods not implemented

    HFCEvent(const HFCEvent& pi_rObj);
    HFCEvent& operator=(const HFCEvent&);


    // Members

    // Used to create the actual event when GetHandle() is invoked.
    const WString   m_Name;
    const bool     m_ManualReset;
    const bool     m_InitialState;

    // Handle to the event object
    HAutoPtr<HFCHandle>
    m_pHandle;

    // Used to synchronize the creation of the actual object in GetHandle()
#ifdef _WIN32
    static struct InternalKey
        {
        InternalKey();
        ~InternalKey();
        void ClaimKey();
        void ReleaseKey();
        CRITICAL_SECTION m_Key;
        }* s_pInternalKey;

    friend struct EventKeyCreator;
#endif
    };

END_IMAGEPP_NAMESPACE

#include "HFCEvent.hpp"

