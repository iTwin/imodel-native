//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFMessages.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HGF message classes
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HMGMessage.h>#include <ImagePP/all/h/HRFMessages.h>

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