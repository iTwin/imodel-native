//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRARaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HRARasterIterator.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRPQuantizedPalette.h>
#include <Imagepp/all/h/HMDContext.h>




HPM_REGISTER_ABSTRACT_CLASS(HRARaster, HGFRaster)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRARaster, HGFRaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRARaster, HRAContentChangedMsg, NotifyContentChanged)
HMG_END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// Default Constructor.
//-----------------------------------------------------------------------------
HRARaster::HRARaster   ()
    : HGFRaster (),
      m_pShape (new HVEShape())
    {
    // set the representative palette cache to NULL
    InitRepPalCache();
    }


//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------
HRARaster::HRARaster   (const HFCPtr<HGF2DCoordSys>& pi_rCoordSys)
    : HGFRaster (pi_rCoordSys),
      m_pShape (new HVEShape(pi_rCoordSys))
    {
    // set the representative palette cache to NULL
    InitRepPalCache();
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRARaster::HRARaster   (const HRARaster& pi_rObj)
    : HGFRaster(pi_rObj),
      m_pShape(new HVEShape(*pi_rObj.m_pShape))
    {
    // Cache to optimize compute histogram
    if (pi_rObj.m_pHistogram != 0)
        m_pHistogram = new HRAHistogramOptions(*pi_rObj.m_pHistogram);

    // set the representative palette cache to NULL
    InitRepPalCache();
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
HRARaster::~HRARaster()
    {
    }


//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HRARaster& HRARaster::operator=(const HRARaster& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGFRaster::operator=(pi_rObj);

        m_pShape = new HVEShape(*pi_rObj.m_pShape);

        // Cache to optimize compute histogram
        if (pi_rObj.m_pHistogram != 0)
            m_pHistogram = new HRAHistogramOptions(*pi_rObj.m_pHistogram);

        // set the representative palette cache to NULL
        InitRepPalCache();
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// public
// SetShape
//-----------------------------------------------------------------------------
void HRARaster::SetShape (const HVEShape& pi_rShape)
    {
    // Create message now with the soon to be old shape.
    HRAEffectiveShapeChangedMsg msg(*m_pShape);

    SetShapeImpl(pi_rShape);

    // Effective shape is now probably dirty
    RecalculateEffectiveShape();

    // Set the modification state of the shape since it have been changed.
    m_pShape->SetModificationState();

    // Notify every registered raster object that our shape has changed
    Propagate(msg);
    }

//-----------------------------------------------------------------------------
// protected
// SetShapeImpl
//-----------------------------------------------------------------------------
void HRARaster::SetShapeImpl (const HVEShape& pi_rShape)
    {
    SetModificationState();
    m_pShape = new HVEShape(pi_rShape);
    }

//-----------------------------------------------------------------------------
// protected
// SetCoordSysImplementation
//-----------------------------------------------------------------------------
void HRARaster::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys)
    {
    // Call ancestor's method
    HGFGraphicObject::SetCoordSysImplementation(pi_rOldCoordSys);

    // Got to move our shape accordingly
    m_pShape->ChangeCoordSys(pi_rOldCoordSys);
    m_pShape->SetCoordSys(GetCoordSys());

    // Effective shape is now dirty
    RecalculateEffectiveShape();
    }


//-----------------------------------------------------------------------------
// Called when some change necessitates a recalc. of the effect. shape
//-----------------------------------------------------------------------------
void HRARaster::RecalculateEffectiveShape ()
    {
    }

/** ---------------------------------------------------------------------------
    Get the raster's extent in a specified CS
    In some circumstances, a raster can override this method to achieve
    performance improvements by avoiding the computation of the effective shape.
    ---------------------------------------------------------------------------
*/
HGF2DExtent HRARaster::GetExtentInCs(HFCPtr<HGF2DCoordSys> pi_pCoordSys) const
    {
    if (pi_pCoordSys != GetEffectiveShape()->GetCoordSys())
        {
        // Copy the effective shape
        HFCPtr<HVEShape> pTmpShape = new HVEShape (*GetEffectiveShape());

        // Bring it into the provided coordsys.
        pTmpShape->ChangeCoordSys(pi_pCoordSys);

        return pTmpShape->GetExtent();
        }
    else
        {
        // Uses the effective shape directly
        return GetEffectiveShape()->GetExtent();
        }
    }

//-----------------------------------------------------------------------------
// Receive "content changed" notification
//-----------------------------------------------------------------------------
bool HRARaster::NotifyContentChanged (const HMGMessage& pi_rMessage)
    {
    // Content change, remove the Histogram cache.
    m_pHistogram = 0;

    // Propagate message
    return true;
    }


//-----------------------------------------------------------------------------
// protected:
//
// UpdateRepPalCache - Update the representative palette cache
//-----------------------------------------------------------------------------
void HRARaster::UpdateRepPalCache(unsigned short pi_CountUsed, const HRPPixelPalette& pi_rPalette)
    {
    // create a new palette cache; a copy of the input palette
    if((m_pRepPalCache = new HRPPixelPalette(pi_rPalette)))
        {
        // validate the palette
        m_RepPalValide = true;

        // number of entries used in the palette
        m_RepPalCountUsed = pi_CountUsed;
        }
    }

//-----------------------------------------------------------------------------
// protected:
//
// InvalidateRepPalCache - Update the representative palette cache
//-----------------------------------------------------------------------------
void HRARaster::InvalidateRepPalCache()
    {
    m_RepPalValide = false;
    }

//-----------------------------------------------------------------------------
// Print the state of the object
//-----------------------------------------------------------------------------
void HRARaster::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE

    // Call the parent
    HGFRaster::PrintState (po_rOutput);

    po_rOutput

            << "HRARaster"
            << endl;

#endif
    }


