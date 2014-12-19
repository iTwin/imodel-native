//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFMessages.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HGF message classes
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HMGMessage.h>
#include <Imagepp/all/h/HRFMessages.h>


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFProgressImageChangedMsg::~HRFProgressImageChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRFProgressImageChangedMsg::Clone() const
    {
    return new HRFProgressImageChangedMsg(*this);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFBlockNotificationMsg::~HRFBlockNotificationMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRFBlockNotificationMsg::Clone() const
    {
    return new HRFBlockNotificationMsg(*this);
    }