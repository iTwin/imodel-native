//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMBlendCorridor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRARasterIterator.h>

#include <Imagepp/all/h/HIMBlendCorridor.h>
#include <Imagepp/all/h/HIMBlendCorridorIterator.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>

#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>

// All these for CreateCorridor
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HRABitmapEditor.h>
#include <Imagepp/all/h/HMDContext.h>

#include <Imagepp/all/h/HFCGrid.h>

// Tile dimensions. We start at the base dimension, but we try
// to adjust so that we only use one tile in each dimension. We
// will never exceed the maximum tile sizes however.
#define BASE_TILE_SIZE_X     256
#define BASE_TILE_SIZE_Y     256
#define MAX_TILE_SIZE_X      512
#define MAX_TILE_SIZE_Y      512


///////////////////////////////////
// HIMBlendCorridorTileToken class
///////////////////////////////////

HPM_REGISTER_CLASS(HIMBlendCorridorTileToken, HPMPersistentObject)


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridorTileToken::~HIMBlendCorridorTileToken ()
    {
    m_pTile->TokenDeleted();
    }


/** ---------------------------------------------------------------------------
    Return our size (for memory management)
    ---------------------------------------------------------------------------
*/
size_t HIMBlendCorridorTileToken::GetAdditionalSize() const
    {
    // Return the tile's size
    return m_pTile->GetAdditionalSize();
    }

/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridorTileToken::HIMBlendCorridorTileToken ()
    {
    m_pTile = 0;
    }


/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridorTileToken::HIMBlendCorridorTileToken (HIMBlendCorridorTile* pi_pTile)
    {
    HPRECONDITION(pi_pTile != 0);

    m_pTile = pi_pTile;
    }

/////////////////////////////
// HIMBlendCorridorTile
/////////////////////////////

/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridorTile::HIMBlendCorridorTile ()
    {
    HASSERT(!"HIMBlendCorridorTile: _TODO_REPLACE_USAGE_OF_HPMRef");
    m_pBlendCorridor = 0;
    m_Deleting       = false;
    }


/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridorTile::HIMBlendCorridorTile (HIMBlendCorridor*        pi_pCorridor,
                                                   uint64_t                pi_TileID,
                                                   const HFCPtr<HRARaster>& pi_rpRaster)
    : m_pTile(pi_rpRaster)
    {
    HPRECONDITION(pi_pCorridor != 0);
    HPRECONDITION(pi_rpRaster != 0);

    m_pBlendCorridor = pi_pCorridor;
    m_Deleting       = false;
    m_TileID         = pi_TileID;
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridorTile::~HIMBlendCorridorTile ()
    {
    m_Deleting = true;
#ifdef _TODO_REPLACE_USAGE_OF_HPMRef
    m_pToken = 0;
#endif
    }


/** ---------------------------------------------------------------------------
    Get the tile ID
    ---------------------------------------------------------------------------
*/
uint64_t HIMBlendCorridorTile::GetTileID () const
    {
    return m_TileID;
    }


/** ---------------------------------------------------------------------------
    Retrieve the raster (the tile)
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMBlendCorridorTile::GetRaster () const
    {
    // Return the tile
    return m_pTile;
    }


/** ---------------------------------------------------------------------------
    Notify the tile that its token has been deleted, so the tile must die...
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridorTile::TokenDeleted () const
    {
    if (!m_Deleting)
        {
#ifdef _TODO_REPLACE_USAGE_OF_HPMRef
        m_pToken = 0;
#endif

        // Notify the buffered image
        m_pBlendCorridor->RemoveTile(m_TileID);
        }
    }


/** ---------------------------------------------------------------------------
    Log the usage of the tile in the pool.

    @returns true if the tile is already in the pool, false if it has
    previously been removed.
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridorTile::LogUsage () const
    {
#ifdef _TODO_REPLACE_USAGE_OF_HPMRef
    if (m_pToken != 0)
        {
        HASSERT(m_pToken->GetPool() != 0);

        m_pToken->MoveToPool(m_pBlendCorridor->GetTilePool());

        return true;
        }
    else
        {
        m_pToken = (HPMLoader<HIMBlendCorridorTileToken>*)
                   (new HIMBlendCorridorTileToken(const_cast<HIMBlendCorridorTile*>(this)))->GetLoader();
        m_pToken->MoveToPool(m_pBlendCorridor->GetTilePool());

        return false;
        }
#else
    return false;
#endif
    }


/** ---------------------------------------------------------------------------
    Equality test (For STL)
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridorTile::operator==(const HIMBlendCorridorTile& pi_rObj) const
    {
    return m_TileID == pi_rObj.m_TileID;
    }


/** ---------------------------------------------------------------------------
    less-than operator (For STL)
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridorTile::operator<(const HIMBlendCorridorTile& pi_rObj) const
    {
    return m_TileID < pi_rObj.m_TileID;
    }


/** ---------------------------------------------------------------------------
    Return our size (for memory management)
    ---------------------------------------------------------------------------
*/
size_t HIMBlendCorridorTile::GetAdditionalSize() const
    {
    // Return the tile's size
    return m_pTile->GetObjectSize();
    }


///////////////////////////////////
// HIMBlendCorridor class
///////////////////////////////////

HPM_REGISTER_CLASS(HIMBlendCorridor, HRARaster)


/** ---------------------------------------------------------------------------
    Default constructor. For persistence only
    ---------------------------------------------------------------------------
*/
HIMBlendCorridor::HIMBlendCorridor()
    :   HRARaster()
    {
    }


/** ---------------------------------------------------------------------------
    Constructor.
    ---------------------------------------------------------------------------
*/
HIMBlendCorridor::HIMBlendCorridor(const HFCPtr<HGF2DCoordSys>&    pi_rpCoordSys,
                                   const HFCPtr<HRARaster>&        pi_rpSource1,
                                   const HFCPtr<HRARaster>&        pi_rpSource2,
                                   const HFCPtr<HVE2DPolySegment>& pi_rpFeatherLine,
                                   HPMPool*                        pi_pPool,
                                   double                          pi_rCorridorWidth)
    :   HRARaster(pi_rpCoordSys)
    {
    m_pSource1 = pi_rpSource1;
    m_pSource2 = pi_rpSource2;

    HASSERT(pi_pPool != 0);
    m_pPool = pi_pPool;

    m_pFeatherLine = static_cast<HVE2DPolySegment*>(pi_rpFeatherLine->AllocateCopyInCoordSys(pi_rpCoordSys));

    m_CorridorWidth = pi_rCorridorWidth;

    HASSERT(m_pSource1 != 0);
    HASSERT(m_pSource2 != 0);

    // Must have same pixeltype, RGB24 or gray8
    HASSERT(m_pSource1->GetPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID) ||
            m_pSource1->GetPixelType()->IsCompatibleWith(HRPPixelTypeGray::CLASS_ID));
    HASSERT(m_pSource2->GetPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID) ||
            m_pSource2->GetPixelType()->IsCompatibleWith(HRPPixelTypeGray::CLASS_ID));

    // We need the same pixel size
    HGF2DExtent Minimum1;
    HGF2DExtent Minimum2;
    HGF2DExtent Maximum1;
    HGF2DExtent Maximum2;
    m_pSource1->GetPixelSizeRange(Minimum1, Maximum1);
    m_pSource2->GetPixelSizeRange(Minimum2, Maximum2);
    Minimum2.ChangeCoordSys(Minimum1.GetCoordSys());
    Maximum2.ChangeCoordSys(Maximum1.GetCoordSys());
    HASSERT(HDOUBLE_EQUAL_EPSILON(Minimum1.GetWidth(), Minimum2.GetWidth()));
    HASSERT(HDOUBLE_EQUAL_EPSILON(Maximum1.GetWidth(), Maximum2.GetWidth()));
    HASSERT(HDOUBLE_EQUAL_EPSILON(Minimum1.GetHeight(), Minimum2.GetHeight()));
    HASSERT(HDOUBLE_EQUAL_EPSILON(Maximum1.GetHeight(), Maximum2.GetHeight()));

    PrecomputeBlendCorridorParameters();
    }


