//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSGraphicTool.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicTool
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSGraphicTool.h>

#include <Imagepp/all/h/HGSGraphicToolImplementationFactory.h>
#include <Imagepp/all/h/HGSSurface.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSGraphicTool::HGSGraphicTool()
    {
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSGraphicTool::HGSGraphicTool(const HGSGraphicToolAttributes& pi_rAttributes)
    : m_Attributes(pi_rAttributes)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSGraphicTool::~HGSGraphicTool()
    {
    }

//-----------------------------------------------------------------------------
// public
// SetFor
//-----------------------------------------------------------------------------
void HGSGraphicTool::SetFor(const HFCPtr<HGSSurface>& pi_rpSurface)
    {
    // a null surface is valid

    if(pi_rpSurface != 0)
        {
        m_pImplementation =  HGSGraphicToolImplementationFactory::GetInstance()->Create(
                                 GetType(),
                                 &m_Attributes,
                                 pi_rpSurface->GetImplementation());


        HASSERT(m_pImplementation != 0);
        }
    else
        {
        m_pImplementation = 0;
        }

    m_pSurface        =  pi_rpSurface;
    }