//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRARaster::Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    _Draw(pio_destSurface, pi_Options);
    }

//-----------------------------------------------------------------------------
// Check if the raster is opaque
//-----------------------------------------------------------------------------
bool HRARaster::IsOpaque() const
    {
    return !ContainsPixelsWithChannel(HRPChannelType::ALPHA);
    }


//-----------------------------------------------------------------------------
// Initialize the Histogram cache, internal use only.
// ComputeHistogram, use this member for optimization.
//
// This method must be Private, but HRSObjectStore must use it, to load
// a Raster form a file.
//-----------------------------------------------------------------------------
void HRARaster::InitializeHistogram(const HRPHistogram&     pi_rHistogram,
                                    const HRPPixelType&     pi_rPixelType)
    {
    m_pHistogram = new HRAHistogramOptions(new HRPHistogram(pi_rHistogram), &pi_rPixelType);

    // Set quality to 100%
    HRASamplingOptions SamplingOpt;
    SamplingOpt.SetPixelsToScan(100);
    SamplingOpt.SetTilesToScan(100);
    SamplingOpt.SetPyramidImageSize(100);
    m_pHistogram->SetSamplingOptions(SamplingOpt);
    }


//-----------------------------------------------------------------------------
// protected
// SetHistogram
//-----------------------------------------------------------------------------
void HRARaster::SetHistogram(const HRAHistogramOptions* pi_pHistogram)
    {
    // TR 185241:
    // For now, we only support native histogram because this is the one that will be written to disk(iTiff, hmr)
    // Histogram update is handle by XYZ::NotifyContentChanged() methods.
    if(pi_pHistogram->GetSamplingColorSpace() == HRPHistogram::NATIVE)
        {
        m_pHistogram = new HRAHistogramOptions(*pi_pHistogram);
        m_pHistogram->GetHistogram()->SetModificationState();
        }
    }


//-----------------------------------------------------------------------------
// StartHistogramEditMode
//
// Start editing actual global histogram.
// (See implementation in class HRAPyramidRaster)
//-----------------------------------------------------------------------------
bool HRARaster::StartHistogramEditMode()
    {
    return false;
    }


