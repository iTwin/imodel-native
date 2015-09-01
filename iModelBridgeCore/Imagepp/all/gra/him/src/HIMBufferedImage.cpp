//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMBufferedImage.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMBufferedImage
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMBufferedImage.h>
#include <Imagepp/all/h/HIMBufferedImageIterator.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HFCMonitor.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HGSTypes.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAClearOptions.h>

#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRAMessages.h>

#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HGFException.h>
#include <Imagepp/all/h/HRPMessages.h>

#include <Imagepp/all/h/HMDContext.h>


///////////////////////////////////
// HIMBufferedImage class
///////////////////////////////////

// Persistence registration
HPM_REGISTER_CLASS(HIMBufferedImage, HRARaster)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HIMBufferedImage, HRARaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HIMBufferedImage, HGFGeometryChangedMsg, NotifyGeometryChanged)
HMG_REGISTER_MESSAGE(HIMBufferedImage, HRAEffectiveShapeChangedMsg, NotifyEffectiveShapeChanged)
HMG_REGISTER_MESSAGE(HIMBufferedImage, HRAContentChangedMsg, NotifyContentChanged)
HMG_REGISTER_MESSAGE(HIMBufferedImage, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_REGISTER_MESSAGE(HIMBufferedImage, HRAProgressImageChangedMsg, NotifyContentChanged)
HMG_END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// Default constructor
//-----------------------------------------------------------------------------
HIMBufferedImage::HIMBufferedImage()
    : HRARaster()
    {
    m_pObjectLog    = 0;
    m_ShapeTheTiles = false;

    // by default, don't use dithering for the sources
    m_UseDithering = false;
    m_UseAveraging = false;
    m_UseBilinear = false;
    m_UseConvolution = false;

    m_MaxSourceResolutionStretchingFactor = 0;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HIMBufferedImage::HIMBufferedImage(const HFCPtr<HRARaster>&       pi_rpSource,
                                   const HFCPtr<HRAStoredRaster>& pi_rpExample,
                                   HPMPool*                       pi_pObjectLog,
                                   double                         pi_rXRatio,
                                   double                         pi_rYRatio,
                                   uint32_t                      pi_TileSizeX,
                                   uint32_t                      pi_TileSizeY,
                                   bool                           pi_ShapeTheTiles,
                                   uint8_t                       pi_MaxResolutionStretchingFactor)
    : HRARaster(pi_rpExample->GetCoordSys()),
      m_XRatio(pi_rXRatio),
      m_YRatio(pi_rYRatio),
      m_pSource(pi_rpSource),
      m_ShapeTheTiles(pi_ShapeTheTiles)
    {
    HPRECONDITION(pi_rpSource != 0);
    HPRECONDITION(pi_rpExample != 0);
    HPRECONDITION(pi_pObjectLog != 0);
    HPRECONDITION(pi_TileSizeX > 0);
    HPRECONDITION(pi_TileSizeY > 0);


    m_pObjectLog = pi_pObjectLog;

    m_TileSizeX    = pi_TileSizeX;
    m_TileSizeY    = pi_TileSizeY;

    // Take a copy of the example raster, and make it 1x1
    m_pExample = (HRAStoredRaster*) pi_rpExample->Clone();
    SetCoordSysOfExample();
    m_pExample->InitSize(pi_TileSizeX, pi_TileSizeY);

    m_pTmpTile = HRABitmap::Create(m_TileSizeX,
                                   m_TileSizeY,
                                   0,
                                   m_pPhysicalCS,
                                   m_pExample->GetPixelType());

    // by default, don't use dithering for the sources
    m_UseDithering = false;
    m_UseAveraging = false;
    m_UseBilinear = false;
    m_UseConvolution = false;

    // Represents a percentage
    HASSERT(pi_MaxResolutionStretchingFactor <= 100);
    m_MaxSourceResolutionStretchingFactor = pi_MaxResolutionStretchingFactor;

    Initialize();

    // Link ourself to our source
    LinkTo(pi_rpSource);
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HIMBufferedImage::HIMBufferedImage(const HIMBufferedImage& pi_rObj)
    : HRARaster((HRARaster&)pi_rObj),
      m_XRatio(pi_rObj.m_XRatio),
      m_YRatio(pi_rObj.m_YRatio),
      m_pSource(pi_rObj.m_pSource),
      m_pPhysicalCS(new HGF2DCoordSys(*pi_rObj.m_pPhysicalCS)),
      m_ShapeTheTiles(pi_rObj.m_ShapeTheTiles)
    {
    m_pObjectLog   = pi_rObj.m_pObjectLog;
    m_TileSizeX    = pi_rObj.m_TileSizeX;
    m_TileSizeY    = pi_rObj.m_TileSizeY;

    m_MaxSourceResolutionStretchingFactor = pi_rObj.m_MaxSourceResolutionStretchingFactor;

    // Keep a copy of the example raster
    m_pExample = static_cast<HRAStoredRaster*>(pi_rObj.m_pExample->Clone(0).GetPtr());

    // Copy construct the internal tile list??

    m_UseDithering = pi_rObj.m_UseDithering;
    m_UseAveraging = pi_rObj.m_UseAveraging;
    m_UseBilinear = pi_rObj.m_UseBilinear;
    m_UseConvolution = pi_rObj.m_UseConvolution;

    Initialize();

    // Link ourself to our source
    LinkTo(m_pSource);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HIMBufferedImage::~HIMBufferedImage()
    {
    HFCMonitor Monitor(m_TileMapKey);

    // remove tile from pool
    TileMap::iterator Itr(m_TileMap.begin());
    HIMBufferedImageTile* pTile;
    while (Itr != m_TileMap.end())
        {
        pTile = Itr->second;
        Itr++;
        pTile->Discard();
        }

    UnlinkFrom(m_pSource);
    }


//-----------------------------------------------------------------------------
// Assignment operation
//-----------------------------------------------------------------------------
HIMBufferedImage& HIMBufferedImage::operator=(const HIMBufferedImage& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Copy the HRARaster portion
        HRARaster::operator=(pi_rObj);

        // Link to the new source
        UnlinkFrom(m_pSource);
        m_pSource = pi_rObj.m_pSource;
        LinkTo(m_pSource);

        // Copy attributes
        m_XRatio         = pi_rObj.m_XRatio;
        m_YRatio         = pi_rObj.m_YRatio;
        m_pObjectLog     = pi_rObj.m_pObjectLog;
        m_TileSizeX      = pi_rObj.m_TileSizeX;
        m_TileSizeY      = pi_rObj.m_TileSizeY;
        m_ShapeTheTiles  = pi_rObj.m_ShapeTheTiles;
        m_UseDithering   = pi_rObj.m_UseDithering;
        m_UseAveraging   = pi_rObj.m_UseAveraging;
        m_UseBilinear    = pi_rObj.m_UseBilinear;
        m_UseConvolution = pi_rObj.m_UseConvolution;
        m_MaxSourceResolutionStretchingFactor = pi_rObj.m_MaxSourceResolutionStretchingFactor;

        m_pPhysicalCS = new HGF2DCoordSys(*pi_rObj.m_pPhysicalCS);

        // Keep a copy of the example raster
        m_pExample = static_cast<HRAStoredRaster*>(pi_rObj.m_pExample->Clone(0).GetPtr());

        // Should we copy the tile list? For now, simply clear it.
        HFCMonitor Monitor(m_TileMapKey);
        // mark all tile deleted so, HIMBufferedImageTile will be not call
        // RemoveTile at destruction
        TileMap::iterator Itr(m_TileMap.begin());
        HIMBufferedImageTile* pTile;
        while (Itr != m_TileMap.end())
            {
            pTile = Itr->second;
            Itr++;
            pTile->Deleted();
            pTile->Discard();
            }
        m_TileMap.clear();
        Monitor.ReleaseKey();

        Initialize();
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Create a shaped iterator
//-----------------------------------------------------------------------------
HRARasterIterator* HIMBufferedImage::CreateIterator(const HRAIteratorOptions& pi_rOptions) const
    {
    // Simply advise if they are different. Anyways, we will use the
    // internal value when filling tiles.
    HASSERT(pi_rOptions.MaxResolutionStretchingFactor() == m_MaxSourceResolutionStretchingFactor);

    // Give an iterator on the buffered image
    return new HIMBufferedImageIterator(HFCPtr<HIMBufferedImage>((HIMBufferedImage*)this),
                                        pi_rOptions);
    }


//-----------------------------------------------------------------------------
// Receive a content changed notification
//-----------------------------------------------------------------------------
bool HIMBufferedImage::NotifyContentChanged (const HMGMessage& pi_rMessage)
    {
    // Simply invalidate the changed region
    Invalidate(((HRAContentChangedMsg&)pi_rMessage).GetShape());

    // Propagate message...
    return true;
    }


//-----------------------------------------------------------------------------
// Receive a shape changed notification
//-----------------------------------------------------------------------------
bool HIMBufferedImage::NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage)
    {
    m_pCachedEffectiveShape = 0;

    if (((HRAEffectiveShapeChangedMsg&)pi_rMessage).GetShape().GetExtent() == m_pSource->GetEffectiveShape()->GetExtent())
        {
        // Invalidate all, but don't flush them
        Invalidate(false);
        }
    else
        {
        // Reset buffer. Lose all current tiles.
        Invalidate(true);

        SetCoordSysOfExample();
        Initialize();
        }

    // Propagate message...
    return true;
    }


//-----------------------------------------------------------------------------
// Receive a geometry changed notification
//-----------------------------------------------------------------------------
bool HIMBufferedImage::NotifyGeometryChanged (const HMGMessage& pi_rMessage)
    {
    // Check transformation. If:
    //      - only a translation
    //      - translation X multiple of TileSizeX
    //      - translation Y multiple of TileSizeY
    // then we can re-use the current tiles, but we probably have
    // to renumber them.

    // ELSE

    m_pCachedEffectiveShape = 0;

    // Reset buffer. Lose all current tiles.
    Invalidate(true);
    SetCoordSysOfExample();
    Initialize();

    // Propagate message...
    return true;
    }

//-----------------------------------------------------------------------------
// Notification for palette change
//-----------------------------------------------------------------------------
bool HIMBufferedImage::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    // the palette changed become a content change here    
    const HRARaster* pSender = (const HRARaster*)pi_rMessage.GetSender();

    // Invalidate the changed region
    Invalidate(*(pSender->GetEffectiveShape()));

    // propagate the new message
    Propagate(HRAContentChangedMsg(*(pSender->GetEffectiveShape())));

    // do not propagate the old message
    return false;
    }

//-----------------------------------------------------------------------------
// Set the buffered image's scaling ratios
//-----------------------------------------------------------------------------
void HIMBufferedImage::SetRatios(double pi_XRatio,
                                 double pi_YRatio)
    {
    HPRECONDITION(pi_XRatio > 0.0); // Don't accept null or negative
    HPRECONDITION(pi_YRatio > 0.0); // ratios

    double TempXRatio(pi_XRatio);
    double TempYRatio(pi_YRatio);

    // Only change buffer if we receive a different ratio
    if ((m_XRatio != pi_XRatio) ||
        (m_YRatio != pi_YRatio))
        {
        bool ParametersAreValid = true;

        HVEShape SourceShape(*m_pSource->GetEffectiveShape());
        SourceShape.ChangeCoordSys(GetCoordSys());
        HGF2DExtent SourceExtent(SourceShape.GetExtent());

        // Check that the extent is defined
        // An extent can be undefined in the rare case the
        // source raster is empty (an empty mosaic for example)
        // In such case, the ratio becomes arbitrary
        if (SourceExtent.IsDefined())
            {
            // The source is not empty ...
            // Ratios must be validated

            // Verify that we don't go beyond the UInt64 limits.
            // Source dimensions will be divided by the ratios to get to
            // the physical dimensions.

            // First, we verify that the number of pixels in X and Y don't everflow
            //
            // We must have:  Width / RatioX <= UINT64_MAX
            // so we verify:  Width <= UINT64_MAX * RatioX
            //
            // Same for Y. The previous test is safe only if Ratio < 1.0. Otherwise,
            // since the ratio divides, the result will never overflow.
            //
            if ((pi_XRatio < 1.0 && UINT64_MAX * pi_XRatio < SourceExtent.GetWidth()) ||
                (pi_YRatio < 1.0 && UINT64_MAX * pi_YRatio < SourceExtent.GetWidth()) )
                {
                ParametersAreValid = false;

                HDEBUGTEXT(L"Overflow: The specified ratios makes the BufferedImage too big!");
                HASSERT(0);
                }

            // Then we check that NumberOfTilesX * NumberOfTilesY doesn't overflow
            //
            // Since we must have:  NumTilesX * NumTilesY <= UINT64_MAX
            // we make sure that:   NumTilesX <= UINT64_MAX / NumTilesY
            //
            // NumTilesX = PixelsX / TileSizeX, and PixelsX = Width / RatioX.
            // Same for Y...
            //
            if (ParametersAreValid &&
                ( UINT64_MAX / ((uint64_t)ceil(SourceExtent.GetHeight()) / pi_YRatio / m_TileSizeY) ) <
                ((uint64_t)ceil(SourceExtent.GetWidth()) / pi_XRatio / m_TileSizeX))
                {
                ParametersAreValid = false;

                HDEBUGTEXT(L"Overflow: The specified ratios makes the BufferedImage too big!");
                HASSERT(0);
                }
            }

        if (ParametersAreValid)
            {
            // Copy the Ratio parameters
            m_XRatio = TempXRatio;
            m_YRatio = TempYRatio;

            SetCoordSysOfExample();

            Initialize();
            }
        }
    }


//-----------------------------------------------------------------------------
// Invalidate all
//-----------------------------------------------------------------------------
void HIMBufferedImage::Invalidate(bool pi_DeleteAllTiles)
    {
    HFCMonitor TileMapMonitor(m_TileMapKey);

    if (pi_DeleteAllTiles)
        {
        // mark all tile deleted so, HIMBufferedImageTile will be not call
        // RemoveTile at destruction
        TileMap::iterator Itr(m_TileMap.begin());
        HIMBufferedImageTile* pTile;
        while (Itr != m_TileMap.end())
            {
            pTile = Itr->second;
            Itr++;
            pTile->Deleted();
            pTile->Discard();
            }
        m_TileMap.clear();
        }
    else
        {
        // Invalidate all tiles!

        TileMap::iterator MapItr(m_TileMap.begin());

        // Pass through all entries for the current resolution...
        while (MapItr != m_TileMap.end())
            {
            // Invalidate the tile
            HFCPtr<HIMBufferedImageTile> pTile((*MapItr++).second);

            HFCMonitor TileMonitor(pTile);

            pTile->SetValidState(false);

            TileMonitor.ReleaseKey();
            }
        }
    }


//-----------------------------------------------------------------------------
// Invalidate a portion
//-----------------------------------------------------------------------------
void HIMBufferedImage::Invalidate(const HVEShape& pi_rRegion)
    {
    // Invalidate all tiles that touch the specified region's extent

    HVEShape TempShape(pi_rRegion);
    TempShape.ChangeCoordSys(GetCoordSys());
    HGF2DExtent RegionExtent(TempShape.GetExtent());

    m_TileMapKey.ClaimKey();

    TileMap::iterator MapItr(m_TileMap.begin());

    // Pass through all tile entries
    while (MapItr != m_TileMap.end())
        {
        // Retrieve the tile (Don't check if discarded because
        // we remove the tiles from our map as soon as they are)
        // The passive cast is used to avoid the pool to delete another object
        HFCPtr<HIMBufferedImageTile> pTile((*MapItr).second);

        HFCMonitor TileMonitor(pTile);

        // If the tile is in the specified region
        if (pTile->IsValid() && RegionExtent.DoTheyOverlap(pTile->GetRaster()->GetExtent()))
            {
            // Invalidate the tile
            pTile->SetValidState(false);
            }

        TileMonitor.ReleaseKey();

        MapItr++;
        }

    m_TileMapKey.ReleaseKey();
    }


//-----------------------------------------------------------------------------
// Calculate and set the internal buffer's coordinate systems.
//-----------------------------------------------------------------------------
void HIMBufferedImage::SetCoordSysOfExample()
    {

    HGF2DExtent MyExtent(GetEffectiveShape()->GetExtent());

    // Create scaling transfo
    HGF2DStretch Transfo;

    // Give it translation if necessary
    if (MyExtent.IsDefined())
        Transfo.SetTranslation(HGF2DDisplacement(MyExtent.GetXMin(), MyExtent.GetYMin()));

    // Give it the scaling factors
    Transfo.SetXScaling(m_XRatio);
    Transfo.SetYScaling(m_YRatio);

    m_pPhysicalCS = new HGF2DCoordSys(Transfo, GetCoordSys());

    // Give the example our logical coordinate system
    // Set a new Model to the example
    m_pExample->SetTransfoModel(Transfo, GetCoordSys());

    // create the temporary tile
    m_pTmpTile = HRABitmap::Create(m_TileSizeX,
                               m_TileSizeY,
                               0,
                               m_pPhysicalCS,
                               m_pExample->GetPixelType());
    m_TmpTileXOrigin = 0;
    m_TmpTileYOrigin = 0;
    }


//-----------------------------------------------------------------------------
// Get the buffered image's effective shape
//-----------------------------------------------------------------------------
HFCPtr<HVEShape> HIMBufferedImage::GetEffectiveShape () const
    {
    if (m_pCachedEffectiveShape == 0)
        {
        // Take the source's shape (also ours)
        const_cast<HIMBufferedImage*>(this)->m_pCachedEffectiveShape = new HVEShape(*m_pSource->GetEffectiveShape());

        // Take it in our coordinate system
        const_cast<HIMBufferedImage*>(this)->m_pCachedEffectiveShape->ChangeCoordSys(GetCoordSys());
        }

    // Return the result
    return m_pCachedEffectiveShape;
    }


//-----------------------------------------------------------------------------
// Set our coordinate system
//-----------------------------------------------------------------------------
void HIMBufferedImage::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pOldCoordSys)
    {
    HPRECONDITION(m_pSource != 0);

    // Delegate call to ancestor
    HRARaster::SetCoordSysImplementation(pi_pOldCoordSys);

    m_pCachedEffectiveShape = 0;

    // Reset the coordinate systems of our internal buffer
    SetCoordSysOfExample();

    // We're invalid now! Delete all tile list
    Invalidate(true);

    Initialize();
    }


//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HIMBufferedImage::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    m_pSource->CopyFromLegacy(pi_pSrcRaster, pi_rOptions);
    }

//-----------------------------------------------------------------------------
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HIMBufferedImage::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    m_pSource->CopyFromLegacy(pi_pSrcRaster);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HIMBufferedImage::_CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options)
    {
    return m_pSource->CopyFrom(srcRaster, options);
    }

