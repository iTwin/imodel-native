//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPQuantizedPalette
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>
    // must be first fptr PreCompiledHeader Option
#include <ImagePP/all/h/HRPQuantizedPalette.h>

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
HRPQuantizedPalette::HRPQuantizedPalette(uint16_t pi_MaxEntries)
    {
    m_MaxEntries = pi_MaxEntries;
    }

//-----------------------------------------------------------------------------
// Destructor for HRPQuantizedPalette
//-----------------------------------------------------------------------------
HRPQuantizedPalette::~HRPQuantizedPalette()
    {
    }

