//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMMosaic.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMMosaic.h>
#include <Imagepp/all/h/HIMBufferedImage.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HGSSurfaceDescriptor.h>

#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAClearOptions.h>

#include <Imagepp/all/h/HFCException.h>

#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRPMessages.h>
#include <Imagepp/all/h/HGFMappedSurface.h>

#include <ImagePPInternal/gra/HRAImageNode.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>

HPM_REGISTER_CLASS(HIMMosaic, HRARaster)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HIMMosaic, HRARaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HIMMosaic, HGFGeometryChangedMsg, NotifyGeometryChanged)
HMG_REGISTER_MESSAGE(HIMMosaic, HRAEffectiveShapeChangedMsg, NotifyEffectiveShapeChanged)
HMG_REGISTER_MESSAGE(HIMMosaic, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_END_MESSAGE_MAP()



// :Ignore

// LookAhead cached result macros.
//
// 1  = Yes, we support a LookAhead
// 0  = No, we don't
// -1 = Result not cached, we don't know...

#define HIMMOSAIC_HAS_LOOKAHEAD           1
#define HIMMOSAIC_NO_LOOKAHEAD            0
#define HIMMOSAIC_RECALCULATE_LOOKAHEAD  -1

// :End Ignore


/** ---------------------------------------------------------------------------
    Default constructor, for Persistence only.
    ---------------------------------------------------------------------------
*/
HIMMosaic::HIMMosaic()
    : HRARaster()
    {
    m_pIndex = 0;

    m_HasLookAhead = HIMMOSAIC_RECALCULATE_LOOKAHEAD;
    }


/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMMosaic::HIMMosaic(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HRARaster(pi_rpCoordSys)
    {
    RelativeIndexType::Parameters RelativeParameters;
    m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(pi_rpCoordSys)));

    // We are the universe!
    SetShape(HVEShape(HVE2DUniverse(pi_rpCoordSys)));

    m_HasLookAhead = HIMMOSAIC_RECALCULATE_LOOKAHEAD;
    }


/** ---------------------------------------------------------------------------
    Copy constructor
    ---------------------------------------------------------------------------
*/
HIMMosaic::HIMMosaic(const HIMMosaic& pi_rObj)
    : HRARaster((HRARaster&)pi_rObj)
    {
    //HChk MR
    // Copy constructors on indexes are not implemented!
    // We fill the list one item at a time. We could at least
    // use the batch load operations on indexes (when they
    // are available)

    RelativeIndexType::Parameters RelativeParameters;
    m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(GetCoordSys())));

    // Fill the images list using the index's contents
    HAutoPtr< IndexType::IndexableList > pObjects(pi_rObj.m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end())
        {
        m_pIndex->Add((*Itr)->GetObject());

        LinkTo((*Itr)->GetObject());

        ++Itr;
        }

    if (pi_rObj.m_pPixelType != NULL)
        m_pPixelType = (HRPPixelType*) pi_rObj.m_pPixelType->Clone();

    m_HasLookAhead = pi_rObj.m_HasLookAhead;
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
HIMMosaic::~HIMMosaic()
    {
    if (m_pIndex)
        {
        // Unlink from all rasters before the quadtree is destroyed
        UnlinkFromAll();

        delete m_pIndex;
        }
    }


/** ---------------------------------------------------------------------------
    Assignment operation
    ---------------------------------------------------------------------------
*/
HIMMosaic& HIMMosaic::operator=(const HIMMosaic& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Copy the HRARaster portion
        HRARaster::operator=(pi_rObj);

        UnlinkFromAll();

        delete m_pIndex;

        m_pEffectiveShape = 0;
        m_pEffectiveExtent = 0;

        //HChk MR
        // Assignment operators on indexes are not implemented!
        // We fill the list one item at a time. We could at least
        // use the batch load operations on indexes (when they
        // are available)

        RelativeIndexType::Parameters RelativeParameters;
        m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(GetCoordSys())));

        // Fill the images list using the index's contents
        HAutoPtr< IndexType::IndexableList > pObjects(pi_rObj.m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            m_pIndex->Add((*Itr)->GetObject());

            LinkTo((*Itr)->GetObject());

            ++Itr;
            }

        m_HasLookAhead = pi_rObj.m_HasLookAhead;
        }

    return *this;
    }

/** ---------------------------------------------------------------------------
    Copy data from the specified source into the mosaic. Since the mosaic is
    composed of many images, the copy will be made in these images.
    ---------------------------------------------------------------------------*/
ImagePPStatus HIMMosaic::_CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& pi_rOptions)
    {
    HPRECONDITION(pi_rOptions.GetEffectiveCopyRegion() != NULL);

    ImagePPStatus status = COPYFROM_STATUS_VoidRegion;

    // Gather all images
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    for(IndexType::IndexableList::const_iterator Itr(pObjects->begin()); Itr != pObjects->end(); ++Itr)
        {
        // Compute clip shape for the source
        HVEShape VisibleShape(*pi_rOptions.GetEffectiveCopyRegion());
        VisibleShape.Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));
        HRACopyFromOptions ThisSourceOptions(pi_rOptions);
        ThisSourceOptions.SetEffectiveCopyRegion(&VisibleShape);

        // Copy in this source
        if(IMAGEPP_STATUS_Success != (status = (*Itr)->GetObject()->CopyFrom(srcRaster, ThisSourceOptions)))
            break;
        }
        
    return status;
    }


/** ---------------------------------------------------------------------------
    Copy data from the specified source into the mosaic. Sicne the mosaic is
    composed of many images, the copy will be made in these images.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    // Take the mosaic's effective shape
    HFCPtr<HVEShape> pDestShape(new HVEShape(*GetEffectiveShape()));

    // Intersect with source's effective shape
    pDestShape->Intersect(*pi_pSrcRaster->GetEffectiveShape());

    // Intersect with specified "copy from" shape if necessary
    if (pi_rOptions.GetDestShape() != 0)
        pDestShape->Intersect(*(pi_rOptions.GetDestShape()));

    if (!pDestShape->IsEmpty())
        {
        // Gather all images
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            // Compute clip shape for the source
            HVEShape VisibleShape(*pDestShape);
            VisibleShape.Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));
            HRACopyFromLegacyOptions ThisSourceOptions(pi_rOptions);
            ThisSourceOptions.SetDestShape(&VisibleShape);

            // Copy in this source
            (*Itr)->GetObject()->CopyFromLegacy(pi_pSrcRaster, ThisSourceOptions);

            ++Itr;
            }
        }
    }


/** ---------------------------------------------------------------------------
    Copy data from the specified source into the mosaic. Sicne the mosaic is
    composed of many images, the copy will be made in these images.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }

//---------------------------------------------------------------------------
// Clear
//---------------------------------------------------------------------------
void HIMMosaic::Clear()
    {
    HRAClearOptions ClearOptions;
    Clear(ClearOptions);
    }

//---------------------------------------------------------------------------
// Clear
//---------------------------------------------------------------------------
void HIMMosaic::Clear(const HRAClearOptions& pi_rOptions)
    {
    // Take the mosaic's effective shape
    HFCPtr<HVEShape> pDestShape(new HVEShape(*GetEffectiveShape()));

    // Intersect with source's effective shape
    if (pi_rOptions.HasShape())
        pDestShape->Intersect(*pi_rOptions.GetShape());

    if (!pDestShape->IsEmpty())
        {
        // Gather all images
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        HRAClearOptions ClearOptions(pi_rOptions);
        ClearOptions.SetApplyRasterClipping(false);
        while (Itr != pObjects->end())
            {
            // Compute clip shape for the source
            HVEShape VisibleShape(*pDestShape);
            VisibleShape.Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));

            ClearOptions.SetShape(&VisibleShape);

            // Clear this source
            (*Itr)->GetObject()->Clear(ClearOptions);

            ++Itr;
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// SetContext
//-----------------------------------------------------------------------------
void HIMMosaic::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    HASSERT(0); //TBD
    }

//-----------------------------------------------------------------------------
// public
// GetContext
//-----------------------------------------------------------------------------
HFCPtr<HMDContext> HIMMosaic::GetContext()
    {
    HASSERT(0); //TBD
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// InvalidateRaster
//-----------------------------------------------------------------------------
void HIMMosaic::InvalidateRaster()
    {
    HASSERT(0); //TBD
    }

/** ---------------------------------------------------------------------------
    Notification for palette change. We will propagate a ContentChanged.
    ---------------------------------------------------------------------------
*/
bool HIMMosaic::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    // the palette changed become a content change here    
    const HRARaster* pSender = (const HRARaster*)pi_rMessage.GetSender();

    Propagate(HRAContentChangedMsg(*(pSender->GetEffectiveShape())));

    // do not propagate the old message
    return false;
    }


