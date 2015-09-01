//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmg/src/HMGMessage.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for classes HMGMessage, HMGSynchronousMessage and HMGAsynchronousMessage
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMGMessage.h>


///////////////////////////////////////////////
// Methods of class HMGMessage
///////////////////////////////////////////////

//-----------------------------------------------------------------------------
// The constructor.
//-----------------------------------------------------------------------------
HMGMessage::HMGMessage(bool pi_Synchronous)
    : m_IsSynchronous(pi_Synchronous),
      m_pSender(0)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HMGMessage::HMGMessage(const HMGMessage& pi_rObj)
    : m_IsSynchronous(pi_rObj.m_IsSynchronous),
      m_pSender(pi_rObj.m_pSender)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HMGMessage::~HMGMessage()
    {
    // Nothing to do, but must be declared (virtual)
    }


//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HMGMessage& HMGMessage::operator=(const HMGMessage& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_IsSynchronous = pi_rObj.m_IsSynchronous;

        m_pSender             = pi_rObj.m_pSender;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
// HMGMessage* HMGMessage::Clone() const
//     {
//     return new HMGMessage(*this);
//     }


///////////////////////////////////////////////
// Methods of class HMGAsynchronousMessage
///////////////////////////////////////////////

//-----------------------------------------------------------------------------
// The constructor.
//-----------------------------------------------------------------------------
HMGAsynchronousMessage::HMGAsynchronousMessage()
    : HMGMessage(false)
    {
    }


//-----------------------------------------------------------------------------
// The copy constructor.
//-----------------------------------------------------------------------------
HMGAsynchronousMessage::HMGAsynchronousMessage(const HMGAsynchronousMessage& pi_rObj)
    : HMGMessage(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HMGAsynchronousMessage::~HMGAsynchronousMessage()
    {
    // Nothing to do, but must be declared (virtual)
    }


///////////////////////////////////////////////
// Methods of class HMGSynchronousMessage
///////////////////////////////////////////////

//-----------------------------------------------------------------------------
// The constructor.
//-----------------------------------------------------------------------------
HMGSynchronousMessage::HMGSynchronousMessage()
    : HMGMessage(true)
    {
    }


//-----------------------------------------------------------------------------
// The copy constructor.
//-----------------------------------------------------------------------------
HMGSynchronousMessage::HMGSynchronousMessage(const HMGSynchronousMessage& pi_rObj)
    : HMGMessage(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HMGSynchronousMessage::~HMGSynchronousMessage()
    {
    // Nothing to do, but must be declared (virtual)
    }

//-----------------------------------------------------------------------------
// Return the message's synchronization state
//-----------------------------------------------------------------------------
bool HMGMessage::IsSynchronous() const
    {
    return m_IsSynchronous;
    }

//-----------------------------------------------------------------------------
// Return the pointer to the sender object
//-----------------------------------------------------------------------------
HMGMessageSender* HMGMessage::GetSender() const
    {
    HPRECONDITION(m_pSender != 0);

    return (HMGMessageSender*) m_pSender;
    }

//-----------------------------------------------------------------------------
// Set the pointer to the sender object
//-----------------------------------------------------------------------------
void HMGMessage::SetSender(HMGMessageSender* pi_pSender)
    {
    HPRECONDITION(pi_pSender != 0);

    m_pSender = pi_pSender;
    }