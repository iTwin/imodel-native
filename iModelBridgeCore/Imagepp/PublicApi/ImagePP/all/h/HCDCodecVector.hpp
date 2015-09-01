//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecVector.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HCDCodecVector
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// public inline method
// GetDataSize
//-----------------------------------------------------------------------------
inline size_t HCDCodecVector::GetDataSize() const
    {
    // return the data size
    return m_DataSize;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetSubsetSize
//-----------------------------------------------------------------------------
inline size_t HCDCodecVector::GetSubsetSize() const
    {
    // return the subset size
    return m_SubsetSize;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetSubsetPos
//-----------------------------------------------------------------------------
inline size_t HCDCodecVector::GetSubsetPos() const
    {
    // return the subset position
    return m_SubsetPos;
    }

END_IMAGEPP_NAMESPACE