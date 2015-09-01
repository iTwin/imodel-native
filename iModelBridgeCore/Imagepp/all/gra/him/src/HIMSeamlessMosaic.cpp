//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMSeamlessMosaic.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMSeamlessMosaic.h>
#include <Imagepp/all/h/HIMColorBalancedImage.h>
#include <Imagepp/all/h/HIMBlendCorridor.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRARasterIterator.h>
#include <ImagePP/all/h/HRACopyFromLegacyOptions.h>
#include <ImagePP/all/h/HRAMessages.h>

#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>

#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HVE2DSegment.h>

#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HRACachedHistogramRaster.h>

#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRPMessages.h>


HPM_REGISTER_CLASS(HIMSeamlessMosaic, HIMMosaic)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HIMSeamlessMosaic, HIMMosaic, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HIMSeamlessMosaic, HGFGeometryChangedMsg, NotifyGeometryChanged)
HMG_REGISTER_MESSAGE(HIMSeamlessMosaic, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_REGISTER_MESSAGE(HIMSeamlessMosaic, HRAContentChangedMsg, NotifyContentChanged)
HMG_END_MESSAGE_MAP()


// :Ignore

// Macros to extract a layer from the internal representation

#define CLIP_LAYER(Image)         ( Image )
#define CLIP_LAYER_TYPED(Image)   ( (HFCPtr<HRAReferenceToRaster>&) Image )

#define BALANCE_LAYER(Image)         ( ((HFCPtr<HRAReferenceToRaster>&) Image)->GetSource() )
#define BALANCE_LAYER_TYPED(Image)   ( (HFCPtr<HIMColorBalancedImage>&) ((HFCPtr<HRAReferenceToRaster>&) Image)->GetSource() )

#define ORIGINAL_IMAGE(Image)     ( ((HFCPtr<HRACachedHistogramRaster>&) ((HFCPtr<HIMColorBalancedImage>&) ((HFCPtr<HRAReferenceToRaster>&) Image)->GetSource())->GetSource())->GetSource() )

// This one is used on images that are inside the blends index only.
#define BLEND_TYPED(Image)        ( (HFCPtr<HIMBlendCorridor>&) Image )

/** -----------------------------------------------------------------------------
    This is the predicate used to sort points returned when intersecting
    segments with a shape.
    -----------------------------------------------------------------------------
*/
class CrossingSorter : public binary_function<HGF2DLocation, HGF2DLocation, bool>
    {
public:

    CrossingSorter(HVE2DSegment& pi_rSegment)
        : m_rSegment(pi_rSegment)
        {
        };

    bool operator()(const HGF2DLocation& pi_rFirst, const HGF2DLocation& pi_rSecond)
        {
        return m_rSegment.CalculateRelativePosition(pi_rFirst) < m_rSegment.CalculateRelativePosition(pi_rSecond);
        };

private:

    HVE2DSegment& m_rSegment;
    };

// :End Ignore




/** ---------------------------------------------------------------------------
    Default constructor. For persistence only
    ---------------------------------------------------------------------------
*/
HIMSeamlessMosaic::HIMSeamlessMosaic()
    : HIMMosaic()
    {
    }


/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMSeamlessMosaic::HIMSeamlessMosaic(const HFCPtr<HGF2DCoordSys>&       pi_rpCoordSys,
                                     const HFCPtr<HIMClippingTopology>& pi_rpClipManager,
                                     HPMPool*                           pi_pPool)
    : HIMMosaic(pi_rpCoordSys)
    {
    HASSERT(pi_rpClipManager != 0);

    m_pClipManager = pi_rpClipManager;

    HASSERT(pi_pPool != 0);
    m_pPool = pi_pPool;

    // The index containing the blend corridors
    m_pBlends = new SpatialIndexType(SpatialIndexType::Parameters(pi_rpCoordSys));

    // Default parameters

    m_ApplyGlobalAlgorithm = true;
    m_ApplyPositionalAlgorithm = true;

    m_BlendWidth = 1.0;

    m_SamplingQuality = SAMPLING_NORMAL;
    }


/** ---------------------------------------------------------------------------
    Copy constructor
    ---------------------------------------------------------------------------
*/
HIMSeamlessMosaic::HIMSeamlessMosaic(const HIMSeamlessMosaic& pi_rObj)
    : HIMMosaic((HIMMosaic&)pi_rObj)
    {
    m_pClipManager = pi_rObj.m_pClipManager;

    // Don't copy the blend corridors
    m_pBlends = new SpatialIndexType(SpatialIndexType::Parameters(GetCoordSys()));

    m_ApplyGlobalAlgorithm = pi_rObj.m_ApplyGlobalAlgorithm;
    m_ApplyPositionalAlgorithm = pi_rObj.m_ApplyPositionalAlgorithm;

    m_BlendWidth = pi_rObj.m_BlendWidth;
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
HIMSeamlessMosaic::~HIMSeamlessMosaic()
    {
    // We must ABSOLUTELY pass through the image list and clear
    // the balance images. Otherwise, there will be circular references
    // to the images (BalancedImages keep pointers to their neighbors),
    // and no image will be destroyed.
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr = pObjects->begin();
    while (Itr != pObjects->end())
        {
        HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
        BALANCE_LAYER_TYPED(pRaster)->ClearNeighbors();
        ++Itr;
        }

    delete m_pBlends;
    }


/** ---------------------------------------------------------------------------
    Assignment operation
    ---------------------------------------------------------------------------
*/
HIMSeamlessMosaic& HIMSeamlessMosaic::operator=(const HIMSeamlessMosaic& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        HIMMosaic::operator=(pi_rObj);

        m_pClipManager = pi_rObj.m_pClipManager;

        // Don't copy the blend corridors
        delete m_pBlends;
        m_pBlends = new SpatialIndexType(SpatialIndexType::Parameters(GetCoordSys()));

        m_ApplyGlobalAlgorithm = pi_rObj.m_ApplyGlobalAlgorithm;
        m_ApplyPositionalAlgorithm = pi_rObj.m_ApplyPositionalAlgorithm;

        m_BlendWidth = pi_rObj.m_BlendWidth;
        }

    return *this;
    }


/** ---------------------------------------------------------------------------
    CopyFromLegacy. This method is currently not supported.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    HASSERT(false);

    }


/** ---------------------------------------------------------------------------
    CopyFromLegacy. This method is currently not supported.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }


/** ---------------------------------------------------------------------------
    Notification for palette change
    ---------------------------------------------------------------------------
*/
bool HIMSeamlessMosaic::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    HFCPtr<HRARaster> pPersistentSender;
    const HRARaster* pSender;

    // get the address of the raster sender
    pSender = (const HRARaster*)pi_rMessage.GetSender();
    
    if (RecomputeRegion(pSender->GetEffectiveShape()))
        {
        // Replace the message with a full content changed.
        Propagate(HRAContentChangedMsg(*GetEffectiveShape()));

        return false;
        }
    else
        {
        // Nothing has been invalidated in the mosaic.
        // Replace the message with a small content changed.
        Propagate(HRAContentChangedMsg(*pSender->GetEffectiveShape()));

        return false;
        }
    }


