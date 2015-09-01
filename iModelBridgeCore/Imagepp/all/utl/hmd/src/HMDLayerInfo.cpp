//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayerInfo.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDLayerInfo.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayerInfo::HMDLayerInfo(const WString& pi_rKeyName,
                                  bool          pi_InitialVisibleState)
    {
    m_KeyName = pi_rKeyName;
    m_InitialVisibleState = pi_InitialVisibleState;
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDLayerInfo::~HMDLayerInfo()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HMDLayerInfo::HMDLayerInfo(const HMDLayerInfo& pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Public
// Assignment operator
//-----------------------------------------------------------------------------
HMDLayerInfo& HMDLayerInfo::operator=(const HMDLayerInfo& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        CopyMemberData(pi_rObj);
        }
    return *this;
    }

//-----------------------------------------------------------------------------
// Public
// Get the initial visibility state
//-----------------------------------------------------------------------------
bool HMDLayerInfo::GetInitialVisibleState() const
    {
    return m_InitialVisibleState;
    }

//-----------------------------------------------------------------------------
// Public
// Get the key name
//-----------------------------------------------------------------------------
const WString& HMDLayerInfo::GetKeyName() const
    {
    return m_KeyName;
    }

//-----------------------------------------------------------------------------
// Public
// Set the initial visible state of a layer info
//-----------------------------------------------------------------------------
void HMDLayerInfo::SetInitialVisibleState(bool pi_VisibleState)
    {
    m_InitialVisibleState = pi_VisibleState;
    }

//-----------------------------------------------------------------------------
// Private
// CopyMemberData
//-----------------------------------------------------------------------------
void HMDLayerInfo::CopyMemberData(const HMDLayerInfo& pi_rObj)
    {
    m_KeyName             = pi_rObj.m_KeyName;
    m_InitialVisibleState = pi_rObj.m_InitialVisibleState;
    }