//-----------------------------------------------------------------------------
// Clear
//-----------------------------------------------------------------------------
void HIMBufferedImage::Clear()
    {
    HRAClearOptions ClearOptions;
    m_pSource->Clear(ClearOptions);
    }


//-----------------------------------------------------------------------------
// Clear
//-----------------------------------------------------------------------------
void HIMBufferedImage::Clear(const HRAClearOptions& pi_rOptions)
    {
    m_pSource->Clear(pi_rOptions);
    }


//-----------------------------------------------------------------------------
// Get the buffered image's pixel size
//-----------------------------------------------------------------------------
HGF2DExtent HIMBufferedImage::GetAveragePixelSize () const
    {
#if (0)
    HGF2DExtent TempPixel(m_pSource->GetAveragePixelSize());
    TempPixel.ChangeCoordSys(GetCoordSys());

    return HGF2DExtent(0.0, 0.0, TempPixel.GetWidth() * m_XRatio, TempPixel.GetHeight() * m_YRatio, GetCoordSys());
#else
    return(HGF2DExtent(0.0, 0.0, 1.0, 1.0, GetPhysicalCoordSys()));
#endif
    }


//-----------------------------------------------------------------------------
// Get the buffered image's pixel size range
//-----------------------------------------------------------------------------
void HIMBufferedImage::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    po_rMinimum = GetAveragePixelSize();

    po_rMaximum = po_rMinimum;
    }


