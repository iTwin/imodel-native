//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSEditorImplementation.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSEditorImplementation
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSEditorImplementation.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSEditorImplementation::HGSEditorImplementation(const HGSGraphicToolAttributes*    pi_pAttributes,
                                                 HGSSurfaceImplementation*          pi_pSurfaceImplementation)
    : HGSGraphicToolImplementation(pi_pAttributes, pi_pSurfaceImplementation)
    {
    m_LongNumberOfPixels = 0;
    m_DoubleNumberOfPixels = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSEditorImplementation::~HGSEditorImplementation()
    {
    }

