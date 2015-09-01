//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmg/src/HMGSink.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMGSink
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMGSink.h>
#include <Imagepp/all/h/HMGMessage.h>
#include <Imagepp/all/h/HMGMessageReceiver.h>


///////////////////////
// HMGMessageEnvelope
///////////////////////

///////////////////////////////////////////////////////////
// Default constructor
///////////////////////////////////////////////////////////
HMGMessageEnvelope::HMGMessageEnvelope()
    {
    m_Approved = false;
    m_pMessage = 0;
    }


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
HMGMessageEnvelope::HMGMessageEnvelope(const HMGMessage& pi_rMessage)
    {
    m_Approved = false;

    m_pMessage = (HMGMessage*) pi_rMessage.Clone();
    }


///////////////////////////////////////////////////////////
// Copy constructor
///////////////////////////////////////////////////////////
HMGMessageEnvelope::HMGMessageEnvelope(const HMGMessageEnvelope& pi_rObj)
    {
    m_Approved = pi_rObj.m_Approved;

    m_pMessage = pi_rObj.m_pMessage ? (HMGMessage*) pi_rObj.m_pMessage->Clone() : 0;
    }

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
HMGMessageEnvelope::~HMGMessageEnvelope()
    {
    delete m_pMessage;
    }

///////////////////////////////////////////////////////////
// Assignment
///////////////////////////////////////////////////////////
HMGMessageEnvelope& HMGMessageEnvelope::operator=(const HMGMessageEnvelope& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        delete m_pMessage;

        m_pMessage = pi_rObj.m_pMessage ? (HMGMessage*) pi_rObj.m_pMessage->Clone() : 0;

        m_Approved = pi_rObj.m_Approved;
        }

    return *this;
    }


///////////////////////////////////////////////////////////
// Approve the message in the envelope
///////////////////////////////////////////////////////////
void HMGMessageEnvelope::Approve()
    {
    m_Approved = true;
    }


///////////////////////////////////////////////////////////
// Verify if the message is approved
///////////////////////////////////////////////////////////
bool HMGMessageEnvelope::IsApproved() const
    {
    return m_Approved;
    }


///////////////////////////////////////////////////////////
// Verify message sender
///////////////////////////////////////////////////////////
bool HMGMessageEnvelope::ComesFrom(HMGMessageSender* pi_pSender) const
    {
    return (m_pMessage->GetSender() == pi_pSender);
    }


///////////////////////////////////////////////////////////
// Retrieve the internal message
///////////////////////////////////////////////////////////
HMGMessage& HMGMessageEnvelope::GetMessageText() const
    {
    return (HMGMessage&) *m_pMessage;
    }


///////////////////////
// HMGSink
///////////////////////


HFCExclusiveKey* HMGSink::s_pInternalKey = 0;

