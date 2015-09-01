//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmg/src/HMGMessageReceiver.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMGMessageReceiver
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMGMessageReceiver.h>
#include <Imagepp/all/h/HMGSink.h>
#include <Imagepp/all/h/HMGMessage.h>
#include <Imagepp/all/h/HMGMessageSender.h>


//-----------------------------------------------------------------------------
// The constructor.
//-----------------------------------------------------------------------------
HMGMessageReceiver::HMGMessageReceiver()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HMGMessageReceiver::HMGMessageReceiver(const HMGMessageReceiver& pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HMGMessageReceiver::~HMGMessageReceiver()
    {
    // Tell our sink we're not in memory anymore.
    if (m_pSink != 0)
        m_pSink->ContainerUnloaded();
    }


//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HMGMessageReceiver& HMGMessageReceiver::operator=(const HMGMessageReceiver& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pSink = 0;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Receive a message.
//-----------------------------------------------------------------------------
bool HMGMessageReceiver::ProcessMessage(const HMGMessage& pi_rMessage)
    {
    // Stop the message from propagating. We are only a
    // receiver anyway...
    return false;
    }

//-----------------------------------------------------------------------------
// Tell if the receiver needs the coherence security mechanism
//-----------------------------------------------------------------------------
bool HMGMessageReceiver::NeedsCoherenceSecurity() const
    {
    // Don't need by default.
    return false;
    }


//-----------------------------------------------------------------------------
// Call Link() on the specified sender.
//-----------------------------------------------------------------------------
void HMGMessageReceiver::LinkTo(HMGMessageSender* pi_pSender,
                                bool pi_ReceiveMessagesInCurrentThread) const
    {
    HPRECONDITION(pi_pSender != 0);

    EnsureSinkIsConstructed();

    pi_pSender->Link(m_pSink, pi_ReceiveMessagesInCurrentThread ? HMGThread::GetCurrentThreadID() : 0);
    }


//-----------------------------------------------------------------------------
// Nothing to do by default.
//-----------------------------------------------------------------------------
void HMGMessageReceiver::SenderDeleted(HMGMessageSender* pi_pSender)
    {
    }

//-----------------------------------------------------------------------------
// Call Unlink() on the specified sender.
//-----------------------------------------------------------------------------
void HMGMessageReceiver::UnlinkFrom(HMGMessageSender* pi_pSender) const
    {
    HPRECONDITION(pi_pSender != 0);

    // Our sink should have been created by the call to LinkTo...
    HPRECONDITION(m_pSink != 0);

    pi_pSender->Unlink(m_pSink);
    }


//-----------------------------------------------------------------------------
// Constructs our sink object if necessary
//-----------------------------------------------------------------------------
void HMGMessageReceiver::EnsureSinkIsConstructed() const
    {
    if (m_pSink == 0)
        {
        const_cast<HMGMessageReceiver*>(this)->m_pSink =
            new HMGSink(const_cast<HMGMessageReceiver*>(this));
        }
    }