//-----------------------------------------------------------------------------
// CreateIterator
// Default returns NULL.
//-----------------------------------------------------------------------------
HRARasterIterator* HRARaster::CreateIterator(const HRAIteratorOptions& pi_rOptions) const
    {
    return 0;
    }


//-----------------------------------------------------------------------------
// ComputeHistogram
// Use the iterator.
//-----------------------------------------------------------------------------
void HRARaster::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                 bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    if (pi_ForceRecompute || (GetHistogram() == 0) || (GetHistogram() != 0 && !GetHistogram()->CanBeUsedInPlaceOf(*pio_pOptions)))
        {
        // we copy the options structure
        HRAHistogramOptions TmpOptions(*pio_pOptions);

        // create an iterator
        HAutoPtr<HRARasterIterator> pSrcIterator(CreateIterator());

        // Inform of invalid iterator onstruction
        HASSERT(pSrcIterator != 0);

        if (pSrcIterator != 0)
            {
            uint32_t Index;
            uint32_t Entries  = pio_pOptions->GetHistogram()->GetEntryFrequenciesSize();
            uint32_t Channels = pio_pOptions->GetHistogram()->GetChannelCount();
            HFCPtr<HRARaster> pRaster;

            while((*pSrcIterator)() != 0)
                {
                // get the current raster
                pRaster = (*pSrcIterator)();

                // copy the histogram for each raster
                TmpOptions.SetHistogram(new HRPHistogram(Entries, Channels));

                // compute the histogram
                pRaster->ComputeHistogram(&TmpOptions, pi_ForceRecompute);

                for (uint32_t ChannelIndex = 0; ChannelIndex < Channels; ChannelIndex++)
                    {
                    // add each entry in the histogram
                    for(Index = 0; Index < Entries; Index++)
                        pio_pOptions->GetHistogram()->IncrementEntryCount(Index,
                                                                          TmpOptions.GetHistogram()->GetEntryCount(Index, ChannelIndex),
                                                                          ChannelIndex);
                    }

                pSrcIterator->Next();
                }

            SetHistogram(pio_pOptions);
            }
        }
    else
        {
        HPRECONDITION(GetHistogram()->GetHistogram()->GetEntryFrequenciesSize() <=
                      pio_pOptions->GetHistogram()->GetEntryFrequenciesSize());

        HFCPtr<HRPHistogram> pHisto(new HRPHistogram(*GetHistogram()->GetHistogram()));
        HFCPtr<HRPHistogram> pOutHisto(pio_pOptions->GetHistogram());

        HASSERT(pOutHisto->GetChannelCount() == pHisto->GetChannelCount());
        // add each entry in the histogram
        for(uint32_t ChannelIndex = 0; ChannelIndex < pHisto->GetChannelCount(); ChannelIndex++)
            {
            for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize(); i++)
                pOutHisto->IncrementEntryCount(i, pHisto->GetEntryCount(i,ChannelIndex),ChannelIndex);
            }
        }
    }


