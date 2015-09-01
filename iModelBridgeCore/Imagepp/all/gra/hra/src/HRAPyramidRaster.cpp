//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAPyramidRaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAPyramidRaster
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAPyramidRaster.h>
#include <Imagepp/all/h/HRAPyramidRasterIterator.h>
#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRATiledRaster.h>
#include <Imagepp/all/h/HRAStripedRaster.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HFCExceptionHandler.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HRAUpdateSubResProgressIndicator.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRPQuantizedPalette.h>
#include <ImagePP/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRATransaction.h>
#include <Imagepp/all/h/HRABlitter.h>
#include <ImagePPInternal/gra/HRAImageNode.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>

HPM_REGISTER_CLASS(HRAPyramidRaster, HRAStoredRaster)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRAPyramidRaster, HRAStoredRaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRAPyramidRaster, HRAContentChangedMsg, NotifyContentChanged)
HMG_REGISTER_MESSAGE(HRAPyramidRaster, HRFProgressImageChangedMsg, NotifyProgressImageChanged)
HMG_REGISTER_MESSAGE(HRAPyramidRaster, HRAProgressImageChangedMsg, NotifyHRAProgressImageChanged)
HMG_REGISTER_MESSAGE(HRAPyramidRaster, HGFGeometryChangedMsg, NotifyGeometryChanged)
HMG_REGISTER_MESSAGE(HRAPyramidRaster, HRAEffectiveShapeChangedMsg, NotifyEffectiveShapeChanged)
HMG_REGISTER_MESSAGE(HRAPyramidRaster, HRAModifiedTileNotSavedMsg, NotifyModifiedTileNotSaved)
HMG_END_MESSAGE_MAP()



//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HRAPyramidRaster::HRAPyramidRaster ()
    : HRAStoredRaster   ()
    {
    m_pLog              = 0;
    m_pRasterModel      = 0;

    m_UseOnlyFirstRes       = false;
    m_ComputeSubImage       = true;
    m_LookAheadEnabled      = false;
    m_HistogramEditMode     = false;
    m_SinglePixelType       = true;
    m_DisableTileStatus     = false;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRAPyramidRaster::HRAPyramidRaster  (HFCPtr<HRATiledRaster>& pi_pRasterModel,
                                     uint64_t                pi_WidthPixels,
                                     uint64_t                pi_HeightPixels,
                                     SubImageDescription*     pi_pSubImageDesc,
                                     unsigned short          pi_NumberOfSubImage,
                                     HPMObjectStore*          pi_pStore,
                                     HPMPool*                 pi_pLog,
                                     HFCPtr<HRATiledRaster>  pi_pMainImageRasterModel,
                                     bool                    pi_DisableTileStatus)

    : HRAStoredRaster (pi_WidthPixels, pi_HeightPixels,
                       pi_pRasterModel->GetTransfoModel(),
                       pi_pRasterModel->GetCoordSys(),
                       pi_pRasterModel->GetPixelType())
    {
    m_pLog              = pi_pLog;
    SetStore(pi_pStore);

    m_ComputeSubImage   = true;
    m_LookAheadEnabled  = false;
    m_HistogramEditMode = false;
    m_SinglePixelType   = true;
    m_UseOnlyFirstRes   = false;
    m_DisableTileStatus = pi_DisableTileStatus;

    Constructor (pi_pRasterModel,
                 pi_pMainImageRasterModel,
                 pi_WidthPixels,
                 pi_HeightPixels,
                 pi_pSubImageDesc,
                 pi_NumberOfSubImage);
    }



//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRAPyramidRaster::HRAPyramidRaster(const HRAPyramidRaster& pi_rObj)
    : HRAStoredRaster (pi_rObj)
    {
    // Perform initialization of the object
    m_pLog          = 0;
    m_ComputeSubImage   = true;
    m_LookAheadEnabled  = false;
    m_HistogramEditMode = false;
    m_SinglePixelType   = true;
    m_UseOnlyFirstRes   = false;
    m_DisableTileStatus = false;

    DeepCopy(pi_rObj);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRAPyramidRaster::~HRAPyramidRaster()
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
HRAPyramidRaster& HRAPyramidRaster::operator=(const HRAPyramidRaster& pi_rObj)
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
void HRAPyramidRaster::InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels)
    {
    // Don't call HRAStoredRaster::InitPhysicalShape because we create a new TiledRaster
    //
    SubImageDescription* pSubImageDesc = MakeSubImageDescriptor();
    HFCPtr<HRAPyramidRaster> pTmpRaster = new HRAPyramidRaster (m_pRasterModel,
                                                                pi_WidthPixels,
                                                                pi_HeightPixels,
                                                                pSubImageDesc,
                                                                (unsigned short)m_pSubImageList.BufSize-1,
                                                                GetStore(),
                                                                m_pLog);
    delete[] pSubImageDesc;

    // Keep the ID
    pTmpRaster->SetID (GetID());

    //DMx Copy or Compute each resolution. (Scale on each sub-images)

    // Replace the object
    ReplaceObject (pTmpRaster);

    delete pTmpRaster;
    }


//-----------------------------------------------------------------------------
// public
// SetShape
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SetShape (const HVEShape& pi_rShape)
    {
    // Change all sub-Image
    //
    HRAStoredRaster::SetShape(pi_rShape);

    for (unsigned short i=0; i<m_pSubImageList.BufSize; i++)
        m_pSubImageList.pData[i].m_pSubImage->SetShape(pi_rShape);

    }


//-----------------------------------------------------------------------------
// protected
// SetCoordSysImplementation
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys)
    {
    // Ancestor before...
    HRAStoredRaster::SetCoordSysImplementation(pi_rOldCoordSys);

    // Set model == to the TiledRaster
    m_pRasterModel->SetCoordSys (GetCoordSys());

    // Change all sub-Image
    for (unsigned short i=0; i<m_pSubImageList.BufSize; i++)
        {
        m_pSubImageList.pData[i].m_pSubImage->SetCoordSys(GetCoordSys());
        }
    }

//-----------------------------------------------------------------------------
// public
// SetTransfoModel
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SetTransfoModel (const HGF2DTransfoModel& pi_rModelCSp_CSl)
    {
    // Set model == to the TiledRaster
    m_pRasterModel->SetTransfoModel (pi_rModelCSp_CSl);

    // Change all sub-Image
    for (unsigned short i=0; i<m_pSubImageList.BufSize; i++)
        {
        // Create scaling transfo
        // Set scaling to the SubImages.
        HGF2DStretch Transfo;
        Transfo.SetXScaling(1.0 / GetSubImagesResolution(i));
        Transfo.SetYScaling(1.0 / GetSubImagesResolution(i));

        // Set a new physical coord. sys. to the buffer
        m_pSubImageList.pData[i].m_pSubImage->SetTransfoModel(*Transfo.ComposeInverseWithDirectOf(pi_rModelCSp_CSl));
        }

    // Call the parent
    HRAStoredRaster::SetTransfoModel (pi_rModelCSp_CSl);
    }


//-----------------------------------------------------------------------------
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRAPyramidRaster::GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    HASSERT(m_pSubImageList.pData[m_pSubImageList.BufSize-1].m_pSubImage != NULL);

    unsigned short NumberOfEntries = HRAStoredRaster::GetRepresentativePalette(pio_pRepPalParms);

    // if no operation has been done at the parent level or if the cache is not
    // updated
    if(NumberOfEntries == 0)
        {
        if(pio_pRepPalParms->GetSamplingOptions().GetPyramidImageSize() != 0)
            {
            // the method does not update its representative palette cache because
            // it gets its palette directly from the smaller raster

            int32_t ImageIndex = (int32_t)(log(100.0 / pio_pRepPalParms->GetSamplingOptions().GetPyramidImageSize()) / log(2.0));

            if(ImageIndex >= (int32_t)m_pSubImageList.BufSize)
                ImageIndex = (int32_t)m_pSubImageList.BufSize - 1;

            // Update Sub_res if not already did
            UpdateSubResolution(ImageIndex);

            // set the look ahead
            if (HasLookAhead())
                {
                SetLookAheadImpl(HVEShape(0,
                                          0,
                                          1,
                                          1,
                                          m_pSubImageList.pData[ImageIndex].m_pSubImage->GetPhysicalCoordSys()),
                                 0,
                                 false,
                                 ImageIndex);
                }

            // get the representative palette on the smaller raster
            NumberOfEntries = m_pSubImageList.pData[ImageIndex].m_pSubImage->GetRepresentativePalette(pio_pRepPalParms);
            }
        else
            {
            // we copy the representative palette parameters structure and
            // its pixel type to not notify the tiled raster after each
            // unlock in the tiles
            HRARepPalParms RepPalParms(*pio_pRepPalParms);
            RepPalParms.SetPixelType((HRPPixelType*)RepPalParms.GetPixelType()->Clone());

            const HRPPixelPalette& rPalette = (RepPalParms.GetPixelType())->GetPalette();

            // create a quantized palette object
            HAutoPtr<HRPQuantizedPalette> pQuantizedPalette(pio_pRepPalParms->CreateQuantizedPalette());

            for(uint32_t ImageIndex = 0; ImageIndex < m_pSubImageList.BufSize; ImageIndex++)
                {
                if(ImageIndex == 0 || ImageIndex == (m_pSubImageList.BufSize - 1))
                    {
                    // Update Sub_res if not already did
                    UpdateSubResolution(ImageIndex);

                    // set the look ahead
                    if (HasLookAhead())
                        {
                        SetLookAheadImpl(HVEShape(0,
                                                  0,
                                                  1,
                                                  1,
                                                  m_pSubImageList.pData[ImageIndex].m_pSubImage->GetPhysicalCoordSys()),
                                         0,
                                         false,
                                         ImageIndex);
                        }

                    // we create a new histogram for each subresolutin
                    RepPalParms.SetHistogram(new HRPHistogram(rPalette));

                    // get the representative palette on the smaller raster
                    NumberOfEntries = m_pSubImageList.pData[ImageIndex].m_pSubImage->GetRepresentativePalette(&RepPalParms);

                    // insert each entry of the representative palette in the
                    // quantized object
                    for(uint32_t EntryIndex = 0; EntryIndex < NumberOfEntries; EntryIndex++)
                        {
                        pQuantizedPalette->AddCompositeValue(
                            rPalette.GetCompositeValue(EntryIndex),
                            RepPalParms.GetHistogram()->GetEntryCount(EntryIndex));
                        }
                    }
                }

            // get the number of entries in the quantized palette
            NumberOfEntries = pQuantizedPalette->GetPalette(&((pio_pRepPalParms->GetPixelType())->LockPalette()),
                                                            pio_pRepPalParms->GetHistogram());
            (pio_pRepPalParms->GetPixelType())->UnlockPalette();
            }
        }

    return NumberOfEntries;
    }

