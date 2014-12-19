//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFMessages.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for HGF message classes
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

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


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HGFBufferContentChangedMsg::~HGFBufferContentChangedMsg()
    {
    }


//-----------------------------------------------------------------------------
// Return a copy of self
//-----------------------------------------------------------------------------
HMGMessage* HGFBufferContentChangedMsg::Clone() const
    {
    return new HGFBufferContentChangedMsg(*this);
    }
