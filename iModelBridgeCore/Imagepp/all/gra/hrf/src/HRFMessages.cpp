//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HGF message classes
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HMGMessage.h>
#include <ImagePP/all/h/HRFMessages.h>


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
