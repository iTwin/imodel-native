//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFTileIDDescriptor.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


// Constants
#define FIELD_LEVEL        (8)
#define MAX_LEVEL          (256)
#define FIELD_TILEINDEX    ((sizeof(uint64_t)*8)-FIELD_LEVEL)

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    Changes the size of the image to which applies the Tile ID descriptor.

    @param pi_Width IN The new width of the image. This value may not be 0

    @param pi_Height IN The new height of the image. This value may not be 0
    -----------------------------------------------------------------------------
*/
inline void HGFTileIDDescriptor::ChangeSize (uint64_t pi_Width,
                                             uint64_t pi_Height)
    {
    m_ImageWidth    = pi_Width;
    m_ImageHeight   = pi_Height;

    m_NumberOfTileX = (m_ImageWidth + (m_TileSizeX-1L)) / m_TileSizeX;
    }

/** -----------------------------------------------------------------------------
    Returns the index associated with the indicated X and Y tile positions provided.

    @param pi_TilePosX IN The X tile position in the tile set.

    @param pi_TilePosY IN The Y tile position in the tile set.

    @return The index of the indicated tile.

    @see ComputeID()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::ComputeIndex (uint64_t pi_TilePosX,
                                                  uint64_t pi_TilePosY) const
    {
    return ((m_NumberOfTileX * (pi_TilePosY / m_TileSizeY)) +
            (pi_TilePosX / m_TileSizeX));
    }


/** -----------------------------------------------------------------------------
    Returns the index associated with the indicated X and Y tile positions provided.

    @param pi_TilePosX IN The X tile position in the tile set.

    @param pi_TilePosY IN The Y tile position in the tile set.

    @param pi_Level IN The level(subres) to which are associated tile X and Y position.

    @return The ID of the indicated tile.

    @see ComputeIndex()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::ComputeID (uint64_t pi_TilePosX,
                                               uint64_t pi_TilePosY,
                                               uint32_t pi_Level) const
    {
    HPRECONDITION(pi_Level < MAX_LEVEL);

    // Add Level
    return ((ComputeIndex(pi_TilePosX, pi_TilePosY) | ((uint64_t)pi_Level << FIELD_TILEINDEX)));
    }

/** -----------------------------------------------------------------------------
    Computes the ID from the index provided the level.

    @param pi_Index The index of the tile.

    @param pi_Level IN The level(subres) to which are associated tile index.

    @return The ID of the indicated tile.

    @see ComputeID()
    @see ComputeIndex()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::ComputeIDFromIndex (uint64_t pi_Index,
                                                        uint32_t pi_Level) const
    {
    HPRECONDITION(pi_Level < MAX_LEVEL);

    // Add Level
    return ((pi_Index | ((uint64_t)pi_Level << FIELD_TILEINDEX)));
    }

/** -----------------------------------------------------------------------------
    Returns the level associated with given tile ID.

    @param pi_TileID The ID to obtain level of.

    @return The level corresponding to the given tile ID.

    @see GetIndex()
    -----------------------------------------------------------------------------
*/
inline uint32_t HGFTileIDDescriptor::GetLevel (uint64_t pi_TileID) const
    {
    // Extract Level
    return (uint32_t)(pi_TileID >> FIELD_TILEINDEX);
    }

/** -----------------------------------------------------------------------------
    Returns the index associated with given tile ID.

    @param pi_TileID The ID to obtain index of.

    @return The index corresponding to the given tile ID.

    @see GetLevel()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::GetIndex (uint64_t pi_TileID) const
    {
    // Remove Level
    pi_TileID <<= FIELD_LEVEL;
    pi_TileID >>= FIELD_LEVEL;
    return (pi_TileID);
    }

/** -----------------------------------------------------------------------------
    Returns the tile positions (X and Y) associated to given ID.

    @param pi_TileID IN The ID to obtain positions of.

    @param po_pTilePosX OUT Pointer to UInt32 that receives the tile X position.

    @param po_pTilePosY OUT Pointer to UInt32 that receives the tile Y position.

    -----------------------------------------------------------------------------
*/
inline void HGFTileIDDescriptor::GetPositionFromID (uint64_t  pi_TileID,
                                                    uint64_t*   po_pTilePosX,
                                                    uint64_t*   po_pTilePosY)
    {
    // Remove Level
    pi_TileID <<= FIELD_LEVEL;
    pi_TileID >>= FIELD_LEVEL;

    *po_pTilePosX = (uint64_t)(pi_TileID % m_NumberOfTileX) * m_TileSizeX;
    *po_pTilePosY = (uint64_t)(pi_TileID / m_NumberOfTileX) * m_TileSizeY;
    }

