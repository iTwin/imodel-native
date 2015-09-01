//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmg/src/HMGMessageDuplex.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMGMessageDuplex
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMGMessageDuplex.h>
#include <Imagepp/all/h/HMGMessage.h>


//-----------------------------------------------------------------------------
// The default constructor.
//-----------------------------------------------------------------------------
HMGMessageDuplex::HMGMessageDuplex()
    : HMGMessageSender(), HMGMessageReceiver()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HMGMessageDuplex::HMGMessageDuplex(const HMGMessageDuplex& pi_rObj)
    : HMGMessageSender(pi_rObj), HMGMessageReceiver(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HMGMessageDuplex& HMGMessageDuplex::operator=(const HMGMessageDuplex& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HMGMessageSender::operator=(pi_rObj);
        HMGMessageReceiver::operator=(pi_rObj);
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Receive a message.
//-----------------------------------------------------------------------------
bool HMGMessageDuplex::ProcessMessage(const HMGMessage& pi_rMessage)
    {
    // Two things are possible here. First of all, maybe none of our
    // childs have implemented ProcessMessage (through the macros). In
    // that case, we must propagate the message here. In the other case,
    // none of our childs have handled the message, so we also have to
    // propagate it. If childs have a message map, returning true would tell
    // them to propagate it again, so we return false since the propagation
    // has been made here.

    // Make ourselves the sender
    const_cast<HMGMessage&>(pi_rMessage).SetSender(this);

    // Propagate the message
    Propagate(pi_rMessage);

    // Do not let the mechanism propagate it again.
    return false;
    }

