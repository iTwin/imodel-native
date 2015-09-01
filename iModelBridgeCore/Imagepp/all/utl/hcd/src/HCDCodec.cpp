//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodec.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodec
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodec.h>

//-----------------------------------------------------------------------------
// protected
// Default constructor
//-----------------------------------------------------------------------------
HCDCodec::HCDCodec(const WString& pi_rCodecName)
    : m_CodecName(pi_rCodecName),
      m_CurrentState(STATE_NONE)     // the default state if none (not in compression or decompression)
    {
    }

//-----------------------------------------------------------------------------
// protected
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodec::HCDCodec(const HCDCodec& pi_rObj)
    {
    // copy the current state
    m_CodecName = pi_rObj.m_CodecName;
    m_CurrentState = pi_rObj.m_CurrentState;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodec::~HCDCodec()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetEstimatedCompressionRatio
//-----------------------------------------------------------------------------
double HCDCodec::GetEstimatedCompressionRatio() const
    {
    // HLX default, redeclare this method in your codec
    return 1.0;
    }

//-----------------------------------------------------------------------------
// public
// HasRandomAccess
//-----------------------------------------------------------------------------
bool HCDCodec::HasRandomAccess() const
    {
    // by default, return false
    return false;
    }

//-----------------------------------------------------------------------------
// public
// IsCodecImage
//-----------------------------------------------------------------------------
bool HCDCodec::IsCodecImage() const
    {
    // default
    return false;
    }

//-----------------------------------------------------------------------------
// public
// IsCodecVector
//-----------------------------------------------------------------------------
bool HCDCodec::IsCodecVector() const
    {
    // default
    return false;
    }

//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodec::Reset()
    {
    // reset the current state to NONE
    m_CurrentState = STATE_NONE;
    }

//-----------------------------------------------------------------------------
// public
// SetCurrentState
//-----------------------------------------------------------------------------
void HCDCodec::SetCurrentState(HCDCodec::State pi_State)
    {
    // set the current state of the codec
    m_CurrentState = pi_State;
    }

//-----------------------------------------------------------------------------
// public
// GetRLEInterface
//-----------------------------------------------------------------------------
HCDCodecRLEInterface* HCDCodec::GetRLEInterface()
    {
    return 0; // Default is no interface.
    }

//-----------------------------------------------------------------------------
// public inline method
// GetName
//-----------------------------------------------------------------------------
const WString& HCDCodec::GetName() const
    {
    return m_CodecName;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetCurrentState
//-----------------------------------------------------------------------------
HCDCodec::State HCDCodec::GetCurrentState() const
    {
    // get the current state of the codec
    return m_CurrentState;
    }