/** ---------------------------------------------------------------------------
    Add an image to the mosaic. The image must not be already in the
    mosaic.
    @note An image that is invalid (bad pixeltype) will simply be ignored.
    @see HIMSeamlessMosaic::Add(const HIMMosaic::RasterList& pi_rRasters)
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::Add(const HFCPtr<HRARaster>& pi_pRaster)
    {
    HPRECONDITION( (HRARaster*) pi_pRaster != this);  // Don't add self to mosaic
    HPRECONDITION(!Contains(pi_pRaster, true));  // Don't add twice

    HPRECONDITION(IsAValidSource(pi_pRaster));
    if (IsAValidSource(pi_pRaster))
        {
        // Create the balanced layer.
        HFCPtr<HRARaster> pBalancedImage(AddInternalLayers(pi_pRaster));

        m_ImageMap.insert(RasterMap::value_type(pi_pRaster, pBalancedImage));

        // Add raster.
        // This will notify, but we aren't updated yet. This could
        // cause a problem
        HIMMosaic::Add(pBalancedImage);

        UpdateNeighbors(pi_pRaster->GetEffectiveShape());
        }
    }


/** ---------------------------------------------------------------------------
    Add images to the mosaic. The images must not already be in the mosaic.
    @note An image that is invalid (bad pixeltype) will simply be ignored.
    @see HIMSeamlessMosaic::Add(const HFCPtr<HRARaster>& pi_pRaster)
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::Add(const HIMMosaic::RasterList& pi_rRasters)
    {
    // Create a new list with wrapped images
    HIMMosaic::RasterList WrappedRasters;
    HFCPtr<HVEShape> pToUpdate(new HVEShape(GetCoordSys()));

    RasterList::const_iterator Itr(pi_rRasters.begin());
    while (Itr != pi_rRasters.end())
        {
        HASSERT(((HRARaster*) (*Itr)) != this);     // Don't add self to mosaic
        HASSERT(!Contains(*Itr, true));             // A raster must not be added twice!

        HPRECONDITION(IsAValidSource(*Itr));
        if (IsAValidSource(*Itr))
            {
            // Create the balanced layer.
            HFCPtr<HRARaster> pBalancedImage(AddInternalLayers(*Itr));

            m_ImageMap.insert(RasterMap::value_type(*Itr, pBalancedImage));

            WrappedRasters.push_back(pBalancedImage);

            pToUpdate->Unify(*(*Itr)->GetEffectiveShape());
            }

        ++Itr;
        }

    // Delegate work to ancestor
    // This will notify, but we aren't updated yet. This could
    // cause a problem
    HIMMosaic::Add(WrappedRasters);

    UpdateNeighbors(pToUpdate);
    }


/** ---------------------------------------------------------------------------
    Add an image to the mosaic, before another one. The image must not be
    already in the mosaic.
    @note An image that is invalid (bad pixeltype) will simply be ignored.
    @see HIMSeamlessMosaic::AddAfter
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::AddBefore(const HFCPtr<HRARaster>& pi_pRaster,
                                  const HFCPtr<HRARaster>& pi_pBefore)
    {
    HPRECONDITION( (HRARaster*) pi_pRaster != this);  // Don't add self to mosaic
    HPRECONDITION(!Contains(pi_pRaster, true));  // Don't add twice

    HPRECONDITION(IsAValidSource(pi_pRaster));
    if (IsAValidSource(pi_pRaster))
        {
        // Check if we contain the "pi_pBefore" raster
        RasterMap::const_iterator Itr(m_ImageMap.find(pi_pBefore));
        if (Itr != m_ImageMap.end())
            {
            // Create the image to add
            HFCPtr<HRARaster> pBalancedImage(AddInternalLayers(pi_pRaster));

            m_ImageMap.insert(RasterMap::value_type(pi_pRaster, pBalancedImage));

            // Delegate work to ancestor
            // This will notify, but we aren't updated yet. This could
            // cause a problem
            HIMMosaic::AddBefore(pBalancedImage, Itr->second);

            UpdateNeighbors(pi_pRaster->GetEffectiveShape());
            }
        }
    }


/** ---------------------------------------------------------------------------
    Add an image to the mosaic, after another one. The image must not be
    already in the mosaic.
    @note An image that is invalid (bad pixeltype) will simply be ignored.
    @see HIMSeamlessMosaic::AddBefore
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::AddAfter(const HFCPtr<HRARaster>& pi_pRaster,
                                 const HFCPtr<HRARaster>& pi_pAfter)
    {
    HPRECONDITION( (HRARaster*) pi_pRaster != this);  // Don't add self to mosaic
    HPRECONDITION(!Contains(pi_pRaster, true));  // Don't add twice

    HPRECONDITION(IsAValidSource(pi_pRaster));
    if (IsAValidSource(pi_pRaster))
        {
        // Check if we contain the "pi_pAfter" raster
        RasterMap::const_iterator Itr(m_ImageMap.find(pi_pAfter));
        if (Itr != m_ImageMap.end())
            {
            // Create the image to add
            HFCPtr<HRARaster> pBalancedImage(AddInternalLayers(pi_pRaster));

            m_ImageMap.insert(RasterMap::value_type(pi_pRaster, pBalancedImage));

            // Delegate work to ancestor
            // This will notify, but we aren't updated yet. This could
            // cause a problem
            HIMMosaic::AddAfter(pBalancedImage, Itr->second);

            UpdateNeighbors(pi_pRaster->GetEffectiveShape());
            }
        }
    }


/** ---------------------------------------------------------------------------
    Remove an image from the mosaic. The image must be in the mosaic.
    @see HIMSeamlessMosaic::RemoveAll()
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::Remove(const HFCPtr<HRARaster>& pi_pRaster)
    {
    // Try to find the specified raster
    RasterMap::iterator Itr(m_ImageMap.find(pi_pRaster));
    if (Itr != m_ImageMap.end())
        {
        HFCPtr<HRARaster> pToRemove(Itr->second);

        // Remove the entry from the map
        m_ImageMap.erase(Itr);

        // Remove the balanced image
        // This will notify, but we aren't updated yet. This could
        // cause a problem
        HIMMosaic::Remove(pToRemove);

        UpdateNeighbors(pi_pRaster->GetEffectiveShape());
        }
    }


/** ---------------------------------------------------------------------------
    Remove all images from the mosaic
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::RemoveAll()
    {
    m_ImageMap.clear();

    delete m_pBlends;
    m_pBlends = new SpatialIndexType(SpatialIndexType::Parameters(GetCoordSys()));

    // This will notify, but we aren't updated yet. This could
    // cause a problem
    HIMMosaic::RemoveAll();
    }


/** ---------------------------------------------------------------------------
    Return a new copy of self
    ---------------------------------------------------------------------------
*/
HRARaster* HIMSeamlessMosaic::Clone (HPMObjectStore* pi_pStore,
                                     HPMPool*        pi_pLog) const
    {
    return new HIMSeamlessMosaic(*this);
    }