/** ---------------------------------------------------------------------------
    Add an image to the mosaic
    @see HIMMosaic::Add(const HIMMosaic::RasterList& pi_rRasters)
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Add(const HFCPtr<HRARaster>& pi_pRaster)
    {
    HPRECONDITION( (HRARaster*) pi_pRaster != this);  // Don't add self to mosaic
    HPRECONDITION(!Contains(pi_pRaster, true));  // Don't add twice

    HVEShape OldEffectiveShape(*GetEffectiveShape());

    // Add raster
    m_pIndex->Add(pi_pRaster);

    // Link to it
    LinkTo(pi_pRaster);

    // invalidate the representative palette cache
    InvalidateRepPalCache();

#if (0)
    RecalculateEffectiveShape();
#else
    // Check if effective shape already computed
    if (m_pEffectiveShape != 0)
        {
        // Since allready computed we simply update
        m_pEffectiveShape->Unify(*(pi_pRaster->GetEffectiveShape()));

        m_pEffectiveShape->Intersect(HRARaster::GetShape());
        }

    m_pEffectiveExtent = 0;
#endif

    // If the added image has a lookahead, we automatically
    // support a lookahead also.
    if (m_HasLookAhead != HIMMOSAIC_HAS_LOOKAHEAD && pi_pRaster->HasLookAhead())
        m_HasLookAhead = HIMMOSAIC_HAS_LOOKAHEAD;

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }


/** ---------------------------------------------------------------------------
    Add a list of images to the mosaic. This will be faster than adding the
    images one by one since the index will be updated only once.
    @see HIMMosaic::Add(const HFCPtr<HRARaster>& pi_pRaster)
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Add(const HIMMosaic::RasterList& pi_rRasters)
    {
    HVEShape OldEffectiveShape(*GetEffectiveShape());

    // Create a list of indexables, so we can call AddIndexables directly.
    // We are already iterating on the objects, so we save one pass...
    HAutoPtr<IndexType::IndexableList> pIndexables(new IndexType::IndexableList);

    RasterList::const_iterator Itr(pi_rRasters.begin());
    while (Itr != pi_rRasters.end())
        {
        HASSERT(((HRARaster*) (*Itr)) != this);     // Don't add self to mosaic
        HASSERT(!Contains(*Itr, true));             // A raster must not be added twice!

        // Link to it
        LinkTo(*Itr);

        pIndexables->push_back(new HIDXIndexable< HFCPtr<HRARaster> >(*Itr));

        ++Itr;
        }

    // Add raster
    m_pIndex->AddIndexables(pIndexables);

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    RecalculateEffectiveShape();

    if (m_HasLookAhead != HIMMOSAIC_HAS_LOOKAHEAD)
        m_HasLookAhead = HIMMOSAIC_RECALCULATE_LOOKAHEAD;

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }


/** ---------------------------------------------------------------------------
    Add an image to the mosaic, before another one
    @param pi_pRaster The new image to add.
    @param pi_pBefore The reference image. We will add the new image on top
    of this reference.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::AddBefore(const HFCPtr<HRARaster>& pi_pRaster,
                          const HFCPtr<HRARaster>& pi_pBefore)
    {
    HPRECONDITION( (HRARaster*) pi_pRaster != this);  // Don't add self to mosaic
    HPRECONDITION(!Contains(pi_pRaster, true));  // Don't add twice
    HPRECONDITION(Contains(pi_pBefore));

    HVEShape OldEffectiveShape(*GetEffectiveShape());

    //HChk MR
    // This will be slow. We will probably need a method
    // on HIDSAListRelativeIndex to insert before...

    // Find the position of the reference object
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    // Since the list is returned from bottom to top, NumberOfDemotes
    // will temporarily hold the number of objects after the reference one
    size_t NumberOfDemotes = 0;
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end() && (*Itr)->GetObject() != pi_pBefore)
        {
        ++NumberOfDemotes;
        ++Itr;
        }

    // Now, calculate the number of demotes to do after the insertion
    // of the new object.
    NumberOfDemotes = pObjects->size() - NumberOfDemotes - 1;

    // Add raster
    m_pIndex->Add(pi_pRaster);

    while (NumberOfDemotes)
        {
        m_pIndex->GetIndex1()->Demote(pi_pRaster);
        --NumberOfDemotes;
        }

    // Link to it
    LinkTo(pi_pRaster);

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    RecalculateEffectiveShape();

    // If the added image has a lookahead, we automatically
    // support a lookahead also.
    if (m_HasLookAhead != HIMMOSAIC_HAS_LOOKAHEAD && pi_pRaster->HasLookAhead())
        m_HasLookAhead = HIMMOSAIC_HAS_LOOKAHEAD;

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }


/** ---------------------------------------------------------------------------
    Add an image to the mosaic, after another one
    @param pi_pRaster The new image to add.
    @param pi_pBefore The reference image. We will add the new image under
    this reference.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::AddAfter(const HFCPtr<HRARaster>& pi_pRaster,
                         const HFCPtr<HRARaster>& pi_pAfter)
    {
    HPRECONDITION( (HRARaster*) pi_pRaster != this);  // Don't add self to mosaic
    HPRECONDITION(!Contains(pi_pRaster, true));  // Don't add twice
    HPRECONDITION(Contains(pi_pAfter));

    HVEShape OldEffectiveShape(*GetEffectiveShape());

    //HChk MR
    // This will be slow. We will probably need a method
    // on HIDSAListRelativeIndex to insert after...

    // Find the position of the reference object
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    // Since the list is returned from bottom to top, NumberOfDemotes
    // will temporarily hold the number of objects after the reference one
    size_t NumberOfDemotes = 0;
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());
    while (Itr != pObjects->end() && (*Itr)->GetObject() != pi_pAfter)
        {
        ++NumberOfDemotes;
        ++Itr;
        }

    // Now, calculate the number of demotes to do after the insertion
    // of the new object.
    NumberOfDemotes = pObjects->size() - NumberOfDemotes;

    // Add raster in quad tree
    m_pIndex->Add(pi_pRaster);

    while (NumberOfDemotes)
        {
        m_pIndex->GetIndex1()->Demote(pi_pRaster);
        --NumberOfDemotes;
        }

    // Link to it
    LinkTo(pi_pRaster);

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    RecalculateEffectiveShape();

    // If the added image has a lookahead, we automatically
    // support a lookahead also.
    if (m_HasLookAhead != HIMMOSAIC_HAS_LOOKAHEAD && pi_pRaster->HasLookAhead())
        m_HasLookAhead = HIMMOSAIC_HAS_LOOKAHEAD;

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }


/** ---------------------------------------------------------------------------
    Remove an image from the mosaic
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Remove(const HFCPtr<HRARaster>& pi_pRaster)
    {
    HPRECONDITION(Contains(pi_pRaster));

    HVEShape OldEffectiveShape(*GetEffectiveShape());

    // Remove raster from quad tree
    m_pIndex->Remove(pi_pRaster);

    // Unlink from it
    UnlinkFrom(pi_pRaster);

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    RecalculateEffectiveShape();

    // If we remove an image with lookahead, we're not
    // sure if there are other ones...
    if (m_HasLookAhead == HIMMOSAIC_HAS_LOOKAHEAD && pi_pRaster->HasLookAhead())
        m_HasLookAhead = HIMMOSAIC_RECALCULATE_LOOKAHEAD;

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }


/** ---------------------------------------------------------------------------
    Remove all images from the mosaic
    ---------------------------------------------------------------------------
*/
void HIMMosaic::RemoveAll()
    {
    HVEShape OldEffectiveShape(*GetEffectiveShape());

    // Unlink from all rasters
    UnlinkFromAll();

    delete m_pIndex;
    RelativeIndexType::Parameters RelativeParameters;
    m_pIndex = new IndexType(IndexType::Parameters(RelativeParameters, SpatialIndexType::Parameters(GetCoordSys())));

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    // We are the universe!
//    SetShape(HVEShape(HVE2DUniverse(GetCoordSys())));

    RecalculateEffectiveShape();

    m_HasLookAhead = HIMMOSAIC_NO_LOOKAHEAD;

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAEffectiveShapeChangedMsg(OldEffectiveShape));
    }


/** ---------------------------------------------------------------------------
    Tell if the mosaic has a homogeneous pixel type.
    @see HIMMosaic::GetPixelType
    ---------------------------------------------------------------------------
*/
bool HIMMosaic::HasSinglePixelType() const
    {
    bool                   SinglePixelType = true;
    HFCPtr<HRPPixelType>    PixelType;
    HFCPtr<HRARaster>       pSource;

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    while (SinglePixelType && Itr != pObjects->end())
        {
        // Take a pointer to the raster
        pSource = (*Itr)->GetObject();

        // If the current source has a unique pixel type
        if (pSource->HasSinglePixelType())
            {
            // If it's the first source we're checking
            if (Itr == pObjects->begin())
                {
                // Keep its pixel type for further tests
                PixelType = pSource->GetPixelType();
                }
            else
                {
                // Check if same pixel type as previous sources
                SinglePixelType = PixelType->HasSamePixelInterpretation(*pSource->GetPixelType());
                }
            }
        else
            {
            // Source doesn't has a unique pixel type, so mosaic doesn't either...
            SinglePixelType = false;
            }

        ++Itr;
        }

    return SinglePixelType;
    }

