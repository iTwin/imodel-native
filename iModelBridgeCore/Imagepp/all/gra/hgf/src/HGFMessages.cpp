//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFMessages.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HGF message classes
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFMessages.h>

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

