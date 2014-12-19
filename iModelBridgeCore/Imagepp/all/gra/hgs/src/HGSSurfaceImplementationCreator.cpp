//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSSurfaceImplementationCreator.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSSurfaceImplementationCreator
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSSurfaceImplementationCreator.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSSurfaceImplementationCreator::HGSSurfaceImplementationCreator()
    {
    m_Priority = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSSurfaceImplementationCreator::~HGSSurfaceImplementationCreator()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetPriority
//-----------------------------------------------------------------------------
uint32_t HGSSurfaceImplementationCreator::GetPriority() const
    {
    return m_Priority;
    }

//-----------------------------------------------------------------------------
// public
// SetPriority
//-----------------------------------------------------------------------------
void HGSSurfaceImplementationCreator::SetPriority(uint32_t pi_Priority)
    {
    m_Priority = pi_Priority;
    }