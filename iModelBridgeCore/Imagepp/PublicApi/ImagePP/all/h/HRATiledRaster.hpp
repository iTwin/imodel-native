//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRATiledRaster.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

///////////////////////////
// HRATileStatus class
///////////////////////////

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Find the byte position that holds flags for the specified tile.
    -----------------------------------------------------------------------------
*/
inline uint64_t HRATileStatus::ComputeByteContainingTile(uint64_t pi_TileID) const
    {
    return pi_TileID / 4;
    }


/** -----------------------------------------------------------------------------
    Obtain the bitmask for the first flag of the specified tile.
    -----------------------------------------------------------------------------
*/
inline Byte HRATileStatus::GetBitmaskForFlagsOfTile(uint64_t pi_TileID) const
    {
    return 0x80 >> ((pi_TileID % 4) * 2);
    }


/** -----------------------------------------------------------------------------
    Retrieve the number of bytes needed to hold the flags.
    @note m_NumberOfTiles must be set.
    -----------------------------------------------------------------------------
*/
inline uint64_t HRATileStatus::GetByteCount() const
    {
    return (m_NumberOfTiles + 3) / 4;
    }

//-----------------------------------------------------------------------------
// Inline methods for class HRATile
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Discartable
//-----------------------------------------------------------------------------
inline void HRATiledRaster::HRATile::Discartable(bool pi_Discartable)
    {
    m_Discartable = pi_Discartable;
    }

//-----------------------------------------------------------------------------
// public
// IsDiscartable
//-----------------------------------------------------------------------------
inline bool HRATiledRaster::HRATile::IsDiscartable() const
    {
    return m_Discartable;
    }

//-----------------------------------------------------------------------------
// public
// Discartable
//-----------------------------------------------------------------------------
inline void HRATiledRaster::HRATile::Invalidate(bool pi_Invalidate)
    {
    m_Invalidate = pi_Invalidate;
    }

//-----------------------------------------------------------------------------
// public
// IsDiscartable
//-----------------------------------------------------------------------------
inline bool HRATiledRaster::HRATile::IsInvalidate() const
    {
    return m_Invalidate;
    }

//-----------------------------------------------------------------------------
// public
// GetTile
//-----------------------------------------------------------------------------
inline HFCPtr<HRABitmapBase>& HRATiledRaster::HRATile::GetTile()
    {
    return m_pTile;
    }


//-----------------------------------------------------------------------------
// public
// SetTile
//-----------------------------------------------------------------------------
inline void HRATiledRaster::HRATile::SetTile(HFCPtr<HRABitmapBase>& pi_rpTile)
    {
    m_pTile = pi_rpTile;
    }


//-----------------------------------------------------------------------------
// protected
// GetExclusiveKey
//-----------------------------------------------------------------------------
inline HFCExclusiveKey& HRATiledRaster::HRATile::GetExclusiveKey()
    {
    return s_Key;
    }

///////////////////////////
// HRATiledRaster class
///////////////////////////

//-----------------------------------------------------------------------------
// public
// GetTileSizeX -
//-----------------------------------------------------------------------------
inline uint64_t HRATiledRaster::GetTileSizeX    () const
    {
    return (m_TileSizeX);
    }

//-----------------------------------------------------------------------------
// public
// GetTileSizeY -
//-----------------------------------------------------------------------------
inline uint64_t HRATiledRaster::GetTileSizeY    () const
    {
    return (m_TileSizeY);
    }

//-----------------------------------------------------------------------------
// private
// GetInternalTileStatusList - Get a reference to the internal tile Status
// object.
//-----------------------------------------------------------------------------
inline HRATileStatus& HRATiledRaster::GetInternalTileStatusList (uint64_t* po_pNumberOfTile)
    {
    if (po_pNumberOfTile)
        *po_pNumberOfTile = m_NumberOfTiles;

    return m_TileStatus;
    }

//-----------------------------------------------------------------------------
// private
// GetNumberOfTileX - Get the number of tile in X
//-----------------------------------------------------------------------------
inline uint64_t HRATiledRaster::GetNumberOfTileX () const
    {
    return (m_NumberOfTileX);
    }

//-----------------------------------------------------------------------------
// private
// GetNumberOfTileY - Get the number of tile in Y
//-----------------------------------------------------------------------------
inline uint64_t HRATiledRaster::GetNumberOfTileY () const
    {
    return (m_NumberOfTileY);
    }

//-----------------------------------------------------------------------------
// protected
// EnableLookAhead
//-----------------------------------------------------------------------------
inline void HRATiledRaster::EnableLookAhead(bool pi_ByExtent)
    {
    HASSERT(GetStore() != 0);

    m_LookAheadEnabled = true;
    m_LookAheadByExtent= pi_ByExtent;
    }

//-----------------------------------------------------------------------------
// private
// SetInternalTileStatusSupported
//-----------------------------------------------------------------------------
inline void HRATiledRaster::SetInternalTileStatusSupported(bool pi_TileStatusSupported)
    {
    m_TileStatusDisabled = !pi_TileStatusSupported;
    }
END_IMAGEPP_NAMESPACE