//-----------------------------------------------------------------------------
// Represent buffered image in the specified system
//-----------------------------------------------------------------------------
void HIMBufferedImage::RepresentIn(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Calculate the current difference
    HFCPtr<HGF2DTransfoModel> pDifference(GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys));

    // Extract stretching parameters
    double StretchX;
    double StretchY;
    HGF2DDisplacement Translation;
    pDifference->GetStretchParams(&StretchX,
                                  &StretchY,
                                  &Translation);

    StretchX = fabs(StretchX);
    StretchY = fabs(StretchY);

    // Adjust our scaling difference
    SetRatios(1.0 / StretchX, 1.0 / StretchY);

    // Remove scaling and translation from the difference
    HGF2DStretch ToRemove(Translation, StretchX, StretchY);
    ToRemove.Reverse();
    pDifference = pDifference->ComposeInverseWithDirectOf(ToRemove);

    if (!pDifference->IsIdentity())
        {
        pDifference->Reverse();

        // Now remove the resulting difference from our logical system

        HFCPtr<HGF2DCoordSys> pReference(GetCoordSys()->GetReference());
        HASSERT(pReference != 0);
        HFCPtr<HGF2DTransfoModel> pCurrentModel(GetCoordSys()->GetTransfoModelTo(pReference));

        SetCoordSys(new HGF2DCoordSys(*pDifference->ComposeInverseWithDirectOf(*pCurrentModel),
                                      pReference));
        }
    }


