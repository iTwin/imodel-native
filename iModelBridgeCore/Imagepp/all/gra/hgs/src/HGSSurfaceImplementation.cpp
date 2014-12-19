//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSSurfaceImplementation.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSSurfaceImplementation
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSSurfaceImplementation.h>


//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSSurfaceImplementation::HGSSurfaceImplementation()
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSSurfaceImplementation::HGSSurfaceImplementation(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor)
    {
    m_pSurfaceDescriptor = pi_rpDescriptor;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSSurfaceImplementation::~HGSSurfaceImplementation()
    {
    }




