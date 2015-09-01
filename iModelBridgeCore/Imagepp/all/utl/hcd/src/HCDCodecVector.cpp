//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecVector.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecVector
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecVector.h>


//-----------------------------------------------------------------------------
// protected
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecVector::HCDCodecVector(const WString& pi_rCodecName)
    : HCDCodec(pi_rCodecName)
    {
    // default values
    m_DataSize = 0;
    m_SubsetSize = 0;
    m_SubsetPos = 0;
    }


//-----------------------------------------------------------------------------
// protected
// Constructor
//-----------------------------------------------------------------------------
HCDCodecVector::HCDCodecVector(const WString&   pi_rCodecName,
                               size_t           pi_DataSize)
    : HCDCodec(pi_rCodecName)
    {
    // set the data size
    m_DataSize = pi_DataSize;

    // by default, the subset size is the same as the data size
    // the subset pos is then 0
    m_SubsetSize = pi_DataSize;
    m_SubsetPos = 0;
    }

//-----------------------------------------------------------------------------
// protected
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecVector::HCDCodecVector(const HCDCodecVector& pi_rObj)
    : HCDCodec(pi_rObj)
    {
    // copy the attributes
    m_DataSize = pi_rObj.m_DataSize;
    m_SubsetSize = pi_rObj.m_SubsetSize;
    m_SubsetPos = pi_rObj.m_SubsetPos;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecVector::~HCDCodecVector()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetMinimumSubsetSize
//-----------------------------------------------------------------------------
size_t HCDCodecVector::GetMinimumSubsetSize() const
    {
    return m_DataSize;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecVector::GetSubsetMaxCompressedSize() const
    {
    // HLXXX by default, return the subet size
    return m_SubsetSize;
    }

//-----------------------------------------------------------------------------
// public
// IsCodecCodec
//-----------------------------------------------------------------------------
bool HCDCodecVector::IsCodecVector() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodecVector::Reset()
    {
    // reset the codec
    HCDCodec::Reset();

    // reset the subset position to 0
    m_SubsetPos = 0;
    }

//-----------------------------------------------------------------------------
// public method
// SetDataSize
//-----------------------------------------------------------------------------
void HCDCodecVector::SetDataSize(size_t pi_DataSize)
    {
    // set the data size
    m_DataSize = pi_DataSize;

    // by defaultm the subset size is the same as the data size
    // the subset position is then 0
    m_SubsetPos = 0;
    m_SubsetSize = pi_DataSize;
    }

//-----------------------------------------------------------------------------
// public
// SetSubsetSize
//-----------------------------------------------------------------------------
void HCDCodecVector::SetSubsetSize(size_t pi_Size)
    {
    m_SubsetSize = pi_Size;
    }

//-----------------------------------------------------------------------------
// public inline method
// SetSubsetPos
//-----------------------------------------------------------------------------
void HCDCodecVector::SetSubsetPos(size_t pi_Pos)
    {
    // set the subset position
    m_SubsetPos = pi_Pos;
    }
