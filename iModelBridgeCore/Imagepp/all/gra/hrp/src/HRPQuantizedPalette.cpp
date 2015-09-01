//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPQuantizedPalette.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPQuantizedPalette
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    // must be first fptr PreCompiledHeader Option
#include <Imagepp/all/h/HRPQuantizedPalette.h>

//-----------------------------------------------------------------------------
// Constructor for HRPQuantizedPalette
//-----------------------------------------------------------------------------
HRPQuantizedPalette::HRPQuantizedPalette()
    {
    m_MaxEntries = 0;
    }

//-----------------------------------------------------------------------------
// Constructor for HRPQuantizedPalette
//-----------------------------------------------------------------------------
HRPQuantizedPalette::HRPQuantizedPalette(unsigned short pi_MaxEntries)
    {
    m_MaxEntries = pi_MaxEntries;
    }

//-----------------------------------------------------------------------------
// Destructor for HRPQuantizedPalette
//-----------------------------------------------------------------------------
HRPQuantizedPalette::~HRPQuantizedPalette()
    {
    }