/** ---------------------------------------------------------------------------
    This method specifies a pixel type for the mosaic.
    To restore the pixel type to it's default value, sets a NULL pixel type.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::SetPixelType (HFCPtr<HRPPixelType> const& pi_pPixelType)
    {
    if (pi_pPixelType != NULL)
        m_pPixelType = (HRPPixelType*) pi_pPixelType->Clone();
    else
        m_pPixelType = NULL;
    }

/** ---------------------------------------------------------------------------
    Get the mosaic's pixel type. If all images inside the mosaic have the
    same pixeltype, this pixeltype is returned. Otherwise, the result will be
    a pixeltype that can contain the colors of all images without loss. For
    example, two 8 bits palette images will force a 24 bits value pixeltype.
    @note The pixeltype of the mosaic can change over time, as we may add
    or remove images.
    @note If the mosaic is empty, the result will be V24R8G8B8.
    ---------------------------------------------------------------------------
*/
HFCPtr<HRPPixelType> HIMMosaic::GetPixelType() const
    {
    // If a pixel type has been specified, return it now
    if (m_pPixelType != NULL)
        return m_pPixelType;

    HFCPtr<HRPPixelType> pPixelType;

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

    if (pObjects->size() == 0)
        {
        // We don't know what pixeltype to use, since we don't have
        // any image. Default to V24RGB.
        pPixelType = new HRPPixelTypeV24R8G8B8();
        }
    else
        {
        if (HasSinglePixelType())
            {
            // Get the pixel type. NULL if no image in mosaic...
            pPixelType = pObjects->front()->GetObject()->GetPixelType();
            }
        else
            {
            // Mosaic with Different PixelTypes
            // Must create a PixelType to include all mosaic Pixeltypes

            IndexType::IndexableList::const_iterator Itr(pObjects->begin());

            size_t IndexBits = 0;
            HRPChannelOrg CurrentOrg((*Itr)->GetObject()->GetPixelType()->GetChannelOrg());
            bool CanUseIndex = true;
            bool ForceRGB = false;
            bool GrayscalePalette = false;
            bool AddBW = false;

            while (Itr != pObjects->end())
                {
                if (CanUseIndex)
                    {
                    size_t SourceIndexBits = (*Itr)->GetObject()->GetPixelType()->CountIndexBits();

                    if (SourceIndexBits > 0)
                        {
                        IndexBits += SourceIndexBits;

                        // Check if there is an alpha channel in the source. If there is, we must
                        // make sure that colors are all opaque or fully transparent. We won't be able
                        // to generate a representative palette for colors that must be blended.
                        bool PartialAlpha = false;
                        HRPChannelOrg SourceOrg((*Itr)->GetObject()->GetPixelType()->GetChannelOrg());
                        size_t ChannelIndex = SourceOrg.GetChannelIndex(HRPChannelType::ALPHA, 0);
                        if (ChannelIndex != HRPChannelType::FREE)
                            {
                            HFCPtr<HRPPixelType> pSourceValuePixelType(HRPPixelTypeFactory::GetInstance()->Create(SourceOrg, 0));
                            if (pSourceValuePixelType == 0)
                                {
                                PartialAlpha = true;
                                }
                            else
                                {
                                HRPPixelPalette const& rSrcPalette = (*Itr)->GetObject()->GetPixelType()->GetPalette();
                                HFCPtr<HRPPixelType> pV32PixelType(new HRPPixelTypeV32R8G8B8A8());
                                HFCPtr<HRPPixelConverter> pConverter(pSourceValuePixelType->GetConverterTo(pV32PixelType));
                                Byte Color[4];

                                for (uint32_t i = 0 ; !PartialAlpha && i < rSrcPalette.CountUsedEntries() ; ++i)
                                    {
                                    pConverter->Convert(rSrcPalette.GetCompositeValue(i), Color);
                                    if (Color[3] > 0 && Color[3] < 255)
                                        PartialAlpha = true;
                                    }
                                }
                            }

                        // Cases where we can't stay in palette mode:
                        //   1) We were already grayscale palette and we're adding index bits
                        //   2) We get a palette with more than 256 entries
                        //   3) We get a 256 entries palette and we must add black and white to it
                        //   4) The pixeltype also has value bits
                        //   5) Some colors have partial alpha values
                        if (GrayscalePalette ||
                            IndexBits > 8   ||
                            (IndexBits == 8 && AddBW) ||
                            (*Itr)->GetObject()->GetPixelType()->CountValueBits() > 0 ||
                            PartialAlpha)
                            CanUseIndex = false;
                        }
                    else
                        {
                        size_t SourceCompositeValueBits = (*Itr)->GetObject()->GetPixelType()->GetChannelOrg().CountPixelCompositeValueBits();

                        if (SourceCompositeValueBits == 1)
                            {
                            // We have a black and white image here
                            AddBW = true;

                            // Don't stay in palette mode if we already use the full 256 entries.
                            if (IndexBits >= 8)
                                CanUseIndex = false;
                            }
                        else if (SourceCompositeValueBits <= 8)
                            {
                            if (IndexBits > 0 && !GrayscalePalette)
                                {
                                CanUseIndex = false;
                                }
                            else
                                {
                                GrayscalePalette = true;
                                ForceRGB  = true;
                                IndexBits = 8;
                                }
                            }
                        else
                            {
                            // More than 8 value bits...
                            CanUseIndex = false;
                            }
                        }
                    }

                // Take the most general ChannelOrg
                if (ForceRGB || CurrentOrg != (*Itr)->GetObject()->GetPixelType()->GetChannelOrg())
                    {
                    HRPChannelOrg TempOrg1(CurrentOrg);
                    HRPChannelOrg TempOrg2((*Itr)->GetObject()->GetPixelType()->GetChannelOrg());
                    unsigned short ContainsAlpha = 0;

                    uint32_t ChannelIndex = TempOrg1.GetChannelIndex(HRPChannelType::ALPHA, 0);
                    if (ChannelIndex != HRPChannelType::FREE)
                        {
                        ContainsAlpha = 1;
                        TempOrg1.DeleteChannel((HRPChannelType*)TempOrg1.GetChannelPtr(ChannelIndex));
                        }
                    ChannelIndex = TempOrg2.GetChannelIndex(HRPChannelType::ALPHA, 0);
                    if (ChannelIndex != HRPChannelType::FREE)
                        {
                        ContainsAlpha = 2;

                        TempOrg2.DeleteChannel((HRPChannelType*)TempOrg2.GetChannelPtr(ChannelIndex));
                        }

                    if (!ForceRGB && TempOrg1 == TempOrg2)
                        {
                        if (ContainsAlpha == 2)
                            CurrentOrg = (*Itr)->GetObject()->GetPixelType()->GetChannelOrg();
                        }
                    else
                        {
                        // Not the same Channel organizations (even without checking alpha).
                        // We will use RGB

                        CurrentOrg = HRPChannelOrgRGB(8,
                                                      8,
                                                      8,
                                                      ContainsAlpha ? 8 : 0,
                                                      HRPChannelType::UNUSED,
                                                      HRPChannelType::INT_CH,
                                                      0);

                        ForceRGB = false;

                        // We use break to save tests. We don't need to continue if
                        // we've hit 32 bits value.
                        if (ContainsAlpha != 0 && (IndexBits == 0 || !CanUseIndex))
                            break;
                        }
                    }

                ++Itr;
                }

            // If there was alpha somewhere, we can't use
            // a pixeltype with a palette
            if (GrayscalePalette)
                {
                size_t ChannelIndex = CurrentOrg.GetChannelIndex(HRPChannelType::ALPHA, 0);
                if (ChannelIndex != HRPChannelType::FREE)
                    IndexBits = 0;
                }

            if (!CanUseIndex)
                IndexBits = 0;

            if (IndexBits > 1)
                IndexBits = 8;

            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(CurrentOrg, (unsigned short)IndexBits);

            if (pPixelType == 0)
                pPixelType = new HRPPixelTypeV32R8G8B8A8();
            else
                {
                if (IndexBits)
                    {
                    // We'll fill our palette with the colors of the palettes of all
                    // our sources since we're sure that we have enough entries.
                    HRPPixelPalette& rPalette = pPixelType->LockPalette();
                    HRPPixelPalette NewPalette(rPalette.GetMaxEntries(), rPalette.GetChannelOrg());
                    bool IndexingWorks = true;

                    if (GrayscalePalette)
                        {
                        Byte Color[4];
                        for (uint32_t i = 0 ; i < NewPalette.GetMaxEntries() ; ++i)
                            {
                            memset(Color, i, 4);    // no trouble, we don't have alpha...
                            NewPalette.AddEntry(Color);
                            }
                        }
                    else
                        {
                        // Need a value pixeltype with the same ChannelOrg for color conversions.
                        HFCPtr<HRPPixelType> pValuePixelType(HRPPixelTypeFactory::GetInstance()->Create(CurrentOrg, 0));
                        if (pValuePixelType == 0)
                            pValuePixelType = new HRPPixelTypeV32R8G8B8A8();

                        // Go through all sources
                        Itr = pObjects->begin();
                        while (IndexingWorks && Itr != pObjects->end())
                            {
                            HASSERT((*Itr)->GetObject()->GetPixelType()->CountIndexBits() > 0);
                            HFCPtr<HRPPixelType> pSourceValuePixelType(HRPPixelTypeFactory::GetInstance()->Create((*Itr)->GetObject()->GetPixelType()->GetChannelOrg(), 0));
                            if (pSourceValuePixelType == 0)
                                {
                                IndexingWorks = false;
                                }
                            else
                                {
                                HRPPixelPalette const& rSrcPalette = (*Itr)->GetObject()->GetPixelType()->GetPalette();
                                HFCPtr<HRPPixelConverter> pConverter(pSourceValuePixelType->GetConverterTo(pValuePixelType));
                                Byte Color[4];

                                for (uint32_t i = 0 ; i < rSrcPalette.CountUsedEntries() ; ++i)
                                    {
                                    // We add every color of the source palette (converted)
                                    pConverter->Convert(rSrcPalette.GetCompositeValue(i), Color);
                                    NewPalette.AddEntry(Color);
                                    }
                                }

                            ++Itr;
                            }

                        if (AddBW)
                            {
                            HFCPtr<HRPPixelConverter> pConverter(HRPPixelTypeV24R8G8B8().GetConverterTo(pValuePixelType));
                            Byte Black[4] = {0,0,0,0};
                            Byte White[4] = {255,255,255,255};
                            Byte Color[4];
                            pConverter->Convert(Black, Color);
                            NewPalette.AddEntry(Color);
                            pConverter->Convert(White, Color);
                            NewPalette.AddEntry(Color);
                            }
                        }

                    rPalette = NewPalette;
                    pPixelType->UnlockPalette();

                    if (!IndexingWorks)
                        pPixelType = new HRPPixelTypeV32R8G8B8A8();
                    }
                }
            }
        }

    // Return result
    return pPixelType;
    }


