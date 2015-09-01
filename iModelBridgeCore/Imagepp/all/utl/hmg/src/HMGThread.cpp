//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmg/src/HMGThread.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMGThread
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMGThread.h>
#include <Imagepp/all/h/HMGMacros.h>
#include <Imagepp/all/h/HMGMessage.h>
#include <Imagepp/all/h/HMGSink.h>


// Definition of class statics

HMGThread::HMGThreadMap HMGThread::s_Threads;
HFCExclusiveKey         HMGThread::s_ThreadsKey;


//-----------------------------------------------------------------------------
// The default constructor.
//-----------------------------------------------------------------------------
HMGThread::HMGThread()
    {
    m_ThreadID = GetCurrentThreadID();

    s_ThreadsKey.ClaimKey();

    // Ensure we only have one HMGThread per system thread!
    HASSERT(s_Threads.find(m_ThreadID) == s_Threads.end());

    s_Threads.insert(HMGThreadMap::value_type(m_ThreadID, this));

    s_ThreadsKey.ReleaseKey();
    }


//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
HMGThread::~HMGThread()
    {
    s_ThreadsKey.ClaimKey();
    s_Threads.erase(m_ThreadID);
    s_ThreadsKey.ReleaseKey();

    // Delete all pending messages...
    MessageList::iterator Itr(m_Messages.begin());
    while (Itr != m_Messages.end())
        {
        // Delete the message entry
        delete (*Itr++);
        }
    }


//-----------------------------------------------------------------------------
// Dispatch
//-----------------------------------------------------------------------------
void HMGThread::DispatchMessages()
    {
    // Take local copy of list and clear it!
    m_MessagesKey.ClaimKey();
    MessageList Messages(m_Messages);
    m_Messages.clear();
    m_MessagesKey.ReleaseKey();

    // Now we can safely iterate on our local copy
    MessageList::iterator Itr(Messages.begin());

    // Pass all message entries
    while (Itr != Messages.end())
        {
        // Have the message sent
        (*Itr)->Send();

        // Delete the message entry
        delete (*Itr);

        Itr++;
        }
    }


//-----------------------------------------------------------------------------
// Post a message to a thread
//-----------------------------------------------------------------------------
void HMGThread::PostMessage(HMGThreadID            pi_DestinationThread,
                            const HFCPtr<HMGSink>& pi_rpSink,
                            const HMGMessage&      pi_rMessage)
    {
    // Can't handle synchronous message when posted!
    HASSERT(!pi_rMessage.IsSynchronous());

    s_ThreadsKey.ClaimKey();
    HMGThreadMap::iterator Itr(s_Threads.find(pi_DestinationThread));

    if (Itr != s_Threads.end())
        {
        (*Itr).second->ReceiveMessage(pi_rpSink, pi_rMessage);
        }
    s_ThreadsKey.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// Retrieve the thread's identificator
//-----------------------------------------------------------------------------
HMGThreadID HMGThread::GetThreadID() const
    {
    return m_ThreadID;
    }


//-----------------------------------------------------------------------------
// Receive a message
//-----------------------------------------------------------------------------
void HMGThread::ReceiveMessage(const HFCPtr<HMGSink>& pi_rpSink,
    const HMGMessage&      pi_rMessage)
    {
    m_MessagesKey.ClaimKey();
    m_Messages.push_back(new MessageEntry(pi_rpSink, pi_rMessage));
    m_MessagesKey.ReleaseKey();
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HMGThread::MessageEntry::MessageEntry(const HFCPtr<HMGSink>& pi_rpSink,
    const HMGMessage&      pi_rMessage)
    {
    m_pSink    = pi_rpSink;
    m_pMessage = (HMGMessage*) pi_rMessage.Clone();
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HMGThread::MessageEntry::~MessageEntry()
    {
    delete m_pMessage;
    }


//-----------------------------------------------------------------------------
// Send the kept message
//-----------------------------------------------------------------------------
void HMGThread::MessageEntry::Send()
    {
    m_pSink->Notify(*m_pMessage);
    }


HMGThreadID HMGThread::GetCurrentThreadID()
    {
#ifdef _WIN32
    return GetCurrentThreadId();
#endif
    }