/** ---------------------------------------------------------------------------
    Copy constructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridor::HIMBlendCorridor(const HIMBlendCorridor& pi_rObj)
    :   HRARaster(pi_rObj)
    {
    m_pSource1 = pi_rObj.m_pSource1;
    m_pSource2 = pi_rObj.m_pSource2;
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
HIMBlendCorridor::~HIMBlendCorridor()
    {
    delete[] m_pTiles;

    delete m_pTileDescriptor;
    }


/** ---------------------------------------------------------------------------
    Return a copy of self.
    ---------------------------------------------------------------------------
*/
HRARaster* HIMBlendCorridor::Clone (HPMObjectStore* pi_pStore,
                                    HPMPool*        pi_pLog) const
    {
    return new HIMBlendCorridor(*this);
    }
/** ---------------------------------------------------------------------------
    Return a copy of self.
    ---------------------------------------------------------------------------
*/
HPMPersistentObject* HIMBlendCorridor::Clone () const
    {
    return new HIMBlendCorridor(*this);
    }


/** ---------------------------------------------------------------------------
    Create a new iterator.
    ---------------------------------------------------------------------------
*/
HRARasterIterator* HIMBlendCorridor::CreateIterator (const HRAIteratorOptions& pi_rOptions) const
    {
    return new HIMBlendCorridorIterator(HFCPtr<HIMBlendCorridor>((HIMBlendCorridor*)this), pi_rOptions);
    }


/** ---------------------------------------------------------------------------
    Notification for content changed
    ---------------------------------------------------------------------------
*/
/*bool HIMBlendCorridor::NotifyContentChanged(HMGMessage& pi_rMessage)
{
    // create a new shape
    HVEShape Shape((((HRAContentChangedMsg&)pi_rMessage).GetShape()).GetExtent());

    // create a new msg with that shape
    HRAContentChangedMsg Msg(Shape);

    // call the parent method with the new msg
    HRAImageView::NotifyContentChanged(Msg);

    // notify the raster that the content has changed
    Propagate(HRAContentChangedMsg(Msg));

    // do not propagate anymore the msg with the old shape
    return false;

    return true;
}
*/


/** ---------------------------------------------------------------------------
    CopyFromLegacy. Do it on the sources
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster,
                                const HRACopyFromLegacyOptions& pi_rOptions)
    {
    }


/** ---------------------------------------------------------------------------
    CopyFromLegacy
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }


/** ---------------------------------------------------------------------------
Clear
---------------------------------------------------------------------------
*/
void HIMBlendCorridor::Clear()
    {
    HASSERT(false);
    }

/** ---------------------------------------------------------------------------
    Clear
---------------------------------------------------------------------------
*/
void HIMBlendCorridor::Clear(const HRAClearOptions& pi_rOptions)
    {
    HASSERT(false);
    }