/** ---------------------------------------------------------------------------
    Retrieve the average pixel size of the mosaic.
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMMosaic::GetAveragePixelSize () const
    {
    double     XMax = 0.0;
    double     YMax = 0.0;
    uint32_t     UsedSources = 0;

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());


    if (pObjects->size() > 0)
        {
        HGF2DExtent CurrentPixelSize;

        while (Itr != pObjects->end())
            {
#if (1)
            // Get pixel size from current source
            HGF2DExtent TempExtent = (*Itr)->GetObject()->GetAveragePixelSize();

            // Check if this extent is defined
            if (TempExtent.IsDefined())
                {
                // Make sure the extent is not null
                HASSERT(TempExtent.GetWidth() != 0.0);
                HASSERT(TempExtent.GetHeight() != 0.0);

#if (0)
                // Obtain the four corners of extent
                HGF2DLocation LowerLeftCorner(TempExtent.GetXMin(), TempExtent.GetYMin(), TempExtent.GetCoordSys());
                HGF2DLocation UpperLeftCorner(TempExtent.GetXMin(), TempExtent.GetYMax(), TempExtent.GetCoordSys());
                HGF2DLocation UpperRightCorner(TempExtent.GetXMax(), TempExtent.GetYMax(), TempExtent.GetCoordSys());
                HGF2DLocation LowerRightCorner(TempExtent.GetXMax(), TempExtent.GetYMin(), TempExtent.GetCoordSys());

                // Convert our coordinate throught the reference transformation
                LowerLeftCorner.ChangeCoordSys(GetCoordSys());
                UpperLeftCorner.ChangeCoordSys(GetCoordSys());
                UpperRightCorner.ChangeCoordSys(GetCoordSys());
                LowerRightCorner.ChangeCoordSys(GetCoordSys());

                // Obtain the size of distances in this coordinate system
                double XLower = (LowerLeftCorner - LowerRightCorner).CalculateLength();
                double XUpper = (UpperLeftCorner - UpperRightCorner).CalculateLength();
                double YLeft  = (LowerLeftCorner - UpperLeftCorner).CalculateLength();
                double YRight = (LowerRightCorner - UpperRightCorner).CalculateLength();

                double AverageX = (XLower + XUpper) / 2.0;
                double AverageY = (YLeft + YRight) / 2.0;

#endif
                HGF2DExtent AdaptedExtent = TempExtent.CalculateApproxExtentIn(GetCoordSys());

                double AverageX = AdaptedExtent.GetWidth();
                double AverageY = AdaptedExtent.GetHeight();

                // Take current source's pixel size, and add it up...
                XMax += AverageX;
                YMax += AverageY;

                UsedSources++;
                }
#else

            // Get pixel size from current source
            HVEShape TempShape((*Itr)->GetObject()->GetAveragePixelSize());

            // Get that info. in our coordinate system
            TempShape.ChangeCoordSys(GetCoordSys());
            CurrentPixelSize = TempShape.GetExtent();

            // Check if extent is defined
            if (CurrentPixelSize.IsDefined())
                {
                // The extent is defined ... make sure pixel is not null ...
                if (CurrentPixelSize.GetWidth() != 0.0 &&
                    CurrentPixelSize.GetHeight() != 0.0)
                    {
                    // Take current source's pixel size, and add it up...
                    XMax += CurrentPixelSize.GetWidth();
                    YMax += CurrentPixelSize.GetHeight();

                    UsedSources++;
                    }
                }
#endif

            ++Itr;
            }
        }

    // Return calculated pixel size
    if (UsedSources == 0)
        {
        // No sources ... return undefined extent
        return HGF2DExtent(GetCoordSys());
        }
    else
        {
        return HGF2DExtent(0.0, 
                           0.0, 
                           XMax / (double)UsedSources,
                           YMax / (double)UsedSources,
                           GetCoordSys());
        }
    }


/** ---------------------------------------------------------------------------
    Return the pixel size range of the mosaic
    ---------------------------------------------------------------------------
*/
void HIMMosaic::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
#if (1)
    bool Initialized = false;
    double MinimumArea=0.0;
    double MaximumArea=0.0;

    // Initialize return pizel sizes to undefined
    po_rMinimum = HGF2DExtent(GetCoordSys());
    po_rMaximum = po_rMinimum;
#endif
#if (0)
    double XMin = 0.0;
    double YMin = 0.0;
    double XMax = 0.0;
    double YMax = 0.0;
#endif
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    if (pObjects->size() > 0)
        {
        HGF2DExtent SourceMinimum;
        HGF2DExtent SourceMaximum;

        while (Itr != pObjects->end())
            {
            // Get pixel size from current source
            (*Itr)->GetObject()->GetPixelSizeRange(SourceMinimum, SourceMaximum);
            // Get the dimensions in our coordinate system
            HVEShape TempSourceMinimumShape(SourceMinimum);
            HVEShape TempSourceMaximumShape(SourceMaximum);
            TempSourceMinimumShape.ChangeCoordSys(GetCoordSys());
            TempSourceMaximumShape.ChangeCoordSys(GetCoordSys());

#if (1)
            // Compute areas of pixel sizes shapes
            double CurrentMinimumArea = TempSourceMinimumShape.GetShapePtr()->CalculateArea();
            double CurrentMaximumArea = TempSourceMaximumShape.GetShapePtr()->CalculateArea();

            if (!HDOUBLE_EQUAL_EPSILON(sqrt(CurrentMinimumArea), 0.0))
                {
                // Check if areas have been initialized
                if (!Initialized)
                    {
                    // Check if extents are defined
                    if (SourceMinimum.IsDefined() && SourceMaximum.IsDefined())
                        {
                        // The return values have not been initialized .. we set with current sourcd pixel sizes
                        MinimumArea = CurrentMinimumArea;
                        MaximumArea = CurrentMaximumArea;
                        po_rMinimum = SourceMinimum;
                        po_rMaximum = SourceMaximum;

                        // Inidicate it has been initialized
                        Initialized = true;
                        }
                    }
                else
                    {
                    // The return values have been initialized ... check if current source minimum is smaller
                    // and defined and not null (exact floating point compare)
                    if (CurrentMinimumArea < MinimumArea && SourceMinimum.IsDefined() && CurrentMinimumArea != 0.0)
                        {
                        // The current source minimum is smaller ... set
                        MinimumArea = CurrentMinimumArea;
                        po_rMinimum = SourceMinimum;
                        }

                    // Check if current source maximum is greater
                    // and defined and not null (exact floating point compare)
                    if (CurrentMaximumArea > MaximumArea && SourceMaximum.IsDefined() && CurrentMaximumArea != 0.0)
                        {
                        // The current source minimum is smaller ... set
                        MaximumArea = CurrentMaximumArea;
                        po_rMaximum = SourceMaximum;
                        }
                    }
#else
            HGF2DExtent Minimum(TempSourceMinimumShape.GetExtent());
            HGF2DExtent Maximum(TempSourceMaximumShape.GetExtent());

            // Make sure the minimum extent is defined
            if (Minimum.IsDefined())
                {
                // Adjust for source's minimum
                if (Minimum.GetWidth() != 0.0 &&
                    Minimum.GetHeight() != 0.0)
                    {
                    XMin = XMin == 0 ? Minimum.GetWidth() : MIN(XMin, Minimum.GetWidth());
                    YMin = YMin == 0 ? Minimum.GetHeight() : MIN(YMin, Minimum.GetHeight());
                    XMax = MAX(XMax, Minimum.GetWidth());
                    YMax = MAX(YMax, Minimum.GetHeight());
                    }
                }

            // Make sure the maximum extent is defined
            if (Maximum.IsDefined())
                {
                // Adjust for source's maximum
                if (Maximum.GetWidth() != 0.0 &&
                    Maximum.GetHeight() != 0.0)
                    {
                    XMin = XMin == 0 ? Maximum.GetWidth() : MIN(XMin, Maximum.GetWidth());
                    YMin = YMin == 0 ? Maximum.GetHeight() : MIN(YMin, Maximum.GetHeight());
                    XMax = MAX(XMax, Maximum.GetWidth());
                    YMax = MAX(YMax, Maximum.GetHeight());
                    }
                }
#endif
                }
            ++Itr;
            }
        }