//-----------------------------------------------------------------------------
// GetRepresentativePaletteCache
//-----------------------------------------------------------------------------
unsigned short HRARaster::GetRepresentativePaletteCache(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short CountUsed = 0;

    if(pio_pRepPalParms->UseCache())
        {
        // do not forget to initialuze n_RepPalCache to NULL and to destruct this same
        // object at then end if not NULL

        const HRPPixelPalette& rPalette = (pio_pRepPalParms->GetPixelType())->GetPalette();

        // Verify if we need to allocate a new palette cache
        // Test if it is the first time we enter in this method
        if(m_pRepPalCache == 0)
            // curent palette cache is not valide
            m_RepPalValide = false;
        else
            // test if the palette organization is different than the previous time
            if((rPalette.CountUsedEntries() != m_pRepPalCache->CountUsedEntries()) ||
               (rPalette.GetChannelOrg() != m_pRepPalCache->GetChannelOrg()) ||
               (rPalette.GetLockedEntryIndex() != m_pRepPalCache->GetLockedEntryIndex()))

                {
                // current palette cache is not valide
                m_RepPalValide = false;
                }


        // test if the palette cache is valide
        if(m_RepPalValide)
            {
            HRPPixelPalette& rChgPalette = (pio_pRepPalParms->GetPixelType())->LockPalette();

            CountUsed = m_RepPalCountUsed;

            uint32_t EntriesCount = CountUsed;

            // if yes, copy all the entries of the cache to the palette parameter
            for(uint32_t Index = 0; Index < rChgPalette.CountUsedEntries() && EntriesCount > 0; Index++)
                {
                if(((int32_t)Index) != rChgPalette.GetLockedEntryIndex())
                    {
                    rChgPalette.SetCompositeValue(Index, m_pRepPalCache->GetCompositeValue(Index));
                    EntriesCount--;
                    }
                }

            (pio_pRepPalParms->GetPixelType())->UnlockPalette();
            }
        }

    return CountUsed;
    }


//-----------------------------------------------------------------------------
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRARaster::GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short CountUsed = GetRepresentativePaletteCache(pio_pRepPalParms);

    // test if not updated
    if(CountUsed == 0)
        {
        // we copy the representative palette parameters structure and
        // its pixel type to not notify the tiled raster after each
        // unlock in the tiles
        HRARepPalParms RepPalParms(*pio_pRepPalParms);
        RepPalParms.SetPixelType((HRPPixelType*)RepPalParms.GetPixelType()->Clone());

        // create an iterator
        HAutoPtr<HRARasterIterator> pSrcIterator(CreateIterator());
        HASSERT(pSrcIterator != 0);

        if (pSrcIterator != 0)
            {
            const HRPPixelPalette& rPalette = (RepPalParms.GetPixelType())->GetPalette();
            uint32_t Index;
            uint32_t Entries;
            HFCPtr<HRARaster> pRaster;

            // create a quantized palette object
            HAutoPtr<HRPQuantizedPalette> pQuantizedPalette (pio_pRepPalParms->CreateQuantizedPalette());
            HASSERT(pQuantizedPalette != 0);

            while((*pSrcIterator)() != 0)
                {
                // get the current raster
                pRaster = (*pSrcIterator)();

                // we create a new histogram for each tile if there is one
                if(pio_pRepPalParms->GetHistogram() != 0)
                    RepPalParms.SetHistogram(new HRPHistogram(rPalette));

                // get the representative palette of an element in the mosaic
                pRaster->GetRepresentativePalette(&RepPalParms);
                Entries = rPalette.CountUsedEntries();

                // insert each entry of the representative palette in the
                // quantized object
                for(Index = 0; Index < Entries; Index++)
                    {
                    if(RepPalParms.GetHistogram() != 0)
                        pQuantizedPalette->AddCompositeValue(
                            rPalette.GetCompositeValue(Index),
                            RepPalParms.GetHistogram()->GetEntryCount(Index));

                    else
                        pQuantizedPalette->AddCompositeValue(rPalette.GetCompositeValue(Index));
                    }

                pSrcIterator->Next();
                }

            // get the number of entries in the quantized palette
            CountUsed = pQuantizedPalette->GetPalette(&((pio_pRepPalParms->GetPixelType())->LockPalette()),
                                                      pio_pRepPalParms->GetHistogram());
            (pio_pRepPalParms->GetPixelType())->UnlockPalette();

            if(pio_pRepPalParms->UseCache())
                {
                // update the palette cache if required
                UpdateRepPalCache(CountUsed, (pio_pRepPalParms->GetPixelType())->GetPalette());
                }
            }
        }

    return CountUsed;
    }


//-----------------------------------------------------------------------------
// public
// IsStoredRaster   - Default implementation.
//-----------------------------------------------------------------------------
bool HRARaster::IsStoredRaster () const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// public
// SetContext : Set the list of volatile layers
//-----------------------------------------------------------------------------
void HRARaster::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    //By default, do nothing
    }