/** ---------------------------------------------------------------------------
    The corridor has a single pixel type.
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridor::HasSinglePixelType() const
    {
    return true;
    }


/** ---------------------------------------------------------------------------
    Get the pixel type
    ---------------------------------------------------------------------------
*/
HFCPtr<HRPPixelType> HIMBlendCorridor::GetPixelType() const
    {
    return m_pSource1->GetPixelType();
    }


/** ---------------------------------------------------------------------------
    Return the average pixel size
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMBlendCorridor::GetAveragePixelSize () const
    {
    return m_pSource1->GetAveragePixelSize();
    }


/** ---------------------------------------------------------------------------
    Return the pixel size range
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    m_pSource1->GetPixelSizeRange(po_rMinimum, po_rMaximum);
    }


/** ---------------------------------------------------------------------------
    Return the effective shape of the corridor
    ---------------------------------------------------------------------------
*/
HFCPtr<HVEShape> HIMBlendCorridor::GetEffectiveShape () const
    {
    return m_pBlendCorridorShape;
    }


/** ---------------------------------------------------------------------------
    Get the extent
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMBlendCorridor::GetExtent() const
    {
    return GetEffectiveShape()->GetExtent();
    }


/** ---------------------------------------------------------------------------
    Tell if the corridor contains pixels of specified channel
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridor::ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                                  Byte                      pi_Id) const
    {
    return m_pSource1->ContainsPixelsWithChannel(pi_Role, pi_Id) ||
           m_pSource2->ContainsPixelsWithChannel(pi_Role, pi_Id);
    }


/** ---------------------------------------------------------------------------
    Create an editor. Unsupported.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMBlendCorridor::CreateEditor(HFCAccessMode pi_Mode)
    {
    return 0;
    }


/** ---------------------------------------------------------------------------
    Create a shaped editor. Unsupported.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMBlendCorridor::CreateEditor(const HVEShape&    pi_rShape,
                                                HFCAccessMode      pi_Mode)
    {
    return 0;
    }


/** ---------------------------------------------------------------------------
    Create an unshaped editor. Unsupported.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMBlendCorridor::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return 0;
    }


/** ---------------------------------------------------------------------------
    Check if one of the sources supports the lookahead mechanism
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridor::HasLookAhead() const
    {
    return m_pSource1->HasLookAhead() ||
           m_pSource2->HasLookAhead();
    }


/** ---------------------------------------------------------------------------
    Give the lookahead request to the sources
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::SetLookAhead(const HVEShape& pi_rShape,
                                    uint32_t        pi_ConsumerID,
                                    bool           pi_Async)
    {
    HVEShape UsefulShape(pi_rShape);
    UsefulShape.Intersect(*GetEffectiveShape());

    if (m_pSource1->HasLookAhead())
        m_pSource1->SetLookAhead(UsefulShape, pi_ConsumerID, pi_Async);

    if (m_pSource2->HasLookAhead())
        m_pSource2->SetLookAhead(UsefulShape, pi_ConsumerID, pi_Async);
    }

/** ---------------------------------------------------------------------------
    Move the source images
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    MoveOneImage(m_pSource1, pi_rDisplacement);
    MoveOneImage(m_pSource2, pi_rDisplacement);
    }


/** ---------------------------------------------------------------------------
    Move one image
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::MoveOneImage(HFCPtr<HRARaster>& pi_rpRaster,
                                    const HGF2DDisplacement& pi_rDisplacement)
    {
    HFCPtr<HGF2DCoordSys> pImageCS = pi_rpRaster->GetCoordSys();
    HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);

    if (pBaseToImage->PreservesParallelism())
        {
        HGF2DLocation Translation(pi_rDisplacement.GetDeltaX(), pi_rDisplacement.GetDeltaY(), GetCoordSys());
        HGF2DLocation Origin(0.0, 0.0, GetCoordSys());
        Translation.ChangeCoordSys(pImageCS);
        Origin.ChangeCoordSys(pImageCS);

        // and move.
        pi_rpRaster->Move(HGF2DDisplacement(Translation.GetX() - Origin.GetX(),
                                            Translation.GetY() - Origin.GetY()));
        }
    else
        {
        HFCPtr<HGF2DTranslation> pTranslation(new HGF2DTranslation(pi_rDisplacement));

        HFCPtr<HGF2DTransfoModel> pTranslationForImage = pBaseToImage->ComposeInverseWithDirectOf(*pTranslation);
        pTranslationForImage = pTranslationForImage->ComposeInverseWithInverseOf(*pBaseToImage);
        HFCPtr<HGF2DTransfoModel> pSimplifiedTranslationForImage(pTranslationForImage->CreateSimplifiedModel());

        pi_rpRaster->SetCoordSys(new HGF2DCoordSys(pSimplifiedTranslationForImage != 0 ?
                                                   *pSimplifiedTranslationForImage :                                                                         *pTranslationForImage,
                                                   pImageCS));
        }
    }


/** ---------------------------------------------------------------------------
    Rotate the source images
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::Rotate(double               pi_Angle,
                              const HGF2DLocation& pi_rOrigin)
    {
    RotateOneImage(m_pSource1, pi_Angle, pi_rOrigin);
    RotateOneImage(m_pSource2, pi_Angle, pi_rOrigin);
    }


/** ---------------------------------------------------------------------------
    Rotate one image
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::RotateOneImage(HFCPtr<HRARaster>&   pi_rpRaster,
                                      double               pi_Angle,
                                      const HGF2DLocation& pi_rOrigin)
    {
    // Create basic rotation transformation (based on our CS)
    HFCPtr<HGF2DSimilitude> pRotation(new HGF2DSimilitude());
    pRotation->AddRotation(pi_Angle, pi_rOrigin.GetX(), pi_rOrigin.GetY());

    // Take the rotation and base it on the image
    HFCPtr<HGF2DCoordSys> pImageCS = pi_rpRaster->GetCoordSys();
    HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);
    HFCPtr<HGF2DTransfoModel> pRotationForImage = pBaseToImage->ComposeInverseWithDirectOf(*pRotation);
    pRotationForImage = pRotationForImage->ComposeInverseWithInverseOf(*pBaseToImage);
    HFCPtr<HGF2DTransfoModel> pSimplifiedRotationForImage(pRotationForImage->CreateSimplifiedModel());

    // Apply the rotation
    pi_rpRaster->SetCoordSys(new HGF2DCoordSys(pSimplifiedRotationForImage != 0 ?
                                               *pSimplifiedRotationForImage :
                                               *pRotationForImage,
                                               pImageCS));
    }


/** ---------------------------------------------------------------------------
    Scale the source images
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::Scale(double              pi_ScaleFactorX,
                             double              pi_ScaleFactorY,
                             const HGF2DLocation& pi_rOrigin)
    {
    ScaleOneImage(m_pSource1, pi_ScaleFactorX, pi_ScaleFactorY, pi_rOrigin);
    ScaleOneImage(m_pSource2, pi_ScaleFactorX, pi_ScaleFactorY, pi_rOrigin);
    }


/** ---------------------------------------------------------------------------
    Scale one image
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::ScaleOneImage(HFCPtr<HRARaster>&   pi_rpRaster,
                                     double              pi_ScaleFactorX,
                                     double              pi_ScaleFactorY,
                                     const HGF2DLocation& pi_rOrigin)
    {
    // Create stretch transformation based on mosaic's CS
    HFCPtr<HGF2DStretch> pScale(new HGF2DStretch());
    pScale->AddAnisotropicScaling(pi_ScaleFactorX, pi_ScaleFactorY,
                                  pi_rOrigin.GetX(), pi_rOrigin.GetY());

    HFCPtr<HGF2DCoordSys> pImageCS = pi_rpRaster->GetCoordSys();
    HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);

    if (pBaseToImage->PreservesDirection())
        {
        // Delegate scale to the current source
        pi_rpRaster->Scale(pi_ScaleFactorX, pi_ScaleFactorY, pi_rOrigin);
        }
    else
        {
        // Base the scaling on the current image and apply

        HFCPtr<HGF2DTransfoModel> pScaleForImage = pBaseToImage->ComposeInverseWithDirectOf(*pScale);
        pScaleForImage = pScaleForImage->ComposeInverseWithInverseOf(*pBaseToImage);
        HFCPtr<HGF2DTransfoModel> pSimplifiedScaleForImage(pScaleForImage->CreateSimplifiedModel());

        pi_rpRaster->SetCoordSys(new HGF2DCoordSys(pSimplifiedScaleForImage != 0 ?
                                                   *pSimplifiedScaleForImage :
                                                   *pScaleForImage,
                                                   pImageCS));
        }
    }


/** ---------------------------------------------------------------------------
    Calculate all possible variables before a tile request.
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::PrecomputeBlendCorridorParameters()
    {
    //
    // Compute the corridor limits
    //

    double CorridorHalfWidth = m_CorridorWidth / 2.0;

    // Obtain parallel copies
    // Obtain right copy
    HFCPtr<HVE2DPolySegment> pRightSide =
        m_pFeatherLine->AllocateParallelCopy(CorridorHalfWidth, HVE2DVector::ALPHA);
    HFCPtr<HVE2DPolySegment> pRightSideFarther =
        m_pFeatherLine->AllocateParallelCopy(CorridorHalfWidth * 1.2, HVE2DVector::ALPHA);

    // Obtain left copy
    HFCPtr<HVE2DPolySegment> pLeftSide =
        m_pFeatherLine->AllocateParallelCopy(CorridorHalfWidth, HVE2DVector::BETA);
    HFCPtr<HVE2DPolySegment> pLeftSideFarther =
        m_pFeatherLine->AllocateParallelCopy(CorridorHalfWidth * 1.2, HVE2DVector::BETA);


    //
    // Create a shape for the feather region
    //

    // Reverse one of them
    pLeftSide->Reverse();

    // Build a closed polysegment
    HFCPtr<HVE2DPolySegment> pClosedPolySegment = new HVE2DPolySegment(*pRightSide);

    uint32_t i;
    for (i = 0 ; i < pLeftSide->GetSize() ;  ++i)
        {
        pClosedPolySegment->AppendPoint(pLeftSide->GetPoint(i));
        }

    // Close it
    pClosedPolySegment->AppendPoint(pClosedPolySegment->GetStartPoint());

    // Create feathering shape
    m_pBlendCorridorShape = new HVEShape(HVE2DPolygonOfSegments(*pClosedPolySegment));


    //
    // Determine which raster is on which side
    //

    HGF2DExtent Raster1Extent(m_pSource1->GetEffectiveShape()->GetExtent());
    HGF2DLocation Raster1Center = Raster1Extent.GetOrigin() + (HGF2DDisplacement(Raster1Extent.GetWidth() / 2.0,
                                                                                 Raster1Extent.GetHeight() / 2.0));
    HGF2DExtent Raster2Extent(m_pSource2->GetEffectiveShape()->GetExtent());
    HGF2DLocation Raster2Center = Raster2Extent.GetOrigin() + (HGF2DDisplacement(Raster2Extent.GetWidth() / 2.0,
                                                                                 Raster2Extent.GetHeight() / 2.0));

    HFCPtr<HVEShape> pBlendCorridorExtentShape = new HVEShape(m_pBlendCorridorShape->GetExtent());

    // Calculate distance between points
    HGF2DLocation ClosestPointOnLeftSide  = pLeftSide->CalculateClosestPoint(Raster1Center);
    HGF2DLocation ClosestPointOnRightSide = pRightSide->CalculateClosestPoint(Raster1Center);
    double TheDistanceFromLeftSide = (Raster1Center-ClosestPointOnLeftSide).CalculateLength();
    double TheDistanceFromRightSide = (Raster1Center-ClosestPointOnRightSide).CalculateLength();

    HGF2DLocation ClosestPointToRaster2 = pBlendCorridorExtentShape->GetShapePtr()->CalculateClosestPoint(Raster2Center);

    if (TheDistanceFromLeftSide < TheDistanceFromRightSide)
        {
        // The left side goes with raster 1
        m_pRaster1Border = pLeftSide;
        m_pRaster2Border = pRightSide;
        m_pRaster2FartherBorder = pRightSideFarther;
        }
    else
        {
        // The right side goes with raster 1
        m_pRaster1Border = pRightSide;
        m_pRaster2Border = pLeftSide;
        m_pRaster2FartherBorder = pLeftSideFarther;
        }


    //
    // Obtain the physical CS to use
    //

    // Extract a physical CS by obtaining a physical bitmap from the source.
    HRARasterIterator* pIterator = m_pSource1->CreateIterator(HRAIteratorOptions());
    HASSERT(pIterator != 0);

    HFCPtr<HGF2DCoordSys> pSourcePhysicalCS;

    HFCPtr<HRARaster> pRaster((*pIterator)());
    if (pRaster != 0)
        {
        if (pRaster->IsCompatibleWith(HRAReferenceToRaster::CLASS_ID))
            pSourcePhysicalCS = ((HFCPtr<HRABitmap>&)((HFCPtr<HRAReferenceToRaster>&)pRaster)->GetSource())->GetPhysicalCoordSys();
        else
            pSourcePhysicalCS = ((HFCPtr<HRABitmap>&)pRaster)->GetPhysicalCoordSys();
        }
    pRaster = 0;
    delete pIterator;
    HASSERT(pSourcePhysicalCS != 0);

    // Compute the grid of physical pixels to blend
    HVEShape CorridorShape(*m_pBlendCorridorShape);
    CorridorShape.ChangeCoordSys(pSourcePhysicalCS);
    HGF2DExtent CorridorExtent(CorridorShape.GetExtent());
    HFCGrid CorridorGrid(CorridorExtent.GetXMin(),
                         CorridorExtent.GetYMin(),
                         CorridorExtent.GetXMax(),
                         CorridorExtent.GetYMax());

    // Create our own physical system that is aligned on
    // the source CS pixels, and that starts at 0,0
    // where we want to blend.
    CHECK_HSINT64_TO_HDOUBLE_CONV(CorridorGrid.GetXMin());
    CHECK_HSINT64_TO_HDOUBLE_CONV(CorridorGrid.GetYMin());

    HGF2DTranslation Alignment(HGF2DDisplacement((double)CorridorGrid.GetXMin(), (double)CorridorGrid.GetYMin()));

    m_pPhysicalCS = new HGF2DCoordSys(Alignment, pSourcePhysicalCS);
    HASSERT(m_pPhysicalCS != 0);


    //
    // Create the tile descriptor
    //
    uint32_t TileSizeX = BASE_TILE_SIZE_X;
    uint32_t TileSizeY = BASE_TILE_SIZE_Y;

    if (CorridorGrid.GetWidth() <= MAX_TILE_SIZE_X)
        {
        HASSERT(CorridorGrid.GetWidth() <= ULONG_MAX);
        TileSizeX = (uint32_t)CorridorGrid.GetWidth();
        }

    if (CorridorGrid.GetHeight() <= MAX_TILE_SIZE_Y)
        {
        HASSERT(CorridorGrid.GetHeight() <= ULONG_MAX);
        TileSizeY = (uint32_t)CorridorGrid.GetHeight();
        }

    m_pTileDescriptor = new HGFTileIDDescriptor(CorridorGrid.GetWidth(),
                                                CorridorGrid.GetHeight(),
                                                TileSizeX,
                                                TileSizeY);
    HASSERT(m_pTileDescriptor != 0);


    //
    // Choose a pixel type.
    //

    // Keep grayscale if the two input images share
    // the same grayscale pixeltype. Otherwise, we use RGB.
    HFCPtr<HRPPixelType> pCorridorPixelType;
    if ((m_pSource1->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) &&
         m_pSource2->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID)) ||
        (m_pSource1->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8GrayWhite8::CLASS_ID) &&
         m_pSource2->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8GrayWhite8::CLASS_ID)))
        {
        pCorridorPixelType =  m_pSource1->GetPixelType();
        }
    else
        {
        pCorridorPixelType =  new HRPPixelTypeV24R8G8B8();
        }
    HASSERT(pCorridorPixelType != 0);


    //
    // Allocate the base tile
    //
    HASSERT(m_pTileDescriptor->GetTileWidth() <= ULONG_MAX);
    HASSERT(m_pTileDescriptor->GetTileHeight() <= ULONG_MAX);

    m_pBaseTile = new HRABitmap((uint32_t)m_pTileDescriptor->GetTileWidth(),
                                (uint32_t)m_pTileDescriptor->GetTileHeight(),
                                0,
                                m_pPhysicalCS,
                                pCorridorPixelType);
    HASSERT(m_pBaseTile != 0);

    // Allocate the array of tiles. All positions are empty by default
    HPRECONDITION(m_pTileDescriptor->GetTileCount() <= SIZE_MAX);
    m_pTiles = new HFCPtr<HIMBlendCorridorTile>[(size_t)m_pTileDescriptor->GetTileCount()];
    HASSERT(m_pTiles != 0);
    }


/** ---------------------------------------------------------------------------
    Create the a corridor tile
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMBlendCorridor::CreateTile(uint64_t pi_Index) const
    {
    HASSERT(pi_Index < m_pTileDescriptor->GetTileCount());
    HASSERT(!m_pBlendCorridorShape->IsEmpty());

    //
    // Allocate and position the tile
    //

    HFCPtr<HRABitmap> pResult( (HRABitmap*) m_pBaseTile->Clone() );
    HASSERT(pResult != 0);
    Result->InitSize(m_pTileDescriptor->GetTileWidth(),m_pTileDescriptor->GetTileHeight());

    uint64_t TilePosX;
    uint64_t TilePosY;
    m_pTileDescriptor->GetPositionFromIndex(pi_Index, &TilePosX, &TilePosY);

    CHECK_HUINT64_TO_HDOUBLE_CONV(TilePosX)
    CHECK_HUINT64_TO_HDOUBLE_CONV(TilePosY)

    pResult->Move(HGF2DDisplacement((double)TilePosX, (double)TilePosY));

    //
    // Pre-fill m_pCorridor with other images
    //

    HRACopyFromOptions NoOptions;


    //
    // Create bitmaps for raster1 and raster2. We will use these to read
    // the pixels inside the feathering corridor directly using editors.
    //

    HFCPtr<HRABitmap> pRaster1Bitmap((HRABitmap*) pResult->Clone());
    HFCPtr<HRABitmap> pRaster2Bitmap((HRABitmap*) pResult->Clone());

    pRaster1Bitmap->CopyFrom(m_pSource1, NoOptions);
    pRaster2Bitmap->CopyFrom(m_pSource2, NoOptions);


    //
    // Fill the corridor with the blend of raster1 and raster2
    //

    HUINTX XPhysical;
    HUINTX YPhysical;

    Byte* pCurrentFeatherPixel;
    Byte* pCurrentRaster1Pixel;
    Byte* pCurrentRaster2Pixel;

    size_t NumPixels;
    size_t BytesPerPixel = pResult->GetPixelType()->CountPixelRawDataBits() / 8;

    // Now we process only the corridor
    HVEShape MyMaskCorridorShape(*m_pBlendCorridorShape);
    MyMaskCorridorShape.Intersect(*(pResult->GetEffectiveShape()));

    if (!MyMaskCorridorShape.IsEmpty())
        {
        MyMaskCorridorShape.ChangeCoordSys(pResult->GetPhysicalCoordSys());
        HRARasterEditor* pFeatheredRasterEditor = pResult->CreateEditor(MyMaskCorridorShape, HFC_READ_WRITE);
        HRARasterEditor* pRaster1RasterEditor   = pRaster1Bitmap->CreateEditor(MyMaskCorridorShape, HFC_READ_WRITE);
        HRARasterEditor* pRaster2RasterEditor   = pRaster2Bitmap->CreateEditor(MyMaskCorridorShape, HFC_READ_WRITE);

        // Make sure the editor is on a bitmap
        HASSERT(pFeatheredRasterEditor->IsCompatibleWith(HRABitmapEditor::CLASS_ID));
        HASSERT(pRaster1RasterEditor->IsCompatibleWith(HRABitmapEditor::CLASS_ID));
        HASSERT(pRaster2RasterEditor->IsCompatibleWith(HRABitmapEditor::CLASS_ID));

        HRAEditor* pFeatheredEditor = static_cast<HRABitmapEditor*>(pFeatheredRasterEditor)->GetSurfaceEditor();
        HRAEditor* pRaster1Editor   = static_cast<HRABitmapEditor*>(pRaster1RasterEditor)->GetSurfaceEditor();
        HRAEditor* pRaster2Editor   = static_cast<HRABitmapEditor*>(pRaster2RasterEditor)->GetSurfaceEditor();

        if (pFeatheredEditor->GetFirstRun(&XPhysical, &YPhysical, &NumPixels))
            {
            double TheWeight;
            double TheWeightComplement;
            const double WeightFactor = 1.0 / (1.1 * m_CorridorWidth);

            if (BytesPerPixel == 3)
                {
                do
                    {
                    // Only process valid lines
                    if (YPhysical < m_pTileDescriptor->GetImageHeight())
                        {
                        HASSERT(m_pTileDescriptor->GetImageWidth() <= ULONG_MAX);
                        NumPixels = MIN(NumPixels, (uint32_t)m_pTileDescriptor->GetImageWidth() - XPhysical);

                        // Obtain pointer to first pixel to process
                        pCurrentFeatherPixel = (Byte*) pFeatheredEditor->GetPixel(XPhysical, YPhysical);
                        pCurrentRaster1Pixel = (Byte*) pRaster1Editor->GetPixel(XPhysical, YPhysical);
                        pCurrentRaster2Pixel = (Byte*) pRaster2Editor->GetPixel(XPhysical, YPhysical);

                        while (NumPixels)
                            {
                            HGF2DLocation ThePoint(XPhysical + 0.5, YPhysical + 0.5, pResult->GetPhysicalCoordSys());

                            ThePoint.ChangeCoordSys(m_pFeatherLine->GetCoordSys());

                            HGF2DLocation ClosestPoint = m_pRaster2FartherBorder->CalculateClosestPoint(ThePoint);

                            // Calculate distance between points
                            double TheDistance = (ThePoint-ClosestPoint).CalculateLength();

                            TheWeight = MAX(MIN(TheDistance * WeightFactor, 1.0), 0.0);

                            TheWeightComplement = 1.0 - TheWeight;

                            pCurrentFeatherPixel[0] = (Byte)((pCurrentRaster2Pixel[0] * TheWeightComplement) + (pCurrentRaster1Pixel[0] * TheWeight));
                            pCurrentFeatherPixel[1] = (Byte)((pCurrentRaster2Pixel[1] * TheWeightComplement) + (pCurrentRaster1Pixel[1] * TheWeight));
                            pCurrentFeatherPixel[2] = (Byte)((pCurrentRaster2Pixel[2] * TheWeightComplement) + (pCurrentRaster1Pixel[2] * TheWeight));

                            pCurrentFeatherPixel += 3;
                            pCurrentRaster1Pixel += 3;
                            pCurrentRaster2Pixel += 3;

                            ++XPhysical;
                            --NumPixels;
                            }
                        }

                    }
                while (pFeatheredEditor->GetNextRun(&XPhysical, &YPhysical, &NumPixels));
                }
            else
                {
                HASSERT(BytesPerPixel == 1);

                do
                    {
                    // Only process valid lines
                    if (YPhysical < m_pTileDescriptor->GetImageHeight())
                        {
                        HASSERT(m_pTileDescriptor->GetImageWidth() <= ULONG_MAX);
                        NumPixels = MIN(NumPixels, (uint32_t)m_pTileDescriptor->GetImageWidth() - XPhysical);

                        // Obtain pointer to first pixel to process
                        pCurrentFeatherPixel = (Byte*) pFeatheredEditor->GetPixel(XPhysical, YPhysical);
                        pCurrentRaster1Pixel = (Byte*) pRaster1Editor->GetPixel(XPhysical, YPhysical);
                        pCurrentRaster2Pixel = (Byte*) pRaster2Editor->GetPixel(XPhysical, YPhysical);

                        while (NumPixels)
                            {
                            HGF2DLocation ThePoint(XPhysical + 0.5, YPhysical + 0.5, pResult->GetPhysicalCoordSys());

                            ThePoint.ChangeCoordSys(m_pFeatherLine->GetCoordSys());

                            HGF2DLocation ClosestPoint = m_pRaster2FartherBorder->CalculateClosestPoint(ThePoint);

                            // Calculate distance between points
                            double TheDistance = (ThePoint-ClosestPoint).CalculateLength();

                            TheWeight = MAX(MIN(TheDistance * WeightFactor, 1.0), 0.0);

                            TheWeightComplement = 1.0 - TheWeight;

                            *pCurrentFeatherPixel = (Byte)((*pCurrentRaster2Pixel * TheWeightComplement) + (*pCurrentRaster1Pixel * TheWeight));

                            ++pCurrentFeatherPixel;
                            ++pCurrentRaster1Pixel;
                            ++pCurrentRaster2Pixel;

                            ++XPhysical;
                            --NumPixels;
                            }
                        }

                    }
                while (pFeatheredEditor->GetNextRun(&XPhysical, &YPhysical, &NumPixels));
                }
            }

        pFeatheredEditor = 0;
        pRaster1Editor = 0;
        pRaster2Editor = 0;
        delete pFeatheredRasterEditor;
        delete pRaster1RasterEditor;
        delete pRaster2RasterEditor;
        }

    // Clip the corridor bitmap ot only display the corridor pixels
    pResult->SetShape(MyMaskCorridorShape);

    return pResult;
    }


/** ---------------------------------------------------------------------------
    Tell if the current instance if working with the specified images
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridor::Blends(const HFCPtr<HRARaster>& pi_rpImage1,
                               const HFCPtr<HRARaster>& pi_rpImage2) const
    {
    return (pi_rpImage1 == m_pSource1 && pi_rpImage2 == m_pSource2) ||
           (pi_rpImage1 == m_pSource2 && pi_rpImage2 == m_pSource1);
    }


/** ---------------------------------------------------------------------------
    Tell if specified source is used in this blend corridor
    ---------------------------------------------------------------------------
*/
bool HIMBlendCorridor::Uses(const HFCPtr<HRARaster>& pi_rpImage) const
    {
    return (pi_rpImage == m_pSource1 || pi_rpImage == m_pSource2);
    }