#if (0)
    // Return calculated pixel size
    po_rMinimum = HGF2DExtent(0.0, 0.0, XMin, YMin, GetCoordSys());
    po_rMaximum = HGF2DExtent(0.0, 0.0, XMax, YMax, GetCoordSys());
#endif
    }

/** ---------------------------------------------------------------------------
    Place the specified raster one position lower (going towards the back).
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Sink(const HFCPtr<HRARaster>& pi_pRaster)
    {
    m_pIndex->GetIndex1()->Demote(pi_pRaster);

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAContentChangedMsg(*pi_pRaster->GetEffectiveShape()));
    }


/** ---------------------------------------------------------------------------
    Place the specified raster one position higher (going towards the top).
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Float(const HFCPtr<HRARaster>& pi_pRaster)
    {
    m_pIndex->GetIndex1()->Promote(pi_pRaster);

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAContentChangedMsg(*pi_pRaster->GetEffectiveShape()));
    }


/** ---------------------------------------------------------------------------
    Place the specified raster object on top of the mosaic
    ---------------------------------------------------------------------------
*/
void HIMMosaic::BringToFront(const HFCPtr<HRARaster>& pi_pRaster)
    {
    // Tell quad tree to do so
    m_pIndex->GetIndex1()->Front(pi_pRaster);

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAContentChangedMsg(*pi_pRaster->GetEffectiveShape()));
    }


/** ---------------------------------------------------------------------------
     Place the specified raster object at the bottom of the mosaic
    ---------------------------------------------------------------------------
*/
void HIMMosaic::SendToBack(const HFCPtr<HRARaster>& pi_pRaster)
    {
    // Tell quad tree to do so
    m_pIndex->GetIndex1()->Back(pi_pRaster);

    // Notify linked rasters that mosaic's content has changed
    Propagate(HRAContentChangedMsg(*pi_pRaster->GetEffectiveShape()));
    }


/** ---------------------------------------------------------------------------
    Return the effective shape of the mosaic
    ---------------------------------------------------------------------------
*/
HFCPtr<HVEShape> HIMMosaic::GetEffectiveShape () const
    {
    // Check if effective shape already computed
    if (m_pEffectiveShape == 0)
        {
        // The effective shape is not computed ...

        // Create an empty shape
        m_pEffectiveShape = new HVEShape(GetCoordSys());

        // Query all rasters in mosaic (all encompassing criterium)
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));


        // For all rasters ... unify together their effective shape
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            m_pEffectiveShape->Unify(*(*Itr)->GetObject()->GetEffectiveShape());
            ++Itr;
            }

        // Finaly the global shape of all rasters contained herein is cliped to raster shape
        m_pEffectiveShape->Intersect(HRARaster::GetShape());
        }

    return m_pEffectiveShape;
    }


/** ---------------------------------------------------------------------------
    Get the mosaic's extent
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMMosaic::GetExtent() const
    {
    if (m_pEffectiveShape == 0)
        {
        // Avoid computing the effective shape, if may be a very long task
        return GetExtentInCs(GetCoordSys());
        }
    else
        {
        // The effective shape is already computed, uses it directly
        return m_pEffectiveShape->GetExtent();
        }
    }

/** ---------------------------------------------------------------------------
    Get the mosaic's extent in a specified CS
    ---------------------------------------------------------------------------
*/
HGF2DExtent HIMMosaic::GetExtentInCs(HFCPtr<HGF2DCoordSys> pi_pCoordSys) const
    {
    // We can optimise the computation of the extent only if the mosaic is not shaped
    if (m_pEffectiveShape == 0 && GetShape().GetShapePtr()->IsCompatibleWith(HVE2DUniverse::CLASS_ID))
        {
        // The effective shape is alrread computed ...
        if (m_pEffectiveExtent != 0)
            return *m_pEffectiveExtent;

        // Create an empty shape
        m_pEffectiveExtent = new HGF2DExtent(pi_pCoordSys);

        // Query all rasters in mosaic (all encompassing criterium)
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));

        // For all rasters ... unify together their effective shape
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());
        while (Itr != pObjects->end())
            {
            // Take a copy of the raster's shape.
            HFCPtr<HVEShape> pTmpShape = new HVEShape(*(*Itr)->GetObject()->GetEffectiveShape());

            // Bring it into the provided coordsys
            pTmpShape->ChangeCoordSys(pi_pCoordSys);

            m_pEffectiveExtent->Add(pTmpShape->GetExtent());
            ++Itr;
            }

        return *m_pEffectiveExtent;
        }
    else
        {
        // Call the ancestor
        return HRARaster::GetExtentInCs(pi_pCoordSys);
        }
    }

/** ---------------------------------------------------------------------------
    Return the raster object at the specified location.
    @return The topmost raster found at the position, or NULL if there is none.
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMMosaic::GetAt(const HGF2DLocation& pi_rPosition) const
    {
    HGF2DExtent TempExtent(pi_rPosition.GetX() - DBL_EPSILON,
                           pi_rPosition.GetY() - DBL_EPSILON,
                           pi_rPosition.GetX() + DBL_EPSILON,
                           pi_rPosition.GetY() + DBL_EPSILON,
                           pi_rPosition.GetCoordSys());

    // Retrieve all objects that may contain the specified point
    HAutoPtr< IndexType::IndexableList > pObjects(
        m_pIndex->QueryIndexables(HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(TempExtent)), true));

    HFCPtr<HRARaster> pResult;

    IndexType::IndexableList::reverse_iterator Itr(pObjects->rbegin());
    HGFGraphicObject::Location PositionOfPoint;

    // Pass through the list, starting at the top
    while (pResult == 0 && Itr != pObjects->rend())
        {
        PositionOfPoint = (*Itr)->GetObject()->Locate(pi_rPosition);

        if (PositionOfPoint == HGFGraphicObject::S_INSIDE ||
            PositionOfPoint == HGFGraphicObject::S_ON_BOUNDARY)
            {
            // The object really contains the location, return it!
            pResult = (*Itr)->GetObject();
            }

        Itr++;
        }

    return pResult;
    }


/** ---------------------------------------------------------------------------
    Return a new copy of self
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMMosaic::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HIMMosaic(*this);
    }
/** ---------------------------------------------------------------------------
    Return a new copy of self
    ---------------------------------------------------------------------------
*/
HPMPersistentObject* HIMMosaic::Clone () const
    {
    return new HIMMosaic(*this);
    }


/** ---------------------------------------------------------------------------
    Tell if mosaic contains pixels of the specified channel
    ---------------------------------------------------------------------------
*/
bool HIMMosaic::ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                           Byte                      pi_Id) const
    {
    bool                ContainsSome = false;

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    // Pass through all sources (until we find a source using the channel...)
    while (!ContainsSome && Itr != pObjects->end())
        {
        // Ask source if it has pixels of specified channel
        ContainsSome = (*Itr)->GetObject()->ContainsPixelsWithChannel(pi_Role, pi_Id);

        ++Itr;
        }

    return ContainsSome;
    }


/** ---------------------------------------------------------------------------
    Create an editor.
    @return NULL, not implemented.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMMosaic::CreateEditor(HFCAccessMode pi_Mode)
    {
    return 0;
    }


/** ---------------------------------------------------------------------------
    Create a shaped editor
    @return NULL, not implemented.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMMosaic::CreateEditor(const HVEShape&    pi_rShape,
                                         HFCAccessMode      pi_Mode)
    {
    return 0;
    }



/** ---------------------------------------------------------------------------
    Create an unshaped editor
    @return NULL, not implemented.
    ---------------------------------------------------------------------------
*/
HRARasterEditor* HIMMosaic::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return 0;
    }

/** ---------------------------------------------------------------------------
    Check if the mosaic supports the lookahead mechanism.
    @return true if at least one image supports the lookahead.
    @see HIMMosaic::SetLookAhead
    ---------------------------------------------------------------------------
*/
bool HIMMosaic::HasLookAhead() const
    {
    if (m_HasLookAhead == HIMMOSAIC_RECALCULATE_LOOKAHEAD)
        {
        m_HasLookAhead = HIMMOSAIC_NO_LOOKAHEAD;

        // try to find at least one LookAheadable source
        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());

        while ((m_HasLookAhead == HIMMOSAIC_NO_LOOKAHEAD) && Itr != pObjects->end())
            {
            // Get the LookAhead capability of the current element
            if ((*Itr)->GetObject()->HasLookAhead())
                m_HasLookAhead = HIMMOSAIC_HAS_LOOKAHEAD;

            // Proceed to the next element
            ++Itr;
            }
        }

    return (m_HasLookAhead == HIMMOSAIC_HAS_LOOKAHEAD);
    }


