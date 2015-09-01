//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMBufferedImage.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMBufferedImage
//-----------------------------------------------------------------------------
// This class describes an image buffer.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAStoredRaster.h"
#include "HFCPtr.h"
#include "HGF2DTranslation.h"
#include "HFCExclusiveKey.h"
#include "HPMPool.h"

BEGIN_IMAGEPP_NAMESPACE
class HIMBufferedImageIterator;
class HRAClearOptions;
class HRABitmap;

// Type used to return a set of tile IDs covering an extent
typedef set<uint64_t> HIMBufImgTileIDSet;


///////////////////////////////////
// HIMBufferedImageTileID class
///////////////////////////////////

class HIMBufferedImageTileID
    {
public:

    // Basic

    HIMBufferedImageTileID(double      pi_ResolutionX = 0.0,
                           double      pi_ResolutionY = 0.0,
                           uint64_t    pi_Position    = 0);

    HIMBufferedImageTileID(const HIMBufferedImageTileID& pi_rObj);
    HIMBufferedImageTileID& operator=(const HIMBufferedImageTileID& pi_rObj);

    // Ordering

    bool           operator<(const HIMBufferedImageTileID& pi_rObj) const;
    bool           operator==(const HIMBufferedImageTileID& pi_rObj) const;

    // Information retrieval

    uint64_t       GetPosition() const;
    void            SetPosition(uint64_t   pi_Position);

    double         GetResolutionX() const;
    void            SetResolutionX(double pi_ResolutionX);

    double         GetResolutionY() const;
    void            SetResolutionY(double pi_ResolutionY);

    bool           IsSameResolution(double pi_ResolutionX,
                                     double pi_ResolutionY) const;

private:

    // Tile's Position
    // The bufferedImage tiles are numbered starting with 0 and
    // working left to right, top to bottom
    //      0 1 2 3 4
    //      5 6 7 ...
    uint64_t       m_Position;

    // Resolution of the tile
    double         m_ResolutionX;
    double         m_ResolutionY;
    };



///////////////////////////////////
// HIMBufferedImageTile class
///////////////////////////////////

class HIMBufferedImage;

class HIMBufferedImageTile : public HPMPoolItem, public HFCExclusiveKey
    {
public:

    // Basic

    HIMBufferedImageTile();
    HIMBufferedImageTile(HIMBufferedImage*             pi_pBufferedImage,
                         const HIMBufferedImageTileID& pi_rID,
                         const HFCPtr<HRARaster>&      pi_rpRaster,
                         bool                         pi_Valid = true);

    ~HIMBufferedImageTile();

    bool           operator==(const HIMBufferedImageTile& pi_rObj) const;
    bool           operator<(const HIMBufferedImageTile& pi_rObj) const;


    // Set

    void            SetValidState(bool pi_IsValid);


    // Information retrieval

    const HIMBufferedImageTileID&
    GetTileID() const;

    HFCPtr<HRARaster>
    GetRaster() const;

    bool           IsValid() const;


    // To be called by our token
    size_t          GetAdditionalSize() const;

    // Called by HIMBufferedImage when the tile is not longer use
    void            Deleted();


protected:

    virtual void                UpdateCachedSize();
    virtual HFCExclusiveKey&    GetExclusiveKey();


private:

    // Disable access to these methods
    HIMBufferedImageTile(const HIMBufferedImageTile& pi_rObj);
    HIMBufferedImageTile& operator=(const HIMBufferedImageTile& pi_rObj);

    // Pointer to container buffered image
    HIMBufferedImage*       m_pBufferedImage;

    // Identifier of the tile
    HIMBufferedImageTileID  m_TileID;

    // Pointer to the tile
    HFCPtr<HRARaster>       m_pTile;

    // Valid state
    bool                   m_IsValid;


    // Used in BufferedImage, serialize access to the tile
    HFCExclusiveKey         m_Key;
    };


///////////////////////////////////
// HIMBufferedImage class
///////////////////////////////////

