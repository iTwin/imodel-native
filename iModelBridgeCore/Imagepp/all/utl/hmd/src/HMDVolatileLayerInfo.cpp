//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDVolatileLayerInfo.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDLayerInfo.h>
#include <Imagepp/all/h/HMDVolatileLayerInfo.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDVolatileLayerInfo::HMDVolatileLayerInfo(const HMDLayerInfo* pi_pLayerInfo)
    {
    m_pLayerInfo = pi_pLayerInfo;
    m_VisibleState = m_pLayerInfo->GetInitialVisibleState();
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDVolatileLayerInfo::~HMDVolatileLayerInfo()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Copy constructor
//-----------------------------------------------------------------------------
HMDVolatileLayerInfo::HMDVolatileLayerInfo(const HMDVolatileLayerInfo& pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Public
// Assignment operator
//-----------------------------------------------------------------------------
HMDVolatileLayerInfo& HMDVolatileLayerInfo::operator=(const HMDVolatileLayerInfo& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        CopyMemberData(pi_rObj);
        }
    return *this;
    }

//-----------------------------------------------------------------------------
// Public
// Get the visible state
//-----------------------------------------------------------------------------
bool HMDVolatileLayerInfo::GetVisibleState() const
    {
    return m_VisibleState;
    }

//-----------------------------------------------------------------------------
// Public
// Set the visible state
//-----------------------------------------------------------------------------
void HMDVolatileLayerInfo::SetVisibleState(bool pi_NewVisibleState)
    {
    m_VisibleState = pi_NewVisibleState;
    }

//-----------------------------------------------------------------------------
// Public
// Get the persistent layer info read from the file
//-----------------------------------------------------------------------------
const HMDLayerInfo* HMDVolatileLayerInfo::GetLayerInfo() const
    {
    return m_pLayerInfo;
    }

//-----------------------------------------------------------------------------
// Private
// CopyMemberData
//-----------------------------------------------------------------------------
void HMDVolatileLayerInfo::CopyMemberData(const HMDVolatileLayerInfo& pi_rObj)
    {
    m_pLayerInfo   = pi_rObj.m_pLayerInfo;
    m_VisibleState = pi_rObj.m_VisibleState;
    }