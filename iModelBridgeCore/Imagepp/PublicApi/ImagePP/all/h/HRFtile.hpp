//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFtile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFTile
//-----------------------------------------------------------------------------

#include "HFCMonitor.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
// COnstructor
//-----------------------------------------------------------------------------
inline HRFTile::HRFTile(uint64_t pi_ID,
                        uint64_t pi_Index,
                        uint64_t pi_PosX,
                        uint64_t pi_PosY,
                        unsigned short pi_Resolution)
    : HFCEvent(true, false)
    {
    m_ID            = pi_ID;
    m_Index         = pi_Index;
    m_PosX          = pi_PosX;
    m_PosY          = pi_PosY;
    m_Resolution    = pi_Resolution;
    m_Valid         = false;        // Not valid until a SetData()
    m_DataSize      = 0;
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
inline HRFTile::~HRFTile()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline uint64_t HRFTile::GetID() const
    {
    return (m_ID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline uint64_t HRFTile::GetIndex() const
    {
    return (m_Index);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline uint64_t HRFTile::GetPosX() const
    {
    return (m_PosX);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline uint64_t HRFTile::GetPosY() const
    {
    return (m_PosY);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline unsigned short HRFTile::GetResolution() const
    {
    return (m_Resolution);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline bool HRFTile::IsValid() const
    {
    HFCMonitor Monitor(const_cast<HRFTile*>(this));

    return (m_Valid);
    }


//-----------------------------------------------------------------------------
// Public
// Get data size of a valid tile
//-----------------------------------------------------------------------------
inline size_t HRFTile::GetDataSize() const
    {
    HPRECONDITION(WaitUntilSignaled(0));   // the tile must have arrived
    HPRECONDITION(IsValid());              // and valid
    HFCMonitor Monitor(const_cast<HRFTile*>(this));

    return (m_DataSize);
    }


//-----------------------------------------------------------------------------
// Public
// Get data of a valid tile
//-----------------------------------------------------------------------------
inline const Byte* HRFTile::GetData() const
    {
    HPRECONDITION(WaitUntilSignaled(0));   // the tile must have arrived
    HPRECONDITION(IsValid());              // and valid
    HPRECONDITION(GetDataSize() > 0);
    HFCMonitor Monitor(const_cast<HRFTile*>(this));

    return (m_pData);
    }


//-----------------------------------------------------------------------------
// Public
// Set the data for a tile.  Will be set to valid.  The source array will now
// be in this object's possession
//-----------------------------------------------------------------------------
inline void HRFTile::SetData(HArrayAutoPtr<Byte>& pi_rpData,
                             size_t                pi_DataSize)
    {
    HPRECONDITION(pi_rpData.get() != 0);
    HPRECONDITION(pi_DataSize > 0);
    HFCMonitor Monitor(this);

    m_pData    = pi_rpData;
    m_DataSize = pi_DataSize;
    m_Valid    = true;
    }

//-----------------------------------------------------------------------------
// Public
// Set the data for a tile.  Will be set to valid.  The source array will copied
//-----------------------------------------------------------------------------
inline void HRFTile::SetData(const Byte*   pi_pData,
                             size_t         pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    HFCMonitor Monitor(this);

    if (m_DataSize != pi_DataSize)
        m_pData = new Byte[pi_DataSize];

    memcpy(m_pData, pi_pData, pi_DataSize);
    m_DataSize = pi_DataSize;
    m_Valid    = true;

    }
END_IMAGEPP_NAMESPACE
