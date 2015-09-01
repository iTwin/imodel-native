//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAUnlimitedResolutionRaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAUnlimitedResolutionRaster
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCExceptionHandler.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAStripedRaster.h>
#include <Imagepp/all/h/HRATiledRaster.h>
#include <Imagepp/all/h/HRAUnlimitedResolutionRaster.h>
#include <Imagepp/all/h/HRAUnlimitedResolutionRasterIterator.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <ImagePP/all/h/HRAClearOptions.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HGFMappedSurface.h>
#include <ImagePP/all/h/HMDContext.h>
#include <ImagePPInternal/gra/HRAImageNode.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>



HPM_REGISTER_CLASS(HRAUnlimitedResolutionRaster, HRAStoredRaster)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRAUnlimitedResolutionRaster, HRAStoredRaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRAUnlimitedResolutionRaster, HRAContentChangedMsg, NotifyContentChanged)
HMG_REGISTER_MESSAGE(HRAUnlimitedResolutionRaster, HRFProgressImageChangedMsg, NotifyProgressImageChanged)
HMG_REGISTER_MESSAGE(HRAUnlimitedResolutionRaster, HRAProgressImageChangedMsg, NotifyHRAProgressImageChanged)
HMG_REGISTER_MESSAGE(HRAUnlimitedResolutionRaster, HGFGeometryChangedMsg, NotifyGeometryChanged)
HMG_REGISTER_MESSAGE(HRAUnlimitedResolutionRaster, HRAEffectiveShapeChangedMsg, NotifyEffectiveShapeChanged)
HMG_END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HRAUnlimitedResolutionRaster::HRAUnlimitedResolutionRaster()
    : HRAStoredRaster()
    {
    m_pLog              = 0;
    m_pTiledRaster      = 0;
    m_CurResolution     = 0.0;

    m_HistogramEditMode = false;
    m_SinglePixelType   = true;
    m_LookAheadEnabled  = false;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRAUnlimitedResolutionRaster::HRAUnlimitedResolutionRaster(HFCPtr<HRATiledRaster>& pi_pRasterModel,
                                                           uint64_t                pi_WidthPixels,
                                                           uint64_t                pi_HeightPixels,
                                                           uint64_t                pi_MinWidth,
                                                           uint64_t                pi_MinHeight,
                                                           uint64_t                pi_MaxWidth,
                                                           uint64_t                pi_MaxHeight,
                                                           HPMObjectStore*          pi_pStore,
                                                           HPMPool*                 pi_pLog)

    : HRAStoredRaster(pi_WidthPixels, pi_HeightPixels,
                      pi_pRasterModel->GetTransfoModel(),
                      pi_pRasterModel->GetCoordSys(),
                      pi_pRasterModel->GetPixelType())
    {
    HPRECONDITION(pi_pStore == 0 || pi_pStore->IsCompatibleWith(HRSObjectStore::CLASS_ID));

    SetStore((HRSObjectStore*)pi_pStore);
    m_pLog              = pi_pLog;
    m_HistogramEditMode = false;
    m_SinglePixelType   = true;
    m_LookAheadEnabled  = false;
    m_CurResolution     = 1.0;

    Constructor(pi_pRasterModel,
                pi_WidthPixels,
                pi_HeightPixels,
                pi_MinWidth,
                pi_MinHeight,
                pi_MaxWidth,
                pi_MaxHeight);
    }



//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRAUnlimitedResolutionRaster::HRAUnlimitedResolutionRaster(const HRAUnlimitedResolutionRaster& pi_rObj)
    : HRAStoredRaster(pi_rObj)
    {
    // Perform initialization of the object
    m_pLog          = 0;
    m_HistogramEditMode = false;
    m_SinglePixelType = true;
    m_LookAheadEnabled = false;
    m_CurResolution = 0.0;

    DeepCopy(pi_rObj);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRAUnlimitedResolutionRaster::~HRAUnlimitedResolutionRaster()
    {
    try
        {
        // Artificially increment our reference count because
        // the message will hold a pointer to us, and this pointer
        // will call our destructor again when the message is
        // deleted.
        IncrementRef();

        // Notify the Store
        // Really needed, because this message will update the Cache/SisterFile
        //
        Propagate(HRAPyramidRasterClosingMsg());

        DecrementRef();

        DeepDelete ();
        }
    catch(...)
        {
        HFCExceptionHandler::HandleException();
        }
    }


//-----------------------------------------------------------------------------
// public
// Assignment operation
//-----------------------------------------------------------------------------
HRAUnlimitedResolutionRaster& HRAUnlimitedResolutionRaster::operator=(const HRAUnlimitedResolutionRaster& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        HRAStoredRaster::operator=(pi_rObj);

        // Delete currently allocated memory for the object
        DeepDelete ();

        m_pLog              = 0;
        m_LookAheadEnabled  = false;

        // Perform initialization of the object
        DeepCopy (pi_rObj);
        }

    return *this;
    }




//-----------------------------------------------------------------------------
// public
// InitPhysicalShape - Init the physical shape, set a new shape and flush
//                     the previous Data.
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels)
    {
    HPRECONDITION(pi_WidthPixels <= m_MaxWidth && pi_HeightPixels <= m_MaxHeight);

    HRAStoredRaster::InitSize(pi_WidthPixels, pi_HeightPixels);
    m_pTiledRaster->HRAStoredRaster::SetTransfoModel (*GetTransfoModel(), GetCoordSys());

    m_pCurrentResolutionRaster = static_cast<HRATiledRaster*>(m_pTiledRaster->Clone(GetStore(), m_pLog).GetPtr());
    m_pCurrentResolutionRaster->SetContext(GetContext());
    m_CurResolution = 1.0;

    m_pCurrentResolutionRaster->SetInternalTileStatusSupported(false);

    m_pCurrentResolutionRaster->InitSize(pi_WidthPixels,
                                         pi_HeightPixels);
    }


