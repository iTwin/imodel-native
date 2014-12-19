//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAPreDrawOptions.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAPreDrawOptions
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRAPreDrawOptions.h>

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRAPreDrawOptions::HRAPreDrawOptions(uint32_t pi_DrawSurfaceMaxWidth,
                                     uint32_t pi_DrawSurfaceMaxHeight)
    {
    HPRECONDITION(pi_DrawSurfaceMaxWidth > 0);
    HPRECONDITION(pi_DrawSurfaceMaxHeight > 0);

    m_DrawSurfaceMaxWidth = pi_DrawSurfaceMaxWidth;
    m_DrawSurfaceMaxHeight = pi_DrawSurfaceMaxHeight;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRAPreDrawOptions::HRAPreDrawOptions(const HRAPreDrawOptions& pi_rOptions)
    {
    *this = pi_rOptions;
    }


//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HRAPreDrawOptions::~HRAPreDrawOptions()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRAPreDrawOptions& HRAPreDrawOptions::operator=(const HRAPreDrawOptions& pi_rObj)
    {
    if(this != &pi_rObj)
        {
        m_DrawSurfaceMaxWidth = pi_rObj.m_DrawSurfaceMaxWidth;
        m_DrawSurfaceMaxHeight = pi_rObj.m_DrawSurfaceMaxHeight;
        }

    return(*this);
    }