//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGThread.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HMGThread
//-----------------------------------------------------------------------------
// This class describes a thread that can receive HMG messages
//-----------------------------------------------------------------------------
#pragma once

#include "HFCExclusiveKey.h"

typedef int HMGThreadID;

class HMGSink;
class HMGMessage;

///////////////////////////////////////////////
// Declaration of class HMGThread
///////////////////////////////////////////////

class HMGThread
    {
public:

    ///////////////////
    // Primary methods
    ///////////////////

    _HDLLu HMGThread();
    _HDLLu ~HMGThread();


    // Retrieve a thread's ID
    _HDLLu HMGThreadID     GetThreadID() const;

    // Ask thread to dispatch its received messages
    _HDLLu void            DispatchMessages();

    // Post a message to a thread
    static void     PostMessage(HMGThreadID            pi_DestinationThread,
                                const HFCPtr<HMGSink>& pi_rpSink,
                                const HMGMessage&      pi_rMessage);

    _HDLLu static HMGThreadID GetCurrentThreadID();


protected:

    // Add a message to the list (called from the static PostMessage method)
    void            ReceiveMessage(const HFCPtr<HMGSink>& pi_rpSink,
                                   const HMGMessage&      pi_rMessage);

private:

    // Disabled methods
    HMGThread(const HMGThread& pi_rObj);

    HMGThread& operator=(const HMGThread& pi_rObj);

    // An entry in the message list
    struct MessageEntry
        {
    public:

        MessageEntry(const HFCPtr<HMGSink>& pi_rpSink,
                     const HMGMessage&      pi_rMessage);

        ~MessageEntry();

        void Send();

        HFCPtr<HMGSink>
        m_pSink;

        HMGMessage*
        m_pMessage;

        };

    typedef map< HMGThreadID, HMGThread*, less<HMGThreadID>, allocator<HMGThread*> > HMGThreadMap;

    typedef list< MessageEntry*, allocator<MessageEntry*> > MessageList;


    ////////////////
    // Attributes
    ////////////////

    // Our thread ID.
    HMGThreadID     m_ThreadID;

    // List of all currently running HMG threads
    static HMGThreadMap
    s_Threads;

    static HFCExclusiveKey
    s_ThreadsKey;

    // List of pending messages (to be dispatched)
    MessageList     m_Messages;

    HFCExclusiveKey m_MessagesKey;
    };


