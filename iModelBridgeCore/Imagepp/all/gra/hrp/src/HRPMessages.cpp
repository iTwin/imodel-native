//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPMessages.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HRP message classes
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRPMessages.h>


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