/** ---------------------------------------------------------------------------
    Return a new copy of self
    ---------------------------------------------------------------------------
*/
HPMPersistentObject* HIMSeamlessMosaic::Clone () const
    {
    return new HIMSeamlessMosaic(*this);
    }

/** ---------------------------------------------------------------------------
    Receive an "effective shape change" notification
    ---------------------------------------------------------------------------
*/
bool HIMSeamlessMosaic::NotifyContentChanged (const HMGMessage& pi_rMessage)
    {
    // Update the affected region. Protect the shape that's in the message
    const HVEShape& rShape = ((HRAContentChangedMsg&)pi_rMessage).GetShape();
    ((HPMShareableObject<HGFGraphicObject>)rShape).IncrementRef();
    bool Updated = RecomputeRegion(HFCPtr<HVEShape>((HVEShape*) &rShape));
    ((HPMShareableObject<HGFGraphicObject>)rShape).DecrementRef();

    if (Updated)
        {
        // Replace the message with a full content changed.
        Propagate(HRAContentChangedMsg(*GetEffectiveShape()));

        return false;
        }
    else
        {
        // Nothing has been recomputed. Let the message continue as is.
        return true;
        }
    }


/** ---------------------------------------------------------------------------
    Receive a "geometry changed" notification
    ---------------------------------------------------------------------------
*/
bool HIMSeamlessMosaic::NotifyGeometryChanged (const HMGMessage& pi_rMessage)
    {
    HFCPtr<HRARaster> pPersistentSender;
    const HRARaster* pSender;

    // get the address of the raster sender
    pSender = (const HRARaster*)pi_rMessage.GetSender();
    
    // Destroy all blend corridors that use the image

    HAutoPtr< SpatialIndexType::IndexableList > pCurrentBlends(
        m_pBlends->QueryIndexables(HIDXSearchCriteria()));
    SpatialIndexType::IndexableList::const_iterator BlendsItr(pCurrentBlends->begin());
    while (BlendsItr != pCurrentBlends->end())
        {
        HFCPtr<HRARaster> pBlendRaster = (*BlendsItr)->GetObject();
        if (BLEND_TYPED(pBlendRaster)->Uses(BALANCE_LAYER(pSender)))
            m_pBlends->RemoveIndexable(*BlendsItr);

        ++BlendsItr;
        }

    // Recompute all BalancedImages that use the image

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                      HIDXSearchCriteria(), true));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end())
        {
        HFCPtr<HRARaster> pBalanceRaster = (*Itr)->GetObject();
        if (BALANCE_LAYER_TYPED(pBalanceRaster)->Uses(BALANCE_LAYER(pSender)))
            {
            HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
            SetupFilterFor(BALANCE_LAYER_TYPED(pRaster));
            }

        ++Itr;
        }

    // Do normal update based on image's new position. This will also
    // compute a new global distribution and set it, and manage the blends
    UpdateNeighbors(pSender->GetEffectiveShape());

    // Replace the message with a full content changed.
    Propagate(HRAContentChangedMsg(*GetEffectiveShape()));

    return false;
    }


/** ---------------------------------------------------------------------------
    Check if the specified raster is in the mosaic
    @param pi_pRaster    The raster to find.
    @param pi_DeepCheck  Indicates if we do a shallow or a deep search. For a
                            deep search, we will look inside logical rasters
                            like other mosaics.
    ---------------------------------------------------------------------------
*/
bool HIMSeamlessMosaic::Contains(const HFCPtr<HRARaster>& pi_pRaster,
                                  bool                    pi_DeepCheck) const
    {
    HPRECONDITION(pi_pRaster != 0);

    bool Contains;

    // Always ask ancestor. If we know the raster, ask ancestor to
    // search for our match of it. Otherwise, let the ancestor do
    // its normal job. This is necessary because HIMMosaic sometimes
    // calls Contains as a check on its own images (assertions).

    RasterMap::const_iterator Itr(m_ImageMap.find(pi_pRaster));
    if (Itr != m_ImageMap.end())
        {
        Contains = HIMMosaic::Contains(Itr->second, pi_DeepCheck);

        }
    else
        {
        Contains = HIMMosaic::Contains(pi_pRaster, pi_DeepCheck);
        }

    return Contains;
    }