//-----------------------------------------------------------------------------
// public
// SetShape
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::SetShape(const HVEShape& pi_rShape)
    {
    HRAStoredRaster::SetShape(pi_rShape);

    m_pCurrentResolutionRaster->SetShape(pi_rShape);
    }


//-----------------------------------------------------------------------------
// protected
// SetCoordSysImplementation
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys)
    {
    // Ancestor before...
    HRAStoredRaster::SetCoordSysImplementation(pi_rOldCoordSys);

    // Set model == to the TiledRaster
    m_pTiledRaster->SetCoordSys (GetCoordSys());

    m_pCurrentResolutionRaster->SetCoordSys(GetCoordSys());
    }

//-----------------------------------------------------------------------------
// public
// SetTransfoModel
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::SetTransfoModel(const HGF2DTransfoModel& pi_rModelCSp_CSl)
    {
    // Set model == to the TiledRaster
    m_pTiledRaster->SetTransfoModel(pi_rModelCSp_CSl);

    // Create scaling transfo
    // Set scaling to the SubImages.
    HGF2DStretch Transfo;
    Transfo.SetXScaling(1.0 / m_CurResolution);
    Transfo.SetYScaling(1.0 / m_CurResolution);

    // Set a new physical coord. sys. to the buffer
    m_pCurrentResolutionRaster->SetTransfoModel(*Transfo.ComposeInverseWithDirectOf(pi_rModelCSp_CSl));

    // Call the parent
    HRAStoredRaster::SetTransfoModel(pi_rModelCSp_CSl);
    }


//-----------------------------------------------------------------------------
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRAUnlimitedResolutionRaster::GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short NumberOfEntries = HRAStoredRaster::GetRepresentativePalette(pio_pRepPalParms);

    // if no operation has been done at the parent level or if the cache is not
    // updated
    if (NumberOfEntries == 0)
        {
        if (pio_pRepPalParms->GetSamplingOptions().GetPyramidImageSize() != 0)
            {
            // the method does not update its representative palette cache because
            // it gets its palette directly from the smaller raster

            double Resolution = ((double)pio_pRepPalParms->GetSamplingOptions().GetPyramidImageSize() / 100.0);
            ChangeResolution(Resolution);

            // set the look ahead
            if (HasLookAhead())
                {
                SetLookAhead(HVEShape(0,
                                      0,
                                      1,
                                      1,
                                      m_pCurrentResolutionRaster->GetPhysicalCoordSys()),
                             false);
                }

            // get the representative palette on the smaller raster
            NumberOfEntries = m_pCurrentResolutionRaster->GetRepresentativePalette(pio_pRepPalParms);
            }
        else
            {
            HASSERT(0);

            // use the Resolution 1.0
            ChangeResolution(1.0);

            // set the look ahead
            if (HasLookAhead())
                {
                SetLookAhead(HVEShape(0,
                                      0,
                                      1,
                                      1,
                                      m_pCurrentResolutionRaster->GetPhysicalCoordSys()),
                             false);
                }

            // get the representative palette on the smaller raster
            NumberOfEntries = m_pCurrentResolutionRaster->GetRepresentativePalette(pio_pRepPalParms);

            }
        }
    return NumberOfEntries;
    }