//-----------------------------------------------------------------------------
// ComputeHistogram
//-----------------------------------------------------------------------------
void HRAPyramidRaster::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                        bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    HASSERT(m_pSubImageList.pData[m_pSubImageList.BufSize -1].m_pSubImage != NULL);

    if (pi_ForceRecompute || (GetHistogram() == 0) || (GetHistogram() != 0 && !GetHistogram()->CanBeUsedInPlaceOf(*pio_pOptions)) || GetHistogram()->GetSamplingColorSpace () !=  pio_pOptions->GetSamplingColorSpace ())
        {
        int32_t ImageIndex = (int32_t)(log(100.0 / pio_pOptions->GetSamplingOptions().GetPyramidImageSize()) / log(2.0));

        if(ImageIndex >= (int32_t)m_pSubImageList.BufSize)
            ImageIndex = (int32_t)m_pSubImageList.BufSize - 1;

        // Update Sub_res if not already did
        UpdateSubResolution(ImageIndex);

        // set the look ahead
        if (HasLookAhead())
            {
            SetLookAheadImpl(HVEShape(0,
                                      0,
                                      1,
                                      1,
                                      m_pSubImageList.pData[ImageIndex].m_pSubImage->GetPhysicalCoordSys()),
                             0,
                             false,
                             ImageIndex);
            }

        // compute the histogram
        m_pSubImageList.pData[ImageIndex].m_pSubImage->ComputeHistogram(pio_pOptions, pi_ForceRecompute);

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
        uint32_t HistogramChannels = pio_pOptions->GetHistogram()->GetChannelCount();

        HASSERT(pOutHisto->GetChannelCount() == pHisto->GetChannelCount());
        for(uint32_t ChannelIndex = 0; ChannelIndex < HistogramChannels; ChannelIndex++)
            {
            // add each entry in the histogram
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
bool HRAPyramidRaster::StartHistogramEditMode()
    {
    if (!m_HistogramEditMode)
        {
        if (GetHistogram()) // we need an histogram to work on
            {
            // Set mode to ON
            m_HistogramEditMode = true;

            // Compute all tile histograms (on full resolution)
            m_pSubImageList.pData[0].m_pSubImage->EngageTileHistogramComputing();

            return true;
            }
        else
            return false;
        }
    else // mode is already ON
        return true;
    }


//-----------------------------------------------------------------------------
// Public
// Passed the request to the source
//-----------------------------------------------------------------------------
bool HRAPyramidRaster::HasLookAhead() const
    {
    return (m_LookAheadEnabled);
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SetLookAhead(const HVEShape& pi_rShape,
                                    uint32_t        pi_ConsumerID,
                                    bool           pi_Async)
    {
    SetLookAheadImpl(pi_rShape, pi_ConsumerID, pi_Async);
    }


//-----------------------------------------------------------------------------
// Public
// Pass the request to the source
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SetLookAheadImpl(const HVEShape& pi_rShape,
                                        uint32_t        pi_ConsumerID,
                                        bool           pi_Async,
                                        int32_t          pi_ResIndex)
    {
    HPRECONDITION(HasLookAhead());

    unsigned short ResIndex;

    if (pi_ResIndex < 0)
        {
        // find the best resolution for the coord sys of the extent
        double Resolution = FindTheBestResolution(pi_rShape.GetCoordSys());

        // if the resolution is bigger than the first one use the first one
        if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[0].m_PhysicalImageResolution))
            {
            ResIndex = 0;
            }
        else
            {
            // find at what index is that resolution
            ResIndex = (unsigned short)m_pSubImageList.BufSize - 1;   // last res of the pyramid
            bool   ResFound = false;
            for (unsigned short Res = 0; (!ResFound) && (Res < m_pSubImageList.BufSize - 1); Res++)
                {
                // if the res is between the current res and the next, use that res
                if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[Res].m_PhysicalImageResolution) &&
                    HDOUBLE_GREATER_EPSILON(Resolution, m_pSubImageList.pData[Res + 1].m_PhysicalImageResolution))
                    {
                    ResIndex = Res;
                    ResFound = true;
                    }
                }
            }
        }
    else
        {
        ResIndex = (unsigned short)pi_ResIndex;
        }

    // Bring the extent to the found resolution's coord sys
    HFCPtr<HVEShape> pShape(new HVEShape(pi_rShape));
    pShape->ChangeCoordSys(GetSubImage(ResIndex)->GetPhysicalCoordSys());

    GetSubImage(ResIndex)->SetLookAhead(*pShape, pi_ConsumerID, pi_Async);
    }


//-----------------------------------------------------------------------------
// public
// Clear - Clear completely the Bitmap
//-----------------------------------------------------------------------------
void HRAPyramidRaster::Clear()
    {
    HFCPtr<HRATransaction> pCurTrans(m_pSubImageList.pData[0].m_pSubImage->GetCurrentTransaction());

    m_pSubImageList.pData[0].m_pSubImage->SetCurrentTransaction(GetCurrentTransaction());
    m_pSubImageList.pData[0].m_pSubImage->Clear();
    m_pSubImageList.pData[0].m_pSubImage->SetCurrentTransaction(pCurTrans);

    UpdateDirtyFlags(*GetEffectiveShape(), 0);
    }


//-----------------------------------------------------------------------------
// public
// Clear - Clear a region in a bitmap
//-----------------------------------------------------------------------------
void HRAPyramidRaster::Clear(const HRAClearOptions& pi_rOptions)
    {
    HPRECONDITION(!pi_rOptions.HasScanlines()); // clear by scanlines is not supported

    HFCPtr<HRATransaction> pCurTrans(m_pSubImageList.pData[0].m_pSubImage->GetCurrentTransaction());

    m_pSubImageList.pData[0].m_pSubImage->SetCurrentTransaction(GetCurrentTransaction());

    const HRAClearOptions* pClearOptions(&pi_rOptions);
    HRAClearOptions  ClearOptions;

    HFCPtr<HVEShape> pClearShape;

    if (pi_rOptions.HasShape())
        {
        if (pi_rOptions.HasApplyRasterClipping())
            {
            ClearOptions = pi_rOptions;
            pClearShape = new HVEShape(*pi_rOptions.GetShape());
            pClearShape->Intersect(*GetEffectiveShape());
            ClearOptions.SetShape(pClearShape);
            ClearOptions.SetApplyRasterClipping(false);
            pClearOptions = &ClearOptions;
            }
        }
    else if (pi_rOptions.HasRLEMask())
        {

        }
    else // clear with the effective shape
        {
        ClearOptions = pi_rOptions;
        ClearOptions.SetShape(GetEffectiveShape());
        ClearOptions.SetApplyRasterClipping(false);
        pClearOptions = &ClearOptions;
        }

    ((HFCPtr<HRARaster>&) m_pSubImageList.pData[0].m_pSubImage)->Clear(*pClearOptions);

    m_pSubImageList.pData[0].m_pSubImage->SetCurrentTransaction(pCurTrans);


    // dirty flags were updated be the HRAContentChanged message
    // no need to update flags again
    //UpdateDirtyFlags(*pClearOptions->GetShape(), 0);
    }