class HIMBufferedImage : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HIMBufferedImageId)

    friend class HIMBufferedImageIterator;
    friend class HIMBufferedImageTile;

public:

    // Primary methods
    IMAGEPP_EXPORT HIMBufferedImage();

    IMAGEPP_EXPORT HIMBufferedImage(const HFCPtr<HRARaster>&        pi_rpSource,
                            const HFCPtr<HRAStoredRaster>&  pi_rpExample,
                            HPMPool*                        pi_pObjectLog,
                            double                          pi_rXRatio = 1.0,
                            double                          pi_rYRatio = 1.0,
                            uint32_t                       pi_TileSizeX = 256,
                            uint32_t                       pi_TileSizeY = 256,
                            bool                            pi_ShapeTheTiles = false,
                            uint8_t                        pi_MaxResolutionStretchingFactor = 0);

    IMAGEPP_EXPORT virtual ~HIMBufferedImage();


    // Overriden from HGFGraphicObject

    virtual HGF2DExtent     GetExtent() const;

    virtual void    Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void    Rotate(double pi_Angle,
                           const HGF2DLocation& pi_rOrigin);
    virtual void    Scale(double pi_ScaleFactorX,
                          double pi_ScaleFactorY,
                          const HGF2DLocation& pi_rOrigin);

    // Overriden from HRARaster
    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions);

    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster);

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;

    virtual HRARasterIterator*
    CreateIterator (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual HRARasterEditor*
    CreateEditor   (HFCAccessMode pi_Mode);

    virtual HRARasterEditor*
    CreateEditor   (const HVEShape& pi_rShape,
                    HFCAccessMode   pi_Mode);

    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual bool   HasSinglePixelType() const;
    virtual HFCPtr<HRPPixelType>
    GetPixelType() const;

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id = 0) const;

    virtual HFCPtr<HVEShape>    GetEffectiveShape () const;

    virtual HGF2DExtent    GetAveragePixelSize () const;
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const;

    virtual const HVEShape&    GetShape    () const;
    virtual void    SetShape    (const HVEShape& pi_rShape);

    virtual HPMPersistentObject* Clone () const;

    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    virtual bool   IsStoredRaster  () const;

    // Added methods

    virtual void    Invalidate(bool pi_DeleteAllTiles = false);
    virtual void    Invalidate(const HVEShape& pi_rRegion);

    virtual void    SetRatios(double pi_XRatio,
                              double pi_YRatio);
    virtual double    GetXRatio() const;
    virtual double    GetYRatio() const;

    const HFCPtr<HRARaster>&    GetSource() const;

    const HFCPtr<HGF2DCoordSys>&    GetPhysicalCoordSys() const;

    void            SetDithering(bool pi_State);
    void            SetAveraging(bool pi_State);
    void            SetBilinear(bool pi_State);
    void            SetConvolution(bool pi_State);

    const HFCPtr<HRARaster>      GetTile(uint64_t pi_ID) const;
    void            GetPositionFromIndex (uint64_t     pi_Index,
                                          uint64_t*      po_pTilePosX,
                                          uint64_t*      po_pTilePosY) const;
    uint64_t       CountTilesX () const;
    uint64_t       CountTilesY () const;
    uint64_t       ComputeIndex (uint64_t     pi_TilePosX,
                                  uint64_t     pi_TilePosY) const;


    // Make the buffered image represent itself in the specified
    // system. Changes its logical system and ratios.
    void            RepresentIn(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    // LookAhead Methods
    virtual bool   HasLookAhead() const;
    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    uint8_t        MaxSourceResolutionStretchingFactor() const;
    void            SetMaxSourceResolutionStretchingFactor(uint8_t pi_Factor);

    // Notification messages

    bool NotifyContentChanged (const HMGMessage& pi_rMessage);

    bool NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage);

    bool NotifyGeometryChanged (const HMGMessage& pi_rMessage);

    bool NotifyPaletteChanged (const HMGMessage& pi_rMessage);

    //Context Methods
    virtual void                      SetContext(const HFCPtr<HMDContext>& pi_rpContext);
    virtual HFCPtr<HMDContext>        GetContext();

    virtual void                      InvalidateRaster();

    // Preparation
    IMAGEPP_EXPORT void            PrepareRegion(const HVEShape& pi_rRegion);

