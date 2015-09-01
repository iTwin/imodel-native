//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFCoordSysContainer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFCoordSysContainer
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFCoordSysContainer.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGFCoordSysContainer::HGFCoordSysContainer()
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HGFCoordSysContainer::HGFCoordSysContainer(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    m_pCoordSys = pi_rpCoordSys;
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HGFCoordSysContainer::HGFCoordSysContainer(const HGFCoordSysContainer& pi_rObj)
    {
    // Copy members
    m_pCoordSys = pi_rObj.m_pCoordSys;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGFCoordSysContainer::~HGFCoordSysContainer()
    {
    }


//-----------------------------------------------------------------------------
// operator=
//-----------------------------------------------------------------------------
HGFCoordSysContainer& HGFCoordSysContainer::operator=(const HGFCoordSysContainer& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        m_pCoordSys = pi_rObj.m_pCoordSys;
        }

    return (*this);
    }


//-----------------------------------------------------------------------------
// GetCoordSys
//-----------------------------------------------------------------------------
HFCPtr<HGF2DCoordSys> HGFCoordSysContainer::GetCoordSys() const
    {
    return(m_pCoordSys);
    }

//-----------------------------------------------------------------------------
// SetCoordSys
//-----------------------------------------------------------------------------
void HGFCoordSysContainer::SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    m_pCoordSys = pi_rpCoordSys;
    }