//-----------------------------------------------------------------------------
// ComputeHistogram
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                                    bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    if (pi_ForceRecompute || (GetHistogram() == 0) || (GetHistogram() != 0 && !GetHistogram()->CanBeUsedInPlaceOf(*pio_pOptions)))
        {
        HASSERT(pio_pOptions->GetSamplingOptions().GetPyramidImageSize() > 0);

        double Resolution = (double)sqrt(pio_pOptions->GetSamplingOptions().GetPyramidImageSize() / 100.0);

        ChangeResolution(Resolution);

        // set the look ahead
        if (HasLookAhead())
            {
            HFCPtr<HVEShape> pLookAheadRegion;

            if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
                {
                pLookAheadRegion = new HVEShape(*pio_pOptions->
                                                GetSamplingOptions().GetRegionToScan());
                }
            else
                {
                pLookAheadRegion = (new HVEShape(m_pCurrentResolutionRaster->
                                                 GetPhysicalExtent()));
                }

            SetLookAhead(*pLookAheadRegion, false);
            }

        // compute the histogram
        m_pCurrentResolutionRaster->ComputeHistogram(pio_pOptions, pi_ForceRecompute);

        // set histogram into HRARaster
        if (pio_pOptions->GetSamplingOptions().GetRegionToScan() == 0)
            SetHistogram(pio_pOptions);
        }
    else
        {
        HPRECONDITION(GetHistogram()->GetHistogram()->GetEntryFrequenciesSize() <=
                      pio_pOptions->GetHistogram()->GetEntryFrequenciesSize());

        HFCPtr<HRPHistogram> pHisto(new HRPHistogram(*GetHistogram()->GetHistogram()));
        HFCPtr<HRPHistogram> pOutHisto(pio_pOptions->GetHistogram());
        uint32_t HistogramChannels = pOutHisto->GetChannelCount();

        HASSERT(pHisto->GetChannelCount() == pOutHisto->GetChannelCount());
        // add each entry in the histogram
        for(uint32_t ChannelIndex = 0; ChannelIndex < HistogramChannels; ChannelIndex++)
            {
            for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize(); i++)
                pOutHisto->IncrementEntryCount(i, pHisto->GetEntryCount(i,ChannelIndex),ChannelIndex);
            }
        }
    }


//-----------------------------------------------------------------------------
// StartHistogramEditMode
//
// Start editing actual global histogram.
// Update will be done constantly on current histogram, so this mode cannot be started if
// no histogram is available (return false) or will cause malfunctioning if current
// histogram is not up-to-date. Be sure to call ComputeHistogram() if necessary before
// calling this method.
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::StartHistogramEditMode()
    {
    bool Result = false;
    if (!m_HistogramEditMode)
        {
        if (GetHistogram()) // we need an histogram to work on
            {
            // Set mode to ON
            m_HistogramEditMode = true;

            // Compute all tile histograms (on full resolution)
            ChangeResolution(1.0);
            m_pCurrentResolutionRaster->EngageTileHistogramComputing();

            Result = true;
            }
        }
    else // mode is already ON
        Result = true;

    return Result;
    }

//-----------------------------------------------------------------------------
// Public
// Pass the request to the source
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::SetLookAhead(const HVEShape& pi_rShape,
                                                    uint32_t        pi_ConsumerID,
                                                    bool           pi_Async)
    {
    HPRECONDITION(HasLookAhead());

    // find the best resolution for the coord sys of the extent
    double Resolution = FindTheBestResolution(pi_rShape.GetCoordSys());

    ChangeResolution(Resolution);
    // Bring the extent to the found resolution's coord sys
    HFCPtr<HVEShape> pShape(new HVEShape(pi_rShape));
    pShape->ChangeCoordSys(m_pCurrentResolutionRaster->GetPhysicalCoordSys());

    m_pCurrentResolutionRaster->SetLookAhead(*pShape, pi_ConsumerID, pi_Async);
    }

