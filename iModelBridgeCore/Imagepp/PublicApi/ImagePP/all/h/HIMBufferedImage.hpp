//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMBufferedImage.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HIMBufferedImage
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/////////////////////////////
// HIMBufferedImageTileID
/////////////////////////////

//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
inline HIMBufferedImageTileID::HIMBufferedImageTileID(double    pi_ResolutionX,
                                                      double    pi_ResolutionY,
                                                      uint64_t  pi_Position)
    {
    m_Position    = pi_Position;
    m_ResolutionX = pi_ResolutionX;
    m_ResolutionY = pi_ResolutionY;
    }


//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
inline HIMBufferedImageTileID::HIMBufferedImageTileID(const HIMBufferedImageTileID& pi_rObj)
    {
    m_Position    = pi_rObj.m_Position;
    m_ResolutionX = pi_rObj.m_ResolutionX;
    m_ResolutionY = pi_rObj.m_ResolutionY;
    }


//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
inline HIMBufferedImageTileID& HIMBufferedImageTileID::operator=(const HIMBufferedImageTileID& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Position    = pi_rObj.m_Position;
        m_ResolutionX = pi_rObj.m_ResolutionX;
        m_ResolutionY = pi_rObj.m_ResolutionY;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// less-than operator
//-----------------------------------------------------------------------------
inline bool HIMBufferedImageTileID::operator<(const HIMBufferedImageTileID& pi_rObj) const
    {
    // Order by X resolution, then Y resolution, then position.
    return (HDOUBLE_SMALLER_EPSILON(m_ResolutionX, pi_rObj.m_ResolutionX) ||
            ( HDOUBLE_EQUAL_EPSILON(m_ResolutionX, pi_rObj.m_ResolutionX) &&
              ( HDOUBLE_SMALLER_EPSILON(m_ResolutionY, pi_rObj.m_ResolutionY) ||
                (HDOUBLE_EQUAL_EPSILON(m_ResolutionY, pi_rObj.m_ResolutionY) &&
                 HDOUBLE_SMALLER_EPSILON(m_Position, pi_rObj.m_Position)))));

    }


//-----------------------------------------------------------------------------
// Retrieve the position
//-----------------------------------------------------------------------------
inline uint64_t HIMBufferedImageTileID::GetPosition() const
    {
    return m_Position;
    }


//-----------------------------------------------------------------------------
// Set the position
//-----------------------------------------------------------------------------
inline void HIMBufferedImageTileID::SetPosition(uint64_t pi_Position)
    {
    m_Position = pi_Position;
    }



//-----------------------------------------------------------------------------
// Retrieve the resolution for X
//-----------------------------------------------------------------------------
inline double HIMBufferedImageTileID::GetResolutionX() const
    {
    return m_ResolutionX;
    }


//-----------------------------------------------------------------------------
// Setthe resolution for X
//-----------------------------------------------------------------------------
inline void HIMBufferedImageTileID::SetResolutionX(double pi_ResolutionX)
    {
    m_ResolutionX = pi_ResolutionX;
    }



//-----------------------------------------------------------------------------
// Retrieve the resolution for Y
//-----------------------------------------------------------------------------
inline double HIMBufferedImageTileID::GetResolutionY() const
    {
    return m_ResolutionY;
    }


//-----------------------------------------------------------------------------
// Set the resolution for Y
//-----------------------------------------------------------------------------
inline void HIMBufferedImageTileID::SetResolutionY(double pi_ResolutionY)
    {
    m_ResolutionY = pi_ResolutionY;
    }


//-----------------------------------------------------------------------------
// Compare the resolution
//-----------------------------------------------------------------------------
inline bool HIMBufferedImageTileID::IsSameResolution(double pi_ResolutionX,
                                                      double pi_ResolutionY) const
    {
    // We don't use epsilon here, we need exact matches
    return (m_ResolutionX == pi_ResolutionX &&
            m_ResolutionY == pi_ResolutionY);
    }


//-----------------------------------------------------------------------------
// Equality test
//-----------------------------------------------------------------------------
inline bool HIMBufferedImageTileID::operator==(const HIMBufferedImageTileID& pi_rObj) const
    {
    // We don't use epsilon here, we need exact matches
    return (m_ResolutionX == pi_rObj.m_ResolutionX &&
            m_ResolutionY == pi_rObj.m_ResolutionY &&
            m_Position    == pi_rObj.m_Position);
    }




/////////////////////////////
// HIMBufferedImageTile
/////////////////////////////

//-----------------------------------------------------------------------------
// Default constructor
//-----------------------------------------------------------------------------
inline HIMBufferedImageTile::HIMBufferedImageTile ()
    {
    m_pBufferedImage = 0;
    m_IsValid        = false;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HIMBufferedImageTile::HIMBufferedImageTile (
    HIMBufferedImage*             pi_pBufferedImage,
    const HIMBufferedImageTileID& pi_rID,
    const HFCPtr<HRARaster>&      pi_rpRaster,
    bool                         pi_Valid)
    : HPMPoolItem(),
      m_TileID(pi_rID),
      m_pTile(pi_rpRaster)
    {
    HPRECONDITION(pi_pBufferedImage != 0);
    HPRECONDITION(pi_rpRaster != 0);

    m_pBufferedImage = pi_pBufferedImage;
    m_IsValid        = pi_Valid;
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline HIMBufferedImageTile::~HIMBufferedImageTile ()
    {
    if (m_pBufferedImage)
        m_pBufferedImage->RemoveTile(m_TileID);
    }


//-----------------------------------------------------------------------------
// Get the tile ID
//-----------------------------------------------------------------------------
inline const HIMBufferedImageTileID& HIMBufferedImageTile::GetTileID () const
    {
    return m_TileID;
    }


//-----------------------------------------------------------------------------
// Get the tile
//-----------------------------------------------------------------------------
inline HFCPtr<HRARaster> HIMBufferedImageTile::GetRaster () const
    {
    // Return the tile
    return m_pTile;
    }


//-----------------------------------------------------------------------------
// Get the valid state
//-----------------------------------------------------------------------------
inline bool HIMBufferedImageTile::IsValid () const
    {
    return m_IsValid;
    }

//-----------------------------------------------------------------------------
// Set the valid state
//-----------------------------------------------------------------------------
inline void HIMBufferedImageTile::SetValidState (bool pi_IsValid)
    {
    m_IsValid = pi_IsValid;
    }


//-----------------------------------------------------------------------------
// Set deleted
//
// Called by HIMBufferedImage when the tile is not longer use. In this case,
// we must not call RemoveTile on HIMBufferedImage.
//-----------------------------------------------------------------------------
inline void HIMBufferedImageTile::Deleted()
    {
    m_pBufferedImage = 0;
    }


//-----------------------------------------------------------------------------
// Equality test (For STL)
//-----------------------------------------------------------------------------
inline bool HIMBufferedImageTile::operator==(const HIMBufferedImageTile& pi_rObj) const
    {
    return m_TileID == pi_rObj.m_TileID;
    }


//-----------------------------------------------------------------------------
// less-than operator (For STL)
//-----------------------------------------------------------------------------
inline bool HIMBufferedImageTile::operator<(const HIMBufferedImageTile& pi_rObj) const
    {
    return m_TileID < pi_rObj.m_TileID;
    }


//-----------------------------------------------------------------------------
// Return our size (for memory management)
//-----------------------------------------------------------------------------
inline size_t HIMBufferedImageTile::GetAdditionalSize() const
    {
    // Return the tile's size
    return m_pTile->GetObjectSize();
    }


//-----------------------------------------------------------------------------
// protected
//
//-----------------------------------------------------------------------------
inline void HIMBufferedImageTile::UpdateCachedSize()
    {
    // m_ObjectSize is a protected member of HPMPoolItem
    if (m_pTile != 0)
        m_ObjectSize = m_pTile->GetObjectSize();
    else
        m_ObjectSize = 0;
    }

//-----------------------------------------------------------------------------
// protected
//
//-----------------------------------------------------------------------------
inline HFCExclusiveKey& HIMBufferedImageTile::GetExclusiveKey()
    {
    return m_Key;
    }


/////////////////////////////
// HIMBufferedImage
/////////////////////////////

//-----------------------------------------------------------------------------
// Does the buffer have a homogeneous pixel type? Sure.
//-----------------------------------------------------------------------------
inline bool HIMBufferedImage::HasSinglePixelType() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// Get the buffered image's shape
//-----------------------------------------------------------------------------
inline const HVEShape& HIMBufferedImage::GetShape() const
    {
    // Ask the source
    return m_pSource->GetShape();
    }


//-----------------------------------------------------------------------------
// Set the buffered image's shape
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::SetShape(const HVEShape& pi_rShape)
    {
    // Set the shape on the source. This will have the effect of calling us back
    // (NotifyEffectiveShapeChanged) because the source's shape will change. We
    // will then update ourselves accordingly.
    m_pSource->SetShape(pi_rShape);

    // Don't call our ancestor's SetShape(), since we are never using our own
    // shape member...
    }




//-----------------------------------------------------------------------------
// Get buffered image's extent
//-----------------------------------------------------------------------------
inline HGF2DExtent HIMBufferedImage::GetExtent() const
    {
    return m_pSource->GetExtent();
    }


//-----------------------------------------------------------------------------
// Get the scaling ratio in X
//-----------------------------------------------------------------------------
inline double HIMBufferedImage::GetXRatio() const
    {
    return m_XRatio;
    }


//-----------------------------------------------------------------------------
// Get the scaling ratio in Y
//-----------------------------------------------------------------------------
inline double HIMBufferedImage::GetYRatio() const
    {
    return m_YRatio;
    }


//-----------------------------------------------------------------------------
// Get the buffered image's source raster
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HIMBufferedImage::GetSource() const
    {
    return m_pSource;
    }

//-----------------------------------------------------------------------------
// Get the number of tiles for all the buffer's surface
//-----------------------------------------------------------------------------
inline uint64_t HIMBufferedImage::CountTiles() const
    {
    return m_NumberOfTilesX * m_NumberOfTilesY;
    }

inline uint64_t HIMBufferedImage::CountTilesX () const
    {
    return m_NumberOfTilesX;
    }
inline uint64_t HIMBufferedImage::CountTilesY () const
    {
    return m_NumberOfTilesY;
    }

//-----------------------------------------------------------------------------
// Get a pointer to the log object
//-----------------------------------------------------------------------------
inline HPMPool* HIMBufferedImage::GetTileLog() const
    {
    return m_pObjectLog;
    }


//-----------------------------------------------------------------------------
// Calculate members
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::Initialize()
    {
    // Take source's extent, in our coordinate system
    HVEShape SourceShape(*m_pSource->GetEffectiveShape());
    SourceShape.ChangeCoordSys(m_pPhysicalCS);
    HGF2DExtent SourceExtent(SourceShape.GetExtent());

    // Make sure the extent of source is defined
    if (SourceExtent.IsDefined())
        {
        if ((m_NumberOfTilesX = (uint64_t)((SourceExtent.GetWidth() + (m_TileSizeX-1)) / m_TileSizeX)) == 0)
            m_NumberOfTilesX = 1;
        if ((m_NumberOfTilesY = (uint64_t)((SourceExtent.GetHeight() + (m_TileSizeY-1)) / m_TileSizeY)) == 0)
            m_NumberOfTilesY = 1;
        }
    else
        {
        m_NumberOfTilesX = 1;
        m_NumberOfTilesY = 1;
        }
    }


//-----------------------------------------------------------------------------
// ComputeIndex
//-----------------------------------------------------------------------------
inline uint64_t HIMBufferedImage::ComputeIndex (uint64_t pi_TilePosX,
                                               uint64_t pi_TilePosY) const
    {
    return ((m_NumberOfTilesX * (pi_TilePosY / m_TileSizeY)) +
            (pi_TilePosX / m_TileSizeX));
    }


//-----------------------------------------------------------------------------
// GetPositionFromIndex
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::GetPositionFromIndex (uint64_t   pi_Index,
                                                    uint64_t*    po_pTilePosX,
                                                    uint64_t*    po_pTilePosY) const
    {
    *po_pTilePosX = (pi_Index % m_NumberOfTilesX) * m_TileSizeX;
    *po_pTilePosY = (pi_Index / m_NumberOfTilesX) * m_TileSizeY;
    }


//-----------------------------------------------------------------------------
// Return the buffered image's physical CS
//-----------------------------------------------------------------------------
inline const HFCPtr<HGF2DCoordSys>& HIMBufferedImage::GetPhysicalCoordSys() const
    {
    return m_pPhysicalCS;
    }

//-----------------------------------------------------------------------------
// Indicate if we must apply the dithering for the sources copying
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::SetDithering(bool pi_State)
    {
    m_UseDithering = pi_State;
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::SetAveraging(bool pi_State)
    {
    m_UseAveraging = pi_State;
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::SetBilinear(bool pi_State)
    {
    m_UseBilinear = pi_State;
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::SetConvolution(bool pi_State)
    {
    m_UseConvolution = pi_State;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
inline uint8_t HIMBufferedImage::MaxSourceResolutionStretchingFactor() const
    {
    return m_MaxSourceResolutionStretchingFactor;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
inline void HIMBufferedImage::SetMaxSourceResolutionStretchingFactor(uint8_t pi_Factor)
    {
    // Represents a percentage
    HASSERT(pi_Factor <= 100);

    if (pi_Factor != m_MaxSourceResolutionStretchingFactor)
        {
        m_MaxSourceResolutionStretchingFactor = pi_Factor;

        // We have to recalculate all of our tiles
        Invalidate(true);
        }
    }
END_IMAGEPP_NAMESPACE