//-----------------------------------------------------------------------------
// public
// GetContext : Get the changeable non persistent information about
//              layers contains in the image, if any.
//-----------------------------------------------------------------------------
HFCPtr<HMDContext> HRARaster::GetContext()
    {
    return 0;
    }


//-----------------------------------------------------------------------------
// public
// Locate
//-----------------------------------------------------------------------------
HGFGraphicObject::Location HRARaster::Locate(const HGF2DLocation& pi_rPoint) const
    {
    return (GetEffectiveShape()->GetShapePtr()->Locate(pi_rPoint));
    }


//-----------------------------------------------------------------------------
// protected
// GetHistogram : Note, the pointer can be 0, if no histogram.
//-----------------------------------------------------------------------------
const HRAHistogramOptions* HRARaster::GetHistogram() const
    {
    return m_pHistogram;
    }

//-----------------------------------------------------------------------------
// private
// Initialise Representative Palette Cache
//-----------------------------------------------------------------------------
void HRARaster::InitRepPalCache ()
    {
    m_pRepPalCache      = 0;
    m_RepPalValide      = false;
    m_RepPalCountUsed   = 0;
    }

//-----------------------------------------------------------------------------
// public
// Clone - Overrides HPMPersistentObject...
//-----------------------------------------------------------------------------
HPMPersistentObject* HRARaster::Clone() const
    {
    return Clone(0,0);
    }

//-----------------------------------------------------------------------------
// Public (must be overriden to be used)
// Indicates if LookAhead optimization can be performed
//-----------------------------------------------------------------------------
bool HRARaster::HasLookAhead() const
    {
    return (false);
    }


//-----------------------------------------------------------------------------
// Public (must be overriden to be used)
// Perform LookAhead optimization on the given extent
//-----------------------------------------------------------------------------
void HRARaster::SetLookAhead(const HVEShape& pi_rShape,
    uint32_t        pi_ConsumerID,
    bool           pi_Async)
    {
    HPRECONDITION(HasLookAhead());
    HASSERT(false);
    }

//-----------------------------------------------------------------------------
// public
// GetShape
//-----------------------------------------------------------------------------
const HVEShape& HRARaster::GetShape () const
    {
    return *m_pShape;
    }

//-----------------------------------------------------------------------------
// public
// GetExtent
//-----------------------------------------------------------------------------
HGF2DExtent HRARaster::GetExtent() const
    {
    // Return extent constructed from our shape
    return GetEffectiveShape()->GetExtent();
    }

//-----------------------------------------------------------------------------
// public
// InvalidateRaster : Invalidate the memory data related to this raster
//-----------------------------------------------------------------------------
void HRARaster::InvalidateRaster()
    {
    //By default, do nothing
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRARaster::BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    return _BuildCopyToContext(imageNode, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRARaster::CopyFrom(HRARaster& srcRaster)
    {
    return CopyFrom(srcRaster, HRACopyFromOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRARaster::CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& options)
    {
    if(options.GetEffectiveCopyRegion() != NULL)
        return _CopyFrom(srcRaster, options);

    // Need to build the effectiveCopyRegion.
    HRACopyFromOptions newOpts(options);

    // Take the destination effective shape
    HFCPtr<HVEShape> pCopyRegion(new HVEShape(*GetEffectiveShape()));

    // Intersect with source's effective shape
    pCopyRegion->Intersect(*srcRaster.GetEffectiveShape());

    // Intersect with specified "copy from" shape if necessary
    if (options.GetDestShape() != 0)
        pCopyRegion->Intersect(*(options.GetDestShape()));

    if(pCopyRegion->IsEmpty())
        return COPYFROM_STATUS_VoidRegion;
    
    newOpts.SetDestShape(NULL);
    newOpts.SetEffectiveCopyRegion(pCopyRegion.GetPtr());

    return _CopyFrom(srcRaster, newOpts);    
    }



