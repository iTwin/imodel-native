//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HRP message classes
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRPMessages.h>


// Constructor
//-----------------------------------------------------------------------------
HRPPaletteChangedMsg::HRPPaletteChangedMsg()
: HMGAsynchronousMessage()
{
}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRPPaletteChangedMsg::HRPPaletteChangedMsg(const HRPPaletteChangedMsg& pi_rObj)
    : HMGAsynchronousMessage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HRPPaletteChangedMsg::Clone() const
    {
    return new HRPPaletteChangedMsg(*this);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPaletteChangedMsg::~HRPPaletteChangedMsg()
    {
    }
