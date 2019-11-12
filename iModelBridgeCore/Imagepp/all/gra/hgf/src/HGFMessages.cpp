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


#include <ImagePP/all/h/HGFMessages.h>

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HGFGeometryChangedMsg::~HGFGeometryChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HGFGeometryChangedMsg::Clone() const
    {
    return new HGFGeometryChangedMsg(*this);
    }