/** ---------------------------------------------------------------------------
    Remove a tile from our list because it has been flushed
    by the pool.
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::RemoveTile(uint64_t pi_TileID)
    {
    HASSERT(pi_TileID < m_pTileDescriptor->GetTileCount());

    HFCMonitor TileArrayMonitor(m_TilesKey);

    m_pTiles[pi_TileID] = 0;
    }


/** ---------------------------------------------------------------------------
    Retrieve a tile
    ---------------------------------------------------------------------------
*/
const HFCPtr<HRARaster> HIMBlendCorridor::GetTile(uint64_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_pTileDescriptor->GetTileCount());

    HFCPtr<HRARaster> pResult;

    HFCPtr<HIMBlendCorridorTile> pTile;
        {
        HFCMonitor TileArrayMonitor(m_TilesKey);

        pTile = m_pTiles[pi_Index];
        }

    if (pTile == 0)
        {
        // We must create the tile

        pResult = CreateTile(pi_Index);

        pTile = new HIMBlendCorridorTile(const_cast<HIMBlendCorridor*>(this), pi_Index, pResult);

        // Place the tile in the array
            {
            HFCMonitor TileArrayMonitor(m_TilesKey);

            m_pTiles[pi_Index] = pTile;
            }

        pTile->LogUsage();
        }
    else
        {
        // Reuse what's there.

        pResult = pTile->GetRaster();

        if (!pTile->LogUsage())
            {
            // The tile has been removed from our array. Put it back.
            HFCMonitor TileArrayMonitor(m_TilesKey);

            m_pTiles[pi_Index] = pTile;
            }
        }

    HASSERT(pResult != 0);

    return pResult;
    }