/** ---------------------------------------------------------------------------
    Set a lookahead shape to the mosaic. It will be given to every image
    that supports the lookahead mechanism.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::SetLookAhead(const HVEShape& pi_rShape,
                             uint32_t        pi_ConsumerID,
                             bool           pi_Async)
    {
    HPRECONDITION(HasLookAhead());

    // Set the lookahead of images that are in the region.

    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                      HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(pi_rShape.GetExtent())), true));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    while (Itr != pObjects->end())
        {
        // Get the LookAhead capability of the current element
        if ((*Itr)->GetObject()->HasLookAhead())
            {
            // Calculate the needed visible part of this image
            HVEShape VisibleShape(pi_rShape);
            VisibleShape.Intersect(HRARaster::GetShape());
            VisibleShape.Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));

            // Send part to the image, if there is one...
            if (!VisibleShape.IsEmpty())
                (*Itr)->GetObject()->SetLookAhead(VisibleShape, pi_ConsumerID, pi_Async);
            }

        // Proceed to the next element
        ++Itr;
        }
    }


/** ---------------------------------------------------------------------------
    Receive an "effective shape change" notification from one of our images.
    ---------------------------------------------------------------------------
*/
bool HIMMosaic::NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage)
    {
    HVEShape OldEffectiveShape(*GetEffectiveShape());
    HFCPtr< HIDXIndexable< HFCPtr<HRARaster> > > pIndexable;
    
    // Can't use Index2 to find it because its shape
    // has changed!
    pIndexable = m_pIndex->GetIndex1()->GetFilledIndexableFor(new HIDXIndexable< HFCPtr<HRARaster> >((HRARaster*)pi_rMessage.GetSender()));


    // The first invalidate can't use interacting objects because the object
    // to invalidate has changed its effective shape.
    m_pIndex->GetIndex1()->Invalidate(pIndexable, false);
    m_pIndex->GetIndex2()->RemoveIndexable(pIndexable);
    m_pIndex->GetIndex2()->AddIndexable(pIndexable);
    m_pIndex->GetIndex1()->Invalidate(pIndexable);

    RecalculateEffectiveShape();

    // Propagate message
    return true;
    }


/** ---------------------------------------------------------------------------
    Receive a "geometry changed" notification from one of our images.
    ---------------------------------------------------------------------------
*/
bool HIMMosaic::NotifyGeometryChanged (const HMGMessage& pi_rMessage)
    {
    HVEShape OldShape(*GetEffectiveShape());
    HFCPtr< HIDXIndexable< HFCPtr<HRARaster> > > pIndexable;

    // Raster has changed its shape
    // Can't use Index2 to find it because its geometry
    // has changed!
    pIndexable = m_pIndex->GetIndex1()->GetFilledIndexableFor(new HIDXIndexable< HFCPtr<HRARaster> >((HRARaster*)pi_rMessage.GetSender()));
    
    // The first invalidate can't use interacting objects because the object
    // to invalidate has changed its geometry.
    m_pIndex->GetIndex1()->Invalidate(pIndexable, false);
    m_pIndex->GetIndex2()->RemoveIndexable(pIndexable);
    m_pIndex->GetIndex2()->AddIndexable(pIndexable);
    m_pIndex->GetIndex1()->Invalidate(pIndexable);

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    RecalculateEffectiveShape();

    // Propagate as an effective shape change
    Propagate(HRAEffectiveShapeChangedMsg(OldShape));

    // Stop the message here!
    return false;
    }


/** ---------------------------------------------------------------------------
    Unlink from all rasters composing the mosaic
    ---------------------------------------------------------------------------
*/
void HIMMosaic::UnlinkFromAll(void)
    {
    HASSERT(m_pIndex != 0);

    // try to find at least one LookAheadable source
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    // Pass through all list
    while (Itr != pObjects->end())
        {
        // Unlink ourselves
        UnlinkFrom((*Itr)->GetObject());
        ++Itr;
        }
    }


/** ---------------------------------------------------------------------------
    Link to all rasters composing the mosaic
    ---------------------------------------------------------------------------
*/
void HIMMosaic::LinkToAll(void)
    {
    HASSERT(m_pIndex != 0);

    // try to find at least one LookAheadable source
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    // Pass through all list
    while (Itr != pObjects->end())
        {
        // Link ourselves
        LinkTo((*Itr)->GetObject());
        ++Itr;
        }
    }


/** ---------------------------------------------------------------------------
    Check if a raster is in the mosaic.
    @param pi_pRaster The image to locate in the mosaic.
    @param pi_DeepCheck Indicate if the search will be deep or shallow. A deep
    check will search inside sources that are themselves mosaics, whereas a
    shallow check will only check inside the current mosaic.
    ---------------------------------------------------------------------------
*/
bool HIMMosaic::Contains(const HFCPtr<HRARaster>& pi_pRaster,
                          bool                    pi_DeepCheck) const
    {
    HPRECONDITION(pi_pRaster != 0);

    bool Contains = false;

    if (pi_DeepCheck)
        {
        // Check for indirect containment
        HFCPtr<HRARaster> pSource;
        bool             CheckFurther;

        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                          HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(pi_pRaster->GetExtent())), false));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());

        // Iterate through all sources in the quad tree, and stop if
        // we find what we are looking for...
        while (!Contains && Itr != pObjects->end())
            {
            // Take a pointer to the current source
            pSource = (*Itr)->GetObject();

            CheckFurther = true;

            while (CheckFurther)
                {
                // Check if the current mosaic's source is the one we are
                // looking for
                if (pSource == pi_pRaster)
                    {
                    // Found it.
                    Contains = true;
                    CheckFurther = false;
                    }
                else
                    {
                    // Check if it could be inside the current raster object.

                    if (pSource->GetClassID() == HIMMosaic::CLASS_ID)
                        {
                        Contains = ((HFCPtr<HIMMosaic>&) pSource)->Contains(pi_pRaster,
                                                                            pi_DeepCheck);
                        CheckFurther = false;
                        }
                    else if (pSource->GetClassID() == HIMBufferedImage::CLASS_ID)
                        {
                        pSource = ((HFCPtr<HIMBufferedImage>&) pSource)->GetSource();
                        }
                    else
                        {
                        CheckFurther = false;
                        }
                    }
                }

            ++Itr;
            }
        }
    else
        {
        // Simple shallow check

        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                          HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(pi_pRaster->GetExtent())), false));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());

        while (!Contains && Itr != pObjects->end())
            {
            if ((*Itr)->GetObject() == pi_pRaster)
                {
                // Found it.
                Contains = true;
                }

            ++Itr;
            }
        }

    return Contains;
    }