//-----------------------------------------------------------------------------
// Public
// Passed the request to the source
//-----------------------------------------------------------------------------
bool HIMBufferedImage::HasLookAhead() const
    {
    HPRECONDITION(m_pSource != 0);

    return (m_pSource->HasLookAhead());
    }


//-----------------------------------------------------------------------------
// Public
// Pass the request to the source
//-----------------------------------------------------------------------------
void HIMBufferedImage::SetLookAhead(const HVEShape& pi_rShape,
                                    uint32_t        pi_ConsumerID,
                                    bool           pi_Async)
    {
    HPRECONDITION(HasLookAhead());

    // Start with an empty shape
    HVEShape LookAheadRegion(m_pPhysicalCS);
    uint64_t XOrigin;
    uint64_t YOrigin;

    // Pass through all needed IDs
    HIMBufImgTileIDSet* pNeededTiles = GetTileIDPoolFor(pi_rShape);

    HFCMonitor TileMapMonitor(m_TileMapKey);

    HIMBufImgTileIDSet::iterator IDSetIterator = pNeededTiles->begin();
    while (IDSetIterator != pNeededTiles->end())
        {
        HIMBufferedImageTileID SearchID(m_XRatio,
                                        m_YRatio,
                                        *IDSetIterator);

        // Look for entry
        TileMap::const_iterator MapItr(m_TileMap.find(SearchID));
        if (MapItr == m_TileMap.end() || !(*MapItr).second->IsValid())
            {
            GetPositionFromIndex(*IDSetIterator, &XOrigin, &YOrigin);
            CHECK_HUINT64_TO_HDOUBLE_CONV(XOrigin + m_TileSizeX);
            CHECK_HUINT64_TO_HDOUBLE_CONV(YOrigin + m_TileSizeY);
            LookAheadRegion.Unify(HVEShape((double)XOrigin, (double)YOrigin, (double)(XOrigin + m_TileSizeX),
                                           (double)(YOrigin + m_TileSizeY), m_pPhysicalCS));
            }

        ++IDSetIterator;
        }

    delete pNeededTiles;

    // Release the key
    TileMapMonitor.ReleaseKey();

    if (LookAheadRegion.GetExtent().IsDefined())
        {
        // Set the new region to the source
        m_pSource->SetLookAhead(LookAheadRegion, pi_ConsumerID, pi_Async);
        }
    }




