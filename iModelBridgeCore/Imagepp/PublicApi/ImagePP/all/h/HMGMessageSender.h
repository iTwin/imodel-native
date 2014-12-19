//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessageSender.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HMGMessageSender
//-----------------------------------------------------------------------------
// This class describes the message sender class used in HMG messaging
//-----------------------------------------------------------------------------
#pragma once

#include "HMGThread.h"

class HMGMessage;
class HMGSink;

// Structure containing information about a notifiee
struct HMGNotifieeInfo
    {
public:

    HMGNotifieeInfo(HMGThreadID pi_ThreadID = 0);

    HMGNotifieeInfo(const HMGNotifieeInfo& pi_rObj);

    HMGNotifieeInfo& operator=(const HMGNotifieeInfo& pi_rObj);

    uint32_t IncrementRef();
    uint32_t DecrementRef();

    HMGThreadID GetThreadID() const;

private:

    HMGThreadID m_ThreadID;

    uint32_t    m_RefCount;
    };


///////////////////////////////////////////////
// Declaration of class HMGMessageSender
///////////////////////////////////////////////

class HMGMessageSender
    {
    friend class HMGMessageReceiver;
    friend class HMGMessage;

public:

    ///////////////////
    // Primary methods
    ///////////////////

    _HDLLu HMGMessageSender();
    _HDLLu HMGMessageSender(const HMGMessageSender& pi_rObj);

    _HDLLu virtual ~HMGMessageSender();

    HMGMessageSender& operator=(const HMGMessageSender& pi_rObj);

    // ATTENTION! This method should logically be protected,
    // because it is called by our friend HMGMessageReceiver.
    // But, since the HMG macros for persistent receivers
    // override the method that calls Link, and since friendship
    // is not inherited, this method must be public for them
    // to be able to access it.
    _HDLLu void            Link(const HFCPtr<HMGSink>& pi_pNewSink, HMGThreadID pi_SendToThread = 0);

protected:

    ////////////////
    // Methods
    ////////////////

    // Send a message to all notifiees
    _HDLLu void            Propagate(const HMGMessage& pi_rMessage);

    // Notifiees list management
    _HDLLu void            Unlink(const HFCPtr<HMGSink>& pi_pToRemove);

    ////////////////
    // Attributes
    ////////////////

    // Type used for the notifiees list
    typedef map< HFCPtr<HMGSink>,
            HMGNotifieeInfo,
            less<HFCPtr<HMGSink> >,
            allocator<HMGNotifieeInfo> > NotifieesList;

    NotifieesList*
    m_pNonPersistentNotifiees;
    };


#include "HMGMessageSender.hpp"

