//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSBlitterImplementation.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSBlitterImplementation
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSBlitterImplementation.h>
#include <Imagepp/all/h/HGSGraphicToolAttributes.h>
#include <Imagepp/all/h/HGSSurfaceImplementation.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSBlitterImplementation::HGSBlitterImplementation(const HGSGraphicToolAttributes* pi_pAttributes,
                                                   HGSSurfaceImplementation*       pi_pSurfaceImplementation)
    : HGSGraphicToolImplementation(pi_pAttributes, pi_pSurfaceImplementation)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSBlitterImplementation::~HGSBlitterImplementation()
    {
    }