//-----------------------------------------------------------------------------
// Get the tile data size
//-----------------------------------------------------------------------------
inline void HGFTileIDDescriptor::GetTileDataSize(uint64_t pi_TileID,
                                                 uint64_t& po_rDataWidth,
                                                 uint64_t& po_rDataHeight)
    {
    uint64_t tilePosX;
    uint64_t tilePosY;

    GetPositionFromID(pi_TileID, &tilePosX, &tilePosY);

    if (tilePosX < (m_NumberOfTileX - 1) * m_TileSizeX)
        {
        po_rDataWidth = m_TileSizeX;
        }
    else
        {
        po_rDataWidth = m_ImageWidth % m_TileSizeX;
        }

    uint64_t numberOfFullTileY = m_ImageHeight / m_TileSizeY;

    if (tilePosY < numberOfFullTileY * m_TileSizeY)
        {
        po_rDataHeight = m_TileSizeY;
        }
    else
        {
        po_rDataHeight = m_ImageHeight % m_TileSizeY;
        }
    }

/** -----------------------------------------------------------------------------
    Returns the tile positions (X and Y) associated to given index.

    @param pi_Index IN The index to obtain positions of.

    @param po_pTilePosX OUT Pointer to UInt32 that receives the tile X position.

    @param po_pTilePosY OUT Pointer to UInt32 that receives the tile Y position.

    -----------------------------------------------------------------------------
*/
inline void HGFTileIDDescriptor::GetPositionFromIndex (uint64_t  pi_Index,
                                                       uint64_t*   po_pTilePosX,
                                                       uint64_t*   po_pTilePosY)
    {
    *po_pTilePosX = (uint64_t)(pi_Index % m_NumberOfTileX) * m_TileSizeX;
    *po_pTilePosY = (uint64_t)(pi_Index / m_NumberOfTileX) * m_TileSizeY;
    }


/** -----------------------------------------------------------------------------
    Returns the index of the next tile to process.

    @return The index of the next tile to process or INDEX_NOT_FOUND if no more tiles
            need processing.

    @see GetFirstTileIndex()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::GetNextTileIndex ()
    {
    if (m_CurIndex != INDEX_NOT_FOUND)
        {
        m_CurIndex++;

        // End of the line of tile
        if (m_CurIndex > m_LastCurIndex)
            {
            // Compute first Tile in next line
            m_FirstCurIndex += m_NumberOfTileX;

            if (m_FirstCurIndex <= m_LastFirstColumn)
                {
                m_LastCurIndex += m_NumberOfTileX;
                m_CurIndex      = m_FirstCurIndex;
                }
            else
                m_CurIndex = INDEX_NOT_FOUND;
            }
        }

    return (m_CurIndex);
    }

/** -----------------------------------------------------------------------------
    Returns the image width associated with tile ID descriptor.

    @return The image width.

    @see GetImageHeight()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::GetImageWidth() const
    {
    return m_ImageWidth;
    }

/** -----------------------------------------------------------------------------
    Returns the image height associated with tile ID descriptor.

    @return The image height.

    @see GetImageWidth()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::GetImageHeight() const
    {
    return m_ImageHeight;
    }

/** -----------------------------------------------------------------------------
    Returns the tile width associated with tile ID descriptor.

    @return The tile width.

    @see GetImageWidth()
    @see GetTileHeight()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::GetTileWidth() const
    {
    return m_TileSizeX;
    }

/** -----------------------------------------------------------------------------
    Returns the tile height associated with tile ID descriptor.

    @return The tile height.

    @see GetImageHeight()
    @see GetTileWidth()
    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::GetTileHeight() const
    {
    return m_TileSizeY;
    }

/** -----------------------------------------------------------------------------
    Returns the total number of tiles associated to this tile ID descriptor.

    @return The number of tiles

    -----------------------------------------------------------------------------
*/
inline uint64_t HGFTileIDDescriptor::GetTileCount() const
    {
    return (m_NumberOfTileX * (uint64_t)((m_ImageHeight + (m_TileSizeY-1L)) / m_TileSizeY));
    }

END_IMAGEPP_NAMESPACE