//-----------------------------------------------------------------------------
// Mark as "dirty" all tiles in all resolutions corresponding to specified
// shape, in order to force a further update of sub-res for them.
//-----------------------------------------------------------------------------
void HRAPyramidRaster::UpdateDirtyFlags(const HVEShape& pi_rShape, short pi_Res)
    {
    HASSERT(!m_DisableTileStatus);

    HVEShape Shape(pi_rShape);
    HGF2DExtent UpdateExtent;

    for (size_t i = pi_Res; i < m_pSubImageList.BufSize; i++)
        {
        HFCPtr<HRATiledRaster> pDestTiledRaster = m_pSubImageList.pData[i].m_pSubImage;
        HGFTileIDDescriptor DestTileDescriptor(*(pDestTiledRaster->GetPtrTileDescription()));
        HRATileStatus& rDestTileStatus = pDestTiledRaster->GetInternalTileStatusList();
        Shape.ChangeCoordSys(pDestTiledRaster->GetPhysicalCoordSys());
        UpdateExtent = Shape.GetExtent();
        uint64_t DestTileIndex = DestTileDescriptor.GetFirstTileIndex(UpdateExtent);

//        HASSERT(DestTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND);
        // This hassert has been replace by a if because of a double conversion bug.
        // in some cases pi_rShape was to short to be treaded as a shape.
        if(DestTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
            m_pSubImageList.pData[i].m_SubResolutionIsDirty = true;

        while (DestTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
            {
            rDestTileStatus.SetDirtyForSubResFlag(DestTileIndex, true);
            DestTileIndex = DestTileDescriptor.GetNextTileIndex();
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// NotifyContentChanged - Message Handler
//-----------------------------------------------------------------------------
bool HRAPyramidRaster::NotifyContentChanged (const HMGMessage& pi_rMessage)
    {
    HRAStoredRaster* pRaster = ((HRAStoredRaster*)pi_rMessage.GetSender());

    if (pRaster == m_pSubImageList.pData[0].m_pSubImage)  // is affected res = 0?
        {
        UpdateDirtyFlags(((HRAContentChangedMsg&)pi_rMessage).GetShape(), 0);

        if (m_HistogramEditMode)
            {
            // Update histogram
            // (To keep good performances we only compute tiles histogram "delta" (before vs now) and
            //  apply this to main histogram)
            HVEShape Shape(((HRAContentChangedMsg&)pi_rMessage).GetShape());
            HGF2DExtent UpdateExtent;

            HFCPtr<HRATiledRaster> pTiledRaster = m_pSubImageList.pData[0].m_pSubImage;
            HGFTileIDDescriptor TileDescriptor(*(pTiledRaster->GetPtrTileDescription()));
            Shape.ChangeCoordSys(pTiledRaster->GetPhysicalCoordSys());
            UpdateExtent = Shape.GetExtent();

            // Scan all modified tiles
            uint64_t TileIndex = TileDescriptor.GetFirstTileIndex(UpdateExtent);

            while (TileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                {
                const HFCPtr<HRABitmapBase> pTile = pTiledRaster->GetTileByIndex(TileIndex)->GetTile();

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
                for(uint32_t ChannelIndex(0); ChannelIndex < ChannelCount; ++ChannelIndex)
                    {
                    uint32_t Entries = OldTileHistogramOptions.GetHistogram()->GetEntryFrequenciesSize(ChannelIndex);

                    // Compute histogram delta (New - Old)
                    for(uint32_t Index(0); Index < Entries; ++Index)
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

                TileIndex = TileDescriptor.GetNextTileIndex();
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
bool HRAPyramidRaster::NotifyProgressImageChanged (const HMGMessage& pi_rMessage)
    {
    HPRECONDITION(pi_rMessage.IsCompatibleWith(HRFProgressImageChangedMsg::CLASS_ID));


    HRFProgressImageChangedMsg& rMessage((HRFProgressImageChangedMsg&)pi_rMessage);
    unsigned short Res = ((HRFProgressImageChangedMsg&)pi_rMessage).GetSubResolution();

    HFCPtr<HRATiledRaster> pResRaster(m_pSubImageList.pData[Res].m_pSubImage);

    // Propagate the message to a specific TiledRaster
    if ((Res < m_pSubImageList.BufSize) &&
        (m_pSubImageList.pData[Res].m_pSubImage != 0))
        {
        m_pSubImageList.pData[Res].m_pSubImage->NotifyProgressImageChanged (pi_rMessage);
        }

    CHECK_HUINT64_TO_HDOUBLE_CONV(rMessage.GetPosX() + pResRaster->GetTileSizeX())
    CHECK_HUINT64_TO_HDOUBLE_CONV(rMessage.GetPosY() + pResRaster->GetTileSizeY())

    HVEShape TileShape((double)rMessage.GetPosX(),
                        (double)rMessage.GetPosY(),
                        (double)rMessage.GetPosX() + pResRaster->GetTileSizeX(),
                        (double)rMessage.GetPosY() + pResRaster->GetTileSizeY(),
                        pResRaster->GetPhysicalCoordSys());

    UpdateDirtyFlags(TileShape, (short)Res);
    SetModificationState();

    return false;
    }


//-----------------------------------------------------------------------------
// public
// NotifyHRAContentChanged - Message Handler
//-----------------------------------------------------------------------------
bool HRAPyramidRaster::NotifyHRAProgressImageChanged (const HMGMessage& pi_rMessage)
    {
    // Nothing to do. Overrides the ContentChanged message.
    // This handling has been made in the HRFProgressImageChanged
    // message handling.
    return true;
    }


//-----------------------------------------------------------------------------
// Receive a shape changed notification
//-----------------------------------------------------------------------------
bool HRAPyramidRaster::NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage)
    {
    // Stop Message...
    return false;
    }


//-----------------------------------------------------------------------------
// Receive a geometry changed notification
//-----------------------------------------------------------------------------
bool HRAPyramidRaster::NotifyGeometryChanged (const HMGMessage& pi_rMessage)
    {
    // Stop Message...
    return false;
    }

//-----------------------------------------------------------------------------
// Receive a modified tile not saved notification
//-----------------------------------------------------------------------------
bool HRAPyramidRaster::NotifyModifiedTileNotSaved(const HMGMessage& pi_rMessage)
    {
    HPRECONDITION(pi_rMessage.IsCompatibleWith(HRAModifiedTileNotSavedMsg::CLASS_ID));

    HRAModifiedTileNotSavedMsg& rMessage((HRAModifiedTileNotSavedMsg&)pi_rMessage);
    uint32_t Res = rMessage.GetResolution();
    HPRECONDITION(Res < m_pSubImageList.BufSize);
    if (Res > 0)
        {
        // first, get the extent of the modified tile
        HFCPtr<HRATiledRaster> pTiledRaster(m_pSubImageList.pData[Res].m_pSubImage);
        HGFTileIDDescriptor TileDescriptor(*pTiledRaster->GetPtrTileDescription());
        HRATileStatus& rTileStatus = pTiledRaster->GetInternalTileStatusList();
        HASSERT(rTileStatus.GetDirtyForSubResFlag(rMessage.GetTileIndex()));

        uint64_t PosX;
        uint64_t PosY;
        TileDescriptor.GetPositionFromIndex(rMessage.GetTileIndex(), &PosX, &PosY);
        HVEShape TileShape((double)PosX,
                            (double)PosY,
                            (double)(PosX + TileDescriptor.GetTileWidth()),
                            (double)(PosY + TileDescriptor.GetTileHeight()),
                            pTiledRaster->GetPhysicalCoordSys());

        HFCPtr<HRATiledRaster> pPrevTiledRaster(m_pSubImageList.pData[Res-1].m_pSubImage);
        HGFTileIDDescriptor PrevTileDescriptor(*pPrevTiledRaster->GetPtrTileDescription());
        HRATileStatus& rPrevTileStatus = pPrevTiledRaster->GetInternalTileStatusList();

        TileShape.ChangeCoordSys(pPrevTiledRaster->GetPhysicalCoordSys());
        uint64_t TileIndex = PrevTileDescriptor.GetFirstTileIndex(TileShape.GetExtent());
        while (TileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
            {
            rPrevTileStatus.SetDirtyForSubResFlag(TileIndex, true);
            TileIndex = PrevTileDescriptor.GetNextTileIndex();
            }
        }

    return true;
    }

//-----------------------------------------------------------------------------
// Update, if necessary, the part of the next sub-image that maps to the
// specified tile.  Applies only to pyramid of tiled rasters.
//-----------------------------------------------------------------------------
void HRAPyramidRaster::UpdateNextRes(int32_t       pi_SubResolution,
                                     uint64_t      pi_TileIndex,
                                     HRABitmapBase* pi_pTile)
    {
    HASSERT(m_pRasterModel->IsCompatibleWith(HRATiledRaster::CLASS_ID));
    HASSERT(!m_DisableTileStatus);

    if (pi_SubResolution < (int32_t)(m_pSubImageList.BufSize - 1)) // update only if we aren't on the last resolution
        {
        HFCPtr<HRATiledRaster> pSrcTiledRaster(m_pSubImageList.pData[pi_SubResolution].m_pSubImage);
        HRATileStatus& rSrcTileStatus = pSrcTiledRaster->GetInternalTileStatusList();

        // update only of the tile status is dirty
        if (rSrcTileStatus.GetDirtyForSubResFlag(pi_TileIndex))
            {
            HGFTileIDDescriptor SrcTileDescriptor(*(pSrcTiledRaster->GetPtrTileDescription()));
            HFCPtr<HGF2DCoordSys> pSrcPhysicalCoordSys = pSrcTiledRaster->GetPhysicalCoordSys();

            // get destination resolution
            HFCPtr<HRATiledRaster> pDestTiledRaster(m_pSubImageList.pData[pi_SubResolution + 1].m_pSubImage);
            HGFTileIDDescriptor DestTileDescriptor(*(pDestTiledRaster->GetPtrTileDescription()));
            HFCPtr<HGF2DCoordSys> pDestPhysicalCoordSys(pDestTiledRaster->GetPhysicalCoordSys());
            
            // set resampling option
            HGSResampling samplingMode(HGSResampling::AVERAGE);
            switch(m_pSubImageList.pData[pi_SubResolution].m_ResamplingType)
                {
                case HGSResampling::NEAREST_NEIGHBOUR:
                    samplingMode = HGSResampling::NEAREST_NEIGHBOUR;
                    break;
                case HGSResampling::AVERAGE:
                    samplingMode = HGSResampling::AVERAGE;
                    break;
                case HGSResampling::ORING4:
                    samplingMode = HGSResampling::NEAREST_NEIGHBOUR;
                    break;
                default:
                    samplingMode = HGSResampling::AVERAGE;
                    break;
                }

            HFCPtr<HGSSurfaceDescriptor> pSrcSurfaceDesc(pi_pTile->GetSurfaceDescriptor());

            double Resolution = GetSubImagesResolution((unsigned short)pi_SubResolution) / GetSubImagesResolution((unsigned short)pi_SubResolution + 1);

            // get the stretch model between resolution
            HGF2DStretch Stretch(HGF2DDisplacement(0.0, 0.0),
                                 Resolution,
                                 Resolution);

            uint64_t PosX;
            uint64_t PosY;
            SrcTileDescriptor.GetPositionFromIndex(pi_TileIndex, &PosX, &PosY);

            pSrcSurfaceDesc->SetDataDimensions((uint32_t)MIN(SrcTileDescriptor.GetImageWidth() - PosX, SrcTileDescriptor.GetTileWidth()),
                                               (uint32_t)MIN(SrcTileDescriptor.GetImageHeight() - PosY, SrcTileDescriptor.GetTileHeight()));

            CHECK_HUINT64_TO_HDOUBLE_CONV(PosX + SrcTileDescriptor.GetTileWidth());
            CHECK_HUINT64_TO_HDOUBLE_CONV(PosX + SrcTileDescriptor.GetTileWidth());

            double SrcPosX = (double)PosX;
            double SrcPosY = (double)PosY;
            double SrcPosXInDst;
            double SrcPosYInDst;
            double SrcTileWidthInDst;
            double SrcTileHeightInDst;

            Stretch.ConvertInverse(SrcPosX,
                                   SrcPosY,
                                   &SrcPosXInDst,
                                   &SrcPosYInDst);

            Stretch.ConvertInverse((double)SrcTileDescriptor.GetTileWidth(),
                                   (double)SrcTileDescriptor.GetTileHeight(),
                                   &SrcTileWidthInDst,
                                   &SrcTileHeightInDst);

            // compute the tile shape extent into the destination resolution
            HGF2DExtent TileExtent(SrcPosXInDst,
                                   SrcPosYInDst,
                                   SrcPosXInDst + SrcTileWidthInDst,
                                   SrcPosYInDst + SrcTileHeightInDst,
                                   pDestPhysicalCoordSys);


            double DstPosX;
            double DstPosY;
            double DstPosXInSrc;
            double DstPosYInSrc;
            uint64_t DstDataWidth;
            uint64_t DstDataHeight;

            list<HFCPtr<HRATiledRaster::HRATile> > DestTileList;
            HFCPtr<HRATiledRaster::HRATile> pDestTile;

            // get touched tile into the next resolution
            uint64_t DestTileIndex = DestTileDescriptor.GetFirstTileIndex(TileExtent);
            HASSERT(DestTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND);

            while (DestTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                {
                pDestTile = pDestTiledRaster->GetTileByIndex(DestTileIndex, true);  // true = not in pool.
                HFCPtr<HGSSurfaceDescriptor> pDstSurfaceDesc(pDestTile->GetTile()->GetSurfaceDescriptor());
                HRASurface destSurface(pDstSurfaceDesc);

                DestTileDescriptor.GetPositionFromIndex(DestTileIndex, &PosX, &PosY);
                CHECK_HUINT64_TO_HDOUBLE_CONV(PosX);
                CHECK_HUINT64_TO_HDOUBLE_CONV(PosX);
                DstPosX = (double)PosX;
                DstPosY = (double)PosY;
                Stretch.ConvertDirect(DstPosX,
                                      DstPosY,
                                      &DstPosXInSrc,
                                      &DstPosYInSrc);

                DstDataWidth = MIN(DestTileDescriptor.GetTileWidth(), DestTileDescriptor.GetImageWidth() - PosX);
                DstDataHeight = MIN(DestTileDescriptor.GetTileHeight(), DestTileDescriptor.GetImageHeight() - PosY);

                Stretch.SetTranslation(HGF2DDisplacement(DstPosXInSrc - SrcPosX, DstPosYInSrc - SrcPosY));

                // shape the blit action
                SrcPosXInDst -= DstPosX;
                SrcPosYInDst -= DstPosY;
                HFCPtr<HVEShape> pShape(new HVEShape(SrcPosXInDst,
                                                     SrcPosYInDst,
                                                     MIN(SrcPosXInDst + SrcTileWidthInDst, DstDataWidth),
                                                     MIN(SrcPosYInDst + SrcTileHeightInDst, DstDataHeight),
                                                     pDestTiledRaster->GetPhysicalCoordSys()));

                HFCPtr<HGSRegion> pTheRegion(new HGSRegion(pShape, pShape->GetCoordSys()));
                destSurface.SetRegion(pTheRegion);

                // create the src surfaces
                HRASurface srcSurface(pSrcSurfaceDesc);

                // create blitter
                HRABlitter blitter(destSurface);
                blitter.SetSamplingMode(samplingMode);

                blitter.BlitFrom(srcSurface, Stretch);

                pDestTile->GetTile()->Updated(pShape);

                DestTileList.push_back(pDestTile);
                DestTileIndex = DestTileDescriptor.GetNextTileIndex();
                }

            // If the copy operation has not been interrupted, set the tile's status as "not dirty"
            // anymore, unless one of corresponding tiles from previous res (if any) is still
            // dirty...

            if (!HRADrawProgressIndicator::GetInstance()->IsIterationStopped())
                {
                rSrcTileStatus.SetDirtyForSubResFlag(pi_TileIndex, false);
                if (pi_SubResolution > 0)
                    {
                    HFCPtr<HRATiledRaster> pPrevTiledRaster = m_pSubImageList.pData[pi_SubResolution-1].m_pSubImage;
                    HGFTileIDDescriptor PrevTileDescriptor(*(pPrevTiledRaster->GetPtrTileDescription()));
                    HRATileStatus& rPrevTileStatus = pPrevTiledRaster->GetInternalTileStatusList();

                    double Resolution = GetSubImagesResolution((unsigned short)pi_SubResolution) / GetSubImagesResolution((unsigned short)pi_SubResolution - 1);
                    HGF2DStretch Stretch(HGF2DDisplacement(0.0, 0.0), Resolution, Resolution);
                    HFCPtr<HGF2DCoordSys> pResCoordSys = new HGF2DCoordSys(Stretch, pSrcPhysicalCoordSys);

                    HVEShape TileShape(pi_pTile->GetShape());
                    TileShape.ChangeCoordSys(pResCoordSys);
                    HGF2DExtent PrevExtent = TileShape.GetExtent();
                    uint64_t PrevTileIndex = PrevTileDescriptor.GetFirstTileIndex(PrevExtent);
                    while (PrevTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                        {
                        if (rPrevTileStatus.GetDirtyForSubResFlag(PrevTileIndex))
                            {
                            rSrcTileStatus.SetDirtyForSubResFlag(pi_TileIndex, true);
                            break;
                            }
                        PrevTileIndex = PrevTileDescriptor.GetNextTileIndex();
                        }
                    }
                }

            // If the target tile is completely up-to-date, we can put it in the pool

            list<HFCPtr<HRATiledRaster::HRATile> >::const_iterator Itr(DestTileList.begin());
            while (Itr != DestTileList.end())
                {
                bool TileIsReady = true;
                    {
                    HVEShape Shape = (*Itr)->GetTile()->GetExtent();
                    Shape.ChangeCoordSys(pSrcPhysicalCoordSys);
                    HGF2DExtent DestExtent = Shape.GetExtent();
                    uint64_t SrcTileIndex = SrcTileDescriptor.GetFirstTileIndex(DestExtent);
                    while (SrcTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                        {
                        if (rSrcTileStatus.GetDirtyForSubResFlag(SrcTileIndex))
                            {
                            TileIsReady = false;
                            break;
                            }
                        SrcTileIndex = SrcTileDescriptor.GetNextTileIndex();
                        }
                    if (TileIsReady)
                        {
                        (*Itr)->NotifyPool();
                        (*Itr)->Discartable(true);
                        }
                    Itr++;
                    }
                }
            }

        }
    }

//-----------------------------------------------------------------------------
// public
// SaveAndFlushAllTiles
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SaveAndFlushAllTiles
(
)
    {
    for (size_t i(0); i < m_pSubImageList.BufSize; ++i)
        {
        if (m_pSubImageList.pData[i].m_pSubImage != 0)
            m_pSubImageList.pData[i].m_pSubImage->SaveAndFlushTiles();
        }
    }

//-----------------------------------------------------------------------------
// public
// UpdateSubResolution - Compute all sub-resolution for each modified tiles.
//                       if SubResolution = -1 then all resolution else
//                       until this SubResolution.
//                       if pi_pExtent == 0 then Update all tiles or Bitmap.
//-----------------------------------------------------------------------------
void HRAPyramidRaster::UpdateSubResolution (int32_t            pi_SubResolution,
                                            const HGF2DExtent*  pi_pExtent)
    {
    // Special support of temporary creation by HOD

    if ((!m_ComputeSubImage) || (m_pSubImageList.BufSize == 0))
        return;

    bool Continue = !HRADrawProgressIndicator::GetInstance()->IsIterationStopped();

    if (Continue && pi_SubResolution == -1)
        pi_SubResolution = (int32_t)m_pSubImageList.BufSize - 1;

    if (m_pSubImageList.pData[pi_SubResolution].m_ComputeResWithOptimizedMethod)
        // Here begins the code handling the computation of sub-resolutions,
        // especially optimized for pyramid of tiled rasters.
        // In that case, this method update only requested resolution.  It makes use
        // of recursion to update higher resolutions, before copying from them to
        // current resolution.
        {
        Continue = !HRADrawProgressIndicator::GetInstance()->IsIterationStopped();

        if (Continue && (pi_SubResolution > 0))
            {
            if (!m_pSubImageList.pData[pi_SubResolution].m_SubResolutionIsDirty && !m_pSubImageList.pData[pi_SubResolution - 1].m_SubResolutionIsDirty)
                return;   // Don't do anything more, current res is ready

            HASSERT(!m_DisableTileStatus);

            IncrementRef();  // Sometimes this raster is not smart-pointed

            HFCPtr<HRATiledRaster> pSrcTiledRaster = m_pSubImageList.pData[pi_SubResolution - 1].m_pSubImage;
            HGFTileIDDescriptor SrcTileDescriptor(*(pSrcTiledRaster->GetPtrTileDescription()));
            HRATileStatus& rSrcTileStatus = pSrcTiledRaster->GetInternalTileStatusList();

            HFCPtr<HRATiledRaster> pDestTiledRaster = m_pSubImageList.pData[pi_SubResolution].m_pSubImage;
            HGFTileIDDescriptor DestTileDescriptor(*(pDestTiledRaster->GetPtrTileDescription()));

            // set resampling option
            HGSResampling samplingMode(HGSResampling::AVERAGE);
            switch(m_pSubImageList.pData[pi_SubResolution].m_ResamplingType)
                {
                case HGSResampling::NEAREST_NEIGHBOUR:
                    samplingMode = HGSResampling::NEAREST_NEIGHBOUR;
                    break;
                case HGSResampling::AVERAGE:
                    samplingMode = HGSResampling::AVERAGE;
                    break;
                case HGSResampling::ORING4:
                    samplingMode = HGSResampling::NEAREST_NEIGHBOUR;
                    break;
                default:
                    samplingMode = HGSResampling::AVERAGE;
                    break;
                }

            // Finding first tile in requested resolution

            HGF2DExtent Extent;
            if (pi_pExtent)
                {
                HVEShape Shape(*pi_pExtent);
                Shape.ChangeCoordSys(pDestTiledRaster->GetPhysicalCoordSys());
                Extent = Shape.GetExtent();
                }
            else
                Extent = pDestTiledRaster->GetPhysicalExtent();


            // Create a CoordSys between resolution
            HFCPtr<HGF2DCoordSys> pDstPhysicalCoordSys = pDestTiledRaster->GetPhysicalCoordSys();
            double Resolution = GetSubImagesResolution((unsigned short)pi_SubResolution - 1) / GetSubImagesResolution((unsigned short)pi_SubResolution);
            HGF2DStretch Stretch(HGF2DDisplacement(0.0, 0.0), Resolution, Resolution);

            Continue = !HRADrawProgressIndicator::GetInstance()->IsIterationStopped();

            // all values must be in destination coord sys
            uint64_t PosX;
            uint64_t PosY;
            double SrcPosX;
            double SrcPosY;
            double DstPosX;
            double DstPosY;
            double SrcPosXInDst;
            double SrcPosYInDst;
            double SrcTileWidthInDst;
            double SrcTileHeightInDst;
            double DstPosXInSrc;
            double DstPosYInSrc;
            double DstTileWidthInSrc;
            double DstTileHeightInSrc;

            Stretch.ConvertInverse((double)SrcTileDescriptor.GetTileWidth(),
                                   (double)SrcTileDescriptor.GetTileHeight(),
                                   &SrcTileWidthInDst,
                                   &SrcTileHeightInDst);

            Stretch.ConvertDirect((double)DestTileDescriptor.GetTileWidth(),
                                  (double)DestTileDescriptor.GetTileHeight(),
                                  &DstTileWidthInSrc,
                                  &DstTileHeightInSrc);


            double DstDataWidth;
            double DstDataHeight;


            // For each tile in requested resolution...
            uint64_t DestTileIndex = DestTileDescriptor.GetFirstTileIndex(Extent);
            while (Continue && (DestTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND) &&
                   HRAUpdateSubResProgressIndicator::GetInstance()->ContinueIteration())
                {
                // Getting that tile, and checking tiles in previous resolution
                // to make them ready to be copied

                HFCPtr<HRATiledRaster::HRATile> pDestTile(pDestTiledRaster->GetTileByIndex(DestTileIndex).GetPtr());

                DestTileDescriptor.GetPositionFromIndex(DestTileIndex, &PosX, &PosY);

                CHECK_HUINT64_TO_HDOUBLE_CONV(PosX + DestTileDescriptor.GetTileWidth())
                CHECK_HUINT64_TO_HDOUBLE_CONV(PosY + DestTileDescriptor.GetTileHeight())
                DstPosX = (double)PosX;
                DstPosY = (double)PosY;

                HGF2DExtent DstTileExtent(DstPosX,
                                          DstPosY,
                                          DstPosX + DestTileDescriptor.GetTileWidth(),
                                          DstPosY + DestTileDescriptor.GetTileHeight(),
                                          pDestTiledRaster->GetPhysicalCoordSys());

                // make pyramid up-to-date for this tile
                UpdateSubResolution(pi_SubResolution-1, &DstTileExtent);
                Continue = !HRADrawProgressIndicator::GetInstance()->IsIterationStopped();

                // For each tile in previous res, if dirty, copy to actual tile.
                Stretch.ConvertDirect(DstPosX,
                                      DstPosY,
                                      &DstPosXInSrc,
                                      &DstPosYInSrc);

                HGF2DExtent SrcExtent(DstPosXInSrc,
                                      DstPosYInSrc,
                                      DstPosXInSrc + DstTileWidthInSrc,
                                      DstPosYInSrc + DstTileHeightInSrc,
                                      pSrcTiledRaster->GetPhysicalCoordSys());

                DstDataWidth = (double)MIN(DestTileDescriptor.GetTileWidth(), DestTileDescriptor.GetImageWidth() - PosX);
                DstDataHeight = (double)MIN(DestTileDescriptor.GetTileHeight(), DestTileDescriptor.GetImageHeight() - PosY);

                uint64_t SrcTileIndex = SrcTileDescriptor.GetFirstTileIndex(SrcExtent);

                HFCPtr<HGSSurfaceDescriptor> pDstSurfaceDesc(pDestTile->GetTile()->GetSurfaceDescriptor());
                HRASurface destSurface(pDstSurfaceDesc);

                // set the surfaces on the toolboxes
                HRABlitter blitter(destSurface);
                blitter.SetSamplingMode(samplingMode);

                HFCPtr<HVEShape> pShape;

                while (Continue && (SrcTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND))
                    {
                    if (rSrcTileStatus.GetDirtyForSubResFlag(SrcTileIndex))
                        {
                        HFCPtr<HRATiledRaster::HRATile> pSrcTile(pSrcTiledRaster->GetTileByIndex(SrcTileIndex));
                        SrcTileDescriptor.GetPositionFromIndex(SrcTileIndex, &PosX, &PosY);
                        CHECK_HUINT64_TO_HDOUBLE_CONV(PosX)
                        CHECK_HUINT64_TO_HDOUBLE_CONV(PosY)
                        SrcPosX = (double)PosX;
                        SrcPosY = (double)PosY;

                        HFCPtr<HGSSurfaceDescriptor> pSrcSurfaceDesc(pSrcTile->GetTile()->GetSurfaceDescriptor());

                        pSrcSurfaceDesc->SetDataDimensions((uint32_t)MIN(SrcTileDescriptor.GetImageWidth() - PosX, SrcTileDescriptor.GetTileWidth()),
                                                           (uint32_t)MIN(SrcTileDescriptor.GetImageHeight() - PosY, SrcTileDescriptor.GetTileHeight()));

                        // compute the position of the src tile into the destination
                        Stretch.ConvertInverse(SrcPosX,
                                               SrcPosY,
                                               &SrcPosXInDst,
                                               &SrcPosYInDst);

                        // translate the tile position to fit with the destination tile
                        SrcPosXInDst -= DstPosX;
                        SrcPosYInDst -= DstPosY;

                        // set the destination shape in physical
                        HFCPtr<HVEShape> pDestShape(new HVEShape(SrcPosXInDst,
                                                                 SrcPosYInDst,
                                                                 MIN(SrcPosXInDst + SrcTileWidthInDst, DstDataWidth),
                                                                 MIN(SrcPosYInDst + SrcTileHeightInDst, DstDataHeight),
                                                                 pDestTiledRaster->GetPhysicalCoordSys()));

                        HFCPtr<HGSRegion> pTheRegion(new HGSRegion(pDestShape, pDestShape->GetCoordSys()));
                        destSurface.SetRegion(pTheRegion);

                        // create the src surfaces
                        HRASurface srcSurface(pSrcSurfaceDesc);

                        // copy the translation between the src and dest tile
                        Stretch.ConvertDirect(SrcPosXInDst,
                                              SrcPosYInDst,
                                              &SrcPosXInDst,
                                              &SrcPosYInDst);
                        Stretch.SetTranslation(HGF2DDisplacement(-SrcPosXInDst, -SrcPosYInDst));

                        blitter.BlitFrom(srcSurface, Stretch);

                        pDestTile->GetTile()->Updated(pDestShape);

                        // If the Copyfrom is complete.(no esc-key, to stop draw), set the tile not dirty
                        if((Continue = !HRADrawProgressIndicator::GetInstance()->IsIterationStopped()))
                            rSrcTileStatus.SetDirtyForSubResFlag(SrcTileIndex, false); // Done with that data

                        // reset translation
                        Stretch.SetTranslation(HGF2DDisplacement(0, 0));

                        }
                    SrcTileIndex = SrcTileDescriptor.GetNextTileIndex();
                    }

                if (Continue)
                    {
                    pDestTile->Discartable(true);
                    if (!pDestTile->HasInPool())
                        pDestTile->NotifyPool();
                    }

                DestTileIndex = DestTileDescriptor.GetNextTileIndex();
                }
				
            DecrementRef();
            }

        if ((!pi_pExtent) && Continue)
            m_pSubImageList.pData[pi_SubResolution].m_SubResolutionIsDirty = false;
        }
    else
        UpdateResolution (pi_SubResolution, pi_pExtent);

    }


//-----------------------------------------------------------------------------
// public
// UpdateResolution - Compute all sub-resolution for each modified tiles.
//                    if pi_pExtent == 0 then Update all tiles or Bitmap.
//-----------------------------------------------------------------------------
void HRAPyramidRaster::UpdateResolution (int32_t           pi_Resolution,
                                         const HGF2DExtent* pi_pExtent)
    {
    uint32_t LowestRes;

    if (pi_Resolution == -1)
        LowestRes = (uint32_t)m_pSubImageList.BufSize - 1;
    else
        LowestRes = pi_Resolution;

    if (LowestRes > 0)
        {
        HASSERT(!m_DisableTileStatus);

        bool Continue = !HRADrawProgressIndicator::GetInstance()->IsIterationStopped();

        IncrementRef();  // Sometimes this raster is not smart-pointed
        for (uint32_t i = 0; Continue && i < LowestRes; i++)
            {
            if (m_pSubImageList.pData[i].m_SubResolutionIsDirty)
                {
                HRACopyFromOptions Options;
                switch(m_pSubImageList.pData[i].m_ResamplingType)
                    {
                    case HGSResampling::NEAREST_NEIGHBOUR:
                        break;
                    case HGSResampling::AVERAGE:
                        Options.SetResamplingMode(HGSResampling(HGSResampling::AVERAGE));
                        break;
                    case HGSResampling::ORING4:
                        break;
                    default:
                        Options.SetResamplingMode(HGSResampling(HGSResampling::AVERAGE));
                        break;
                    }

                // Specify we're in overview mode.
                // This has the effect that HRABlitter will not use the center of the pixel for the pixel coordinate.
                // *** The new copyFrom fill everything it touches. So no need to hack the pixel selection strategy with 0.3 instead of 0.5.
                // Options.SetOverviewMode(true);

                HFCPtr<HRATiledRaster> pDestRaster = m_pSubImageList.pData[i + 1].m_pSubImage;
                HGF2DExtent Extent;
                if (pi_pExtent)
                    Extent = *pi_pExtent;
                else
                    Extent = pDestRaster->GetPhysicalExtent();

                HFCPtr<HRATiledRaster> pSrcTiledRaster = m_pSubImageList.pData[i].m_pSubImage;

                HGFTileIDDescriptor SrcTileDescriptor(*(pSrcTiledRaster->GetPtrTileDescription()));
                HRATileStatus& rSrcTileStatus = pSrcTiledRaster->GetInternalTileStatusList();

                HVEShape Shape(Extent);
                Shape.ChangeCoordSys(pSrcTiledRaster->GetPhysicalCoordSys());
                Extent = Shape.GetExtent();

                uint64_t SrcTileIndex = SrcTileDescriptor.GetFirstTileIndex(Extent);
                while (Continue && (SrcTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND))
                    {
                    if (rSrcTileStatus.GetDirtyForSubResFlag(SrcTileIndex))
                        {
                        HFCPtr<HRATiledRaster::HRATile> pSrcTile(pSrcTiledRaster->GetTileByIndex(SrcTileIndex));
                        pDestRaster->CopyFrom(*pSrcTile->GetTile().GetPtr(), Options);

                        if ((Continue = !HRADrawProgressIndicator::GetInstance()->IsIterationStopped()))
                            rSrcTileStatus.SetDirtyForSubResFlag(SrcTileIndex, false); // Done with that data
                        }
                    SrcTileIndex = SrcTileDescriptor.GetNextTileIndex();
                    }

                // m_pRasterModel is compatible with HRATiledRaster, pDestRaster can be cast in HRATiledRaster
                HFCPtr<HRATiledRaster> pDstTiledRaster = pDestRaster;
                // convert the shape from src coordsys to dst coordsys
                Shape.ChangeCoordSys(pDstTiledRaster->GetPhysicalCoordSys());
                HGFTileIDDescriptor DstTileDescriptor(*(pDstTiledRaster->GetPtrTileDescription()));

                uint64_t DstTileIndex = DstTileDescriptor.GetFirstTileIndex(Shape.GetExtent());
                uint64_t PosX;
                uint64_t PosY;
                HFCPtr<HRATiledRaster::HRATile> pDstTile;
                while (DstTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                    {
                    // build the dst tile shape
                    DstTileDescriptor.GetPositionFromID(DstTileIndex, &PosX, &PosY);

                    CHECK_HUINT64_TO_HDOUBLE_CONV(PosX + DstTileDescriptor.GetTileWidth())
                    CHECK_HUINT64_TO_HDOUBLE_CONV(PosY + DstTileDescriptor.GetTileHeight())

                    HVEShape TileExtent((double)PosX,
                                        (double)PosY,
                                        (double)PosX + DstTileDescriptor.GetTileWidth(),
                                        (double)PosY + DstTileDescriptor.GetTileHeight(),
                                        pDstTiledRaster->GetPhysicalCoordSys());
                    // convert the dst tile shape into the src coordsys
                    TileExtent.ChangeCoordSys(pSrcTiledRaster->GetPhysicalCoordSys());

                    // verify if all src tile is up to date
                    SrcTileIndex = SrcTileDescriptor.GetFirstTileIndex(TileExtent.GetExtent());
                    bool TileReady = true;
                    while (SrcTileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND && TileReady)
                        {
                        TileReady = !rSrcTileStatus.GetDirtyForSubResFlag(SrcTileIndex);
                        SrcTileIndex = SrcTileDescriptor.GetNextTileIndex();
                        }

                    // all src tiles is up to date, the dst tile can discartable
                    if (TileReady)
                        {
                        // get tile from tiledraster only if the tile is already loaded
                        if ((pDstTile = pDstTiledRaster->GetTileFromLoaded(DstTileIndex)) != 0)
                            {
                            pDstTile->NotifyPool();
                            pDstTile->Discartable(true);
                            }
                        }
                    DstTileIndex = DstTileDescriptor.GetNextTileIndex();
                    }

                if ((!pi_pExtent) && Continue)
                    m_pSubImageList.pData[i].m_SubResolutionIsDirty = false;
                }
            }
        DecrementRef();
        }
    }



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void HRAPyramidRaster::EnableSubImageComputing(bool pi_Enable)
    {
    m_ComputeSubImage = pi_Enable;
    }



//-----------------------------------------------------------------------------
// public
// GetResamplingForSubResolution
//
// Note : if pi_SubImageIndex == -1 the method return
//        HGSResampling::ResamplingMethod::UNDEFINED if all resolution was not
//        with the same subsampling otherwise, return the raster subsampling.
//-----------------------------------------------------------------------------
HGSResampling::ResamplingMethod HRAPyramidRaster::GetResamplingForSubResolution(int32_t pi_SubImageIndex) const
    {
    HPRECONDITION((pi_SubImageIndex == -1 && m_pSubImageList.BufSize > 0) ||
                  (pi_SubImageIndex >= 0 && pi_SubImageIndex < (int32_t)m_pSubImageList.BufSize));

    HGSResampling::ResamplingMethod ResamplingType;
    if (pi_SubImageIndex == -1)
        {
        ResamplingType = m_pSubImageList.pData[0].m_ResamplingType;
        bool SameResampling = true;
        for (size_t i = 1; i < m_pSubImageList.BufSize && SameResampling; i++)
            SameResampling = ResamplingType == m_pSubImageList.pData[i].m_ResamplingType;

        if (!SameResampling)
            ResamplingType = HGSResampling::UNDEFINED;
        }
    else
        ResamplingType = m_pSubImageList.pData[pi_SubImageIndex].m_ResamplingType;

    return ResamplingType;
    }

//-----------------------------------------------------------------------------
// public
// SetResamplingForSubResolution : Returns false if the resampling method
//                                 is invalide for the pixeltype.
//
// Note: if pi_SubImageIndex == -1, each resolution will be set with pi_Type
//-----------------------------------------------------------------------------
bool HRAPyramidRaster::SetResamplingForSubResolution(HGSResampling::ResamplingMethod pi_Type,
                                                      bool                           pi_RecomputeSubResolution,
                                                      int32_t                        pi_SubImageIndex)
    {
    HPRECONDITION(pi_SubImageIndex == -1 ||
                  (pi_SubImageIndex >= 0 && pi_SubImageIndex < (int32_t)m_pSubImageList.BufSize));

    bool Ret = true;

    // Default, supported by all pixeltypes.
    HGSResampling::ResamplingMethod ResamplingMethod = HGSResampling::NEAREST_NEIGHBOUR;

    if (pi_Type == HGSResampling::VECTOR_AWARENESS)
        ResamplingMethod = HGSResampling::VECTOR_AWARENESS;
    else if (pi_Type == HGSResampling::AVERAGE)
        {
        // Apply only if V24 > and V8
        if ((GetPixelType()->CountPixelRawDataBits() >= 24) ||
            ((GetPixelType()->CountIndexBits() == 0) && (GetPixelType()->CountPixelRawDataBits() == 8)) )
            ResamplingMethod = HGSResampling::AVERAGE;
        else
            Ret = false;
        }
    else if (pi_Type == HGSResampling::ORING4)
        {
        // Apply only if 1 bit
        if (GetPixelType()->CountPixelRawDataBits() == 1)
            ResamplingMethod = HGSResampling::ORING4;
        else
            Ret = false;
        }
    else if(pi_Type == HGSResampling::UNDEFINED)
        ResamplingMethod = HGSResampling::UNDEFINED;

    if (Ret)
        {
        int32_t SubImageIndex = -1;
        if (pi_SubImageIndex == -1)
            {
            // found the first sub resolution that we need to change the resampling method
            for (uint32_t i = 0; i < m_pSubImageList.BufSize; i++)
                {
                if (SubImageIndex == -1 && m_pSubImageList.pData[i].m_ResamplingType != ResamplingMethod)
                    SubImageIndex = i;

                m_pSubImageList.pData[i].m_ResamplingType = ResamplingMethod;
                }
            }
        else if (m_pSubImageList.pData[pi_SubImageIndex].m_ResamplingType != ResamplingMethod)
            {
            m_pSubImageList.pData[pi_SubImageIndex].m_ResamplingType = ResamplingMethod;
            SubImageIndex = pi_SubImageIndex;
            }

        if (SubImageIndex > -1)
            {
            if (pi_RecomputeSubResolution)
                {
                for (size_t res = SubImageIndex; res < m_pSubImageList.BufSize; res++)
                    {
                    // Sets all sub-resolution dirty
                    m_pSubImageList.pData[res].m_SubResolutionIsDirty = true;

                    uint64_t CountTiles;
                    HRATileStatus& rSrcTileStatus = m_pSubImageList.pData[res].m_pSubImage->GetInternalTileStatusList(&CountTiles);
                    for (uint32_t j=0; j<CountTiles; j++)
                        rSrcTileStatus.SetDirtyForSubResFlag(j, true);
                    }
                }

            SetModificationState();
            }
        }

    return (Ret);
    }


//------------------------------------------------------------- Privates

//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
void HRAPyramidRaster::Constructor (const HFCPtr<HRATiledRaster>&  pi_rpRasterModel,
                                    const HFCPtr<HRATiledRaster>&  pi_rpMainImageRasterModel,
                                    uint64_t                       pi_WidthPixels,
                                    uint64_t                       pi_HeightPixels,
                                    SubImageDescription*            pi_pSubImageDesc,
                                    unsigned short                 pi_NumberOfSubImage)
    {
    HPRECONDITION (pi_rpRasterModel != 0);

    // Exception-Safe
    try
        {
        // Make a copy of the RasterModel.
        // Set CoordSys and Extent (1,1)
        //
        m_pRasterModel = (HRATiledRaster*)pi_rpRasterModel->Clone();
        if (GetTransfoModel () != 0)
            {
            m_pRasterModel->HRAStoredRaster::SetTransfoModel (*GetTransfoModel (), GetCoordSys ());
            }

        m_pRasterModel->InitSize (1, 1);

        // Use this object only to compute the ID, specialy the Level(SubImage Number)
        HGFTileIDDescriptor TileDescriptor (1, 1, 1, 1);
        uint32_t            ImageNumber = 0;

        // Create the list of subImages ???
        m_pSubImageList.BufSize  = pi_NumberOfSubImage+1;    // +1 include the main image
        m_pSubImageList.pData    = new ResolutionInfo[m_pSubImageList.BufSize];

        // Flag for SubResolutionDirty image
        // Init, all image not dirty
        for (size_t i = 0; i < m_pSubImageList.BufSize; ++i)
            m_pSubImageList.pData[i].m_SubResolutionIsDirty = false;


        if (m_pSubImageList.BufSize > 0)
            {
            HFCPtr<HRATiledRaster> pRasterModel;

            pRasterModel = pi_rpMainImageRasterModel == 0 ?  pi_rpRasterModel :
                           pi_rpMainImageRasterModel;

            // Set the full image resolution.
            m_pSubImageList.pData[0].m_pSubImage = CreateSubResRaster(pRasterModel,
                                                                      pi_WidthPixels,
                                                                      pi_HeightPixels,
                                                                      0,
                                                                      0,
                                                                      false,
                                                                      TileDescriptor.ComputeID(0,
                                                                              0,
                                                                              ImageNumber));

            HFCPtr<HRPPixelType> pMainImagePixelType(m_pSubImageList.pData[0].m_pSubImage->GetPixelType());

            m_pSubImageList.pData[0].m_ImageResolution = 1.0;
            m_pSubImageList.pData[0].m_PhysicalImageResolution = 1.0;
            m_pSubImageList.pData[0].m_ScaleFromMainX = 1.0;
            m_pSubImageList.pData[0].m_ScaleFromMainY = 1.0;
            m_pSubImageList.pData[0].m_ResamplingType = HGSResampling::UNDEFINED;
            m_pSubImageList.pData[0].m_ComputeResWithOptimizedMethod = true; // not use


            // Link ourselves to the Sub-Resolution Raster, to receive notifications
            LinkTo(m_pSubImageList.pData[0].m_pSubImage);

            // Set all Sub-Images
            //
            for (ImageNumber=1; ImageNumber < m_pSubImageList.BufSize; ImageNumber++)
                {
                // Compute or take image dimension
                //
                uint32_t Width;
                uint32_t Height;
                if (pi_pSubImageDesc[ImageNumber-1].UseDimension)
                    {
                    Width = pi_pSubImageDesc[ImageNumber-1].DimX;
                    Height= pi_pSubImageDesc[ImageNumber-1].DimY;
                    }
                else
                    {
                    Width = (uint32_t)((pi_WidthPixels+1) * pi_pSubImageDesc[ImageNumber-1].Resolution);
                    Height= (uint32_t)((pi_HeightPixels+1) * pi_pSubImageDesc[ImageNumber-1].Resolution);
                    }

                // Associate each image at the Store

                pRasterModel = pi_pSubImageDesc[ImageNumber-1].pTiledRaster == 0 ? pi_rpRasterModel : pi_pSubImageDesc[ImageNumber-1].pTiledRaster;

                m_pSubImageList.pData[ImageNumber].m_pSubImage =
                    CreateSubResRaster(pRasterModel,
                                       Width,
                                       Height,
                                       pi_pSubImageDesc[ImageNumber-1].DimBlockX,
                                       pi_pSubImageDesc[ImageNumber-1].DimBlockY,
                                       true,
                                       TileDescriptor.ComputeID(0,
                                                                0,
                                                                ImageNumber));

                m_SinglePixelType = m_SinglePixelType &&
                                    pMainImagePixelType == m_pSubImageList.pData[ImageNumber].m_pSubImage->GetPixelType();
                // SubImages
                m_pSubImageList.pData[ImageNumber].m_ImageResolution = pi_pSubImageDesc[ImageNumber-1].Resolution;


                // Compute physical resolution
                m_pSubImageList.pData[ImageNumber].m_ScaleFromMainX = (double)(pi_WidthPixels)  / (double)pi_pSubImageDesc[ImageNumber - 1].DimX;
                m_pSubImageList.pData[ImageNumber].m_ScaleFromMainY = (double)(pi_HeightPixels) / (double)pi_pSubImageDesc[ImageNumber - 1].DimY;

                // Take the better resolution between X and Y.
                m_pSubImageList.pData[ImageNumber].m_PhysicalImageResolution = MAX (pi_pSubImageDesc[ImageNumber-1].DimX / (double)(pi_WidthPixels),
                                                                                    pi_pSubImageDesc[ImageNumber-1].DimY / (double)(pi_HeightPixels));

                m_pSubImageList.pData[ImageNumber].m_ResamplingType = pi_pSubImageDesc[ImageNumber-1].ResamplingType;

                
                // CreateSubResRaster create a raster in the same type of pRasterModel. In this case,
                // the resolution before is necessary in the same type of the current resolution
                // so, we have a HRATiledRaster

                uint64_t MainWidth;
                uint64_t MainHeight;
                uint64_t ResWidth;
                uint64_t ResHeight;
                m_pSubImageList.pData[ImageNumber - 1].m_pSubImage->GetSize(&MainWidth, &MainHeight);
                m_pSubImageList.pData[ImageNumber].m_pSubImage->GetSize(&ResWidth, &ResHeight);

                uint64_t MainBlockSizeX = m_pSubImageList.pData[ImageNumber - 1].m_pSubImage->GetTileSizeX();
                uint64_t MainBlockSizeY = m_pSubImageList.pData[ImageNumber - 1].m_pSubImage->GetTileSizeY();
                uint64_t ResBlockSizeX  = m_pSubImageList.pData[ImageNumber].m_pSubImage->GetTileSizeX();
                uint64_t ResBlockSizeY  = m_pSubImageList.pData[ImageNumber].m_pSubImage->GetTileSizeY();

                double MainResolution = (ImageNumber == 1 ? 1.0 : pi_pSubImageDesc[ImageNumber-2].Resolution);
                double ResResolution  = pi_pSubImageDesc[ImageNumber-1].Resolution;

                double RemainderX;
                RemainderX = fmod((double)MainBlockSizeX * MainResolution,
                                    (double)ResBlockSizeX * ResResolution);

                double RemainderY;
                RemainderY = fmod((double)MainBlockSizeY * MainResolution,
                                    (double)ResBlockSizeY * ResResolution);

                // Guaranty that the source data will not be discard before all output data will use it.
                //  ok if modulo == 0
                //  ok if only 1 source data for 1 output data
                //  ok if only 1 output data
                m_pSubImageList.pData[ImageNumber].m_ComputeResWithOptimizedMethod =
                    ((HDOUBLE_EQUAL_EPSILON(RemainderX, 0.0) ||
                        (MainWidth == MainBlockSizeX && ResWidth == ResBlockSizeX) ||
                        (ResWidth == ResBlockSizeX))     // TR 176702
                        &&
                        (HDOUBLE_EQUAL_EPSILON(RemainderY, 0.0) ||
                        (MainHeight == MainBlockSizeY && ResHeight == ResBlockSizeY) ||
                        (ResHeight == ResBlockSizeY)));  // TR 176702

                // Create scaling transfo
                // Set scaling to the SubImages.
                HGF2DStretch Transfo;
                Transfo.SetXScaling(1.0 / m_pSubImageList.pData[ImageNumber].m_ImageResolution);
                Transfo.SetYScaling(1.0 / m_pSubImageList.pData[ImageNumber].m_ImageResolution);

                // Set a new physical coord. sys. to the buffer
                m_pSubImageList.pData[ImageNumber].m_pSubImage->SetTransfoModel(*Transfo.ComposeInverseWithDirectOf (*GetTransfoModel()));

                // Link ourselves to the Sub-Resolution Raster, to receive notifications
                LinkTo(m_pSubImageList.pData[ImageNumber].m_pSubImage);
                }

            // Check if all sub-images are already computed,
            // else set a new state to compute it.
            // Important Note:
            // Can be call only by the constructor
            // Important Note: we assume the HRATiledRaster TileStatus are set to false
            SetSubImageNotComputed  (pi_pSubImageDesc, pi_NumberOfSubImage);
            }
        }
    catch(...)
        {
        DeepDelete();
        }
    }


//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
HFCPtr<HRATiledRaster> HRAPyramidRaster::CreateSubResRaster (const HFCPtr<HRATiledRaster>& pi_rpRasterModel,
                                                             uint64_t                      pi_Width,
                                                             uint64_t                      pi_Height,
                                                             uint64_t                      pi_BlockWidth,
                                                             uint64_t                      pi_BlockHeight,
                                                             bool                          pi_UseBlockSize,
                                                             uint64_t                      pi_TileID) const
    {
    HPRECONDITION(pi_rpRasterModel != 0);
    HPRECONDITION(pi_rpRasterModel->IsCompatibleWith(HRATiledRaster::CLASS_ID));
    HFCPtr<HRATiledRaster> pResult;

    uint64_t BlockHeight = pi_UseBlockSize? pi_BlockHeight : pi_rpRasterModel->GetTileSizeY();

    if (pi_rpRasterModel->IsCompatibleWith(HRAStripedRaster::CLASS_ID))
        {
        HASSERT(pi_Width <= ULONG_MAX);
        HASSERT(pi_Height <= ULONG_MAX);
        HASSERT(BlockHeight <= ULONG_MAX);

        pResult = new HRAStripedRaster(pi_rpRasterModel->m_pBitmapModel,
                                       (uint32_t)BlockHeight,
                                       (uint32_t)pi_Width,
                                       (uint32_t)pi_Height,
                                       GetStore(),
                                       m_pLog,
                                       m_DisableTileStatus);

        // Set RasterID, only use for the moment if a TiledRaster
        pResult->SetID(pi_TileID);
        }
    else
        {
        uint64_t BlockWidth = pi_UseBlockSize? pi_BlockWidth : pi_rpRasterModel->GetTileSizeX();
        
        pResult = new HRATiledRaster(pi_rpRasterModel->m_pBitmapModel,
                                     BlockWidth,
                                     BlockHeight,
                                     pi_Width,
                                     pi_Height,
                                     GetStore(),
                                     m_pLog,
                                     m_DisableTileStatus);

        // Set RasterID, only use for the moment if a TiledRaster
        pResult->SetID(pi_TileID);
        }

    return pResult;
    }



//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HRAPyramidRaster::DeepCopy(const HRAPyramidRaster& pi_rRaster,
                                HPMObjectStore*         pi_pStore,
                                HPMPool*                pi_pLog)
    {
    // Exception-Safe
    try
        {
        // Copy the model
        m_pRasterModel = (HRATiledRaster*)pi_rRaster.m_pRasterModel->Clone();

        m_SinglePixelType = pi_rRaster.m_SinglePixelType;

        m_pSubImageList.BufSize = pi_rRaster.m_pSubImageList.BufSize;
        m_pSubImageList.pData = new ResolutionInfo[m_pSubImageList.BufSize];

        for (unsigned short i = 0; i < m_pSubImageList.BufSize; ++i)
            {
            m_pSubImageList.pData[i].m_ImageResolution = pi_rRaster.m_pSubImageList.pData[i].m_ImageResolution;
            m_pSubImageList.pData[i].m_PhysicalImageResolution = pi_rRaster.m_pSubImageList.pData[i].m_PhysicalImageResolution;
            m_pSubImageList.pData[i].m_ScaleFromMainX = pi_rRaster.m_pSubImageList.pData[i].m_ScaleFromMainX;
            m_pSubImageList.pData[i].m_ScaleFromMainY = pi_rRaster.m_pSubImageList.pData[i].m_ScaleFromMainY;
            m_pSubImageList.pData[i].m_ComputeResWithOptimizedMethod = pi_rRaster.m_pSubImageList.pData[i].m_ComputeResWithOptimizedMethod;
            m_pSubImageList.pData[i].m_SubResolutionIsDirty = false;
            m_pSubImageList.pData[i].m_ResamplingType = pi_rRaster.m_pSubImageList.pData[i].m_ResamplingType;

            // Copy sub-resolution.
            m_pSubImageList.pData[i].m_pSubImage = static_cast<HRATiledRaster*>(pi_rRaster.m_pSubImageList.pData[i].m_pSubImage->Clone (pi_pStore, pi_pLog).GetPtr());

            // Link ourselves to the Sub-Resolution Raster, to receive notifications
            LinkTo(m_pSubImageList.pData[i].m_pSubImage);
            }

        m_ComputeSubImage   = pi_rRaster.m_ComputeSubImage;
        m_UseOnlyFirstRes   = pi_rRaster.m_UseOnlyFirstRes;
        m_DisableTileStatus = pi_rRaster.m_DisableTileStatus;

        m_pLog          = pi_pLog;
        SetStore(pi_pStore);
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
void HRAPyramidRaster::DeepDelete()
    {
    delete[] m_pSubImageList.pData;
    m_pSubImageList.pData   = 0;
    m_pSubImageList.BufSize = 0;
    }


//-----------------------------------------------------------------------------
// private
// ReplaceObject - Replace the object by the object in parameter and delete it
//-----------------------------------------------------------------------------
void HRAPyramidRaster::ReplaceObject (HFCPtr<HRAPyramidRaster>& pio_pRaster)
    {
    // For the performance do not call HRATiledRaster::Operator=
    //
    HRAStoredRaster::operator=(*pio_pRaster);

//DMx
//    Remove each resolution from the Store(HOD)
//      1- TiledRaster with only tile in the Store
//      2- TiledRaster completely on a Store.


    // Delete currently allocated memory for the object
    DeepDelete ();

    // Replace value and pointer
    m_pSubImageList.pData           = pio_pRaster->m_pSubImageList.pData;
    m_pSubImageList.BufSize         = pio_pRaster->m_pSubImageList.BufSize;

    m_ComputeSubImage               = pio_pRaster->m_ComputeSubImage;

    m_SinglePixelType               = pio_pRaster->m_SinglePixelType;

    // Link the list of TiledRaster with the object and
    // Unlink the list from the pio_pRaster
    for (uint32_t ImageNumber=0; ImageNumber < m_pSubImageList.BufSize; ImageNumber++)
        {
        pio_pRaster->UnlinkFrom (m_pSubImageList.pData[ImageNumber].m_pSubImage);
        LinkTo (m_pSubImageList.pData[ImageNumber].m_pSubImage);
        }


    // Set to 0 --> Destructor must not delete these members.
    pio_pRaster->m_pSubImageList.pData                      = 0;
    pio_pRaster->m_pRasterModel                             = 0;

    // NOP the UpdateSubResolution method
    pio_pRaster->m_pSubImageList.BufSize     = 0;
    }

//-----------------------------------------------------------------------------
// private
// FindTheBestResolution  -
// This function studies the scaling factor around the raster to estimate
// the resolution that must be used to satisfy the granularity of the
// provided coordinate system.
//-----------------------------------------------------------------------------

double HRAPyramidRaster::FindTheBestResolution (const HFCPtr<HGF2DCoordSys>& pi_rPhysicalCoordSys,
                                                 const HFCPtr<HGF2DCoordSys>  pi_pNewLogicalCoordSys) const
    {
    // Do you have sub-resolution ?
    if ((m_pSubImageList.BufSize > 1) && pi_rPhysicalCoordSys != 0)
        {
        HFCPtr<HGF2DCoordSys> pEffPhysicalCs = GetPhysicalCoordSys();
        if (pi_pNewLogicalCoordSys != 0)
            {
            HFCPtr<HGF2DTransfoModel> pTransfo = pi_rPhysicalCoordSys->GetTransfoModelTo(pi_pNewLogicalCoordSys);
            pTransfo = pTransfo->ComposeInverseWithDirectOf(*GetCoordSys()->GetTransfoModelTo(GetPhysicalCoordSys()));
            pTransfo->Reverse();
            pEffPhysicalCs = new HGF2DCoordSys(*pTransfo, pi_rPhysicalCoordSys);
            }
        HVEShape extent(GetExtent());
        extent.ChangeCoordSys(GetPhysicalCoordSys());
        extent.SetCoordSys(pEffPhysicalCs);

        double scale = 1.0 / EvaluateScaleFactor(pi_rPhysicalCoordSys, pEffPhysicalCs, extent);
        
        return (scale);
        }

    return 1.0;
    }

//-----------------------------------------------------------------------------
// private
// SetSubImageNotComputed - Set state, to compute the current sub-image and
//                          all ohter after it.
// Important Note:
// Can be call only by the constructor
// Important Note: we assume the HRATiledRaster TileStatus are set to false
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SetSubImageNotComputed (SubImageDescription*  pi_pSubImageDesc,
                                               unsigned short       pi_NumberOfSubImage)
    {
    // Don't set any resolution dirty if we have m_DisableTileStatus == true
    // Normally m_DisableTileStatus == true if the file is ReadOnly.
    if (!m_DisableTileStatus)
        {
        unsigned short i;
        for (i = 0; i < pi_NumberOfSubImage; i++)
            {
            // Rem: pi_NumberOfSubImage don't include the main image.
            //
            if (!pi_pSubImageDesc[i].ResolutionComputed)
                {
                // If the Raster is a TileRaster
                //      Change State of each tile in the previous image, to force to recompute
                //      the Sub-Images
                
                uint64_t CountTiles;
                HRATileStatus& rSrcTileStatus = m_pSubImageList.pData[i].m_pSubImage->GetInternalTileStatusList(&CountTiles);

                for (uint32_t j = 0; j < CountTiles; j++)
                    rSrcTileStatus.SetDirtyForSubResFlag(j, true);

                // Set the previous image is Dirty --> compute the Sub-Resolution.
                m_pSubImageList.pData[i].m_SubResolutionIsDirty = true;
                }
            // else Important Note: we assume the HRATiledRaster TileStatus are set to false
            }

        // Set Dirty the last resolution
        m_pSubImageList.pData[i].m_SubResolutionIsDirty = true;
        }
    }

void HRAPyramidRaster::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    HRAStoredRaster::CopyFromLegacy(pi_pSrcRaster, pi_rOptions);
    }

void HRAPyramidRaster::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSinkNodePtr HRAPyramidRaster::_GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType)
    {
    //&&OPTIMIZATION: We need to do better and avoid cloning the shape. pyramid and resolution 0 do not have the same CS. why? For now change CS, to accommodate tiledRaster.
    HVEShape shapeInTiledRasterCS(sinkShape);
    shapeInTiledRasterCS.ChangeCoordSys(m_pSubImageList.pData[0].m_pSubImage->GetPhysicalCoordSys());

    // Editing always occurs on the main res.
    return m_pSubImageList.pData[0].m_pSubImage->GetSinkNode(status, shapeInTiledRasterCS, pReplacingPixelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAPyramidRaster::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR pi_Options)
    {
    BeAssert(NULL != pi_Options.GetShape()); // Required to update sub-res
    BeAssert(NULL != pi_Options.GetShape()->GetShapePtr()); // Required to update sub-res

    unsigned short resolutionIndex = -1;

    // Do you have sub-resolution and the image is not a binary image
    //completely loaded in memory (optimization)
    if ((m_pSubImageList.BufSize > 1) && (m_UseOnlyFirstRes == false))
        {
        // find the best resolution for the surface
        double Resolution = FindTheBestResolution(imageNode.GetPhysicalCoordSys(), pi_Options.GetReplacingCoordSys());

        // if the resolution is bigger than the first one use the first one
        if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[0].m_PhysicalImageResolution))
            {
            resolutionIndex = 0;
            }
        else
            {
            // find at what index is that resolution
            resolutionIndex  = (unsigned short)m_pSubImageList.BufSize - 1;   // last res of the pyramid
            bool   ResFound = false;
            for (unsigned short Res = 0; (!ResFound) && (Res < m_pSubImageList.BufSize - 1); Res++)
                {
                // if the res is between the current res and the next, use that res
                if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[Res].m_PhysicalImageResolution) &&
                    HDOUBLE_GREATER_EPSILON(Resolution, m_pSubImageList.pData[Res + 1].m_PhysicalImageResolution))
                    {
                    resolutionIndex = Res;
                    ResFound = true;
                    }
                }
            }
        }
    else
        resolutionIndex = 0;

    //&&Backlog update resolution must take the neighbourhood into account. Can we test and update tiles as we request them? We should! 
    //  >>> If we request physical tiles we would not need all this CS stuff below.
    //  >>> Old way of doing flags is slow. See new tiledraster prototype.
    if (resolutionIndex > 0)
        {
        HGF2DExtent destExtent(pi_Options.GetShape()->GetExtent());

        // If a replacing coord sys is provided, move the shape over the source as seen from the destination.
        if (pi_Options.GetReplacingCoordSys() != 0)
            {
            // Set the stroking tolerance for the shape
            // Set a quarter of a pixel tolerance
            double CenterX = ((destExtent.GetXMin() + destExtent.GetXMax()) / 2.0);
            double CenterY = ((destExtent.GetYMin() + destExtent.GetYMax()) / 2.0);
            HFCPtr<HGFTolerance> pTol = new HGFTolerance (CenterX - DEFAULT_PIXEL_TOLERANCE, CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                          CenterX + DEFAULT_PIXEL_TOLERANCE, CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                          pi_Options.GetShape()->GetCoordSys());

            HFCPtr<HVEShape> pDestShape = new HVEShape(*pi_Options.GetShape());
            pDestShape->SetStrokeTolerance(pTol);

            pDestShape->ChangeCoordSys(pi_Options.GetReplacingCoordSys());
            pDestShape->SetCoordSys(GetCoordSys());

            destExtent = pDestShape->GetExtent();
            }

        UpdateSubResolution(resolutionIndex, &destExtent);
        }

    ImagePPStatus status = m_pSubImageList.pData[resolutionIndex].m_pSubImage->BuildCopyToContext(imageNode, pi_Options);

    if (status != IMAGEPP_STATUS_Success)
        return status;

    BeAssert(NULL != imageNode.GetSourceNodeP());
    BeAssert(0.0 != m_pSubImageList.pData[resolutionIndex].m_ScaleFromMainX);
    BeAssert(0.0 != m_pSubImageList.pData[resolutionIndex].m_ScaleFromMainY);
    imageNode.GetSourceNodeP()->SetScaleFactorFromMain(m_pSubImageList.pData[resolutionIndex].m_ScaleFromMainX, m_pSubImageList.pData[resolutionIndex].m_ScaleFromMainY);

    return status;
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRAPyramidRaster::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    HRADrawOptions Options(pi_Options);

    int32_t resolutionIndex = 0;
    
    // Do you have sub-resolution and the image is not a binary image
    //completely loaded in memory (optimization)
    if ((m_pSubImageList.BufSize > 1) && (m_UseOnlyFirstRes == false))
        {
        // find the best resolution for the surface
        double Resolution = FindTheBestResolution(pio_destSurface.GetCoordSys(), Options.GetReplacingCoordSys());

        // if the resolution is bigger than the first one use the first one
        if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[0].m_PhysicalImageResolution))
            {
            resolutionIndex = 0;
            }
        else
            {
            // find at what index is that resolution
            resolutionIndex  = (unsigned short)m_pSubImageList.BufSize - 1;   // last res of the pyramid
            bool   ResFound = false;
            for (unsigned short Res = 0; (!ResFound) && (Res < m_pSubImageList.BufSize - 1); Res++)
                {
                // if the res is between the current res and the next, use that res
                if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[Res].m_PhysicalImageResolution) &&
                    HDOUBLE_GREATER_EPSILON(Resolution, m_pSubImageList.pData[Res + 1].m_PhysicalImageResolution))
                    {
                    resolutionIndex = Res;
                    ResFound = true;
                    }
                }
            }
        }
    else
        resolutionIndex = 0;

    if (resolutionIndex > 0)
        {
        HFCPtr<HGSSurfaceDescriptor> pDstDescriptor(pio_destSurface.GetSurfaceDescriptor());
        HVEShape SurfaceExtent(0.0, 0.0, pDstDescriptor->GetWidth(), pDstDescriptor->GetHeight(), pio_destSurface.GetSurfaceCoordSys());

        // Set the stroking tolerance for the shape
        // Set a quarter of a pixel tolerance
        double CenterX = pDstDescriptor->GetWidth() / 2.0;
        double CenterY = pDstDescriptor->GetHeight() / 2.0;
        HFCPtr<HGFTolerance> pTol = new HGFTolerance (CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                        CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                        CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                        CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                        pio_destSurface.GetSurfaceCoordSys());

        SurfaceExtent.SetStrokeTolerance(pTol);

        if (Options.GetReplacingCoordSys() != 0)
            {
            SurfaceExtent.ChangeCoordSys(Options.GetReplacingCoordSys());
            SurfaceExtent.SetCoordSys(GetCoordSys());
            }

        HGF2DExtent Extent(SurfaceExtent.GetExtent());
        ((HRAPyramidRaster*)this)->UpdateSubResolution(resolutionIndex, &Extent);
        }

    // Draw the selected resolution
    m_pSubImageList.pData[resolutionIndex].m_pSubImage->Draw(pio_destSurface, pi_Options);
    }

//-----------------------------------------------------------------------------
// public
// GetNbSubResTilesToUpdate
//-----------------------------------------------------------------------------
uint64_t HRAPyramidRaster::GetNbSubResTilesToUpdate(unsigned short pi_ResIndex, HGF2DExtent& pi_rSurfaceExtent)
    {
    HPRECONDITION(pi_ResIndex > 0);

    HGF2DExtent                RequestedExtent = pi_rSurfaceExtent;
    HFCPtr<HRATiledRaster>    pDestTiledRaster = 0;
    double                    CutTileSize = 0;
    uint64_t                  NbTilesToUpdate = 0;

    for (int SubResInd = pi_ResIndex; SubResInd > 0; SubResInd--)
        {
        pDestTiledRaster = m_pSubImageList.pData[SubResInd].m_pSubImage;

        HVEShape Shape(RequestedExtent);
        Shape.ChangeCoordSys(pDestTiledRaster->GetPhysicalCoordSys());
        RequestedExtent = Shape.GetExtent();

        if (m_pSubImageList.pData[SubResInd].m_SubResolutionIsDirty || m_pSubImageList.pData[SubResInd-1].m_SubResolutionIsDirty)
            {
            NbTilesToUpdate += pDestTiledRaster->GetPtrTileDescription()->GetTileCount(RequestedExtent);
            }

        if (SubResInd > 1)
            {
            HASSERT(pDestTiledRaster->GetPtrTileDescription()->GetTileWidth() <= ULONG_MAX);
            HASSERT(pDestTiledRaster->GetPtrTileDescription()->GetTileHeight() <= ULONG_MAX);

            uint32_t TileWidth = (uint32_t)pDestTiledRaster->GetPtrTileDescription()->GetTileWidth();
            uint32_t TileHeight = (uint32_t)pDestTiledRaster->GetPtrTileDescription()->GetTileHeight();

            double tempVal = RequestedExtent.GetXMin();
            tempVal = RequestedExtent.GetXMin() - fmod(RequestedExtent.GetXMin(), TileWidth);
            RequestedExtent.SetXMin(tempVal);
            CutTileSize = fmod(RequestedExtent.GetXMax(), TileWidth);
            if (CutTileSize > 0)
                {
                tempVal = RequestedExtent.GetXMax() + (TileWidth - CutTileSize);
                RequestedExtent.SetXMax(tempVal);
                }
            tempVal = (RequestedExtent.GetYMin() - fmod(RequestedExtent.GetYMin(), TileHeight));
            RequestedExtent.SetYMin(tempVal);
            CutTileSize = fmod(RequestedExtent.GetYMax(), TileWidth);
            if (CutTileSize > 0)
                {
                tempVal = (RequestedExtent.GetYMax() + (TileHeight - CutTileSize));
                RequestedExtent.SetYMax(tempVal);
                }
            }
        }

    return NbTilesToUpdate;
    }

//-----------------------------------------------------------------------------
// public
// SetContext
//-----------------------------------------------------------------------------
void HRAPyramidRaster::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    HRAStoredRaster::SetContext(pi_rpContext);

    //Should set the context to each tiled raster
    for (unsigned short ResInd = 0; ResInd < m_pSubImageList.BufSize; ResInd++)
        {
        m_pSubImageList.pData[ResInd].m_pSubImage->SetContext(pi_rpContext);
        }
    }

//-----------------------------------------------------------------------------
// public
// Clone -
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRAPyramidRaster::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    // Make a tmp Raster
    HRAPyramidRaster* pTmpRaster = new HRAPyramidRaster();

    pTmpRaster->HRAStoredRaster::operator=(*this);

    pTmpRaster->DeepDelete();
    pTmpRaster->DeepCopy (*this, pi_pStore, pi_pLog);

    return pTmpRaster;
    }
//-----------------------------------------------------------------------------
// public
// Clone -
//-----------------------------------------------------------------------------
HPMPersistentObject* HRAPyramidRaster::Clone () const
    {
    // Make a tmp Raster
    return new HRAPyramidRaster(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned short HRAPyramidRaster::EvaluateResolution(double Resolution) const
    {
    // if the resolution is bigger than the first one use the first one
    if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[0].m_PhysicalImageResolution))
        return 0;

    // find at what index is that resolution
    for (unsigned short Res = 0; Res < m_pSubImageList.BufSize - 1; ++Res)
        {
        // if the res is between the current res and the next, use that res
        if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(Resolution, m_pSubImageList.pData[Res].m_PhysicalImageResolution) &&
            HDOUBLE_GREATER_EPSILON(Resolution, m_pSubImageList.pData[Res + 1].m_PhysicalImageResolution))
            return Res;
        }

    return (unsigned short)m_pSubImageList.BufSize - 1;   // last res of the pyramid
    }