/** ---------------------------------------------------------------------------
    Move the mosaic. In fact, since the mosaic is only a logical entity
    composed of other rasters, it is the composing rasters that are moved.

    It is assumed that the displacement parameter is specified relative to
    the logical coordinate system of the mosaic. The displacement will be
    converted into each image's system before being applied.

    @note This will have the effect of changing the geometry of the sources
          of the mosaic.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    RasterMap::const_iterator Itr(m_ImageMap.begin());

    // Pass through all images
    while (Itr != m_ImageMap.end())
        {
        HFCPtr<HGF2DCoordSys> pImageCS = Itr->first->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);

        if (pBaseToImage->PreservesParallelism())
            {
            HGF2DLocation Translation(pi_rDisplacement.GetDeltaX(), pi_rDisplacement.GetDeltaY(), GetCoordSys());
            HGF2DLocation Origin(0.0, 0.0, GetCoordSys());
            Translation.ChangeCoordSys(pImageCS);
            Origin.ChangeCoordSys(pImageCS);

            // and move.
            Itr->first->Move(HGF2DDisplacement(Translation.GetX() - Origin.GetX(),
                                               Translation.GetY() - Origin.GetY()));
            }
        else
            {
            HFCPtr<HGF2DTranslation> pTranslation(new HGF2DTranslation(pi_rDisplacement));

            HFCPtr<HGF2DTransfoModel> pTranslationForImage = pBaseToImage->ComposeInverseWithDirectOf(*pTranslation);
            pTranslationForImage = pTranslationForImage->ComposeInverseWithInverseOf(*pBaseToImage);
            HFCPtr<HGF2DTransfoModel> pSimplifiedTranslationForImage(pTranslationForImage->CreateSimplifiedModel());

            Itr->first->SetCoordSys(new HGF2DCoordSys(pSimplifiedTranslationForImage != 0 ?
                                                      *pSimplifiedTranslationForImage :                                                                         *pTranslationForImage,
                                                      pImageCS));
            }

        ++Itr;
        }

    RecalculateEffectiveShape();

    // Don't notify because sources will do it for us
    }


/** ---------------------------------------------------------------------------
    Rotate the mosaic around a point. In fact, since the mosaic is only a
    logical entity composed of other rasters, it is the composing rasters
    that are rotated.

    It is assumed that the rotation and origin parameters are specified relative to
    the logical coordinate system of the mosaic. The rotation will be
    converted into each image's system before being applied.

    @note This will have the effect of changing the geometry of the sources
          of the mosaic.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::Rotate(double               pi_Angle,
                               const HGF2DLocation& pi_rOrigin)
    {
    RasterMap::const_iterator Itr(m_ImageMap.begin());

    // Create basic rotation transformation (based on mosaic's CS)
    HFCPtr<HGF2DSimilitude> pRotation(new HGF2DSimilitude());
    pRotation->AddRotation(pi_Angle, pi_rOrigin.GetX(), pi_rOrigin.GetY());

    // Pass through all images
    while (Itr != m_ImageMap.end())
        {
        // Take the rotation and base it on the current image
        HFCPtr<HGF2DCoordSys> pImageCS = Itr->first->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);
        HFCPtr<HGF2DTransfoModel> pRotationForImage = pBaseToImage->ComposeInverseWithDirectOf(*pRotation);
        pRotationForImage = pRotationForImage->ComposeInverseWithInverseOf(*pBaseToImage);
        HFCPtr<HGF2DTransfoModel> pSimplifiedRotationForImage(pRotationForImage->CreateSimplifiedModel());

        // Apply the rotation
        Itr->first->SetCoordSys(new HGF2DCoordSys(pSimplifiedRotationForImage != 0 ?
                                                  *pSimplifiedRotationForImage :
                                                  *pRotationForImage,
                                                  pImageCS));

        ++Itr;
        }

    RecalculateEffectiveShape();

    // Don't notify because sources will do it for us
    }


/** ---------------------------------------------------------------------------
    Scale the mosaic. In fact, since the mosaic is only a logical entity
    composed of other rasters, it is the composing rasters that are scaled.

    It is assumed that the scale parameters are specified relative to
    the logical coordinate system of the mosaic. The scaling will be
    converted into each image's system before being applied.

    @note This will have the effect of changing the geometry of the sources
          of the mosaic.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::Scale(double              pi_ScaleFactorX,
                              double              pi_ScaleFactorY,
                              const HGF2DLocation& pi_rOrigin)
    {
    RasterMap::const_iterator Itr(m_ImageMap.begin());

    // Create stretch transformation based on mosaic's CS
    HFCPtr<HGF2DStretch> pScale(new HGF2DStretch());
    pScale->AddAnisotropicScaling(pi_ScaleFactorX, pi_ScaleFactorY,
                                  pi_rOrigin.GetX(), pi_rOrigin.GetY());

    // Iterate through all sources
    while (Itr != m_ImageMap.end())
        {
        HFCPtr<HGF2DCoordSys> pImageCS = Itr->first->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);

        if (pBaseToImage->PreservesDirection())
            {
            // Delegate scale to the current source
            Itr->first->Scale(pi_ScaleFactorX, pi_ScaleFactorY, pi_rOrigin);
            }
        else
            {
            // Base the scaling on the current image and apply

            HFCPtr<HGF2DTransfoModel> pScaleForImage = pBaseToImage->ComposeInverseWithDirectOf(*pScale);
            pScaleForImage = pScaleForImage->ComposeInverseWithInverseOf(*pBaseToImage);
            HFCPtr<HGF2DTransfoModel> pSimplifiedScaleForImage(pScaleForImage->CreateSimplifiedModel());

            Itr->first->SetCoordSys(new HGF2DCoordSys(pSimplifiedScaleForImage != 0 ?
                                                      *pSimplifiedScaleForImage :
                                                      *pScaleForImage,
                                                      pImageCS));
            }
        }

    RecalculateEffectiveShape();

    // Don't notify because sources will do it for us
    }


/** ---------------------------------------------------------------------------
    Find and set the neighbors for one balanced image.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::SetupFilterFor(const HFCPtr<HIMColorBalancedImage>& pi_rpRaster) const
    {
    HFCPtr<HRARaster> pLeftImage;
    HFCPtr<HRARaster> pRightImage;
    HFCPtr<HRARaster> pTopImage;
    HFCPtr<HRARaster> pBottomImage;

    pi_rpRaster->ClearNeighbors();

    HFCPtr<HGF2DCoordSys> pPhysicalCS(ObtainPhysicalCoordSys((HFCPtr<HRARaster>&)pi_rpRaster));

    if (pPhysicalCS != 0)
        {
        SelectNeighbors(pi_rpRaster, pPhysicalCS, pLeftImage, pRightImage, pTopImage, pBottomImage);

        HVEShape RasterShape(*pi_rpRaster->GetEffectiveShape());
        RasterShape.ChangeCoordSys(pPhysicalCS);
        HGF2DExtent RasterExtent(RasterShape.GetExtent());

        double XMin = RasterExtent.GetOrigin().GetX();
        double XMax = RasterExtent.GetCorner().GetX();
        double YMin = RasterExtent.GetOrigin().GetY();
        double YMax = RasterExtent.GetCorner().GetY();

        if (pLeftImage != 0)
            {
            pi_rpRaster->SetLeftNeighbor((HFCPtr<HIMColorBalancedImage>&) pLeftImage);

            HFCPtr<HVE2DPolySegment> pBoundary(m_pClipManager->GetBoundary(pi_rpRaster->GetSource(),
                                                                           ((HFCPtr<HIMColorBalancedImage>&)pLeftImage)->GetSource()));
            if (pBoundary != 0)
                {
                HFCPtr<HVE2DVector> pNewBoundary(pBoundary->AllocateCopyInCoordSys(pPhysicalCS));

                HGF2DExtent BoundaryExtent(pNewBoundary->GetExtent());

                // All the boundary region will be at 100%
                XMin = MAX(XMin, BoundaryExtent.GetXMax());
                }
            }


        if (pRightImage != 0)
            {
            pi_rpRaster->SetRightNeighbor((HFCPtr<HIMColorBalancedImage>&) pRightImage);

            HFCPtr<HVE2DPolySegment> pBoundary(m_pClipManager->GetBoundary(pi_rpRaster->GetSource(),
                                                                           ((HFCPtr<HIMColorBalancedImage>&)pRightImage)->GetSource()));
            if (pBoundary != 0)
                {
                HFCPtr<HVE2DVector> pNewBoundary(pBoundary->AllocateCopyInCoordSys(pPhysicalCS));

                HGF2DExtent BoundaryExtent(pNewBoundary->GetExtent());

                // All the boundary region will be at 100%
                XMax = MIN(XMax, BoundaryExtent.GetXMin());
                }
            }

        if (pTopImage != 0)
            {
            pi_rpRaster->SetTopNeighbor((HFCPtr<HIMColorBalancedImage>&) pTopImage);

            HFCPtr<HVE2DPolySegment> pBoundary(m_pClipManager->GetBoundary(pi_rpRaster->GetSource(),
                                                                           ((HFCPtr<HIMColorBalancedImage>&)pTopImage)->GetSource()));
            if (pBoundary != 0)
                {
                HFCPtr<HVE2DVector> pNewBoundary(pBoundary->AllocateCopyInCoordSys(pPhysicalCS));

                HGF2DExtent BoundaryExtent(pNewBoundary->GetExtent());

                // All the boundary region will be at 100%
                YMin = MAX(YMin, BoundaryExtent.GetYMax());
                }
            }

        if (pBottomImage != 0)
            {
            pi_rpRaster->SetBottomNeighbor((HFCPtr<HIMColorBalancedImage>&) pBottomImage);

            HFCPtr<HVE2DPolySegment> pBoundary(m_pClipManager->GetBoundary(pi_rpRaster->GetSource(),
                                                                           ((HFCPtr<HIMColorBalancedImage>&)pBottomImage)->GetSource()));
            if (pBoundary != 0)
                {
                HFCPtr<HVE2DVector> pNewBoundary(pBoundary->AllocateCopyInCoordSys(pPhysicalCS));

                HGF2DExtent BoundaryExtent(pNewBoundary->GetExtent());

                // All the boundary region will be at 100%
                YMax = MIN(YMax, BoundaryExtent.GetYMin());
                }
            }

        pi_rpRaster->SetApplicationShape(new HVEShape(XMin, YMin, XMax, YMax, pPhysicalCS));
        }
    else
        {
        // Make sure we always have a valid shape
        pi_rpRaster->SetApplicationShape(pi_rpRaster->GetEffectiveShape());
        }
    }


/** ---------------------------------------------------------------------------
    Select the neighbors using positions of the overlap shapes, working
    in the physical system of the image. We intersect the shape of each image
    with the four sides of our physical extent. We keep as neighbor the images
    that overlap the biggest percentages on each side.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::SelectNeighbors(const HFCPtr<HIMColorBalancedImage>& pi_rpRaster,
                                        HFCPtr<HGF2DCoordSys>&               pi_rpPhysicalCS,
                                        HFCPtr<HRARaster>&                   po_rpLeftNeighbor,
                                        HFCPtr<HRARaster>&                   po_rpRightNeighbor,
                                        HFCPtr<HRARaster>&                   po_rpTopNeighbor,
                                        HFCPtr<HRARaster>&                   po_rpBottomNeighbor) const
    {
    HASSERT(pi_rpRaster != 0);
    HASSERT(pi_rpPhysicalCS != 0);

    double PercentLeft   = 0;
    double PercentRight  = 0;
    double PercentTop    = 0;
    double PercentBottom = 0;

    HVEShape RasterShape(*pi_rpRaster->GetEffectiveShape());
    RasterShape.ChangeCoordSys(pi_rpPhysicalCS);
    HGF2DExtent RasterExtent(RasterShape.GetExtent());

    // Create one segment object for each side of the image extent
    HVE2DSegment LeftSegment(RasterExtent.GetOrigin(),
                             HGF2DDisplacement(0.0, RasterExtent.GetHeight()));
    HVE2DSegment BottomSegment(LeftSegment.GetEndPoint(),
                               HGF2DDisplacement(RasterExtent.GetWidth(), 0.0));
    HVE2DSegment RightSegment(BottomSegment.GetEndPoint(),
                              HGF2DDisplacement(0.0, -RasterExtent.GetHeight()));
    HVE2DSegment TopSegment(RightSegment.GetEndPoint(),
                            HGF2DDisplacement(-RasterExtent.GetWidth(), 0.0));

    // Find images that touch the current one
    HAutoPtr< HIMSeamlessMosaic::IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                                         HIDXSearchCriteria(m_pIndex->GetIndex2(),
                                                                                 new HGFSpatialCriteria(pi_rpRaster->GetExtent())), true));

    HIMSeamlessMosaic::IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end())
        {
        double CurrentPercentLeft = 0.0;
        double CurrentPercentRight = 0.0;
        double CurrentPercentTop = 0.0;
        double CurrentPercentBottom = 0.0;
        HFCPtr<HRARaster> pBalanceRaster = (*Itr)->GetObject();
        if (BALANCE_LAYER(pBalanceRaster) != (HFCPtr<HRARaster>&) pi_rpRaster)
            {
            // Take the shape of the current image
            HVEShape Intersection(*BALANCE_LAYER(pBalanceRaster)->GetEffectiveShape());
            Intersection.ChangeCoordSys(pi_rpPhysicalCS);
            HGF2DLocationCollection ThePoints;

            // Compute the length of each segment that is inside the image shape

            if (Intersection.GetShapePtr()->Intersect(LeftSegment, &ThePoints) > 0)
                {
                if (Intersection.GetShapePtr()->IsPointIn(LeftSegment.GetStartPoint()))
                    ThePoints.push_back(LeftSegment.GetStartPoint());

                if (Intersection.GetShapePtr()->IsPointIn(LeftSegment.GetEndPoint()))
                    ThePoints.push_back(LeftSegment.GetEndPoint());

                if (ThePoints.size() % 2 == 0)
                    {
                    sort(ThePoints.begin(), ThePoints.end(), CrossingSorter(LeftSegment));

                    double CurrentLength = 0.0;
                    HGF2DLocationCollection::const_iterator Itr(ThePoints.begin());
                    while (Itr != ThePoints.end())
                        {
                        const HGF2DLocation& FirstPoint = *Itr;
                        ++Itr;
                        CurrentLength += HVE2DSegment(FirstPoint, *Itr).CalculateLength();
                        ++Itr;
                        }

                    CurrentPercentLeft = CurrentLength / RasterExtent.GetHeight();
                    HASSERT(CurrentPercentLeft <= 100.0);
                    HASSERT(CurrentPercentLeft >= 0.0);
                    }

                ThePoints.clear();
                }

            if (Intersection.GetShapePtr()->Intersect(RightSegment, &ThePoints) > 0)
                {
                if (Intersection.GetShapePtr()->IsPointIn(RightSegment.GetStartPoint()))
                    ThePoints.push_back(RightSegment.GetStartPoint());

                if (Intersection.GetShapePtr()->IsPointIn(RightSegment.GetEndPoint()))
                    ThePoints.push_back(RightSegment.GetEndPoint());

                if (ThePoints.size() % 2 == 0)
                    {
                    sort(ThePoints.begin(), ThePoints.end(), CrossingSorter(RightSegment));

                    double CurrentLength = 0.0;
                    HGF2DLocationCollection::const_iterator Itr(ThePoints.begin());
                    while (Itr != ThePoints.end())
                        {
                        const HGF2DLocation& FirstPoint = *Itr;
                        ++Itr;
                        CurrentLength += HVE2DSegment(FirstPoint, *Itr).CalculateLength();
                        ++Itr;
                        }

                    CurrentPercentRight = CurrentLength / RasterExtent.GetHeight();
                    HASSERT(CurrentPercentRight <= 100.0);
                    HASSERT(CurrentPercentRight >= 0.0);
                    }

                ThePoints.clear();
                }

            if (Intersection.GetShapePtr()->Intersect(TopSegment, &ThePoints) > 0)
                {
                if (Intersection.GetShapePtr()->IsPointIn(TopSegment.GetStartPoint()))
                    ThePoints.push_back(TopSegment.GetStartPoint());

                if (Intersection.GetShapePtr()->IsPointIn(TopSegment.GetEndPoint()))
                    ThePoints.push_back(TopSegment.GetEndPoint());

                if (ThePoints.size() % 2 == 0)
                    {
                    sort(ThePoints.begin(), ThePoints.end(), CrossingSorter(TopSegment));

                    double CurrentLength = 0.0;
                    HGF2DLocationCollection::const_iterator Itr(ThePoints.begin());
                    while (Itr != ThePoints.end())
                        {
                        const HGF2DLocation& FirstPoint = *Itr;
                        ++Itr;
                        CurrentLength += HVE2DSegment(FirstPoint, *Itr).CalculateLength();
                        ++Itr;
                        }

                    CurrentPercentTop = CurrentLength / RasterExtent.GetWidth();
                    HASSERT(CurrentPercentTop <= 100.0);
                    HASSERT(CurrentPercentTop >= 0.0);
                    }

                ThePoints.clear();
                }

            if (Intersection.GetShapePtr()->Intersect(BottomSegment, &ThePoints) > 0)
                {
                if (Intersection.GetShapePtr()->IsPointIn(BottomSegment.GetStartPoint()))
                    ThePoints.push_back(BottomSegment.GetStartPoint());

                if (Intersection.GetShapePtr()->IsPointIn(BottomSegment.GetEndPoint()))
                    ThePoints.push_back(BottomSegment.GetEndPoint());

                if (ThePoints.size() % 2 == 0)
                    {
                    sort(ThePoints.begin(), ThePoints.end(), CrossingSorter(BottomSegment));

                    double CurrentLength = 0.0;
                    HGF2DLocationCollection::const_iterator Itr(ThePoints.begin());
                    while (Itr != ThePoints.end())
                        {
                        const HGF2DLocation& FirstPoint = *Itr;
                        ++Itr;
                        CurrentLength += HVE2DSegment(FirstPoint, *Itr).CalculateLength();
                        ++Itr;
                        }

                    CurrentPercentBottom = CurrentLength / RasterExtent.GetWidth();
                    HASSERT(CurrentPercentBottom <= 100.0);
                    HASSERT(CurrentPercentBottom >= 0.0);
                    }

                ThePoints.clear();
                }

            // We will use the current image for the side that is used
            // the most (percentage inside the image shape).

            if (CurrentPercentLeft > CurrentPercentRight &&
                CurrentPercentLeft > CurrentPercentTop &&
                CurrentPercentLeft > CurrentPercentBottom &&
                CurrentPercentLeft > PercentLeft)
                {
                PercentLeft = CurrentPercentLeft;
                po_rpLeftNeighbor = BALANCE_LAYER(pBalanceRaster);
                }
            if (CurrentPercentRight > CurrentPercentLeft &&
                CurrentPercentRight > CurrentPercentTop &&
                CurrentPercentRight > CurrentPercentBottom &&
                CurrentPercentRight > PercentRight)
                {
                PercentRight = CurrentPercentRight;
                po_rpRightNeighbor = BALANCE_LAYER(pBalanceRaster);
                }
            if (CurrentPercentTop > CurrentPercentRight &&
                CurrentPercentTop > CurrentPercentLeft &&
                CurrentPercentTop > CurrentPercentBottom &&
                CurrentPercentTop > PercentTop)
                {
                PercentTop = CurrentPercentTop;
                po_rpTopNeighbor = BALANCE_LAYER(pBalanceRaster);
                }
            if (CurrentPercentBottom > CurrentPercentRight &&
                CurrentPercentBottom > CurrentPercentTop &&
                CurrentPercentBottom > CurrentPercentLeft &&
                CurrentPercentBottom > PercentBottom)
                {
                PercentBottom = CurrentPercentBottom;
                po_rpBottomNeighbor = BALANCE_LAYER(pBalanceRaster);
                }
            }

        ++Itr;
        }
    }


/** ---------------------------------------------------------------------------
    Applies the neighbor selection algorithm to all images that touch the
    specified region.
    Since some neighbors may be changed, the global distribution must be
    recomputed.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::UpdateNeighbors(const HFCPtr<HVEShape>& pi_rpUpdateShape)
    {
    // Find images that touch the current one
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                      HIDXSearchCriteria(m_pIndex->GetIndex2(),
                                                                         new HGFSpatialCriteria(pi_rpUpdateShape->GetExtent())), true));

    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end())
        {
        // Setup even if we currently don't do color balancing.
        // If we change later, the parameters will already be computed.
        HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
        SetupFilterFor(BALANCE_LAYER_TYPED(pRaster));

        ++Itr;
        }

    SetGlobalDispersion();

    // Flush all precomputed data, since the balanced images have changed
    InvalidateBlends();

    // Synchronize the blend instances with the balanced images.
    ManageBlends(pi_rpUpdateShape);
    }


/** ---------------------------------------------------------------------------
    Recompute everything that touches the specified region. Balanced images
    will compute new histograms, blends will be flushed.
    ---------------------------------------------------------------------------
*/
bool HIMSeamlessMosaic::RecomputeRegion(const HFCPtr<HVEShape>& pi_rpUpdateShape)
    {
    bool Updated = false;

    // Find images that touch the current one
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                      HIDXSearchCriteria(m_pIndex->GetIndex2(),
                                                                         new HGFSpatialCriteria(pi_rpUpdateShape->GetExtent())), true));

    // OPTIMIZATION: If only one image is touched by the change region,
    // we don't update a thing. This is because all the parameters are
    // based on image overlaps. If only 1 image is touched, then the
    // region isn't inside an overlap.
    if (pObjects->size() > 1)
        {
        // Have all the histograms recomputed
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
            BALANCE_LAYER_TYPED(pRaster)->RecomputeHistograms();

            ++Itr;
            }

        SetGlobalDispersion();

        // All blends have to be recomputed
        InvalidateBlends();

        Updated = true;
        }

    return Updated;
    }