/** ---------------------------------------------------------------------------
    Move the mosaic. To accomplish this, the mosaic's source images will
    be moved.
    @param pi_rDisplacement The translation to apply, in the mosaic logical
    coordinate system.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    // Pass through all list
    while (Itr != pObjects->end())
        {
        HFCPtr<HGF2DCoordSys> pImageCS = (*Itr)->GetObject()->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);

        if (pBaseToImage->PreservesParallelism())
            {
            HGF2DLocation Translation(pi_rDisplacement.GetDeltaX(), pi_rDisplacement.GetDeltaY(), GetCoordSys());
            HGF2DLocation Origin(0.0, 0.0, GetCoordSys());
            Translation.ChangeCoordSys(pImageCS);
            Origin.ChangeCoordSys(pImageCS);

            // and move.
            (*Itr)->GetObject()->Move(HGF2DDisplacement(Translation.GetX() - Origin.GetX(),
                                                        Translation.GetY() - Origin.GetY()));
            }
        else
            {
            HFCPtr<HGF2DTranslation> pTranslation(new HGF2DTranslation(pi_rDisplacement));

            HFCPtr<HGF2DTransfoModel> pTranslationForImage = pBaseToImage->ComposeInverseWithDirectOf(*pTranslation);
            pTranslationForImage = pTranslationForImage->ComposeInverseWithInverseOf(*pBaseToImage);
            HFCPtr<HGF2DTransfoModel> pSimplifiedTranslationForImage(pTranslationForImage->CreateSimplifiedModel());

            (*Itr)->GetObject()->SetCoordSys(new HGF2DCoordSys(pSimplifiedTranslationForImage != 0 ?
                                                               *pSimplifiedTranslationForImage :                                                                         *pTranslationForImage,
                                                               pImageCS));
            }

        ++Itr;
        }

    RecalculateEffectiveShape();

    // Don't notify because sources will do it for us
    }


/** ---------------------------------------------------------------------------
    Rotate the mosaic. To accomplish this, the mosaic's source images will
    be rotated.
    @param pi_Angle The angle of rotation, relative to the mosaic's
    logical coordinate system.
    @param pi_rOrigin The rotation's origin point, also in logical system.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Rotate(double pi_Angle,
                       const HGF2DLocation& pi_rOrigin)
    {
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    // Create basic rotation transformation (based on mosaic's CS)
    HFCPtr<HGF2DSimilitude> pRotation(new HGF2DSimilitude);
    pRotation->AddRotation(pi_Angle, pi_rOrigin.GetX(), pi_rOrigin.GetY());

    // Iterate through all sources
    while (Itr != pObjects->end())
        {
        // Take the rotation and base it on the current image
        HFCPtr<HGF2DCoordSys> pImageCS = (*Itr)->GetObject()->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);
        HFCPtr<HGF2DTransfoModel> pRotationForImage = pBaseToImage->ComposeInverseWithDirectOf(*pRotation);
        pRotationForImage = pRotationForImage->ComposeInverseWithInverseOf(*pBaseToImage);
        HFCPtr<HGF2DTransfoModel> pSimplifiedRotationForImage(pRotationForImage->CreateSimplifiedModel());

        // Apply the rotation
        (*Itr)->GetObject()->SetCoordSys(new HGF2DCoordSys(pSimplifiedRotationForImage != 0 ?
                                                           *pSimplifiedRotationForImage :
                                                           *pRotationForImage,
                                                           pImageCS));

        ++Itr;
        }

    RecalculateEffectiveShape();

    // Don't notify because sources will do it for us
    }


/** ---------------------------------------------------------------------------
    Scale the mosaic. To accomplish this, the mosaic's source images will
    be scaled.
    @param pi_ScaleFactorX The scale on the X axis, relative to the mosaic's
    logical coordinate system.
    @param pi_ScaleFactorY The scale on the Y axis, relative to the mosaic's
    logical coordinate system.
    @param pi_rOrigin The scale's origin point, also in logical system.
    ---------------------------------------------------------------------------
*/
void HIMMosaic::Scale(double              pi_ScaleFactorX,
                      double              pi_ScaleFactorY,
                      const HGF2DLocation& pi_rOrigin)
    {
    HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria()));
    IndexType::IndexableList::const_iterator Itr(pObjects->begin());

    // Create stretch transformation based on mosaic's CS
    HFCPtr<HGF2DStretch> pScale(new HGF2DStretch());
    pScale->AddAnisotropicScaling(pi_ScaleFactorX, pi_ScaleFactorY,
                                  pi_rOrigin.GetX(), pi_rOrigin.GetY());

    // Iterate through all sources
    while (Itr != pObjects->end())
        {
        HFCPtr<HGF2DCoordSys> pImageCS = (*Itr)->GetObject()->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = GetCoordSys()->GetTransfoModelTo(pImageCS);

        if (pBaseToImage->PreservesDirection())
            {
            // Delegate scale to the current source
            (*Itr)->GetObject()->Scale(pi_ScaleFactorX, pi_ScaleFactorY, pi_rOrigin);
            }
        else
            {
            // Base the scaling on the current image and apply

            HFCPtr<HGF2DTransfoModel> pScaleForImage = pBaseToImage->ComposeInverseWithDirectOf(*pScale);
            pScaleForImage = pScaleForImage->ComposeInverseWithInverseOf(*pBaseToImage);
            HFCPtr<HGF2DTransfoModel> pSimplifiedScaleForImage(pScaleForImage->CreateSimplifiedModel());

            (*Itr)->GetObject()->SetCoordSys(new HGF2DCoordSys(pSimplifiedScaleForImage != 0 ?
                                                               *pSimplifiedScaleForImage :
                                                               *pScaleForImage,
                                                               pImageCS));
            }
        }

    RecalculateEffectiveShape();

    // Don't notify because sources will do it for us
    }


/** ---------------------------------------------------------------------------
    Called when some change necessitates a recalculation of the effective shape
    ---------------------------------------------------------------------------
*/
void HIMMosaic::RecalculateEffectiveShape ()
    {
    m_pEffectiveShape = 0;
    m_pEffectiveExtent = 0;
    }

