//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSGraphicToolImplementation.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicToolImplementation
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSGraphicToolImplementation.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSGraphicToolImplementation::HGSGraphicToolImplementation(const HGSGraphicToolAttributes*  pi_pAttributes,
                                                           HGSSurfaceImplementation*        pi_pSurfaceImplementation)
    {
    HPRECONDITION(pi_pSurfaceImplementation != 0);

    m_pAttributes = pi_pAttributes;
    m_pSurfaceImplementation = pi_pSurfaceImplementation;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSGraphicToolImplementation::~HGSGraphicToolImplementation()
    {
    }

//-----------------------------------------------------------------------------
// public
// SetAttributes
//-----------------------------------------------------------------------------
void HGSGraphicToolImplementation::SetAttributes(const HGSGraphicToolAttributes* pi_pAttributes)
    {
    m_pAttributes = pi_pAttributes;
    }