/** ---------------------------------------------------------------------------
    Wrap the specified raster in a colorbalance filter and a reference
    for clipping.
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMSeamlessMosaic::AddInternalLayers(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    HPRECONDITION(pi_rpRaster != 0);

    HFCPtr<HRARaster> pCBImage(new HIMColorBalancedImage(new HRACachedHistogramRaster(pi_rpRaster),
                                                         m_ApplyGlobalAlgorithm,
                                                         m_ApplyPositionalAlgorithm));
    HASSERT(pCBImage != 0);

    HFCPtr<HVEShape> pClipShape( m_pClipManager->GetPolygon(pi_rpRaster) );

    HFCPtr<HRARaster> pClippedImage;
    if (pClipShape != 0)
        {
        // Add a clipped image at the top
        pClippedImage = new HRAReferenceToRaster(pCBImage, *pClipShape);
        }
    else
        {
        pClippedImage = new HRAReferenceToRaster(pCBImage);

        HDEBUGTEXT(L"No clip shape found!\n");
        }

    HASSERT(pClippedImage != 0);

    return pClippedImage;
    }


/** ---------------------------------------------------------------------------
    Specify if the global color balancing will be applied
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::ApplyGlobalAlgorithm(bool pi_Apply)
    {
    if (pi_Apply != m_ApplyGlobalAlgorithm)
        {
        m_ApplyGlobalAlgorithm = pi_Apply;

        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr = pObjects->begin();
        while (Itr != pObjects->end())
            {
            HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
            BALANCE_LAYER_TYPED(pRaster)->ApplyGlobalAlgorithm(m_ApplyGlobalAlgorithm);
            ++Itr;
            }

        Propagate(HRAContentChangedMsg(*GetEffectiveShape()));
        }
    }


/** ---------------------------------------------------------------------------
    Specify if the positional color balancing will be applied
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::ApplyPositionalAlgorithm(bool pi_Apply)
    {
    if (pi_Apply != m_ApplyPositionalAlgorithm)
        {
        m_ApplyPositionalAlgorithm = pi_Apply;
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr = pObjects->begin();
        while (Itr != pObjects->end())
            {
            HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
            BALANCE_LAYER_TYPED(pRaster)->ApplyPositionalAlgorithm(m_ApplyPositionalAlgorithm);
            ++Itr;
            }

        Propagate(HRAContentChangedMsg(*GetEffectiveShape()));
        }
    }


/** ---------------------------------------------------------------------------
    Synchronize the blends with the images
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::ManageBlends(const HFCPtr<HVEShape>& pi_rpUpdateShape)
    {
    // Find images that are inside the update shape
    HAutoPtr< SpatialIndexType::IndexableList > pObjects(
        m_pIndex->GetIndex2()->QueryIndexables(
            HIDXSearchCriteria(m_pIndex->GetIndex2(),
                               new HGFSpatialCriteria(pi_rpUpdateShape->GetExtent()))));

    SpatialIndexType::IndexableList::const_iterator ObjectsItr(pObjects->begin());
    while (ObjectsItr != pObjects->end())
        {
        HFCPtr<HRARaster> pBalanceRaster = (*ObjectsItr)->GetObject();
        // Obtain all neighbors of the current image
        HAutoPtr< SpatialIndexType::IndexableList > pTouching(
            m_pIndex->GetIndex2()->QueryIndexables(
                HIDXSearchCriteria(m_pIndex->GetIndex2(),
                                   new HGFSpatialCriteria(BALANCE_LAYER(pBalanceRaster)->GetExtent()))));

        // Obtain current blends that touch the current image
        HAutoPtr< SpatialIndexType::IndexableList > pCurrentBlends(
            m_pBlends->QueryIndexables(
                HIDXSearchCriteria(m_pBlends,
                                   new HGFSpatialCriteria(BALANCE_LAYER(pBalanceRaster)->GetExtent()))));

        SpatialIndexType::IndexableList::const_iterator TouchingItr(pTouching->begin());
        while (TouchingItr != pTouching->end())
            {
            if ((*ObjectsItr)->GetObject() != (*TouchingItr)->GetObject())
                {
                HFCPtr<HRARaster> pTouchingRaster = (*TouchingItr)->GetObject();

                // Scan for the pair in the blends list
                SpatialIndexType::IndexableList::iterator TheBlend(pCurrentBlends->end());
                SpatialIndexType::IndexableList::iterator BlendsItr(pCurrentBlends->begin());
                while (BlendsItr != pCurrentBlends->end() && TheBlend == pCurrentBlends->end())
                    {
                    HFCPtr<HRARaster> pBlendRaster = (*BlendsItr)->GetObject();
                    if (BLEND_TYPED(pBlendRaster)->Blends(BALANCE_LAYER(pBalanceRaster),
                                                                       BALANCE_LAYER(pTouchingRaster)))
                        TheBlend = BlendsItr;

                    ++BlendsItr;
                    }

                if (TheBlend != pCurrentBlends->end())
                    {
                    // The blend for the current pair already exists, keep it that way.
                    // We remove it from the query.
                    pCurrentBlends->erase(TheBlend);
                    }
                else
                    {
                    HFCPtr<HVE2DPolySegment> pBoundary(m_pClipManager->GetBoundary(BALANCE_LAYER(pBalanceRaster),
                                                                                   BALANCE_LAYER(pTouchingRaster)));

                    if (pBoundary != 0)
                        {
                        // We need that blend. Construct a new one.
                        m_pBlends->Add(HFCPtr<HRARaster>(new HIMBlendCorridor(GetCoordSys(),
                                                                              BALANCE_LAYER(pBalanceRaster),
                                                                              BALANCE_LAYER(pTouchingRaster),
                                                                              pBoundary,
                                                                              m_pPool,
                                                                              m_BlendWidth)));
                        }
                    HWARNING(pBoundary == 0, L"Can't obtain boundary to create blend corridor.\n");
                    }
                }

            ++TouchingItr;
            }

        // If there are still blends in the query, these are not used anymore.
        // We will delete them.
        SpatialIndexType::IndexableList::const_iterator BlendsItr(pCurrentBlends->begin());
        while (BlendsItr != pCurrentBlends->end())
            {
            HFCPtr<HRARaster> pBlendRaster = (*BlendsItr)->GetObject();
            if (BLEND_TYPED(pBlendRaster)->Uses(BALANCE_LAYER(pBalanceRaster)))
                m_pBlends->RemoveIndexable(*BlendsItr);

            ++BlendsItr;
            }

        ++ObjectsItr;
        }
    }


/** ---------------------------------------------------------------------------
    Get the physical coordinate system of the raster.
    ---------------------------------------------------------------------------
*/
HFCPtr<HGF2DCoordSys> HIMSeamlessMosaic::ObtainPhysicalCoordSys(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    HFCPtr<HGF2DCoordSys> pPhysicalCS;
    HRARasterIterator* pIterator = pi_rpRaster->CreateIterator(HRAIteratorOptions());
    HASSERT(pIterator != 0);

    HFCPtr<HRARaster> pRaster = (*pIterator)();
    if (pRaster != 0)
        {
        if (pRaster->IsCompatibleWith(HRAReferenceToRaster::CLASS_ID))
            pPhysicalCS = ((HFCPtr<HRABitmap>&)((HFCPtr<HRAReferenceToRaster>&)pRaster)->GetSource())->GetPhysicalCoordSys();
        else
            pPhysicalCS = ((HFCPtr<HRABitmap>&)pRaster)->GetPhysicalCoordSys();
        }
    pRaster = 0;
    delete pIterator;

    return pPhysicalCS;
    }