//-----------------------------------------------------------------------------
// public
// NotifyContentChanged - Message Handler
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::NotifyContentChanged(const HMGMessage& pi_rMessage)
    {
    HRAStoredRaster* pRaster = ((HRAStoredRaster*)pi_rMessage.GetSender());

    HPRECONDITION(pRaster == m_pCurrentResolutionRaster);
    if (pRaster == m_pCurrentResolutionRaster)
        {
        if (m_HistogramEditMode)
            {
            // Update histogram
            // (To keep good performances we only compute tiles histogram "delta" (before vs now) and
            //  apply this to main histogram)
            HFCPtr<HVEShape> pShape(new HVEShape(((HRAContentChangedMsg&)pi_rMessage).GetShape()));
            pShape->ChangeCoordSys(m_pCurrentResolutionRaster->GetPhysicalCoordSys());

            // m_HistogramEditMode can be true only if m_pCurrentResolutionRaster is a HRATiledRaster
            HFCPtr<HRATiledRaster> pTiledRaster = m_pCurrentResolutionRaster;

            HAutoPtr<HRARasterIterator> pRasterIterator(pTiledRaster->CreateIterator(HRAIteratorOptions(pShape,
                                                                                     pTiledRaster->GetPhysicalCoordSys(),
                                                                                     false)));

            if (pRasterIterator != 0)
                {
                HFCPtr<HRARaster> pTile;

                while ((pTile = (*pRasterIterator)()) != 0)
                    {
                    // Tile histograms (old and new)
                    HRAHistogramOptions OldTileHistogramOptions(pTile->GetPixelType());
                    HRAHistogramOptions NewTileHistogramOptions(pTile->GetPixelType());

                    // Main histogram
                    HRAHistogramOptions NewPyramidHistogramOptions(*(GetHistogram()));

                    // All histo must be NATIVE
                    HASSERT(OldTileHistogramOptions.GetSamplingColorSpace() == HRPHistogram::NATIVE &&
                            NewTileHistogramOptions.GetSamplingColorSpace() == HRPHistogram::NATIVE &&
                            NewPyramidHistogramOptions.GetSamplingColorSpace() == HRPHistogram::NATIVE);

                    // Set quality to 100%
                    HRASamplingOptions SamplingOpt;
                    SamplingOpt.SetPixelsToScan(100);
                    SamplingOpt.SetTilesToScan(100);
                    SamplingOpt.SetPyramidImageSize(100);

                    OldTileHistogramOptions.SetSamplingOptions(SamplingOpt);
                    NewTileHistogramOptions.SetSamplingOptions(SamplingOpt);

                    pTile->ComputeHistogram(&OldTileHistogramOptions);       // Get current tile histogram
                    pTile->ComputeHistogram(&NewTileHistogramOptions, true); // true -> Recompute histogram

                    uint32_t ChannelCount(OldTileHistogramOptions.GetHistogram()->GetChannelCount());
                    for (uint32_t ChannelIndex(0); ChannelIndex < ChannelCount; ++ChannelIndex)
                        {
                        uint32_t Entries = OldTileHistogramOptions.GetHistogram()->GetEntryFrequenciesSize(ChannelIndex);

                        // Compute histogram delta (New - Old)
                        for (uint32_t Index(0); Index < Entries; ++Index)
                            {
                            int32_t Delta = NewTileHistogramOptions.GetHistogram()->GetEntryCount(Index, ChannelIndex) -
                                           OldTileHistogramOptions.GetHistogram()->GetEntryCount(Index, ChannelIndex);

                            uint32_t Value = NewPyramidHistogramOptions.GetHistogram()->GetEntryCount(Index, ChannelIndex);

                            // Check for an invalid entry count that could be caused by a corrupted file histogram
                            HASSERT(!( (Delta < 0) && ((((uint32_t)Delta) * -1) > Value) ));

                            // Avoid inconsistence
                            if (!( (Delta < 0) && ((((uint32_t)Delta) * -1) > Value) ))
                                Value += Delta; // OK
                            else
                                Value = 0; // This means pyramid histogram was not correct!

                            NewPyramidHistogramOptions.GetHistogram()->SetEntryCount(Index, Value, ChannelIndex);
                            }
                        }

                    SetHistogram(&NewPyramidHistogramOptions);

                    pRasterIterator->Next();
                    }
                }
            }
        }

    SetModificationState();

    return true;
    }


//-----------------------------------------------------------------------------
// public
// NotifyProgressImageChanged - Message Handler
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::NotifyProgressImageChanged(const HMGMessage& pi_rMessage)
    {
    
    uint32_t Res = ((HRFProgressImageChangedMsg&)pi_rMessage).GetSubResolution();
    HPRECONDITION(Res == 0);

    m_pCurrentResolutionRaster->NotifyProgressImageChanged (pi_rMessage);
    SetModificationState();
    return false;
    }


//-----------------------------------------------------------------------------
// public
// NotifyHRAContentChanged - Message Handler
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::NotifyHRAProgressImageChanged(const HMGMessage& pi_rMessage)
    {
    // Nothing to do. Overrides the ContentChanged message.
    // This handling has been made in the HRFProgressImageChanged
    // message handling.
    return true;
    }


//-----------------------------------------------------------------------------
// Receive a shape changed notification
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::NotifyEffectiveShapeChanged(const HMGMessage& pi_rMessage)
    {
    // Stop Message...
    return false;
    }


//-----------------------------------------------------------------------------
// Receive a geometry changed notification
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::NotifyGeometryChanged(const HMGMessage& pi_rMessage)
    {
    // Stop Message...
    return false;
    }