/** ---------------------------------------------------------------------------
    Set the width of the blend corridor, in the blend's logical
    coordinate system. This will invalidate all previously computed tiles.
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::SetCorridorWidth(double pi_CorridorWidth)
    {
    HASSERT(pi_CorridorWidth > 0.0);

    m_CorridorWidth = pi_CorridorWidth;

    // We don't keep old tiles, they may use a bad width. We must
    // initialize like a new construction.

    delete[] m_pTiles;
    delete m_pTileDescriptor;

    PrecomputeBlendCorridorParameters();
    }


/** ---------------------------------------------------------------------------
    Flush all the computed tiles.
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::Invalidate()
    {
    uint64_t TileCount = m_pTileDescriptor->GetTileCount();
    for (uint32_t Tile = 0 ; Tile < TileCount; ++Tile)
        {
        m_pTiles[Tile] = 0;
        }
    }


/** ---------------------------------------------------------------------------
    Flush the precomputed tiles that touch the specified region.
    ---------------------------------------------------------------------------
*/
void HIMBlendCorridor::Invalidate(HFCPtr<HVEShape>& pi_rpShape)
    {
    HFCPtr<HVEShape> pPhysicalShape(new HVEShape(*pi_rpShape));
    pPhysicalShape->ChangeCoordSys(m_pPhysicalCS);

    uint64_t TileCount = m_pTileDescriptor->GetTileCount();

    HVETileIDIterator TileIterator(m_pTileDescriptor, pPhysicalShape);
    for (uint64_t Tile = TileIterator.GetFirstTileIndex() ;
         Tile < TileCount ;
         Tile = TileIterator.GetNextTileIndex())
        {
        m_pTiles[Tile] = 0;
        }
    }