//-----------------------------------------------------------------------------
// Move the buffered image
//-----------------------------------------------------------------------------
void HIMBufferedImage::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HPRECONDITION(m_pSource != 0);

    // Create a translation
    HGF2DTranslation NewModel (pi_rDisplacement);

    // Reset our logical coordSys...
    SetCoordSys(new HGF2DCoordSys(*NewModel.ComposeInverseWithDirectOf (
                                      *(GetCoordSys()->GetTransfoModelTo (
                                            m_pSource->GetCoordSys()))),
                                  m_pSource->GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// Rotate the buffered image around a point
//-----------------------------------------------------------------------------
void HIMBufferedImage::Rotate(double               pi_Angle,
                              const HGF2DLocation& pi_rOrigin)
    {
    HPRECONDITION(m_pSource != 0);

    // Create a rotation
    HGF2DSimilitude Rotation;
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Rotation.AddRotation(pi_Angle, LogicalLocation.GetX(), LogicalLocation.GetY());

    // Reset our logical coordSys...
    SetCoordSys(new HGF2DCoordSys(*Rotation.ComposeInverseWithDirectOf (
                                      *(GetCoordSys()->GetTransfoModelTo (
                                            m_pSource->GetCoordSys()))),
                                  m_pSource->GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// Scale the buffered image
//-----------------------------------------------------------------------------
void HIMBufferedImage::Scale(double pi_ScaleFactorX,
                             double pi_ScaleFactorY,
                             const HGF2DLocation& pi_rOrigin)
    {
    HPRECONDITION(m_pSource != 0);

    // Create a scaling model
    HGF2DStretch Scale;
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Scale.AddAnisotropicScaling(pi_ScaleFactorX, pi_ScaleFactorY,
                                LogicalLocation.GetX(), LogicalLocation.GetY());

    // Reset our logical CoordSys
    SetCoordSys(new HGF2DCoordSys(*Scale.ComposeInverseWithDirectOf (
                                      *(GetCoordSys()->GetTransfoModelTo (
                                            m_pSource->GetCoordSys()))),
                                  m_pSource->GetCoordSys()));
    }



//-----------------------------------------------------------------------------
// Get a tile
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster> HIMBufferedImage::GetTile(uint64_t pi_ID) const
    {

    HPRECONDITION(pi_ID < m_NumberOfTilesX * m_NumberOfTilesY);

    HFCPtr<HRARaster> pRaster;

    HIMBufferedImageTileID SearchID(m_XRatio, m_YRatio, pi_ID);

    // Lock the tile map. Don't keep the lock while making operations that
    // may flush another tile...
    HFCMonitor TileMapMonitor(m_TileMapKey);

    // Look for entry
    TileMap::const_iterator MapItr(m_TileMap.find(SearchID));

    if (MapItr != m_TileMap.end())
        {
        HFCPtr<HIMBufferedImageTile> pTile((*MapItr).second);

        // Release the key so that invalidations can take place. At least,
        // we're sure that pTile will be in memory while we need it.
        TileMapMonitor.ReleaseKey();

        pRaster = pTile->GetRaster();

        if (!pTile->IsValid())
            {
            pRaster->Clear();

            HRACopyFromLegacyOptions CopyFromOptions(true);
            if(m_UseDithering)
                {
                // HChk MR
                // Wrap source in a filteredimage with DitherFilter
                }
            if(m_UseAveraging)
                {
                CopyFromOptions.SetResamplingMode(HGSResampling(HGSResampling::AVERAGE));
                //CopyFromOptions.SetMosaicSupersampling(s_Supersampling);
                }
            if (m_UseBilinear)
                {
                CopyFromOptions.SetResamplingMode(HGSResampling(HGSResampling::BILINEAR));
                }
            if(m_UseConvolution)
                {
                CopyFromOptions.SetResamplingMode(HGSResampling(HGSResampling::CUBIC_CONVOLUTION));
                }

            HFCMonitor TileMonitor(pTile);

            pRaster->CopyFromLegacy(m_pSource, CopyFromOptions);    //WIP_LEGACY_COPYFROM

            // Now the tile is valid
            pTile->SetValidState(true);

            TileMonitor.ReleaseKey();
            }

        pTile->NotifyPool();
        }
    else
        {
        // Don't have it. Check if necessary...

        // ReleaseKey the key for now...
        TileMapMonitor.ReleaseKey();

        // Obtain the tile's position
        uint64_t XOrigin;
        uint64_t YOrigin;
        GetPositionFromIndex(pi_ID, &XOrigin, &YOrigin);

        CHECK_HUINT64_TO_HDOUBLE_CONV(XOrigin + m_TileSizeX);
        CHECK_HUINT64_TO_HDOUBLE_CONV(YOrigin + m_TileSizeY);
        HVEShape TileShape((double)XOrigin,
                           (double)YOrigin,
                           (double)(XOrigin + m_TileSizeX),
                           (double)(YOrigin + m_TileSizeY),
                           m_pPhysicalCS);

        // Set the stroking tolerance for the tile's shape
        // Set a quarter of a pixel tolerance
        double CenterX = XOrigin + m_TileSizeX / 2.0;
        double CenterY = YOrigin + m_TileSizeY / 2.0;
        HFCPtr<HGFTolerance> pTol = new HGFTolerance (CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                      CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                      CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                      CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                      m_pPhysicalCS);

        TileShape.SetStrokeTolerance(pTol);
        HVEShape LogicalTileShape;
        if (m_ShapeTheTiles)
            {
            // Calculate the effective shape of the tile
            LogicalTileShape = TileShape;
            LogicalTileShape.Intersect(*GetEffectiveShape());
            }

        if (!m_ShapeTheTiles || !LogicalTileShape.IsEmpty())
            {
            // Create the tile
            pRaster = m_pExample->Clone(0);

            // The RasterModel already have the good dimension
            //
            pRaster->Move(HGF2DDisplacement(XOrigin * m_XRatio, YOrigin * m_YRatio));

            // Clear its background
            pRaster->Clear();

            // Fill it with data
            HRACopyFromLegacyOptions CopyFromOptions(true);
            CopyFromOptions.SetMaxResolutionStretchingFactor(m_MaxSourceResolutionStretchingFactor);
            if(m_UseDithering)
                {
                // HChk MR
                // Wrap source in a filteredimage with DitherFilter
                }
            if(m_UseAveraging)
                {
                CopyFromOptions.SetResamplingMode(HGSResampling(HGSResampling::AVERAGE));
                //CopyFromOptions.SetMosaicSupersampling(s_Supersampling);
                }
            if(m_UseBilinear)
                {
                CopyFromOptions.SetResamplingMode(HGSResampling(HGSResampling::BILINEAR));
                }
            if(m_UseConvolution)
                {
                CopyFromOptions.SetResamplingMode(HGSResampling(HGSResampling::CUBIC_CONVOLUTION));
                }

            // Insert entry in tile map
            HFCPtr<HIMBufferedImageTile> pNewBufferedImageTile(
                new HIMBufferedImageTile(const_cast<HIMBufferedImage*>(this),
                                         SearchID,
                                         (const HFCPtr<HRARaster>&) pRaster,
                                         true));


            TileMapMonitor.ReleaseKey();


            pRaster->CopyFromLegacy(m_pSource, CopyFromOptions);    //WIP_LEGACY_COPYFROM

            // Apply the shape on the tile. Done after the copy because the
            // Previous CopyFrom does not need it. The copy will be made with
            // the source's EffectiveShape, and that's what we apply on the
            // tile anyways. The CopyFrom will be slightly faster this way.
            if (m_ShapeTheTiles)
                pRaster->SetShape(LogicalTileShape);

            pNewBufferedImageTile->SetValidState(true);

            // insert the tile in our map
            TileMapMonitor.Assign(m_TileMapKey);
            m_TileMap.insert(TileMap::value_type(SearchID, pNewBufferedImageTile));
            TileMapMonitor.ReleaseKey();

            // add tile in to pool
            pNewBufferedImageTile->MoveToPool(m_pObjectLog);
            }
        }


    return pRaster;
    }


//-----------------------------------------------------------------------------
// Get the ID set representing a region
//-----------------------------------------------------------------------------
HIMBufImgTileIDSet* HIMBufferedImage::GetTileIDPoolFor(const HVEShape& pi_rShape) const
    {
    HIMBufImgTileIDSet* pTileSet = new HIMBufImgTileIDSet;

    // Intersect specified shape with ours...
    HVEShape TempShape(pi_rShape);
    TempShape.Intersect(*GetEffectiveShape());

    if (!TempShape.IsEmpty())
        {
        // Take shape's extent, in our physical CS
        TempShape.ChangeCoordSys(m_pPhysicalCS);
        HGF2DExtent Extent(TempShape.GetExtent());

        // See HGFTileIDDescriptor for details on the calculations...
        uint64_t XMin = (uint64_t)MAX (0.0, Extent.GetXMin());
        uint64_t YMin = (uint64_t)MAX (0.0, Extent.GetYMin());
        uint64_t XMax = (uint64_t)MIN ((m_NumberOfTilesX * m_TileSizeX) - 1, (uint64_t)(MAX(0, ceil(Extent.GetXMax()))) - 1);
        uint64_t YMax = (uint64_t)MIN ((m_NumberOfTilesY * m_TileSizeY) - 1, (uint64_t)(MAX(0, ceil(Extent.GetYMax()))) - 1);

        if ((XMin < XMax) && (YMin < YMax))
            {
            // There are some tiles to process...
            uint64_t FirstOfCurrentLine = ComputeIndex(XMin, YMin);
            uint64_t LastOfCurrentLine  = ComputeIndex(XMax, YMin);
            uint64_t FirstOfLastLine    = ComputeIndex(XMin, YMax);

            uint64_t CurrentID = FirstOfCurrentLine;

            // pass through all lines
            while (CurrentID <= FirstOfLastLine)
                {
                // Loop to process current line
                while (CurrentID <= LastOfCurrentLine)
                    {
                    // Add current ID, and increment it
                    pTileSet->insert(CurrentID++);
                    }

                // Advance to next line
                FirstOfCurrentLine += m_NumberOfTilesX;
                LastOfCurrentLine  += m_NumberOfTilesX;
                CurrentID           = FirstOfCurrentLine;
                }
            }
        }

    return pTileSet;
    }



//-----------------------------------------------------------------------------
// Remove a tile from our list
//-----------------------------------------------------------------------------
void HIMBufferedImage::RemoveTile(const HIMBufferedImageTileID& pi_rTileID)
    {
    HFCMonitor TileMapMonitor(m_TileMapKey);

    TileMap::iterator MapItr(m_TileMap.find(pi_rTileID));

    if (MapItr != m_TileMap.end())
        m_TileMap.erase(MapItr);        // Remove the specified entry from the map
    }


// These methods must be implemented for the class to compile.
// However, there is no need for these on a BufferedImage.
HRARasterEditor*
HIMBufferedImage::CreateEditor   (HFCAccessMode pi_Mode)
    {
    return 0;
    }
HRARasterEditor*
HIMBufferedImage::CreateEditor   (const HVEShape& pi_rShape,
                                  HFCAccessMode   pi_Mode)
    {
    return 0;
    }
HRARasterEditor* HIMBufferedImage::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// Draw the buffered image
//-----------------------------------------------------------------------------
void HIMBufferedImage::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    HFCPtr<HVEShape> pClipShape;
    const HFCPtr<HGSRegion>& pClipRegion(pio_destSurface.GetRegion());
    if (pClipRegion != 0)
        pClipShape = pClipRegion->GetShape();
    else
        pClipShape = new HVEShape(pio_destSurface.GetExtent());

    // Get list of tiles that we're gonna draw. GetTileIDPoolFor will
    // intersect the requested region with our effective shape...
    HAutoPtr<HIMBufImgTileIDSet> pIDSet(GetTileIDPoolFor(*pClipShape));

    // Pass all tiles
    HIMBufImgTileIDSet::iterator IDSetIterator = pIDSet->begin();
    while (IDSetIterator != pIDSet->end() && HRADrawProgressIndicator::GetInstance()->ContinueIteration())
        {
        try
            {
            const HFCPtr<HRARaster> pTile(GetTile(*IDSetIterator));
            if (pTile != 0)
                {
                pTile->Draw(pio_destSurface, pi_Options);
                }
		}
        catch(HGFmzGCoordException&)
		{
		
		}
        ++IDSetIterator;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HIMBufferedImage::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    BeAssert(false);
    return IMAGEPP_STATUS_NoImplementation;
    // return GetSource()->BuildCopyToContext(imageNode, options);
    }

//-----------------------------------------------------------------------------
// Set the context
//-----------------------------------------------------------------------------
void HIMBufferedImage::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    m_pSource->SetContext(pi_rpContext);
    }

//-----------------------------------------------------------------------------
//Get the current context
//-----------------------------------------------------------------------------
HFCPtr<HMDContext> HIMBufferedImage::GetContext()
    {
    return m_pSource->GetContext();
    }

// ----------------------------------------------------------------------------
// Invalidate raster
//-----------------------------------------------------------------------------
void HIMBufferedImage::InvalidateRaster()
    {
    m_pSource->InvalidateRaster();
    Invalidate(true);
    }

//-----------------------------------------------------------------------------
// Pre-calculate a set of tiles.
//
// NOTE: If the region is big, we may calculate more tiles than our
// pool can hold. This would have the disastrous effect of calculating
// the tiles for nothing...
//-----------------------------------------------------------------------------
void HIMBufferedImage::PrepareRegion(const HVEShape& pi_rRegion)
    {
    // Get list of tiles to prepare. GetTileIDPoolFor will
    // intersect the requested region with our effective shape...
    HIMBufImgTileIDSet* pIDSet = GetTileIDPoolFor(pi_rRegion);

    HIMBufImgTileIDSet::iterator IDSetIterator = pIDSet->begin();
    while (IDSetIterator != pIDSet->end())
        {
        // Prepare the tile, but don't use it here...
        GetTile(*IDSetIterator);

        ++IDSetIterator;
        }

    delete pIDSet;
    }


//-----------------------------------------------------------------------------
// public
// IsStoredRaster
//-----------------------------------------------------------------------------
bool HIMBufferedImage::IsStoredRaster () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// Get the buffer's pixel type
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HIMBufferedImage::GetPixelType() const
    {
    // Ask the example
    return m_pExample->GetPixelType();
    }

//-----------------------------------------------------------------------------
// Tell if the buffer has pixels using the specified channel
//-----------------------------------------------------------------------------
bool HIMBufferedImage::ContainsPixelsWithChannel(
    HRPChannelType::ChannelRole pi_Role,
    Byte                      pi_Id) const
    {
    return m_pExample->ContainsPixelsWithChannel(pi_Role, pi_Id);
    }


//-----------------------------------------------------------------------------
// Return a new copy of self
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HIMBufferedImage::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HIMBufferedImage(*this);
    }

/** ---------------------------------------------------------------------------
    Return a new copy of self
    ---------------------------------------------------------------------------
*/
HPMPersistentObject* HIMBufferedImage::Clone () const
    {
    return new HIMBufferedImage(*this);
    }