//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::CopyFromLegacy(const HFCPtr<HRARaster>&    pi_pSrcRaster, const HRACopyFromLegacyOptions&   pi_rOptions)
    {

    HRAStoredRaster::CopyFromLegacy(pi_pSrcRaster, pi_rOptions);
    }

//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    // find the best resolution for the surface
    double Resolution = FindTheBestResolution(pio_destSurface.GetCoordSys(), pi_Options.GetReplacingCoordSys());
    (const_cast<HRAUnlimitedResolutionRaster*>(this))->ChangeResolution(Resolution);
    
    // Draw the selected resolution
    m_pCurrentResolutionRaster->Draw(pio_destSurface, pi_Options);
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::Constructor(const HFCPtr<HRATiledRaster>&    pi_rpRasterModel,
                                               uint64_t                         pi_WidthPixels,
                                               uint64_t                         pi_HeightPixels,
                                               uint64_t                         pi_MinWidth,
                                               uint64_t                         pi_MinHeight,
                                               uint64_t                         pi_MaxWidth,
                                               uint64_t                         pi_MaxHeight)
    {
    HPRECONDITION (pi_rpRasterModel != 0);

    // Exception-Safe
    try
        {
        m_MinWidth = pi_MinWidth;
        m_MinHeight = pi_MinHeight;
        m_MaxWidth = pi_MaxWidth;
        m_MaxHeight = pi_MaxHeight;

        // Make a copy of the RasterModel.
        // Set CoordSys and Extent (1,1)
        //
        m_pTiledRaster = (HRATiledRaster*)pi_rpRasterModel->Clone();
        m_pTiledRaster->HRAStoredRaster::SetTransfoModel (*GetTransfoModel (), GetCoordSys ());
        m_pTiledRaster->InitSize (1, 1);
        m_pCurrentResolutionRaster = static_cast<HRATiledRaster*>(m_pTiledRaster->Clone(GetStore(), m_pLog).GetPtr());
        m_pCurrentResolutionRaster->SetContext(GetContext());
        m_pCurrentResolutionRaster->SetID(HRSObjectStore::ID_TiledRaster);

        m_pCurrentResolutionRaster->SetInternalTileStatusSupported(false);

        m_pCurrentResolutionRaster->InitSize(pi_WidthPixels,
                                             pi_HeightPixels);

        // calculate the minnest scalling supported
        m_MinResolution = MIN((double)pi_MinWidth / (double)pi_WidthPixels, (double)pi_MinHeight / (double)pi_HeightPixels);

        // calculate the bigger scaling supported
        m_MaxResolution = MIN((double)pi_MaxWidth / (double)pi_WidthPixels, (double)pi_MaxHeight / (double)pi_HeightPixels);

        LinkTo (m_pCurrentResolutionRaster);

        }
    catch(...)
        {
        DeepDelete();
        }
    }


