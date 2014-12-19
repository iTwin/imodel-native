//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFMessages.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF message classes
//-----------------------------------------------------------------------------
// Inline methods for Message classes used in HGF.
//-----------------------------------------------------------------------------


///////////////////////////
// HGFGeometryChangedMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGFGeometryChangedMsg::HGFGeometryChangedMsg()
    : HMGAsynchronousMessage()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HGFGeometryChangedMsg::HGFGeometryChangedMsg(const HGFGeometryChangedMsg& pi_rObj)
    : HMGAsynchronousMessage(pi_rObj)
    {
    }


///////////////////////////
// HGFBufferContentChangedMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGFBufferContentChangedMsg::HGFBufferContentChangedMsg()
    : HMGAsynchronousMessage()
    {
    m_TileIndex     = 0;
    m_IsEmpty       = true;
    m_IsOverwritten = false;
    }

//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HGFBufferContentChangedMsg::HGFBufferContentChangedMsg(const HGFBufferContentChangedMsg& pi_rObj)
    : HMGAsynchronousMessage(pi_rObj)
    {
    m_TileIndex     = pi_rObj.m_TileIndex;
    m_IsEmpty       = pi_rObj.m_IsEmpty;
    m_IsOverwritten = pi_rObj.m_IsOverwritten;
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGFBufferContentChangedMsg::HGFBufferContentChangedMsg(uint32_t pi_TileIndex, bool  pi_IsEmpty, bool pi_IsOverwritten)
    : HMGAsynchronousMessage()
    {
    m_TileIndex     = pi_TileIndex;
    m_IsEmpty       = pi_IsEmpty;
    m_IsOverwritten = pi_IsOverwritten;
    }

//-----------------------------------------------------------------------------
// Public
// the Tile index
//-----------------------------------------------------------------------------
inline uint32_t HGFBufferContentChangedMsg::GetTileIndex() const
    {
    return m_TileIndex;
    }

//-----------------------------------------------------------------------------
// Public
// Emptiness state of the tile
//-----------------------------------------------------------------------------
inline bool HGFBufferContentChangedMsg::IsEmpty() const
    {
    return m_IsEmpty;
    }

//-----------------------------------------------------------------------------
// Public
// if the tile data was overwritten at least once
//-----------------------------------------------------------------------------
inline bool HGFBufferContentChangedMsg::IsOverwritten() const
    {
    return m_IsOverwritten;
    }