/*---------------------------------------------------------------------------------**//**
* MosaicNode
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<MosaicNode> MosaicNode::Create(HRARaster& mosaic, HFCPtr<HGF2DCoordSys> pPhysicalCS, HGF2DExtent const& physExtent, HFCPtr<HRPPixelType>& pixelType)
    {
    return new MosaicNode(mosaic, pPhysicalCS, physExtent, pixelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MosaicNode::MosaicNode(HRARaster& mosaic, HFCPtr<HGF2DCoordSys> pPhysicalCS, HGF2DExtent const& physExtent, HFCPtr<HRPPixelType>& pixelType)
:ImageSourceNode(pixelType),        // Take replacing pixeltype into acocunt. 
 m_physicalExtent(physExtent), m_pPhysicalCS(pPhysicalCS)
    {
    BeAssert(physExtent.GetCoordSys() == GetPhysicalCoordSys());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HGF2DExtent const& MosaicNode::_GetPhysicalExtent() const { return m_physicalExtent; }
HFCPtr<HGF2DCoordSys>& MosaicNode::_GetPhysicalCoordSys() { return m_pPhysicalCS; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus MosaicNode::_GetRegion(HRAImageSurfacePtr& pOutSurface, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator)
    {
    ImagePPStatus status = IMAGEPP_STATUS_Success;

    HRAImageSamplePtr pSample = HRAImageSample::CreateSample(status, (uint32_t)region.GetWidth(), (uint32_t)region.GetHeight(), GetPixelType(), allocator);
    if (IMAGEPP_STATUS_Success != status)
        return status;

    outOffset.x = region.GetXMin();
    outOffset.y = region.GetYMin();
    HRASampleSurfacePtr pSampleSurface = HRASampleSurface::Create(*pSample);

    // Mosaic is a middle man source producer and some of the source might have alpha which we will blend. 
    // In case of alpha source, a clear is required. Detect that in the buildCopyToContext?
    // Clear the input sample with the pixeltype default raw data.
    // what to do if nothing intersect or non rectangular clip shape. The out sample will be left uninitialized.
    // No need to blend if only one child.
    // Since OnDemandMosaic's effective shape is always an Extent, we should clear the sample.
    // >>> Detection of IsClearRequired is too tricky for now always clear.
    pSampleSurface->Clear();
   
    for (auto pChild : m_children)
        {
        // PixelOffset64 is in destination CS. Since a mosaic is generated at destination resolution we do not need to change it.
        BeAssert(GetPhysicalCoordSys().GetPtr() == GetParentP()->GetPhysicalCoordSys().GetPtr());
        BeAssert(pChild->AsImageTransformNodeP() != NULL);   // We always expect a transform node.

        // Skip child that do not overlap with the current region
        if (pChild->AsImageTransformNodeP() == NULL ||
            pChild->AsImageTransformNodeP()->GetCurrentRegionGrid().GetWidth() == 0 || pChild->AsImageTransformNodeP()->GetCurrentRegionGrid().GetHeight() == 0)
            continue;

        // A transform node always produce at our physical CS so no need to change CS here
        BeAssert(GetPhysicalCoordSys().GetPtr() == pChild->AsImageTransformNodeP()->GetPhysicalCoordSys().GetPtr());

        HFCInclusiveGrid sampleGrid;
        sampleGrid.InitFromLenght((double)outOffset.x, (double)outOffset.y, pSampleSurface->GetWidth(), pSampleSurface->GetHeight());
        if (!sampleGrid.HasIntersect(pChild->AsImageTransformNodeP()->GetCurrentRegionGrid()))
            continue;

        if (IMAGEPP_STATUS_Success != (status = pChild->AsImageTransformNodeP()->Produce(*pSampleSurface, outOffset, allocator)))
            break;
        }

    if (IMAGEPP_STATUS_Success != status)
        return status;

    pOutSurface = pSampleSurface;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HIMMosaic::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    // Take a copy of the shape to avoid modifying the original shape
    HVEShape RegionToDrawInCs(*options.GetShape());

    // If a replacing CS is provided, we must move the region to draw since this region does not represents the real position of the mosaic.
    // The replacing CS is a replacement of the mosaic logical CS. It represents the position of the mosaic as seen by the destination.
    if (options.GetReplacingCoordSys())
        {
        // Express the region to draw in the replacing CS. Then set the CS of the mosaic. 
        // This will move the region to draw at the right location in the mosaic CS.
        RegionToDrawInCs.ChangeCoordSys(options.GetReplacingCoordSys());
        RegionToDrawInCs.SetCoordSys(GetCoordSys());
        }
    else
        {
        RegionToDrawInCs.ChangeCoordSys(GetCoordSys());
        }

    // Query for visible items
    HAutoPtr<IndexType::IndexableList> pObjects(m_pIndex->QueryIndexables(
        HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(RegionToDrawInCs.GetExtent())), true));

    // Compute the extent enclosing the images that are visible in the region to draw.
    // This shape is the union of the extent of the rasters that overlaps the region to draw.
    HGF2DExtent enclosingShape(GetCoordSys());
    std::vector< pair< HFCPtr<HRARaster>, HFCPtr<HVEShape> > > enclosedRasters;
    for (IndexType::IndexableList::const_iterator Itr(pObjects->begin()); Itr != pObjects->end(); ++Itr)
        {
        HFCPtr<HRARaster> pSrcRaster((*Itr)->GetObject());
        HFCPtr<HVEShape> pVisibleSurface = m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr);

        if (RegionToDrawInCs.HasIntersect(*pVisibleSurface))
            {
            enclosedRasters.push_back(make_pair(pSrcRaster, pVisibleSurface));
            enclosingShape.Add(pVisibleSurface->GetExtent());
            }
        }

    // Finally the global shape of all rasters contained herein is clipped to raster shape
    enclosingShape.Intersect(HRARaster::GetShape().GetExtent());
        
    // Intersect with the region to draw
    enclosingShape.Intersect(RegionToDrawInCs.GetExtent());

    // If a replacing CS is provided, move the shape of visible rasters.
    if (options.GetReplacingCoordSys())
        {
        enclosingShape.ChangeCoordSys(GetCoordSys());
        enclosingShape.SetCoordSys(options.GetReplacingCoordSys());
        }

    // Express the shape in the destination node physical CS.
    enclosingShape.ChangeCoordSys(imageNode.GetPhysicalCoordSys());

    if (!enclosingShape.IsDefined())
        return IMAGEPP_STATUS_Success;

    // Take replacing pixeltype into account.
    BeAssert(options.GetReplacingPixelType() == NULL || options.GetReplacingPixelType()->CountPixelRawDataBits() == GetPixelType()->CountPixelRawDataBits());
    HFCPtr<HRPPixelType> pEffectivePixelType = (options.GetReplacingPixelType() != NULL) ? options.GetReplacingPixelType() : GetPixelType();
    
    // If we are working with binary data we prefer RLE format.
    if (pEffectivePixelType->CountPixelRawDataBits() == 1)
        {
        //Image Node and Image sample use the pixel type to differentiate between 1bit and Rle data. Unlike HRABitmap[RLE] that use pixeltype and codec.
        HFCPtr<HRPPixelType> pRlePixelType = ImageNode::TransformToRleEquivalent(pEffectivePixelType);
        if (pRlePixelType == NULL)
            {
            BeAssert(!"Incompatible replacing pixelType");
            return COPYFROM_STATUS_IncompatiblePixelTypeReplacer;
            }
        pEffectivePixelType = pRlePixelType;
        }
    
    RefCountedPtr<MosaicNode> pMosaicNode = MosaicNode::Create(*this, imageNode.GetPhysicalCoordSys(), enclosingShape, pEffectivePixelType);
    ImagePPStatus buildMosaicStatus = IMAGEPP_STATUS_Success;

    // Process one image at a time, bottom up
    for (auto srcRasterPair : enclosedRasters)
        {
        HFCPtr<HRARaster>& pSrcRaster = srcRasterPair.first;

        // Always setup the clip per raster because the mosaic ::Produce will be called upon the rasters extent and this extent might
        // be beyond this raster effective rectangle. If we do not provide a shape, the outputmerger will clip with the mosaic extent that
        // is somehow generated the Produce iteration.
        HFCPtr<HVEShape> pCurrShape = new HVEShape(*srcRasterPair.second);
        
        // Move the shape as seen by the destination
        if (options.GetReplacingCoordSys())
            {
            pCurrShape->ChangeCoordSys(GetCoordSys());
            pCurrShape->SetCoordSys(options.GetReplacingCoordSys());
            }

        // Convert to destination physical coordinates
        pCurrShape->ChangeCoordSys(pMosaicNode->GetPhysicalCoordSys());

        ImagePPStatus linkToStatus = IMAGEPP_STATUS_UnknownError;
        ImageTransformNodePtr pTransformNode = ImageTransformNode::CreateAndLink(linkToStatus, *pMosaicNode, pCurrShape);
        if (IMAGEPP_STATUS_Success != linkToStatus)
            return linkToStatus;

        pTransformNode->SetResamplingMode(options.GetResamplingMode());
        // It is assumed that mosaic rendering always need to blend to produce the right result
        pTransformNode->SetAlphaBlend(enclosedRasters.size() > 1); 
               
        HRACopyToOptions newOptions(options);
        if (options.GetReplacingCoordSys() != NULL)
            {
            // Adapt replacing CS so that it works on the current image.

            // Extract the transformation we're applying to our source (T1)
            HFCPtr<HGF2DTransfoModel> pRefTransfo(options.GetReplacingCoordSys()->GetTransfoModelTo(GetCoordSys()));

            // Calculate the transformation between our source's CS and the CS
            // the object to return uses. (T2)
            HFCPtr<HGF2DTransfoModel> pCurrentToSource(pSrcRaster->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

            // Create the CS by composing everything correctly :)
            // T2 o T1 o -T2
            HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToSource->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToSource));
            HFCPtr<HGF2DCoordSys> pCSToApply = new HGF2DCoordSys(*pToApply, pSrcRaster->GetCoordSys());

            newOptions.SetReplacingCoordSys(pCSToApply);
            }

        ImagePPStatus buildCopyToContextStatus = IMAGEPP_STATUS_Success;
        if (IMAGEPP_STATUS_Success != (buildCopyToContextStatus = pSrcRaster->BuildCopyToContext(*pTransformNode, newOptions)))
            {
            // set global status but keep first error
            if (IMAGEPP_STATUS_Success == buildMosaicStatus)
                buildMosaicStatus = buildCopyToContextStatus;
            }
        }

    if (pMosaicNode->GetChildCount() > 0)
        {
        ImagePPStatus linkToStatus = IMAGEPP_STATUS_UnknownError;
        if (IMAGEPP_STATUS_Success != (linkToStatus = imageNode.LinkTo(*pMosaicNode)))
            return linkToStatus;

        BeAssert(pMosaicNode->GetPhysicalCoordSys() == imageNode.GetPhysicalCoordSys());
        }

    return buildMosaicStatus;
 }

//-----------------------------------------------------------------------------
// Draw the mosaic
//-----------------------------------------------------------------------------
void HIMMosaic::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    bool DrawDone = false;

    HRADrawOptions Options(pi_Options);
    HFCPtr<HGF2DCoordSys> pReplacingCS(Options.GetReplacingCoordSys());

    HVEShape RegionToDraw(Options.GetShape() != 0 ? *Options.GetShape() : HRARaster::GetShape());

    HFCPtr<HGSRegion> pClipRegion(pio_destSurface.GetRegion());
    if (pClipRegion != 0)
        {
        // Intersect it with the destination
        HFCPtr<HVEShape> pSurfaceShape(pClipRegion->GetShape());

        if (Options.GetReplacingCoordSys())
            {
            pSurfaceShape->ChangeCoordSys(Options.GetReplacingCoordSys());
            pSurfaceShape->SetCoordSys(GetCoordSys());
            }

        RegionToDraw.Intersect(*pSurfaceShape);
        }
    else
        {
        // Create a rectangular clip region to stay
        // inside the destination surface.
        HVEShape DestSurfaceShape(0.0, 0.0, pio_destSurface.GetSurfaceDescriptor()->GetWidth(), pio_destSurface.GetSurfaceDescriptor()->GetHeight(), pio_destSurface.GetSurfaceCoordSys());

        // Set the stroking tolerance for the surface's shape
        // Set a quarter of a pixel tolerance
        double CenterX = pio_destSurface.GetSurfaceDescriptor()->GetWidth() / 2.0;
        double CenterY = pio_destSurface.GetSurfaceDescriptor()->GetHeight() / 2.0;
        HFCPtr<HGFTolerance> pTol = new HGFTolerance (CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                      CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                      CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                      CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                      pio_destSurface.GetSurfaceCoordSys());

        if (Options.GetReplacingCoordSys())
            {
            DestSurfaceShape.ChangeCoordSys(Options.GetReplacingCoordSys());
            DestSurfaceShape.SetCoordSys(GetCoordSys());
            }

        RegionToDraw.Intersect(DestSurfaceShape);
        }

    // Standard draw code, one image at a time, bottom up
    if (!DrawDone)
        {
        // Take a copy of the shape to avoid modifying the original shape
        HVEShape RegionToDrawInCs(RegionToDraw);
        RegionToDrawInCs.ChangeCoordSys(GetCoordSys());

        HAutoPtr< IndexType::IndexableList > pObjects(m_pIndex->QueryIndexables(
                                                          HIDXSearchCriteria(m_pIndex->GetIndex2(), new HGFSpatialCriteria(RegionToDrawInCs.GetExtent())), true));
        IndexType::IndexableList::const_iterator Itr(pObjects->begin());

        while (Itr != pObjects->end())
            {
            // Calculate the needed visible part of this image
            HFCPtr<HVEShape> pVisibleShape(new HVEShape(RegionToDraw));
            pVisibleShape->Intersect(*m_pIndex->GetIndex1()->GetVisibleSurfaceOf(*Itr));

            // Send part to the image, if there is one...
            if (!pVisibleShape->IsEmpty())
                {
                if (pReplacingCS != 0)
                    {
                    // Adapt replacing CS so that it works on the current image.

                    // Extract the transformation we're applying to our source (T1)
                    HFCPtr<HGF2DTransfoModel> pRefTransfo(pReplacingCS->GetTransfoModelTo(GetCoordSys()));

                    // Calculate the transformation between our source's CS and the CS
                    // the object to return uses. (T2)
                    HFCPtr<HGF2DTransfoModel> pCurrentToSource((*Itr)->GetObject()->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

                    // Create the CS by composing everything correctly :)
                    // T2 o T1 o -T2
                    HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToSource->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToSource));
                    HFCPtr<HGF2DCoordSys> pCSToApply = new HGF2DCoordSys(*pToApply, (*Itr)->GetObject()->GetCoordSys());

                    Options.SetReplacingCoordSys(pCSToApply);
                    }

                Options.SetShape(pVisibleShape);
                (*Itr)->GetObject()->Draw(pio_destSurface, Options);
                }

            // Proceed to the next element
            ++Itr;
            }
        }
    }

