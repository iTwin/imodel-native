//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCHomeMadeKey.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHomeMadeKey
//-----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCHomeMadeKey.h>


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HFCHomeMadeKey::~HFCHomeMadeKey()
    {
    }


//-----------------------------------------------------------------------------
// Equality test
//-----------------------------------------------------------------------------
bool HFCHomeMadeKey::operator==(const HFCHomeMadeKey& pi_rObj) const
    {
    return m_Representation == pi_rObj.m_Representation;
    }