//-----------------------------------------------------------------------------
// private
// FindTheBestResolution  -
// This function studies the scaling factor around the raster to estimate
// the resolution that must be used to satisfy the granularity of the
// provided coordinate system.
//-----------------------------------------------------------------------------
double HRAUnlimitedResolutionRaster::FindTheBestResolution(const HFCPtr<HGF2DCoordSys>& pi_rPhysicalCoordSys,
                                                            const HFCPtr<HGF2DCoordSys>  pi_pNewLogicalCoordSys) const
    {
    double            Res(1.0);

    // Do you have sub-resolution ?
    if (pi_rPhysicalCoordSys != 0)
        {
        double            ScaleFactorX;
        double            ScaleFactorY;
        HGF2DDisplacement  Displacement;

        HFCPtr<HGF2DTransfoModel> pTransfo;
        if (pi_pNewLogicalCoordSys != 0)
            {
            pTransfo = pi_rPhysicalCoordSys->GetTransfoModelTo (pi_pNewLogicalCoordSys);
            pTransfo = pTransfo->ComposeInverseWithDirectOf(*GetCoordSys()->GetTransfoModelTo(GetPhysicalCoordSys()));
            }
        else
            {
            pTransfo = pi_rPhysicalCoordSys->GetTransfoModelTo (GetPhysicalCoordSys());
            }

        // Check if we can extract Scaling
        // Here we check if the model represents a parallelism preserving relation
        // With parallelism preserving relation, even though the model cannot be properly
        // represented by a stretch, insures that the scaling factors are not dependant upon the
        // position into space.
        if (pTransfo->PreservesParallelism())
            {
            pTransfo->GetStretchParams (&ScaleFactorX, &ScaleFactorY, &Displacement);
            Res = 1.0 / MIN (fabs(ScaleFactorX), fabs(ScaleFactorY));
            }
        else
            {
            // The model does not preserve parallelism, therefore the scaling factors
            // are dependant upon the position into space of the application
            // Fortunately, we know where the current pyramid raster is located

            // We will simply use the extent
            HGF2DExtent RasterExtent(GetExtent());

            // Check if raster has a size
            if (RasterExtent.IsDefined())
                {
                // Raster is sizeable
                // Obtain extent four corners
                HGF2DLocation LowerLeft(RasterExtent.GetXMin(), RasterExtent.GetYMin(), RasterExtent.GetCoordSys());
                HGF2DLocation LowerRight(RasterExtent.GetXMax(), RasterExtent.GetYMin(), RasterExtent.GetCoordSys());
                HGF2DLocation UpperLeft(RasterExtent.GetXMin(), RasterExtent.GetYMax(), RasterExtent.GetCoordSys());
                HGF2DLocation UpperRight(RasterExtent.GetXMax(), RasterExtent.GetYMax(), RasterExtent.GetCoordSys());

                // Change to the raster physical coordinate system
                LowerLeft.ChangeCoordSys(GetPhysicalCoordSys());
                LowerRight.ChangeCoordSys(GetPhysicalCoordSys());
                UpperLeft.ChangeCoordSys(GetPhysicalCoordSys());
                UpperRight.ChangeCoordSys(GetPhysicalCoordSys());

                HGF2DLocation OtherLowerLeft(LowerLeft, GetCoordSys());
                HGF2DLocation OtherLowerRight(LowerRight, GetCoordSys());
                HGF2DLocation OtherUpperLeft(UpperLeft, GetCoordSys());
                HGF2DLocation OtherUpperRight(UpperRight, GetCoordSys());

                if (pi_pNewLogicalCoordSys != 0)
                    {
                    OtherLowerLeft.SetCoordSys(pi_pNewLogicalCoordSys);
                    OtherLowerRight.SetCoordSys(pi_pNewLogicalCoordSys);
                    OtherUpperLeft.SetCoordSys(pi_pNewLogicalCoordSys);
                    OtherUpperRight.SetCoordSys(pi_pNewLogicalCoordSys);
                    }

                OtherLowerLeft.ChangeCoordSys(pi_rPhysicalCoordSys);
                OtherLowerRight.ChangeCoordSys(pi_rPhysicalCoordSys);
                OtherUpperLeft.ChangeCoordSys(pi_rPhysicalCoordSys);
                OtherUpperRight.ChangeCoordSys(pi_rPhysicalCoordSys);

                // Create these four corners into other coordinate system

                // Compute scales

                // Extent may not be undefined
                HASSERT((LowerLeft - LowerRight).CalculateLength() != 0.0);
                HASSERT((UpperLeft - UpperRight).CalculateLength() != 0.0);
                HASSERT((UpperLeft - LowerLeft).CalculateLength() != 0.0);
                HASSERT((UpperRight - LowerRight).CalculateLength() != 0.0);

                double LowerScale = (OtherLowerLeft - OtherLowerRight).CalculateLength() / (LowerLeft - LowerRight).CalculateLength();
                double UpperScale  = (OtherUpperLeft - OtherUpperRight).CalculateLength() / (UpperLeft - UpperRight).CalculateLength();
                double LeftScale = (OtherUpperLeft - OtherLowerLeft).CalculateLength() / (UpperLeft - LowerLeft).CalculateLength();
                double RightScale = (OtherUpperRight - OtherLowerRight).CalculateLength() / (UpperRight - LowerRight).CalculateLength();

                double Scale = MAX(fabs(LowerScale), MAX(fabs(UpperScale), MAX(fabs(LeftScale), fabs(RightScale))));

                HASSERT(Scale != 0.0);

                Res = Scale;
                }
            else
                {
                // Raster has no size
                // Give anything valid
                Res = 1.0;
                }
            }
        }

    return (Res);
    }


//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::DeepCopy(const HRAUnlimitedResolutionRaster&    pi_rRaster,
                                            HPMObjectStore*                        pi_pStore,
                                            HPMPool*                               pi_pLog)
    {
    HPRECONDITION(pi_pStore == 0 || pi_pStore->IsCompatibleWith(HRSObjectStore::CLASS_ID));

    // Exception-Safe
    try
        {
        m_MinWidth = pi_rRaster.m_MinWidth;
        m_MinHeight = pi_rRaster.m_MinHeight;
        m_MaxWidth = pi_rRaster.m_MaxWidth;
        m_MaxHeight = pi_rRaster.m_MaxHeight;

        // Copy the model
        m_pTiledRaster = (HRATiledRaster*)pi_rRaster.m_pTiledRaster->Clone();

        m_SinglePixelType = pi_rRaster.m_SinglePixelType;

        SetStore((HRSObjectStore*)pi_pStore);
        m_pLog          = pi_pLog;

        m_pCurrentResolutionRaster  = static_cast<HRATiledRaster*>(pi_rRaster.m_pCurrentResolutionRaster->Clone (pi_pStore, pi_pLog).GetPtr());
        // Link ourselves to the Sub-Resolution Raster, to receive notifications
        LinkTo(m_pCurrentResolutionRaster);
        }
    catch(...)
        {
        DeepDelete();
        }
    }


