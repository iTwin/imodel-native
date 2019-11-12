//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HCDPacket
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public inline method
// GetDataSize
//-----------------------------------------------------------------------------
inline size_t HCDPacket::GetDataSize() const
    {
    // return the data size
    return m_DataSize;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetBufferSize
//-----------------------------------------------------------------------------
inline size_t HCDPacket::GetBufferSize() const
    {
    // return the buffer size
    return m_BufferSize;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetBufferAddtess
//-----------------------------------------------------------------------------
inline Byte* HCDPacket::GetBufferAddress() const
    {
    // return the buffer address
    return m_pBuffer;
    }

//-----------------------------------------------------------------------------
// public inline method
// HasBufferOwnership
//-----------------------------------------------------------------------------
inline bool HCDPacket::HasBufferOwnership() const
    {
    return m_BufferOwner;
    }

END_IMAGEPP_NAMESPACE