protected:

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;

    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options) override;

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;
        
    // From HGFGraphicObject
    virtual void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pOldCoordSys);

    // API for iterator
    uint64_t       CountTiles() const;
    HIMBufImgTileIDSet*     GetTileIDPoolFor(const HVEShape& pi_rShape) const;

    // Remove a tile, given its ID. Used when the log tells
    // a tile to remove itself.
    void            RemoveTile(const HIMBufferedImageTileID& pi_rTileID);

    // Get a pointer to the log object
    HPMPool*        GetTileLog() const;

private:

    //////////////////
    // Methods
    //////////////////

    // Not yet implemented
    HIMBufferedImage(const HIMBufferedImage& pi_rObj);
    HIMBufferedImage& operator=(const HIMBufferedImage& pi_rObj);

    // Calculate and set the internal buffer's coordinate systems
    void            SetCoordSysOfExample();

    void            Initialize();


    // Type used to extract all used ratios
    typedef vector< double, allocator<double> > RatiosList;

    // Sort the used ratios
    class Predicate : public binary_function<double, double, bool>
        {
    public:

        Predicate(double pi_CenterRatio)
            : m_CenterRatio(pi_CenterRatio)
            {
            };

        bool operator()(double pi_First, double pi_Second)
            {
            // Order by distance to center ratio.
            if (pi_First > pi_Second)
                return (pi_First / m_CenterRatio) < (m_CenterRatio / pi_Second);
            else
                return (m_CenterRatio / pi_First) < (pi_Second / m_CenterRatio);
            };

    private:

        double m_CenterRatio;
        };


    //////////////////
    // Attributes
    //////////////////

    // The buffered image's source
    HFCPtr<HRARaster>       m_pSource;

    // A copy of the given example raster
    HFCPtr<HRAStoredRaster> m_pExample;

    // HRABitmap used to draw a temporary data
    HFCPtr<HRABitmap>       m_pTmpTile;
    mutable double         m_TmpTileXOrigin;
    mutable double         m_TmpTileYOrigin;

    // The log we use for the buffer's tiles
    HPMPool*                m_pObjectLog;

    // The tile dimensions
    uint32_t                m_TileSizeX;
    uint32_t                m_TileSizeY;

    // The number of tiles for the logical buffer surface
    uint64_t               m_NumberOfTilesX;
    uint64_t               m_NumberOfTilesY;

    // The pixel size for the buffer
    double             m_XRatio;
    double             m_YRatio;

    HFCPtr<HGF2DCoordSys>   m_pPhysicalCS;

    // For optimization. If true, the tiles will have
    // a shape set (EffectiveShape of the source). If
    // false, no shape will be applied (Fastest).
    bool                   m_ShapeTheTiles;

    // parameter indicating if we must dither the sources when copying
    bool                   m_UseDithering;

    bool                   m_UseAveraging;
    bool                   m_UseBilinear;
    bool                   m_UseConvolution;

    // Typedef and member for tile map.
    typedef map<HIMBufferedImageTileID,
            HIMBufferedImageTile*,
            less<HIMBufferedImageTileID>,
            allocator< HIMBufferedImageTile* > > TileMap;

    mutable TileMap         m_TileMap;

    mutable HFCExclusiveKey m_TileMapKey;

    // Our effective shape, kept here to stop calculating it
    // every second...
    HFCPtr<HVEShape>        m_pCachedEffectiveShape;

    // Used when copying into buffered tiles
    uint8_t                m_MaxSourceResolutionStretchingFactor;

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };

END_IMAGEPP_NAMESPACE
#include "HIMBufferedImage.hpp"

