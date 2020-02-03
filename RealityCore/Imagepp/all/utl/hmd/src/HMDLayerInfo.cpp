//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDLayerInfo.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayerInfo::HMDLayerInfo(const Utf8String& pi_rKeyName,
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
const Utf8String& HMDLayerInfo::GetKeyName() const
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