/** ---------------------------------------------------------------------------
    Set the width of the blend corridor. The specified width will be
    interpreted in the mosaic's logical coordinate system. All blends
    will be updated immediately.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::SetBlendWidth(double pi_BlendWidth)
    {
    HPRECONDITION(pi_BlendWidth > 0.0);

    if (!HDOUBLE_EQUAL_EPSILON(m_BlendWidth, pi_BlendWidth) &&
        pi_BlendWidth > 0,0)
        {
        m_BlendWidth = pi_BlendWidth;

        HAutoPtr< SpatialIndexType::IndexableList > pCurrentBlends(
            m_pBlends->QueryIndexables(HIDXSearchCriteria()));

        SpatialIndexType::IndexableList::const_iterator BlendsItr(pCurrentBlends->begin());
        while (BlendsItr != pCurrentBlends->end())
            {
            HFCPtr<HRARaster> pBlendRaster = (*BlendsItr)->GetObject();
            // This also has the effect of an Invalidate call
            BLEND_TYPED(pBlendRaster)->SetCorridorWidth(m_BlendWidth);

            ++BlendsItr;
            }

        Propagate(HRAContentChangedMsg(*GetEffectiveShape()));
        }
    }


/** ---------------------------------------------------------------------------
    Set the quality/speed for histograms computing.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::SetSamplingQuality(HIMSeamlessMosaic::SamplingQuality pi_Quality)
    {
    if (m_SamplingQuality != pi_Quality)
        {
        m_SamplingQuality = pi_Quality;

        HAutoPtr< IndexType::IndexableList > pObjects(
            m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
            switch (m_SamplingQuality)
                {
                case SAMPLING_FAST:
                    BALANCE_LAYER_TYPED(pRaster)->SetSamplingQuality(HIMColorBalancedImage::SAMPLING_FAST);
                    break;
                case SAMPLING_NORMAL:
                    BALANCE_LAYER_TYPED(pRaster)->SetSamplingQuality(HIMColorBalancedImage::SAMPLING_NORMAL);
                    break;
                case SAMPLING_HIGH_QUALITY:
                    BALANCE_LAYER_TYPED(pRaster)->SetSamplingQuality(HIMColorBalancedImage::SAMPLING_HIGH_QUALITY);
                    break;
                default:
                    HASSERT(false);
                }

            ++Itr;
            }

        SetGlobalDispersion();

        InvalidateBlends();

        Propagate(HRAContentChangedMsg(*GetEffectiveShape()));
        }
    }


/** ---------------------------------------------------------------------------
    Flush all previously computed data in the blend corridors.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::InvalidateBlends()
    {
    HAutoPtr< SpatialIndexType::IndexableList > pCurrentBlends(
        m_pBlends->QueryIndexables(HIDXSearchCriteria()));

    SpatialIndexType::IndexableList::const_iterator BlendsItr(pCurrentBlends->begin());
    while (BlendsItr != pCurrentBlends->end())
        {
        HFCPtr<HRARaster> pBlendRaster = (*BlendsItr)->GetObject();
        BLEND_TYPED(pBlendRaster)->Invalidate();

        ++BlendsItr;
        }
    }


/** ---------------------------------------------------------------------------
    Retrieve the image at the specified location
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMSeamlessMosaic::GetAt(const HGF2DLocation& pi_rPosition) const
    {
    HFCPtr<HRARaster> pResult = HIMMosaic::GetAt(pi_rPosition);

    if (pResult != 0)
        {
        pResult = ORIGINAL_IMAGE(pResult);
        }

    return pResult;
    }


/** ---------------------------------------------------------------------------
    Compute and give the global dispersion to all balanced images.
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::SetGlobalDispersion()
    {
    HIMColorBalancedImage::RGBDispersion ImageDispersion;
    HIMColorBalancedImage::RGBDispersion GlobalDispersion;

    // Accumulate

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr = pObjects->begin();
    int32_t NumberOfImages = 0;
    while (Itr != pObjects->end())
        {
        HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
        ImageDispersion = BALANCE_LAYER_TYPED(pRaster)->GetLocalDispersionForGlobalAlgorithm();
        if (!ImageDispersion.IsEmpty())
            {
            GlobalDispersion += ImageDispersion;
            ++NumberOfImages;
            }

        ++Itr;
        }
    GlobalDispersion /= NumberOfImages;

    // Give global dispersion to all images

    Itr = pObjects->begin();
    while (Itr != pObjects->end())
        {
        HFCPtr<HRARaster> pRaster = (*Itr)->GetObject();
        BALANCE_LAYER_TYPED(pRaster)->SetGlobalDispersion(GlobalDispersion);

        ++Itr;
        }
    }


/** ---------------------------------------------------------------------------
    Force the mosaic to ask for the new clipping polygon of the specified
    image. This will also force the recomputing of everything that uses
    the specified image, like blend corridors and overlap histograms.

    @note It could be interesting to add a pi_RecomputeOverlaps parameter
    to specify if we want to recalculate histograms or simply update
    the blends. When the application changes a frontier between two images,
    it will call this method twice, once for each image. The parameter could
    be set true only on the second call, this would save some unnecessary
    histogram computing.

    @see SetSamplingQuality
    @see HIMClippingTopology
    ---------------------------------------------------------------------------
*/
void HIMSeamlessMosaic::UpdateClippingOf(const HFCPtr<HRARaster>& pi_rpImage)
    {
    // Try to find the specified raster
    RasterMap::const_iterator Itr(m_ImageMap.find(pi_rpImage));
    if (Itr != m_ImageMap.end())
        {
        HFCPtr<HRARaster> pToUpdate(Itr->second);

        HFCPtr<HVEShape> pClipShape( m_pClipManager->GetPolygon(pi_rpImage) );
        if (pClipShape != 0)
            {
            CLIP_LAYER_TYPED(pToUpdate)->SetShape(*pClipShape);

            // The previous call will update everything correctly because
            // we will receive a GeometryChanged notification from the
            // HRAReferenceToRaster layer.
            }

        HWARNING(pClipShape == 0, L"UpdateClippingOf: Can't obtain clipping for image.\n");
        }

    HWARNING(Itr == m_ImageMap.end(), L"UpdateClippingOf: Image is not in the mosaic.\n");
    }