//-----------------------------------------------------------------------------
// The constructor.
//-----------------------------------------------------------------------------
HMGSink::HMGSink(HMGMessageReceiver* pi_pReceiver)
    : m_pReceiver(pi_pReceiver)
    {
    m_CurrentQueuePosition = 0;

    m_KeepMessages = false;

    m_CoherenceSecurityInitialized = false;

    if (s_pInternalKey == 0)
        s_pInternalKey = new HFCExclusiveKey();
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HMGSink::~HMGSink()
    {
    }


//-----------------------------------------------------------------------------
// Set the container's pointer
//-----------------------------------------------------------------------------
void HMGSink::ContainerLoaded(HMGMessageReceiver* pi_pReceiver)
    {
    HPRECONDITION(pi_pReceiver != 0);

    // Take current memory address
    m_pReceiver = pi_pReceiver;

    // We're just loaded, so we re-start processing message
    // front the beginning
    m_CurrentQueuePosition =  0;

    s_pInternalKey->ClaimKey();
    MessageQueue TempMessages = m_Messages;
    s_pInternalKey->ReleaseKey();

    // Feed the container with all pending messages
    while (m_pReceiver && m_CurrentQueuePosition < TempMessages.size())
        {
        // Send the oldest message
        m_pReceiver->ProcessMessage(TempMessages[m_CurrentQueuePosition++].GetMessageText());
        }
    }


//-----------------------------------------------------------------------------
// Container is now unloaded.
//-----------------------------------------------------------------------------
void HMGSink::ContainerUnloaded()
    {
    // Memory address is now invalid!
    m_pReceiver = 0;
    }


//-----------------------------------------------------------------------------
// Receive a message
//-----------------------------------------------------------------------------
void HMGSink::Notify(const HMGMessage& pi_rMessage)
    {
    if (m_pReceiver == 0)
        {
        // Keep the message for later
        s_pInternalKey->ClaimKey();
        m_Messages.push_back(pi_rMessage);
        s_pInternalKey->ReleaseKey();
        }
    else
        {
        // Container is alive. Give it the message directly

        if (!m_CoherenceSecurityInitialized)
            {
            m_KeepMessages = m_pReceiver->NeedsCoherenceSecurity();
            m_CoherenceSecurityInitialized = true;
            }

        // Save message if we need coherence security
        if (m_KeepMessages)
            {
            s_pInternalKey->ClaimKey();
            m_Messages.push_back(pi_rMessage);
            s_pInternalKey->ReleaseKey();

            m_CurrentQueuePosition++;
            }

        m_pReceiver->ProcessMessage(pi_rMessage);
        }
    }

//-----------------------------------------------------------------------------
// Our container has been saved
//-----------------------------------------------------------------------------
void HMGSink::ContainerSaved()
    {
    if (m_CurrentQueuePosition >= m_Messages.size())
        {
        // All messages have been processed. Simply
        // clear the list
        s_pInternalKey->ClaimKey();
        m_Messages.clear();
        s_pInternalKey->ReleaseKey();
        }
    else
        {
        // Erase messages up to current position
        for (size_t i = 0 ; i < m_CurrentQueuePosition ; i++)
            {
            // Very slow. Should theoretically never get here, though...
            s_pInternalKey->ClaimKey();
            m_Messages.erase(m_Messages.begin());
            s_pInternalKey->ReleaseKey();
            }
        }

    m_CurrentQueuePosition = 0;
    }


//-----------------------------------------------------------------------------
// Commit messages received from the sender
//-----------------------------------------------------------------------------
void HMGSink::SenderSaved(HMGMessageSender* pi_pSender)
    {
    s_pInternalKey->ClaimKey();

    MessageQueue::iterator Itr(m_Messages.begin());

    // Pass all messages
    while (Itr != m_Messages.end())
        {
        // Approve message if it comes from the specified sender
        if ((*Itr).ComesFrom(pi_pSender))
            (*Itr).Approve();

        Itr++;
        }

    s_pInternalKey->ReleaseKey();
    }


//-----------------------------------------------------------------------------
// Rollback unsaved messages received from the sender
//-----------------------------------------------------------------------------
void HMGSink::SenderDestroyed(HMGMessageSender* pi_pSender)
    {
    s_pInternalKey->ClaimKey();

    MessageQueue::iterator  Itr(m_Messages.begin());
    MessageQueue            KeptMessages;
    MessageQueue::size_type Position = 0;

    // Pass all messages
    while (Itr != m_Messages.end())
        {
        // Check if message comes from the specified sender,
        // without being approved yet...
        if ((*Itr).ComesFrom(pi_pSender) && !(*Itr).IsApproved())
            {
            // Got to remove message from the queue

            // Adjust current position if necessary
            if (Position < m_CurrentQueuePosition)
                m_CurrentQueuePosition--;
            }
        else
            {
            // Copy element. We don't remove it...
            KeptMessages.push_back(*Itr);
            }

        Itr++;
        Position++;
        }

    // Now, replace current queue with new calculated one.
    m_Messages = KeptMessages;

    // Tell our container about the deletion.
    if (m_pReceiver != 0)
        {
        m_pReceiver->SenderDeleted(pi_pSender);
        }

    s_pInternalKey->ReleaseKey();
    }


//-----------------------------------------------------------------------------
// Rollback all messages received from the sender
//-----------------------------------------------------------------------------
void HMGSink::SenderUnlinked(HMGMessageSender* pi_pSender)
    {
    s_pInternalKey->ClaimKey();

    MessageQueue::iterator  Itr(m_Messages.begin());
    MessageQueue            KeptMessages;
    MessageQueue::size_type Position = 0;

    // Pass all messages
    while (Itr != m_Messages.end())
        {
        // Check if message comes from the specified sender,
        if ((*Itr).ComesFrom(pi_pSender))
            {
            // Got to remove message from the queue

            // Adjust current position if necessary
            if (Position < m_CurrentQueuePosition)
                m_CurrentQueuePosition--;
            }
        else
            {
            // Copy element. We don't remove it...
            KeptMessages.push_back(*Itr);
            }

        Itr++;
        Position++;
        }

    // Now, replace current queue with new calculated one.
    m_Messages = KeptMessages;

    s_pInternalKey->ReleaseKey();
    }

///////////////////////////////////////////////////////////
// Check if the sink is valid
///////////////////////////////////////////////////////////
bool HMGSink::IsSinkValid() const
    {
    return m_pReceiver != 0;
    }