//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::DeepDelete()
    {
    }


//-----------------------------------------------------------------------------
// private
// ChangeResolution
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::ChangeResolution(double pi_NewResolution)
    {
    // change resolution if
    //  - the new resolution is different of the current resolution
    //  - the new resolution is smaller than the minimum resolution and the current resolution is different of the MIN resolution
    //  - the new resolution is greater than the maximum resolution and the current resolution is different of the MAX resolution
    if (!HDOUBLE_EQUAL_EPSILON(m_CurResolution, pi_NewResolution))
        {
        bool ChangeResolution = false;
        if (pi_NewResolution < m_MinResolution && m_CurResolution != m_MinResolution)
            {
            m_CurResolution = m_MinResolution;
            ChangeResolution = true;
            }
        else if (pi_NewResolution > m_MaxResolution && m_CurResolution != m_MaxResolution)
            {
            m_CurResolution = m_MaxResolution;
            ChangeResolution = true;
            }
        else if (pi_NewResolution >= m_MinResolution && pi_NewResolution <= m_MaxResolution)
            {
            m_CurResolution = pi_NewResolution;
            ChangeResolution = true;
            }

        if (ChangeResolution)
            {
            HPRECONDITION(GetStore() != 0);
            uint64_t   Width;
            uint64_t   Height;

            HFCPtr<HGF2DTransfoModel> pNewModel;

            ((HRSObjectStore*)GetStore())->ChangeResolution(0, m_CurResolution, &Width, &Height);

            HGF2DExtent  PhysicalExtent = GetPhysicalExtent();
            HGF2DStretch TransfoScale;

            //TR 251344 - Ensure that the size of the resolution in physical units (i.e. : meter)
            //            is equal to the size of the first resolution (i.e. : HRFResolutionDescriptor).
            TransfoScale.SetXScaling(PhysicalExtent.GetWidth() / Width);
            TransfoScale.SetYScaling(PhysicalExtent.GetHeight() / Height);

            pNewModel = TransfoScale.ComposeInverseWithDirectOf(*GetTransfoModel());


            m_pCurrentResolutionRaster = static_cast<HRATiledRaster*>(m_pTiledRaster->Clone(GetStore(), m_pLog).GetPtr());
            m_pCurrentResolutionRaster->SetID(HRSObjectStore::ID_TiledRaster);
            m_pCurrentResolutionRaster->SetTransfoModel(*pNewModel);

            m_pCurrentResolutionRaster->SetContext(GetContext());

            m_pCurrentResolutionRaster->SetInternalTileStatusSupported(false);

            m_pCurrentResolutionRaster->InitSize(Width, Height);
            LinkTo (m_pCurrentResolutionRaster);


            // TR #181278
            if (m_LookAheadEnabled)
                m_pCurrentResolutionRaster->EnableLookAhead(m_LookAheadByBlockEnabled);
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// CreateSubImage
//
// This method is called by HRAUnlimitedResolutionRasterIterator
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRAUnlimitedResolutionRaster::CreateSubImage(double pi_Resolution) const
    {
    HPRECONDITION(HDOUBLE_GREATER_EPSILON(pi_Resolution, 0.0));

    // create a raster for the resolution
    HFCPtr<HRATiledRaster> pSubImageRaster = (HRATiledRaster*)m_pTiledRaster->Clone();

    HGF2DStretch TransfoScale;
    TransfoScale.SetXScaling(1.0 / pi_Resolution);
    TransfoScale.SetYScaling(1.0 / pi_Resolution);

    uint64_t Width;
    uint64_t Height;
    GetSize(&Width, &Height);
    Width  = (uint64_t)(Width  * pi_Resolution);
    Height = (uint64_t)(Height * pi_Resolution);

    m_pCurrentResolutionRaster->SetInternalTileStatusSupported(false);

    pSubImageRaster->InitSize(Width, Height);
    pSubImageRaster->HRAStoredRaster::SetTransfoModel(*TransfoScale.ComposeInverseWithDirectOf(*GetTransfoModel()), GetCoordSys());

    return (HFCPtr<HRARaster>)pSubImageRaster;
    }

//-----------------------------------------------------------------------------
// public
// Clear - Clear bitmap
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::Clear()
    {
    HRAClearOptions ClearOptions;
    ((HFCPtr<HRARaster>&)m_pCurrentResolutionRaster)->Clear(ClearOptions);
    }

//-----------------------------------------------------------------------------
// public
// Clear - Clear a region in a bitmap
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::Clear(const HRAClearOptions& pi_rOptions)
    {
    ((HFCPtr<HRARaster>&)m_pCurrentResolutionRaster)->Clear(pi_rOptions);
    }


//-----------------------------------------------------------------------------
// Public
// Passed the request to the source
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::HasLookAhead() const
    {
    return (m_LookAheadEnabled);
    }

//-----------------------------------------------------------------------------
// Public
// Passed the request to the source
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::InvalidateRaster()
    {
    return m_pCurrentResolutionRaster->InvalidateRaster();
    }

//-----------------------------------------------------------------------------
// Public
// Passed the request to the source
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    HRAStoredRaster::SetContext(pi_rpContext);
    m_pCurrentResolutionRaster->SetContext(pi_rpContext);
    }

//-----------------------------------------------------------------------------
// public
// CreateIterator    - Create an iterator.
//-----------------------------------------------------------------------------
HRARasterIterator* HRAUnlimitedResolutionRaster::CreateIterator (const HRAIteratorOptions& pi_rOptions) const
    {
    return new HRAUnlimitedResolutionRasterIterator(HFCPtr<HRAUnlimitedResolutionRaster>((HRAUnlimitedResolutionRaster*)this),
                                                    pi_rOptions,
                                                    FindTheBestResolution(pi_rOptions.GetPhysicalCoordSys()));
    }


//-----------------------------------------------------------------------------
// public
// Create an editor
//-----------------------------------------------------------------------------
HRARasterEditor* HRAUnlimitedResolutionRaster::CreateEditor (HFCAccessMode pi_Mode)
    {
    return 0;
    }


//-----------------------------------------------------------------------------
// public
// Create a shaped editor
//-----------------------------------------------------------------------------
HRARasterEditor* HRAUnlimitedResolutionRaster::CreateEditor (const HVEShape& pi_rShape,
                                                                    HFCAccessMode  pi_Mode)
    {
    return 0;
    }



HRARasterEditor* HRAUnlimitedResolutionRaster::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return 0;
    }


//-----------------------------------------------------------------------------
// public
// Clone -
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRAUnlimitedResolutionRaster::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    // Make a tmp Raster
    HRAUnlimitedResolutionRaster* pTmpRaster = new HRAUnlimitedResolutionRaster();

    pTmpRaster->HRAStoredRaster::operator=(*this);

    pTmpRaster->DeepDelete();
    pTmpRaster->DeepCopy (*this, pi_pStore, pi_pLog);

    return pTmpRaster;
    }

