//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSWarperImplementation.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSWarperImplementation
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSWarperImplementation.h>
#include <Imagepp/all/h/HGSSurfaceImplementation.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSWarperImplementation::HGSWarperImplementation(const HGSGraphicToolAttributes*    pi_pAttributes,
                                                 HGSSurfaceImplementation*          pi_pSurfaceImplementation)
    : HGSGraphicToolImplementation(pi_pAttributes,
                                   pi_pSurfaceImplementation)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSWarperImplementation::~HGSWarperImplementation()
    {
    }