/** -----------------------------------------------------------------------------
    Set the volatile layer info
    -----------------------------------------------------------------------------
*/
void HIMBlendCorridor::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    HASSERT(0); //TBD
    }

/** -----------------------------------------------------------------------------
    Get the current context
    -----------------------------------------------------------------------------
*/
HFCPtr<HMDContext> HIMBlendCorridor::GetContext()
    {
    HASSERT(0); //TBD
    return 0;
    }

/** -----------------------------------------------------------------------------
    Invalidate the raster
    -----------------------------------------------------------------------------
*/
void HIMBlendCorridor::InvalidateRaster()
    {
    HASSERT(0); //TBD
    }

//-----------------------------------------------------------------------------
// public
// IsStoredRaster
//-----------------------------------------------------------------------------
bool HIMBlendCorridor::IsStoredRaster () const
    {
    return (true);
    }

/** ---------------------------------------------------------------------------
    Retrieve the first source image
    ---------------------------------------------------------------------------
*/
const HFCPtr<HRARaster>& HIMBlendCorridor::GetSource1() const
    {
    return m_pSource1;
    }


/** ---------------------------------------------------------------------------
    Retrieve the second source image
    ---------------------------------------------------------------------------
*/
const HFCPtr<HRARaster>& HIMBlendCorridor::GetSource2() const
    {
    return m_pSource2;
    }


/** ---------------------------------------------------------------------------
    Get the pool
    ---------------------------------------------------------------------------
*/
HPMPool* HIMBlendCorridor::GetTilePool() const
    {
    return m_pPool;
    }


/** ---------------------------------------------------------------------------
    Retrieve the tile organization
    ---------------------------------------------------------------------------
*/
HGFTileIDDescriptor* HIMBlendCorridor::GetTileDescriptor() const
    {
    return m_pTileDescriptor;
    }


/** ---------------------------------------------------------------------------
    Retrieve the physical CoordSys used for tiles
    ---------------------------------------------------------------------------
*/
HFCPtr<HGF2DCoordSys> HIMBlendCorridor::GetPhysicalCoordSys() const
    {
    return m_pPhysicalCS;
    }


/** ---------------------------------------------------------------------------
    Retrieve the width of the blend corridor, in the blend's logical
    coordinate system.
    ---------------------------------------------------------------------------
*/
double HIMBlendCorridor::GetCorridorWidth() const
    {
    return m_CorridorWidth;
    }


