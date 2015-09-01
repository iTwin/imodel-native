//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmg/src/HMGMessageSender.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMGMessageSender
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMGMessageSender.h>
#include <Imagepp/all/h/HMGMessage.h>
#include <Imagepp/all/h/HMGSink.h>

//-----------------------------------------------------------------------------
// The default constructor.
//-----------------------------------------------------------------------------
HMGMessageSender::HMGMessageSender()
    {
    m_pNonPersistentNotifiees = 0;
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HMGMessageSender::HMGMessageSender(const HMGMessageSender& pi_rObj)
    {
    // Do not copy notifiees list.
    m_pNonPersistentNotifiees = 0;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HMGMessageSender::~HMGMessageSender()
    {
    if (m_pNonPersistentNotifiees != 0)
        {
        NotifieesList::iterator Itr = m_pNonPersistentNotifiees->begin();

        // non-persistent receivers...
        while (Itr != m_pNonPersistentNotifiees->end())
            {
            // Tell them to remove all messages received after
            // our last save.
            (*Itr++).first->SenderDestroyed(this);
            }

        delete m_pNonPersistentNotifiees;
        }
    }


//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HMGMessageSender& HMGMessageSender::operator=(const HMGMessageSender& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // Do not copy notifiees list.
        delete m_pNonPersistentNotifiees;
        m_pNonPersistentNotifiees = 0;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Send a message to all notifies
//-----------------------------------------------------------------------------
void HMGMessageSender::Propagate(const HMGMessage& pi_rMessage)
    {
    if (m_pNonPersistentNotifiees != 0)
        {
        // Now pass through all sinks of non-persistent receivers
        NotifieesList::iterator Itr = m_pNonPersistentNotifiees->begin();

        // Pass through all the sinks
        while (Itr != m_pNonPersistentNotifiees->end())
            {
            // Set the message's sender object.
            // Set it each time because the receiver may change the
            // message's sender for its own purposes...
            const_cast<HMGMessage&>(pi_rMessage).SetSender(this);

            if ((*Itr).second.GetThreadID() != 0 &&
                (*Itr).second.GetThreadID() != HMGThread::GetCurrentThreadID())
                {
                // Synchronous messages will not be sent between threads!
                if (!pi_rMessage.IsSynchronous())
                    {
                    // Have the message posted!
                    HMGThread::PostMessage((*Itr).second.GetThreadID(), (*Itr).first, pi_rMessage);
                    }
                }
            else
                {
                // Send the message directly
                (*Itr).first->Notify(pi_rMessage);
                }

            Itr++;
            }
        }
    }

//-----------------------------------------------------------------------------
// Add a new notifiee.
//-----------------------------------------------------------------------------
void HMGMessageSender::Link(const HFCPtr<HMGSink>& pi_pNewSink,
                            HMGThreadID            pi_SendToThread)
    {
    HPRECONDITION(pi_pNewSink != 0);

    NotifieesList::iterator Itr;

    if (m_pNonPersistentNotifiees == 0)
        {
        // Create the list
        m_pNonPersistentNotifiees = new NotifieesList;
        }

    // Add to non-persistent receivers list

    // Ensure we don't add sink twice
    if ((Itr = m_pNonPersistentNotifiees->find(pi_pNewSink)) != m_pNonPersistentNotifiees->end())
        {
        (*Itr).second.IncrementRef();
        }
    else
        {
        // Add new sink to the list
        m_pNonPersistentNotifiees->insert(NotifieesList::value_type(pi_pNewSink, HMGNotifieeInfo(pi_SendToThread)));
        }
    }



//-----------------------------------------------------------------------------
// Remove a notifiee
//-----------------------------------------------------------------------------        
void HMGMessageSender::Unlink(const HFCPtr<HMGSink>& pi_pToRemove)
    {
    HPRECONDITION(pi_pToRemove != 0);

    NotifieesList::iterator Itr;

    // Not found. Check in non-persistent receivers
    if (m_pNonPersistentNotifiees != 0 && (Itr = m_pNonPersistentNotifiees->find(pi_pToRemove)) != m_pNonPersistentNotifiees->end())
        {
        // Decrement ref. count
        if ((*Itr).second.DecrementRef() == 0)
            {
            // No more references. Tell sink we're going out
            (*Itr).first->SenderUnlinked(this);

            // Remove entry from our list
            m_pNonPersistentNotifiees->erase(Itr);
            }
        }
    }