//-----------------------------------------------------------------------------
// Return a new copy of self
//-----------------------------------------------------------------------------
HPMPersistentObject* HRAUnlimitedResolutionRaster::Clone () const
    {
    return new HRAUnlimitedResolutionRaster(*this);
    }


//-----------------------------------------------------------------------------
// public
// HasSinglePixeltype -
//-----------------------------------------------------------------------------
bool HRAUnlimitedResolutionRaster::HasSinglePixelType  () const
    {
    return m_SinglePixelType;
    }


//-----------------------------------------------------------------------------
// public
// GetMaxResolutionSize
//
// Get the maximum size that a resolution can have.
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::GetMaxResolutionSize(uint64_t* po_pMaxWidth,
                                                               uint64_t* po_pMaxHeight)
    {
    HPRECONDITION(po_pMaxWidth != 0 && po_pMaxHeight != 0);

    *po_pMaxWidth = m_MaxWidth;
    *po_pMaxHeight = m_MaxHeight;
    }



//-----------------------------------------------------------------------------
// protected
// EnableLookAhead
//-----------------------------------------------------------------------------
void HRAUnlimitedResolutionRaster::EnableLookAhead(bool pi_ByBlock)
    {
    HASSERT(GetStore() != 0);

    m_LookAheadEnabled = true;
    m_LookAheadByBlockEnabled = pi_ByBlock;

    m_pCurrentResolutionRaster->EnableLookAhead(pi_ByBlock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAUnlimitedResolutionRaster::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    // find the best resolution for the surface
    double Resolution = FindTheBestResolution(imageNode.GetPhysicalCoordSys(), options.GetReplacingCoordSys());
    ChangeResolution(Resolution);

    // Draw the selected resolution
    return m_pCurrentResolutionRaster->BuildCopyToContext(imageNode, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSinkNodePtr HRAUnlimitedResolutionRaster::_GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType)
    {
    // We do not support writing of unlimitedRaster.
    status = IMAGEPP_STATUS_NoImplementation;
    return NULL;
    }