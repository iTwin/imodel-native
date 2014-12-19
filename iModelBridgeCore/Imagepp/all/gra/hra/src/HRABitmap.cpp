//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABitmap.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRABitmap
// ----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>


#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRABitmapIterator.h>
#include <Imagepp/all/h/HGF2DLocation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGFLightnessColorSpace.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRASamplingOptions.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRARasterEditor.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HPMPool.h>

#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGSBlitter.h>
#include <Imagepp/all/h/HGSWarper.h>
#include <Imagepp/all/h/HGSToolbox.h>
#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSEditor.h>
#include <Imagepp/all/h/HRPComplexFilter.h>
#include <Imagepp/all/h/HVE2DRectangle.h>

#include <Imagepp/all/h/HRABitmapEditor.h>
#include <Imagepp/all/h/HGFScanLines.h>

#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRASurface.h>

#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16x16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeGray.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HGFTolerance.h>
#include <Imagepp/all/h/HRPMessages.h>
#include <Imagepp/all/h/HRPQuantizedPalette.h>

HPM_REGISTER_CLASS(HRABitmap, HRABitmapBase)

HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRABitmap, HRABitmapBase, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRABitmap, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// public
// Default constructor. Create a HRABitmap (1,1) pixel (24 bits).
//-----------------------------------------------------------------------------
HRABitmap::HRABitmap (HRPPixelType* pi_pPixelType)
    : HRABitmapBase (pi_pPixelType),
      m_pPacket(new HCDPacket(new HCDCodecIdentity(), 0, 0, 0))
    {
    m_pPool = 0;
    m_BitsAlignment =   8;
    m_SLO           =   UPPER_LEFT_HORIZONTAL;
    }

//-----------------------------------------------------------------------------
// public
// HRABitmap::HRABitmap - Full featured constructor.
//-----------------------------------------------------------------------------
HRABitmap::HRABitmap(size_t                         pi_WidthPixels,
                     size_t                         pi_HeightPixels,
                     const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                     const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                     const HFCPtr<HRPPixelType>&    pi_rpPixel,
                     uint32_t                       pi_BitsAlignment,
                     HRABitmap::SLO                 pi_SLO,
                     const HFCPtr<HCDCodec>&        pi_rpCodec)
    : HRABitmapBase(pi_WidthPixels,
                    pi_HeightPixels,
                    pi_pModelCSp_CSl,
                    pi_rpLogicalCoordSys,
                    pi_rpPixel,
                    pi_BitsAlignment,
                    pi_SLO)
    {
    HPRECONDITION(pi_rpPixel != 0);

    m_pPool = 0;

    if (pi_rpCodec != 0)
        m_pPacket = new HCDPacket(pi_rpCodec, 0, 0, 0);
    else
        m_pPacket = new HCDPacket(new HCDCodecIdentity(), 0, 0, 0);

    // set the codec if there is one
    size_t BytesPerRow = (pi_WidthPixels * pi_rpPixel->CountPixelRawDataBits() + m_BitsAlignment - 1) / m_BitsAlignment;
    BytesPerRow = (BytesPerRow * m_BitsAlignment + 7) / 8;

    HCDCodecImage::SetCodecForImage(m_pPacket->GetCodec(),
                                    pi_WidthPixels,
                                    pi_HeightPixels,
                                    pi_rpPixel->CountPixelRawDataBits(),
                                    (BytesPerRow * 8) - (pi_WidthPixels * pi_rpPixel->CountPixelRawDataBits()));

    if ((m_pPacket->GetCodec() != 0) &&
        m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
        ((const HFCPtr<HCDCodecHMRRLE1>&)m_pPacket->GetCodec())->EnableLineIndexesTable(true);
    }

//-----------------------------------------------------------------------------
// public
// HRABitmap::~HRABitmap - Destructor
//-----------------------------------------------------------------------------
HRABitmap::~HRABitmap()
    {
    DeepDelete();

    // Memory Manager stuff
    m_pSurfaceDescriptor = 0;

    if (GetPool() != 0 && GetPool()->IsMemoryMgrEnabled() && m_pPacket->GetRefCount() <= 1)
        {
        // Don't use the memory manager if the packet is compressed.
        if (((m_pPacket->GetCodec() == 0) ||
             m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID)) &&
            m_pPacket->HasBufferOwnership())
            {
            m_pPacket->SetBufferOwnership(false);
            GetPool()->FreeMemory(m_pPacket->GetBufferAddress(), m_pPacket->GetBufferSize());
            }
        }

    m_pPacket = 0;
    }

//-----------------------------------------------------------------------------
// public
// Clone - Store and Log are not used.
//-----------------------------------------------------------------------------
HRARaster* HRABitmap::Clone (HPMObjectStore* pi_pStore,
                             HPMPool*        pi_pLog) const
    {
    return new HRABitmap(*this);
    }
//-----------------------------------------------------------------------------
// public
// Clone - Store and Log are not used.
//-----------------------------------------------------------------------------
HPMPersistentObject* HRABitmap::Clone () const
    {
    return new HRABitmap(*this);
    }

//-----------------------------------------------------------------------------
// protected
// HRABitmap::HRABitmap - Copy constructor
//-----------------------------------------------------------------------------
HRABitmap::HRABitmap(const HRABitmap& pi_rBitmap)
    : HRABitmapBase(pi_rBitmap)
    {
    // Perform initialization of the object
    DeepCopy(pi_rBitmap);
    }

//-----------------------------------------------------------------------------
// protected
// HRABitmap::operator= - Assignment operator
//-----------------------------------------------------------------------------
HRABitmap& HRABitmap::operator=(const HRABitmap& pi_rBitmap)
    {
    if (this != &pi_rBitmap)
        {
        HRABitmapBase::operator=(pi_rBitmap);

        // Delete currently allocated memory for the object
        DeepDelete();

        // Perform initialization of the object
        DeepCopy(pi_rBitmap);
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// CreateEditor
//
// No inline problem with the compilation.
//-----------------------------------------------------------------------------
HRARasterEditor* HRABitmap::CreateEditor (HFCAccessMode pi_Mode)
    {
    return (HRARasterEditor*)new HRABitmapEditor(this, pi_Mode);
    }

HRARasterEditor* HRABitmap::CreateEditor (const HVEShape& pi_rShape,
                                          HFCAccessMode   pi_Mode)
    {
    return (HRARasterEditor*)new HRABitmapEditor(this, pi_rShape, pi_Mode);
    }

HRARasterEditor* HRABitmap::CreateEditor (const HGFScanLines& pi_rScanLines,
                                          HFCAccessMode       pi_Mode)
    {
    return (HRARasterEditor*)new HRABitmapEditor(this, pi_rScanLines, pi_Mode);
    }

HRARasterEditor* HRABitmap::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return (HRARasterEditor*)new HRABitmapEditor(this, pi_Mode);
    }


//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRABitmap::Clear()
    {
    HRAClearOptions ClearOptions;
    Clear(ClearOptions);
    }


//-----------------------------------------------------------------------------
// Clear
//-----------------------------------------------------------------------------
void HRABitmap::Clear(const HRAClearOptions& pi_rOptions)
    {
    // patch to be sure it works with the editor even if the raster is not
    // in a HFCPtr somewhere
    IncrementRef();
        {
        HAutoPtr<HRABitmapEditor> pEditor;
        const HVEShape* pClearShape = 0;
        HFCPtr<HVEShape> pShape;

        if (pi_rOptions.HasRLEMask())
            {
            HASSERT(!L"Not supported yet");
            }
        else if (pi_rOptions.HasScanlines())
            {
            pEditor = (HRABitmapEditor*)CreateEditor(*pi_rOptions.GetScanlines(), HFC_WRITE_ONLY);
            }
        else
            {
            if (!pi_rOptions.HasShape())
                pClearShape = GetEffectiveShape();
            else if (pi_rOptions.HasApplyRasterClipping())
                {
                pShape = new HVEShape(*pi_rOptions.GetShape());
                pShape->Intersect(*GetEffectiveShape());
                pClearShape = pShape;
                }
            else
                pClearShape = pi_rOptions.GetShape();

            pEditor = (HRABitmapEditor*)CreateEditor(*pClearShape, HFC_WRITE_ONLY);
            }

        HASSERT(pEditor != 0);

        HFCPtr<HGSEditor> pSurfaceEditor = pEditor->GetSurfaceEditor();

        if ((m_pPacket->GetCodec() != 0) &&
            m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
            {
            HPRECONDITION(pSurfaceEditor != 0);
            HPRECONDITION(pSurfaceEditor->GetSurface() != 0);
            HPRECONDITION(pSurfaceEditor->GetSurface()->GetSurfaceDescriptor() != 0);
            HPRECONDITION(pSurfaceEditor->GetSurface()->GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

            HFCPtr<HRPPixelType> pPixelTypeRLE1 = ((const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pSurfaceEditor->GetSurface()->GetSurfaceDescriptor())->GetPixelType();

            // We assume the converter will return RLE run
            HPOSTCONDITION(pPixelTypeRLE1 != 0 &&
                           (pPixelTypeRLE1->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pPixelTypeRLE1->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID)));

            HFCPtr<HRPPixelConverter> pConverter(GetPixelType()->GetConverterTo(pPixelTypeRLE1));
            unsigned short OutData[4];
            pConverter->Convert((pi_rOptions.GetRawDataValue() != 0 ?
                                 pi_rOptions.GetRawDataValue() :
                                 GetPixelType()->GetDefaultRawData()),
                                &OutData);
            OutData[0] = OutData[0] ? 0 : 1; // TR 159986: must clear with state(0,1) not RLE run.
            pSurfaceEditor->Clear(OutData,
                                  (void*)GetCurrentTransaction());
            }
        else
            {
            pSurfaceEditor->Clear((pi_rOptions.GetRawDataValue() != 0 ?
                                   pi_rOptions.GetRawDataValue() :
                                   GetPixelType()->GetDefaultRawData()),
                                  (void*)GetCurrentTransaction());
            }

        Updated(pClearShape); // if we clear with scanlines, pClearShape == 0
        }

    DecrementRef();
    }



//HCHK DM   // Need trap in the destructor of Editor, but don't have editor, can't
//             use the StoredRasterEditor.
//             Don't forget to modify the method GetRawDataPtr if reactivate
#if 0
//-----------------------------------------------------------------------------
// public
// SetSingleColor -  The Raster has only one color, if pi_pRawData == 0, the
//                   Raster is not a SingleColor.
//-----------------------------------------------------------------------------
void HRABitmap::SetSingleColor(const void* pi_pRawData)
    {
    // Set or Reset SingleColor Mode.
    if (pi_pRawData == 0)
        {
        // Remove the SingleColor mode.
        if (m_pSingleColor.pData)
            {
            delete[] m_pSingleColor.pData;
            m_pSingleColor.pData    = 0;
            m_pSingleColor.BufSize  = 0;
            }
        }
    else
        {
        // Set the SingleColor mode

        // Free memory
        MakeEmpty();

        // Save the Color
        m_pSingleColor.BufSize = (GetPixelType()->CountPixelRawDataBits() <= 8 ? 1 :
                                  ((GetPixelType()->CountPixelRawDataBits()+7) / 8));
        m_pSingleColor.pData   = new Byte[m_pSingleColor.BufSize];
        }
    }

//-----------------------------------------------------------------------------
// public
// IsSingleColor -  true if the Raster is singleColor.
//-----------------------------------------------------------------------------
bool HRABitmap::IsSingleColor() const
    {
    return ((m_pSingleColor.pData != 0) && IsEmpty());
    }

//HCHK DM Don't use it, in testing only
//-----------------------------------------------------------------------------
// public
// GetSingleColor -  Return the single color or 0.
//-----------------------------------------------------------------------------
const void* HRABitmap::GetSingleColor() const
    {
    return m_pSingleColor.pData;
    }

#endif

//-----------------------------------------------------------------------------
// public
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRABitmap::GetRepresentativePalette(
    HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short CountUsed = HRABitmapBase::GetRepresentativePalette(pio_pRepPalParms);

    // if no operation has been done at the parent level or if the cache is not
    // updated
    if(CountUsed == 0)
        {

        HFCPtr<HRPPixelType> pSrcType;
        // test if we must use the source pixel type or a pixel type replacer
        if(pio_pRepPalParms->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
            pSrcType = pio_pRepPalParms->GetSamplingOptions().GetSrcPixelTypeReplacer();
        else
            pSrcType = GetPixelType();

        HFCPtr<HRPPixelConverter> pConverter;
        HRPPixelType* pPixelType;
        HAutoPtr<HRABitmapEditor> pEditor;

        // create a quantized palette object
        HAutoPtr<HRPQuantizedPalette> pQuantizedPalette(pio_pRepPalParms->CreateQuantizedPalette());
        // if not enough memory
        HASSERT(pQuantizedPalette != 0);

        // allocate an editor
        HVEShape CurShape(*GetEffectiveShape());
        CurShape.ChangeCoordSys(GetPhysicalCoordSys());
        if (GetPhysicalExtent() == CurShape.GetExtent())
            pEditor = (HRABitmapEditor*)CreateEditor(HFC_READ_ONLY);
        else
            pEditor = (HRABitmapEditor*)CreateEditor(CurShape, HFC_READ_ONLY);

        if(pEditor != 0)
            {
            const void* pRawData;

            // go to the first pixel
            if((pRawData = GetFirstResamplingPixel(pEditor,
                                                   (Byte)(100 / pio_pRepPalParms->GetSamplingOptions().GetPixelsToScan()))) != 0)
                {
                pPixelType = pio_pRepPalParms->GetPixelType();

                // get a converter object
                pConverter = pSrcType->GetConverterTo(pPixelType);
                HASSERT(pConverter != 0);

                // a variable that contain each composite value converted
                // 32 is an arbitrary number larger enough to contain any kind of
                // composite value
                Byte Value[32];

                do
                    {
                    pConverter->ConvertToValue(pRawData,(void*)Value);

                    // we add it in the quantized palette
                    pQuantizedPalette->AddCompositeValue((void*)Value);
                    }
                while((pRawData = GetNextResamplingPixel()) != 0);

                // get the number of entries in the quantized palette
                CountUsed = pQuantizedPalette->GetPalette(
                                &(pPixelType->LockPalette()), pio_pRepPalParms->GetHistogram());
                pPixelType->UnlockPalette();

                if(pio_pRepPalParms->UseCache())
                    {
                    // update the palette cache if required
                    UpdateRepPalCache(CountUsed, pPixelType->GetPalette());
                    }

                }

            TerminateResamplingPixel();
            }
        }

    return CountUsed;
    }

//-----------------------------------------------------------------------------
// public
// ComputeHistogram
//-----------------------------------------------------------------------------
void HRABitmap::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                 bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    if (pi_ForceRecompute || (GetHistogram() == 0) || (GetHistogram() != 0 && !GetHistogram()->CanBeUsedInPlaceOf(*pio_pOptions)))
        {
        Byte PixelsInterval = (Byte)(100 / pio_pOptions->GetSamplingOptions().GetPixelsToScan());

        HFCPtr<HRPPixelType> pSrcPixelType;
        // test if we must use the source pixel type or a pixel type replacer
        if(pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
            GetSurfaceDescriptor(&pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer(), &pSrcPixelType);
        else
            GetSurfaceDescriptor(0, &pSrcPixelType);

        bool ComputeRLEHistogram = false;

        // optimization case
        // if we need to compute a full histogram on an 1bit RLE image, use ComputeHistogramRLE
        if (PixelsInterval == 1 &&
            (pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
             pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID)))
            {
            if (pio_pOptions->GetPixelType() == 0)
                ComputeRLEHistogram = true;
            else
                {
                ComputeRLEHistogram = (pio_pOptions->GetPixelType()->CountIndexBits() > 0 &&
                                       pSrcPixelType->GetPalette() == pio_pOptions->GetPixelType()->GetPalette());
                }
            }

        if (ComputeRLEHistogram)
            ComputeHistogramRLE(pio_pOptions);
        else if (pio_pOptions->GetSamplingColorSpace() == HRPHistogram::LIGHTNESS)
            ComputeLightnessHistogram(pio_pOptions);
        else if (pio_pOptions->GetSamplingColorSpace() == HRPHistogram::GRAYSCALE)
            ComputeGrayscaleHistogram(pio_pOptions);
        else if (pio_pOptions->GetSamplingColorSpace() == HRPHistogram::RGB)
            ComputeRGBHistogram(pio_pOptions);
        else if (pio_pOptions->GetSamplingColorSpace() == HRPHistogram::RGBA)
            ComputeRGBAHistogram(pio_pOptions);
        else
            {
            HASSERT(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::NATIVE);

            HFCPtr<HRPPixelConverter> pConverter;
            HAutoPtr<HRABitmapEditor> pEditor;

            // allocate an editor
            if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
                {
                HVEShape PhysicalRegion(*pio_pOptions->GetSamplingOptions().GetRegionToScan());
                PhysicalRegion.Intersect(*GetEffectiveShape());
                PhysicalRegion.ChangeCoordSys(GetPhysicalCoordSys());

                if (!PhysicalRegion.IsEmpty())
                    pEditor = (HRABitmapEditor*)CreateEditor(PhysicalRegion, HFC_READ_ONLY);
                }
            else
                pEditor = (HRABitmapEditor*)CreateEditor(HFC_READ_ONLY);

            if (pEditor != 0)
                {
                size_t PixelCount;

                // go to the first pixel
                const void* pRawData = GetFirstResamplingLine(pEditor, PixelsInterval, &PixelCount);

                if(pRawData != 0)
                    {
                    // a variable that contain each composite value converted
                    // 32 is an arbitrary number larger enough to contain any kind of
                    // composite value
                    Byte Value[32];
                    uint32_t PaddingBits = 0;

                    if (pio_pOptions->GetPixelType() != 0 && pio_pOptions->GetPixelType()->CountIndexBits() != 0)
                        {
                        HRPPixelType const* pPixelType = pio_pOptions->GetPixelType();

                        if (pPixelType->CountIndexBits() < 8)
                            PaddingBits = 8 - pPixelType->CountIndexBits();

                        // test if the pixel type of the options and the one of the raster are indexed
                        // and having the same palette
                        bool SrcCompatible;
                        if(pPixelType->CountIndexBits() <= 8 &&
                           pPixelType->GetClassID() == pSrcPixelType->GetClassID() &&
                           pPixelType->GetPalette() == pSrcPixelType->GetPalette())
                            {
                            SrcCompatible = true;
                            }
                        else
                            {
                            SrcCompatible = false;

                            // get a converter object
                            pConverter = pSrcPixelType->GetConverterTo(pPixelType);
                            HASSERT(pConverter != 0);
                            }

                        do
                            {
                            for(size_t PixelInd = 0; PixelInd < PixelCount; PixelInd += PixelsInterval)
                                {
                                if(!SrcCompatible)
                                    {
                                    // try to find the same value
                                    pConverter->Convert(pRawData,(void*)Value);

                                    // we add the entry in the histogram
                                    pio_pOptions->GetHistogram()->IncrementEntryCount((*(Byte*)&Value) >> PaddingBits);
                                    }
                                else
                                    {
                                    // if the source pixel type and options palette are equal
                                    // simply increment the count of the appropriate entry
                                    pio_pOptions->GetHistogram()->IncrementEntryCount(*((Byte*)pRawData) >> PaddingBits);
                                    }
                                pRawData = ((Byte*)pRawData) + PixelsInterval;
                                }
                            }
                        while((pRawData = GetNextResamplingLine(&PixelCount)) != 0);

                        if (pio_pOptions->GetSamplingOptions().GetRegionToScan() == 0)
                            SetHistogram(pio_pOptions); // set histogram into HRARaster
                        }
                    else
                        {
                        if (pSrcPixelType->CountPixelRawDataBits() <= 8)
                            {
                            PaddingBits = 8 - pSrcPixelType->CountPixelRawDataBits();

                            pRawData = GetFirstResamplingPixel(pEditor, PixelsInterval);
                            do
                                {
                                pio_pOptions->GetHistogram()->IncrementEntryCount((*(Byte*)pRawData) >> PaddingBits);

                                }
                            while((pRawData = GetNextResamplingPixel()) != 0);

                            if (pio_pOptions->GetSamplingOptions().GetRegionToScan() == 0)
                                SetHistogram(pio_pOptions); // set histogram into HRARaster
                            }
                        else
                            {
                            if (pSrcPixelType->GetChannelOrg().GetChannelPtr(0)->GetSize() == 8)
                                {
                                HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();

                                HUINTX XPhysical;
                                HUINTX YPhysical;
                                size_t NumPixels;
                                Byte* pCurrentPixel;
                                HFCPtr<HGSEditor> pSurfaceEditor(pEditor->GetSurfaceEditor());
                                if (pSurfaceEditor->GetFirstRun(&XPhysical, &YPhysical, &NumPixels))
                                    {
                                    do
                                        {
                                        // Obtain pointer to first pixel to process
                                        pCurrentPixel = (Byte*) pSurfaceEditor->GetPixel(XPhysical, YPhysical);

                                        if (pio_pOptions->GetHistogram()->GetChannelCount() == 4)
                                            {
                                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID) ||
                                                    pSrcPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8X8::CLASS_ID));

                                            for( ; NumPixels ; --NumPixels)
                                                {
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 0);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 1);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 2);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 3);
                                                }
                                            }
                                        else if (pio_pOptions->GetHistogram()->GetChannelCount() == 3)
                                            {
                                            if (pSrcPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
                                                {
                                                for( ; NumPixels ; --NumPixels)
                                                    {
                                                    pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 0);
                                                    pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 1);
                                                    pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 2);
                                                    }
                                                }
                                            else if (pSrcPixelType->IsCompatibleWith(HRPPixelTypeV24B8G8R8::CLASS_ID))
                                                {
                                                for( ; NumPixels ; --NumPixels)
                                                    {
                                                    pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 2);
                                                    pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 1);
                                                    pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 0);
                                                    }
                                                }
                                            else
                                                HASSERT(false);
                                            }
                                        else
                                            {
                                            for( ; NumPixels ; --NumPixels)
                                                {
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 0);
                                                }
                                            }
                                        }
                                    while (pSurfaceEditor->GetNextRun(&XPhysical, &YPhysical, &NumPixels));
                                    }
                                }
                            else if (pSrcPixelType->GetChannelOrg().GetChannelPtr(0)->GetSize() == 16)
                                {
                                HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();

                                HUINTX   XPhysical;
                                HUINTX   YPhysical;
                                size_t   NumPixels;
                                uint16_t* pCurrentPixel;
                                HFCPtr<HGSEditor> pSurfaceEditor(pEditor->GetSurfaceEditor());
                                if (pSurfaceEditor->GetFirstRun(&XPhysical, &YPhysical, &NumPixels))
                                    {
                                    do
                                        {
                                        // Obtain pointer to first pixel to process
                                        pCurrentPixel = (uint16_t*) pSurfaceEditor->GetPixel(XPhysical, YPhysical);

                                        if (pio_pOptions->GetHistogram()->GetChannelCount() == 4)
                                            {
                                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV64R16G16B16A16::CLASS_ID) ||
                                                    pSrcPixelType->IsCompatibleWith(HRPPixelTypeV64R16G16B16X16::CLASS_ID));

                                            for( ; NumPixels ; --NumPixels)
                                                {
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 0);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 1);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 2);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 3);
                                                }
                                            }
                                        else if (pio_pOptions->GetHistogram()->GetChannelCount() == 3)
                                            {
                                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV48R16G16B16::CLASS_ID));

                                            for( ; NumPixels ; --NumPixels)
                                                {
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 0);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 1);
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 2);
                                                }
                                            }
                                        else
                                            {
                                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID));

                                            for( ; NumPixels ; --NumPixels)
                                                {
                                                pHistogram->IncrementEntryCount(*pCurrentPixel++, 1, 0);
                                                }
                                            }
                                        }
                                    while (pSurfaceEditor->GetNextRun(&XPhysical, &YPhysical, &NumPixels));
                                    }
                                }
                            }
                        }
                    }
                TerminateResamplingPixel();
                }
            }
        }
    else
        {
        HPRECONDITION(GetHistogram()->GetHistogram()->GetEntryFrequenciesSize() <=
                      pio_pOptions->GetHistogram()->GetEntryFrequenciesSize());

        HPRECONDITION(GetHistogram()->GetHistogram()->GetChannelCount() <=
                      pio_pOptions->GetHistogram()->GetChannelCount());

        HFCPtr<HRPHistogram> pHisto(new HRPHistogram(*GetHistogram()->GetHistogram()));
        HFCPtr<HRPHistogram> pOutHisto(pio_pOptions->GetHistogram());

        for (uint32_t ChannelIndex = 0; ChannelIndex < pHisto->GetChannelCount(); ChannelIndex++)
            {
            for (uint32_t FreqIndex = 0; FreqIndex < pHisto->GetEntryFrequenciesSize(ChannelIndex); FreqIndex++)
                pOutHisto->IncrementEntryCount(FreqIndex, pHisto->GetEntryCount(FreqIndex, ChannelIndex), ChannelIndex);
            }
        }
    }


//------------------------------------------------------------------- Privates

//-----------------------------------------------------------------------------
// private
// DeepCopy - Deep copies the source object
//-----------------------------------------------------------------------------
void HRABitmap::DeepCopy(const HRABitmap& pi_rBitmap)
    {
    m_pPool         = pi_rBitmap.m_pPool;
    m_pPacket       = (HCDPacket*)pi_rBitmap.m_pPacket->Clone();
    m_BitsAlignment = pi_rBitmap.m_BitsAlignment;
    m_SLO           = pi_rBitmap.m_SLO;
    }

//-----------------------------------------------------------------------------
// public
// InitSize
//-----------------------------------------------------------------------------
void HRABitmap::InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels)
    {
    m_pPacket->SetBuffer(0, 0);
    m_pPacket->SetDataSize(0);
    m_pSurfaceDescriptor = 0;

    // set the codec size
    HFCPtr<HRPPixelType> pPixelType(GetPixelType());

    // set the codec if there is one
    HASSERT(pi_WidthPixels <= ULONG_MAX);
    HASSERT(pi_HeightPixels <= ULONG_MAX);

    uint32_t BytesPerRow = ((uint32_t)pi_WidthPixels * pPixelType->CountPixelRawDataBits() + m_BitsAlignment - 1) / m_BitsAlignment;
    BytesPerRow = (BytesPerRow * m_BitsAlignment + 7) / 8;

    HCDCodecImage::SetCodecForImage(m_pPacket->GetCodec(),
                                    (uint32_t)pi_WidthPixels,
                                    (uint32_t)pi_HeightPixels,
                                    GetPixelType()->CountPixelRawDataBits(),
                                    (BytesPerRow * 8) - ((uint32_t)pi_WidthPixels * pPixelType->CountPixelRawDataBits()));

    HRABitmapBase::InitSize(pi_WidthPixels, pi_HeightPixels);
    }

//-----------------------------------------------------------------------------
// public
// GetAdditionalSize
//-----------------------------------------------------------------------------
size_t HRABitmap::GetAdditionalSize() const
    {
    size_t DataSize = m_pPacket->GetBufferSize();

    // if the buffer has not yet been allocated, compute the normal size
    if (DataSize == 0)
        {
        // compute uncompressed size
        uint64_t Width;
        uint64_t Height;
        GetSize(&Width, &Height);

        HASSERT(Width <= ULONG_MAX);
        HASSERT(Height <= ULONG_MAX);

        HFCPtr<HRPPixelType> pPixelType(GetPixelType());
        size_t BytesPerRow = ((uint32_t)Width * pPixelType->CountPixelRawDataBits() + m_BitsAlignment - 1) / m_BitsAlignment;
        BytesPerRow = (BytesPerRow * m_BitsAlignment + 7) / 8;
        size_t RawDataSize = BytesPerRow * (uint32_t)Height;

        // test if the tile is compressed
        if (m_pPacket->GetCodec() != 0)
            {
            // if yes, compute the ESTIMATED compressed size (e.g., NOT THE MAX)
            DataSize = (size_t)(m_pPacket->GetCodec()->GetEstimatedCompressionRatio() * RawDataSize);
            }
        else
            {
            // if not, return the normal uncompressed size
            DataSize = RawDataSize;
            }
        }

    return DataSize;
    }

// ----------------------------------------------------------------------------
// public
// GetSurfaceDescriptor
// ----------------------------------------------------------------------------
HFCPtr<HGSSurfaceDescriptor> HRABitmap::GetSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                                                             HFCPtr<HRPPixelType>* po_ppOutputPixelType) const
    {
    HFCPtr<HGSSurfaceDescriptor> pDescriptor;

    if((pi_ppReplacingPixelType != 0 && *pi_ppReplacingPixelType != 0) ||
       po_ppOutputPixelType != 0)
        {
        pDescriptor = CreateSurfaceDescriptor(pi_ppReplacingPixelType, po_ppOutputPixelType);

        //TR 300554 : Ensure that the data dimension of the newly created surface is correct.
        if (m_pSurfaceDescriptor != 0)
            {
            pDescriptor->SetDataDimensions(m_pSurfaceDescriptor->GetDataWidth(), m_pSurfaceDescriptor->GetDataHeight());
            }
        }
    else if(m_pSurfaceDescriptor == 0)
        {
        pDescriptor = CreateSurfaceDescriptor(pi_ppReplacingPixelType, po_ppOutputPixelType);
        ((HRABitmap*)this)->m_pSurfaceDescriptor = pDescriptor;
        }
    else
        {
        pDescriptor = m_pSurfaceDescriptor;
        }

    return pDescriptor;
    }

//-----------------------------------------------------------------------------
// public
// Updated. Tell the bitmap that its content has been tampered with.
//-----------------------------------------------------------------------------
void HRABitmap::Updated(const HVEShape* pi_pModifiedContent)
    {
    // indicate that we have modified the bitmap
    InvalidateRepPalCache();

    SetModificationState();

    // test if the user has provided a shape
    if(pi_pModifiedContent != 0)
        {
        // Raster's content has changed.
        // Notify the registered sinks.
        Propagate(HRAContentChangedMsg(*pi_pModifiedContent));
        }
    else
        {
        Propagate(HRAContentChangedMsg(*GetEffectiveShape()));
        }
    }

// ----------------------------------------------------------------------------
// public
// GetPacket
// ----------------------------------------------------------------------------
const HFCPtr<HCDPacket>& HRABitmap::GetPacket() const
    {
    // test if there is already a buffer allocated
    if (m_pPacket->GetBufferSize() == 0)
        {
        if ((m_pPacket->GetCodec() != 0) &&
            m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
            {
            uint64_t Width;
            uint64_t Height;
            GetSize(&Width, &Height);

            HASSERT(Width <= ULONG_MAX);
            HASSERT(Height <= ULONG_MAX);

            uint32_t RunsPerLine = ((uint32_t)Width / 32767) * 2 + 1;
            size_t BufferSize = RunsPerLine * (uint32_t)Height * 2;

            m_pPacket->SetBuffer(new Byte[BufferSize], BufferSize);
            m_pPacket->SetBufferOwnership(true);
            unsigned short* pData = (unsigned short*)m_pPacket->GetBufferAddress();
            // create an offset table
            HArrayAutoPtr<uint32_t> pTmpLineOffsets(new uint32_t[(uint32_t)Height]);
            uint32_t* pLineOffset = pTmpLineOffsets;
            for(uint32_t LineIndex = 0; LineIndex < Height; LineIndex++)
                {
                uint32_t PixelCount = (uint32_t)Width;

                while(PixelCount != 0)
                    {
                    if(PixelCount > 32767)
                        {
                        *pData = 32767;
                        PixelCount -= 32767;
                        pData++;

                        *pData = 0;
                        pData++;
                        }
                    else
                        {
                        *pData = (unsigned short)PixelCount;
                        pData++;

                        PixelCount = 0;
                        }
                    }

                *pLineOffset = LineIndex * RunsPerLine;
                pLineOffset++;
                }
            m_pPacket->SetDataSize((Byte*)pData - (Byte*)m_pPacket->GetBufferAddress());
            ((HFCPtr<HCDCodecHMRRLE1>&)m_pPacket->GetCodec())->SetLineIndexesTable(pTmpLineOffsets);

            // In case of RLE, let the Clear method do the allocation
            //  ((HRABitmap*)this)->Clear();
            }
        else
            {
            // In other cases, compute the bytes per row for the allocation
            // get the size of the surfaces
            uint64_t Width;
            uint64_t Height;
            GetSize(&Width, &Height);

            HASSERT(Width <= ULONG_MAX);
            HASSERT(Height <= ULONG_MAX);

            HFCPtr<HRPPixelType> pPixelType(GetPixelType());

            size_t BytesPerRow = ((uint32_t)Width * pPixelType->CountPixelRawDataBits() + m_BitsAlignment - 1) / m_BitsAlignment;
            BytesPerRow = (BytesPerRow * m_BitsAlignment + 7) / 8;

            size_t RawDataSize = BytesPerRow * (uint32_t)Height;

            // Memory Manager stuff.
            if (GetPool() != 0 && GetPool()->IsMemoryMgrEnabled())
                {
                unsigned char* MemBlock = 0;

                // Don't use the memory manager if the packet is compressed.
                if ((m_pPacket->GetCodec() == 0) ||
                    m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
                    MemBlock = GetPool()->AllocMemory(RawDataSize);

                if (MemBlock == 0)
                    m_pPacket->SetBuffer(new Byte[RawDataSize], RawDataSize);
                else
                    m_pPacket->SetBuffer(MemBlock, RawDataSize);
                }
            else
                m_pPacket->SetBuffer(new Byte[RawDataSize], RawDataSize);

            m_pPacket->SetDataSize(RawDataSize);
            m_pPacket->SetBufferOwnership(true);
            }
        }

    return m_pPacket;
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRABitmap::PreDraw(HRADrawOptions* pio_pOptions)
    {
    //Not Implemented
    HASSERT(0);
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRABitmap::Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const
    {
    HPRECONDITION(pio_pSurface != 0);

    // If the Raster is empty, skip it
    if (!IsEmpty())
        {
        HRADrawOptions Options(pi_pOptions);

        // set the effective coordsys
        if (Options.GetReplacingCoordSys() == 0)
            // case when there is no replacing coordsys
            Options.SetReplacingCoordSys(GetCoordSys());

        // get the destination descriptor
        HFCPtr<HGSSurfaceDescriptor> pDstDescriptor(pio_pSurface->GetSurfaceDescriptor());

        //
        // Compute the effective clip shape, and give it
        // to the destination surface.
        //
        HFCPtr<HVEShape> pClipShape;
        if (Options.GetShape() != 0)
            pClipShape = new HVEShape(*Options.GetShape());
        else
            pClipShape = new HVEShape(*GetEffectiveShape());

        pClipShape->ChangeCoordSys(GetCoordSys());
        pClipShape->SetCoordSys(Options.GetReplacingCoordSys());
        pClipShape->ChangeCoordSys(pio_pSurface->GetSurfaceCoordSys());

        // store the old clip region
        HFCPtr<HGSRegion> pOldClipRegion(static_cast<HGSRegion*>(pio_pSurface->GetOption(HGSRegion::CLASS_ID).GetPtr()));

        if (pOldClipRegion != 0)
            {
            // construct a shape from the clip region
            HFCPtr<HVEShape> pSurfaceShape(pOldClipRegion->GetShape());

            // if yes, intersect it with the destination
            pClipShape->Intersect(*pSurfaceShape);
            }
        else
            {
            // Create a rectangular clip region to stay
            // inside the destination surface.
            HVEShape DestSurfaceShape(0.0, 0.0, pDstDescriptor->GetWidth(), pDstDescriptor->GetHeight(), pio_pSurface->GetSurfaceCoordSys());
            HFCPtr<HGFTolerance> pTol (new HGFTolerance(GetSurfaceDescriptor()->GetWidth() / 2.0 - DEFAULT_PIXEL_TOLERANCE,
                                                        GetSurfaceDescriptor()->GetHeight() / 2.0 - DEFAULT_PIXEL_TOLERANCE,
                                                        GetSurfaceDescriptor()->GetWidth() / 2.0 + DEFAULT_PIXEL_TOLERANCE,
                                                        GetSurfaceDescriptor()->GetHeight() / 2.0 + DEFAULT_PIXEL_TOLERANCE,
                                                        DestSurfaceShape.GetCoordSys()));

            DestSurfaceShape.SetStrokeTolerance(pTol);

            pClipShape->Intersect(DestSurfaceShape);
            }

        if ((m_pPacket->GetCodec() != 0) &&
            m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecImage::CLASS_ID))
            {
            // SOMETIMES. THE SOURCE COMPRESSED DATA IS INCOMPLETE!, WE MUST CLIP
            HFCPtr<HCDCodecImage> pCodecImage;
            pCodecImage = (HFCPtr<HCDCodecImage>&)m_pPacket->GetCodec();

            uint64_t WidthPixels;
            uint64_t HeightPixels;
            GetSize(&WidthPixels, &HeightPixels);

            if (pCodecImage->GetHeight() < HeightPixels)
                {
                HFCPtr<HVEShape> pShape(new HVEShape(0, 0, static_cast<double>(pCodecImage->GetWidth()), static_cast<double>(pCodecImage->GetHeight()),  GetPhysicalCoordSys()));
                pShape->ChangeCoordSys(GetCoordSys());
                pShape->SetCoordSys(Options.GetReplacingCoordSys());
                pShape->ChangeCoordSys(pio_pSurface->GetSurfaceCoordSys());

                pClipShape->Intersect(*pShape);
                }
            }

        if (!pClipShape->IsEmpty())
            {
            // test if this is a simple rectangle covering the entire surface
            bool Clip = true;
            if(pClipShape->GetShapePtr()->GetClassID() == HVE2DRectangle::CLASS_ID)
                {
                HGF2DExtent Extent(pClipShape->GetShapePtr()->GetExtent());

                // if yes, do not clip
                if(Extent.GetWidth() >= pDstDescriptor->GetWidth() && Extent.GetHeight() >= pDstDescriptor->GetHeight())
                    Clip = false;
                }
            if(Clip)
                {
                HFCPtr<HGSRegion> pTheRegion(new HGSRegion(pClipShape, pClipShape->GetCoordSys()));
                pio_pSurface->SetOption((const HFCPtr<HGSSurfaceOption>&)pTheRegion);
                }

            //
            // Verify / modify certain options
            //

            // set the effective pixel type
            bool ReplacingPixelType;
            HFCPtr<HRPPixelType> pEffectivePixelType;
            if (Options.GetReplacingPixelType() != 0)
                {
                pEffectivePixelType = Options.GetReplacingPixelType();
                ReplacingPixelType = true;
                }
            else
                {
                pEffectivePixelType = GetPixelType();
                ReplacingPixelType = false;
                }

            if (ReplacingPixelType)
                Options.SetReplacingPixelType(pEffectivePixelType);

            // set the effective alpha blend
            if (Options.ApplyAlphaBlend() &&
                pEffectivePixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE)
                {
                Options.SetAlphaBlend(false);
                }

            //
            // Do the pixel copy, using the appropriate tool
            //

            // Model Physical --> Physical
            // Get the model between model Physical Destination CoordSys to
            //                             Physical Source CoordSys
            // get the transfo model between the low level and high level
            HFCPtr<HGF2DTransfoModel> pTransfoModel = pio_pSurface->GetSurfaceCoordSys()->GetTransfoModelTo (Options.GetReplacingCoordSys());
            pTransfoModel = pTransfoModel->ComposeInverseWithInverseOf(*GetTransfoModel());

            bool CanStretch = pTransfoModel->IsStretchable(HGLOBAL_EPSILON);
            if (CanStretch)
                {
                // Extract stretching parameters
                double StretchX;
                double StretchY;
                HGF2DDisplacement Translation;
                pTransfoModel->GetStretchParams(&StretchX,
                                                &StretchY,
                                                &Translation);

                // create a real stretch model
                pTransfoModel = new HGF2DStretch(Translation,
                                                 StretchX,
                                                 StretchY);

                }

            if (CanStretch)
                StretchWithHGS(pio_pSurface, &Options, pTransfoModel);
            else
                WarpWithHGS(pio_pSurface, &Options, pTransfoModel);

            // set the old clip region back
            if (Clip)
                {
                if (pOldClipRegion != 0)
                    pio_pSurface->SetOption((const HFCPtr<HGSSurfaceOption>&)pOldClipRegion);
                else
                    pio_pSurface->RemoveOption(HGSRegion::CLASS_ID);
                }
            }
        }
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void HRABitmap::StretchWithHGS(HGFMappedSurface* pio_pSurface,
                               const HRADrawOptions* pi_pOptions,
                               const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) const
    {
    HPRECONDITION(pi_rpTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID));

    // create the toolboxes with the editors
    HFCPtr<HGSBlitter> pDstBlitter(new HGSBlitter());
    pDstBlitter->AddAttribute(HGSResamplingAttribute(pi_pOptions->GetResamplingMode()));
    if(pi_pOptions->ApplyAlphaBlend())
        pDstBlitter->AddAttribute(HGSColorConversionAttribute(HGSColorConversion::COMPOSE));
    if (pi_pOptions->ApplyGridShape())
        pDstBlitter->AddAttribute(HGSScanlinesAttribute(HGSScanlineMethod(HGSScanlineMethod::GRID)));
    if (pi_pOptions->GetOverviewMode())
        pDstBlitter->AddAttribute(HGSPurposeAttribute(HGSPurpose(HGSPurpose::OVERVIEWS)));

    HGSToolbox DstToolbox((HFCPtr<HGSGraphicTool>&)pDstBlitter);

    // create the descriptor for the source
    HFCPtr<HRPPixelType> PixelReplace(pi_pOptions->GetReplacingPixelType());
    HFCPtr<HGSSurfaceDescriptor> pSrcDescriptor(GetSurfaceDescriptor(&PixelReplace));

    // test if the destination surface is compatible, otherwise HASSERT
    HAutoPtr<HGSSurfaceCapabilities> pSrcRequiredSurfaceCapabilities(pSrcDescriptor->GetRequiredSurfaceCapabilities());
    pio_pSurface->RequestCompatibility(&DstToolbox, pSrcRequiredSurfaceCapabilities);

    // create the destination surfaces
    HFCPtr<HGSSurface> pSrcSurface = pio_pSurface->CreateCompatibleSurface(pSrcDescriptor);

    // set the surfaces on the toolboxes
    DstToolbox.SetFor(pio_pSurface);

    pDstBlitter->BlitFrom(pSrcSurface, *pi_rpTransfoModel, pi_pOptions->GetTransaction());
    }

//-----------------------------------------------------------------------------
// WarpWithHGS
//-----------------------------------------------------------------------------
void HRABitmap::WarpWithHGS(HGFMappedSurface*                   pio_pSurface,
                            const HRADrawOptions*               pi_pOptions,
                            const HFCPtr<HGF2DTransfoModel>&    pi_rpTransfoModel) const
    {
    // create the toolboxes with the editors
    HFCPtr<HGSWarper> pDstWarper(new HGSWarper());
    pDstWarper->AddAttribute(HGSResamplingAttribute(pi_pOptions->GetResamplingMode()));
    if(pi_pOptions->ApplyAlphaBlend())
        pDstWarper->AddAttribute(HGSColorConversionAttribute(HGSColorConversion::COMPOSE));
    if (pi_pOptions->ApplyGridShape())
        pDstWarper->AddAttribute(HGSScanlinesAttribute(HGSScanlineMethod(HGSScanlineMethod::GRID)));
    HGSToolbox DstToolbox((HFCPtr<HGSGraphicTool>&)pDstWarper);

    // create the descriptor for the source
    HFCPtr<HRPPixelType> PixelReplace(pi_pOptions->GetReplacingPixelType());
    HFCPtr<HGSSurfaceDescriptor> pSrcDescriptor(GetSurfaceDescriptor(&PixelReplace));

    // test if the destination surface is compatible, otherwise HASSERT
    HAutoPtr<HGSSurfaceCapabilities> pSrcRequiredSurfaceCapabilities(pSrcDescriptor->GetRequiredSurfaceCapabilities());
    pio_pSurface->RequestCompatibility(&DstToolbox, pSrcRequiredSurfaceCapabilities);

    // create the destination surfaces
    HFCPtr<HGSSurface> pSrcSurface = pio_pSurface->CreateCompatibleSurface(pSrcDescriptor);

    // set the surfaces on the toolboxes
    DstToolbox.SetFor(pio_pSurface);

    pDstWarper->WarpFrom(pSrcSurface,
                         *pi_rpTransfoModel,
                         pi_pOptions->GetTransaction());
    }


//-----------------------------------------------------------------------------
// private
// CreateSurfaceDescriptor
//-----------------------------------------------------------------------------
HFCPtr<HGSSurfaceDescriptor> HRABitmap::CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                                                                HFCPtr<HRPPixelType>*       po_ppOutputPixelType) const
    {
    // get the size of the surfaces
    uint64_t Width;
    uint64_t Height;
    GetSize(&Width, &Height);

    HASSERT(Width <= ULONG_MAX);
    HASSERT(Height <= ULONG_MAX);

    HFCPtr<HRPPixelType> pSrcPixelType;

    if (pi_ppReplacingPixelType == 0 || *pi_ppReplacingPixelType == 0)
        pSrcPixelType = GetPixelType();
    else
        pSrcPixelType = *pi_ppReplacingPixelType;

    uint32_t BytesPerRow=0;
    if ((m_pPacket->GetCodec() == 0) ||
        m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
        {
        BytesPerRow = ((uint32_t)Width * GetPixelType()->CountPixelRawDataBits() + m_BitsAlignment - 1) / m_BitsAlignment;
        BytesPerRow = (BytesPerRow * m_BitsAlignment + 7) / 8;

        if (po_ppOutputPixelType != 0)
            *po_ppOutputPixelType = pSrcPixelType;
        }
    else if (m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
        {
        // we have an RLE1
        HASSERT(pSrcPixelType->CountPixelRawDataBits() == 1);

        Width =  ((HFCPtr<HCDCodecHMRRLE1>&)(GetPacket()->GetCodec()))->GetWidth();
        Height = ((HFCPtr<HCDCodecHMRRLE1>&)(GetPacket()->GetCodec()))->GetHeight();

        // Transforme I1RGB to I1RGBRLE
        //
        if (pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
            pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
            {
            // Nothing todo
            }
        else if (pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8::CLASS_ID) ||
                 pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8::CLASS_ID))
            {
            HASSERT(pSrcPixelType->Get1BitInterface() != NULL);

            HFCPtr<HRPPixelType> pOldSrcPixelType = pSrcPixelType;

            if(pOldSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8::CLASS_ID))
                pSrcPixelType = new HRPPixelTypeI1R8G8B8RLE(pOldSrcPixelType->GetPalette());
            else
                pSrcPixelType = new HRPPixelTypeI1R8G8B8A8RLE(pOldSrcPixelType->GetPalette());

            if(pOldSrcPixelType->Get1BitInterface()->IsForegroundStateDefined())
                pSrcPixelType->Get1BitInterface()->SetForegroundState(pOldSrcPixelType->Get1BitInterface()->GetForegroundState());
            }
        else
            HASSERT(false);

        if (po_ppOutputPixelType != 0)
            *po_ppOutputPixelType = pSrcPixelType;

        BytesPerRow = 0;
        }
    else
        {
        HASSERT(0);
        }

    // be sure that the buffer has been allocated
    GetPacket();

    // create the descriptor
    HFCPtr<HGSSurfaceDescriptor> pDescriptor(
        new HGSMemorySurfaceDescriptor((uint32_t)Width,
                                       (uint32_t)Height,
                                       pSrcPixelType,
                                       m_pPacket,
                                       (HGFSLO)m_SLO,
                                       BytesPerRow));

    pDescriptor->SetOffsets(m_XPosInRaster,
                            m_YPosInRaster);

    return pDescriptor;
    }

//-----------------------------------------------------------------------------
// Notification for palette change
//-----------------------------------------------------------------------------
bool HRABitmap::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    // we only propagate the "palette changed" messages coming from
    // the pixeltype of the tiled raster, not from the tiles.

    // compare the address of the sender with the one of the pixel type
    // if they are not equal, do not propagate the current message
    if(pi_rMessage.GetSender() != GetPixelType().GetPtr())
        return false;
    
    SetModificationState();

    // reset the surface descriptor
    m_pSurfaceDescriptor = 0;

    // otherwise, propagate the message
    return true;
    }


//-----------------------------------------------------------------------------
// Get the first line into the surface
//-----------------------------------------------------------------------------
const void* HRABitmap::GetFirstResamplingLine(HRABitmapEditor*     pi_pEditor,
                                              Byte               pi_PixelsInterval,
                                              size_t*              po_pPixelCount)
    {
    HPRECONDITION(pi_pEditor != 0);
    HPRECONDITION(pi_PixelsInterval > 0);

    m_PixelsInterval = pi_PixelsInterval;
    m_pResamplingSurfaceEditor = pi_pEditor->GetSurfaceEditor();

    return m_pResamplingSurfaceEditor->GetFirstRun(&m_CurrentPixelX,
                                                   &m_CurrentPixelY,
                                                   po_pPixelCount);
    }


//-----------------------------------------------------------------------------
// Get the next line into the surface
//-----------------------------------------------------------------------------
const void* HRABitmap::GetNextResamplingLine(size_t* po_pPixelCount)
    {
    return  m_pResamplingSurfaceEditor->GetNextRun(&m_CurrentPixelX,
                                                   &m_CurrentPixelY,
                                                   po_pPixelCount);
    }


//-----------------------------------------------------------------------------
// Get the first pixel into the surface
//-----------------------------------------------------------------------------
const void* HRABitmap::GetFirstResamplingPixel(HRABitmapEditor*     pi_pEditor,
                                               Byte               pi_PixelsInterval)
    {
    HPRECONDITION(pi_pEditor != 0);
    HPRECONDITION(pi_PixelsInterval > 0);

    m_PixelsInterval = pi_PixelsInterval;
    m_pResamplingSurfaceEditor = pi_pEditor->GetSurfaceEditor();

    size_t PixelCount;
    void* pRawData;

    if (m_PixelsInterval == 1)
        pRawData = m_pResamplingSurfaceEditor->GetFirstPixel(&m_CurrentPixelX, &m_CurrentPixelY);
    else
        {
        pRawData = m_pResamplingSurfaceEditor->GetFirstRun(&m_CurrentPixelX,
                                                           &m_CurrentPixelY,
                                                           &PixelCount);
        m_LastPixelX = m_CurrentPixelX + PixelCount;
        }

    return pRawData;
    }


//-----------------------------------------------------------------------------
// Get the next pixel into the surface
//-----------------------------------------------------------------------------
const void* HRABitmap::GetNextResamplingPixel()
    {
    void* pRawData;

    if (m_PixelsInterval == 1)
        pRawData = m_pResamplingSurfaceEditor->GetNextPixel(&m_CurrentPixelX,
                                                            &m_CurrentPixelY);
    else
        {
        m_CurrentPixelX += m_PixelsInterval;

        if (m_CurrentPixelX >= m_LastPixelX)
            {
            size_t PixelCount;
            pRawData = m_pResamplingSurfaceEditor->GetNextRun(&m_CurrentPixelX,
                                                              &m_CurrentPixelY,
                                                              &PixelCount);
            m_LastPixelX = m_CurrentPixelX + PixelCount;
            }
        else
            pRawData = m_pResamplingSurfaceEditor->GetPixel(m_CurrentPixelX, m_CurrentPixelY);
        }
    return pRawData;
    }

//-----------------------------------------------------------------------------
// TerminateResamplingPixel
//-----------------------------------------------------------------------------
void HRABitmap::TerminateResamplingPixel()
    {
    m_pResamplingSurfaceEditor = 0;
    }

//-----------------------------------------------------------------------------
// Compute histogram on an RLE surface
//-----------------------------------------------------------------------------
void HRABitmap::ComputeHistogramRLE(HRAHistogramOptions* pio_pOptions)
    {
    HPRECONDITION(m_pPacket != 0);
    HPRECONDITION(m_pPacket->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID));
    HPRECONDITION(((const HFCPtr<HCDCodecHMRRLE1>&)m_pPacket->GetCodec())->HasLineIndexesTable());

    uint32_t* pLineIndexes = ((const HFCPtr<HCDCodecHMRRLE1>&)m_pPacket->GetCodec())->GetLineIndexesTable();

    bool HasShape = (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0);
    bool ComputeHistogram = true;
    HVEShape PhysicalRegion;

    // allocate an editor
    if (HasShape)
        {
        PhysicalRegion = *pio_pOptions->GetSamplingOptions().GetRegionToScan();
        PhysicalRegion.Intersect(*GetEffectiveShape());
        PhysicalRegion.ChangeCoordSys(GetPhysicalCoordSys());

        ComputeHistogram = !PhysicalRegion.IsEmpty();
        HasShape = !PhysicalRegion.IsEmpty();
        }

    if (ComputeHistogram)
        {
        if (HasShape)
            {
            HGFScanLines Scanlines;
            PhysicalRegion.GenerateScanLines(Scanlines);

            bool RunFound = Scanlines.GotoFirstRun();

            while (RunFound)
                {
                HSINTX RunPosX;
                HSINTX RunPosY;
                size_t RunLen;
                Scanlines.GetCurrentRun(&RunPosX, &RunPosY, &RunLen);
                HPOSTCONDITION(RunPosX >= 0);
                HPOSTCONDITION(RunPosY >= 0);
                HPOSTCONDITION(RunLen < ULONG_MAX);

                unsigned short* pRun = &((unsigned short*)m_pPacket->GetBufferAddress())[pLineIndexes[RunPosY]];
                bool RunState = false;
                size_t PixelFromRun = (size_t)RunPosX;
                while (PixelFromRun >= *pRun)
                    {
                    PixelFromRun -= *pRun;
                    pRun++;
                    RunState = !RunState;
                    }

                if (PixelFromRun != 0)
                    {
                    uint32_t CurrentRunLen = (uint32_t)min(PixelFromRun, RunLen);
                    pio_pOptions->GetHistogram()->IncrementEntryCount((RunState ? 1 : 0),
                                                                      CurrentRunLen);

                    RunLen -= CurrentRunLen;
                    pRun++;
                    RunState = !RunState;
                    }

                while (RunLen > 0)
                    {
                    pio_pOptions->GetHistogram()->IncrementEntryCount((RunState ? 1 : 0),
                                                                      *pRun);
                    HPRECONDITION(*pRun <= RunLen);
                    RunLen -= *pRun;
                    pRun++;
                    RunState = !RunState;
                    }

                RunFound = Scanlines.GotoNextRun();
                }
            }
        else
            {
            // no shape
            uint64_t Width;
            uint64_t Height;
            GetSize(&Width, &Height);

            HASSERT(Width <= ULONG_MAX);
            HASSERT(Height <= ULONG_MAX);

            uint32_t PixelCount;
            bool RunState;
            unsigned short* pRun;

            for (uint32_t i = 0; i < (uint32_t)Height; i++)
                {
                pRun = &((unsigned short*)m_pPacket->GetBufferAddress())[pLineIndexes[i]];
                PixelCount = (uint32_t)Width;
                RunState = false;
                while (PixelCount > 0)
                    {
                    pio_pOptions->GetHistogram()->IncrementEntryCount((RunState ? 1 : 0),
                                                                      *pRun);
                    PixelCount -= *pRun;
                    pRun++;
                    RunState = !RunState;
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// ComputeLightnessHistogram
//-----------------------------------------------------------------------------

void HRABitmap::ComputeLightnessHistogram(HRAHistogramOptions* pio_pOptions)
    {
    HPRECONDITION(pio_pOptions != 0);
    HPRECONDITION(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::LIGHTNESS);

    Byte PixelsInterval = (Byte)(100 / pio_pOptions->GetSamplingOptions().GetPixelsToScan());
    HFCPtr<HRPPixelType> pSrcPixelType;

    // test if we must use the source pixel type or a pixel type replacer
    if(pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
        GetSurfaceDescriptor(&pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer(), &pSrcPixelType);
    else
        GetSurfaceDescriptor(0, &pSrcPixelType);

    HFCPtr<HRPPixelConverter> pConverter;
    HFCPtr<HRPPixelType>      pPixelType;
    HAutoPtr<HRABitmapEditor> pEditor;

    // allocate an editor
    if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
        {
        HVEShape PhysicalRegion(*pio_pOptions->GetSamplingOptions().GetRegionToScan());
        PhysicalRegion.Intersect(*GetEffectiveShape());
        PhysicalRegion.ChangeCoordSys(GetPhysicalCoordSys());

        if (!PhysicalRegion.IsEmpty())
            pEditor = (HRABitmapEditor*)CreateEditor(PhysicalRegion, HFC_READ_ONLY);
        }
    else
        pEditor = (HRABitmapEditor*)CreateEditor(HFC_READ_ONLY);

    if (pEditor != 0)
        {
        // go to the first pixel
        const void* pRawData = GetFirstResamplingPixel(pEditor, PixelsInterval);
        if(pRawData != 0)
            {
            // a variable that contain each composite value converted
            // 32 is an arbitrary number larger enough to contain any kind of
            // composite value
            Byte Value[32];

            if ((pio_pOptions->GetPixelType() == 0 || pio_pOptions->GetPixelType()->CountIndexBits() == 0) &&
                pSrcPixelType->GetChannelOrg().GetChannelPtr(0)->GetSize() == 8)
                {
                HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();

                HUINTX XPhysical;
                HUINTX YPhysical;
                size_t NumPixels;
                Byte* pCurrentPixel;
                HFCPtr<HGSEditor> pSurfaceEditor(pEditor->GetSurfaceEditor());
                if (pSurfaceEditor->GetFirstRun(&XPhysical, &YPhysical, &NumPixels))
                    {
                    double LightnessValue;
                    uint32_t MaxFrequencyValue = pHistogram->GetEntryFrequenciesSize(0) - 1;
                    double Scale = (double)(MaxFrequencyValue) / 100.0 + HGLOBAL_EPSILON;

                    HGFLightnessColorSpace ColorSpaceConverter(DEFAULT_GAMMA_FACTOR, 8);
                    do
                        {
                        // Obtain pointer to first pixel to process
                        pCurrentPixel = (Byte*) pSurfaceEditor->GetPixel(XPhysical, YPhysical);
                        size_t ChannelCount = pSrcPixelType->GetChannelOrg().CountChannels();

                        if (ChannelCount == 4)
                            {
                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID) ||
                                    pSrcPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8X8::CLASS_ID));

                            for( ; NumPixels ; --NumPixels)
                                {
                                LightnessValue = ColorSpaceConverter.ConvertFromRGB(pCurrentPixel[0],
                                                                                    pCurrentPixel[1],
                                                                                    pCurrentPixel[2]);
                                // LightnessValue *= Scale;
                                // LightnessValue  = min(LightnessValue, MaxFrequencyValue);
                                // pHistogram->IncrementEntryCount((UInt32)LightnessValue, 1, 0);

                                pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                }
                            }
                        else if (ChannelCount == 3)
                            {
                            if (pSrcPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
                                {
                                for( ; NumPixels ; --NumPixels)
                                    {
                                    LightnessValue = ColorSpaceConverter.ConvertFromRGB(pCurrentPixel[0],
                                                                                        pCurrentPixel[1],
                                                                                        pCurrentPixel[2]);

                                    pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                                    ++pCurrentPixel;
                                    ++pCurrentPixel;
                                    ++pCurrentPixel;
                                    }
                                }
                            else if (pSrcPixelType->IsCompatibleWith(HRPPixelTypeV24B8G8R8::CLASS_ID))
                                {
                                for( ; NumPixels ; --NumPixels)
                                    {
                                    LightnessValue = ColorSpaceConverter.ConvertFromRGB(pCurrentPixel[2],
                                                                                        pCurrentPixel[1],
                                                                                        pCurrentPixel[0]);

                                    pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                                    ++pCurrentPixel;
                                    ++pCurrentPixel;
                                    ++pCurrentPixel;
                                    }
                                }
                            else
                                HASSERT(false);
                            }
                        else
                            {
                            for( ; NumPixels ; --NumPixels)
                                {
                                LightnessValue = ColorSpaceConverter.ConvertFromRGB(*pCurrentPixel,
                                                                                    *pCurrentPixel,
                                                                                    *pCurrentPixel);

                                pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                                ++pCurrentPixel;
                                }
                            }
                        }
                    while (pSurfaceEditor->GetNextRun(&XPhysical, &YPhysical, &NumPixels));
                    }
                }
            else if (pSrcPixelType->GetChannelOrg().GetChannelPtr(0)->GetSize() == 16)
                {
                HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();

                HUINTX   XPhysical;
                HUINTX   YPhysical;
                size_t   NumPixels;
                uint16_t* pCurrentPixel;
                HFCPtr<HGSEditor> pSurfaceEditor(pEditor->GetSurfaceEditor());
                if (pSurfaceEditor->GetFirstRun(&XPhysical, &YPhysical, &NumPixels))
                    {
                    double LightnessValue;
                    uint32_t MaxFrequencyValue = pHistogram->GetEntryFrequenciesSize(0) - 1;
                    double Scale = (double)(pHistogram->GetEntryFrequenciesSize(0)) / 100.0;

                    HGFLightnessColorSpace ColorSpaceConverter(DEFAULT_GAMMA_FACTOR, 16);

                    do
                        {
                        // Obtain pointer to first pixel to process
                        pCurrentPixel = (uint16_t*) pSurfaceEditor->GetPixel(XPhysical, YPhysical);
                        size_t ChannelCount = pSrcPixelType->GetChannelOrg().CountChannels();

                        if (ChannelCount == 4)
                            {
                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV64R16G16B16A16::CLASS_ID) || pSrcPixelType->IsCompatibleWith(HRPPixelTypeV64R16G16B16X16::CLASS_ID));

                            for( ; NumPixels ; --NumPixels)
                                {
                                LightnessValue = ColorSpaceConverter.ConvertFromRGB(pCurrentPixel[0],
                                                                                    pCurrentPixel[1],
                                                                                    pCurrentPixel[2]);

                                pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                }
                            }
                        else if (ChannelCount == 3)
                            {
                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV48R16G16B16::CLASS_ID));

                            for( ; NumPixels ; --NumPixels)
                                {
                                LightnessValue = ColorSpaceConverter.ConvertFromRGB(pCurrentPixel[0],
                                                                                    pCurrentPixel[1],
                                                                                    pCurrentPixel[2]);

                                pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                ++pCurrentPixel;
                                }
                            }
                        else
                            {
                            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID));

                            for( ; NumPixels ; --NumPixels)
                                {
                                LightnessValue = ColorSpaceConverter.ConvertFromRGB(*pCurrentPixel,
                                                                                    *pCurrentPixel,
                                                                                    *pCurrentPixel);

                                pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                                ++pCurrentPixel;
                                }
                            }
                        }
                    while (pSurfaceEditor->GetNextRun(&XPhysical, &YPhysical, &NumPixels));
                    }
                }
            else
                {
                HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();

                double LightnessValue;
                uint32_t MaxFrequencyValue = pHistogram->GetEntryFrequenciesSize(0) - 1;
                double Scale = (double)(pHistogram->GetEntryFrequenciesSize(0)) / 100.0;

                if (MaxFrequencyValue == USHRT_MAX)
                    {
                    //16 bits histogram
                    HRPPixelTypeV48R16G16B16 ConvertedPixelType;
                    unsigned short Value[3];
                    pConverter = ConvertedPixelType.GetConverterFrom(pSrcPixelType);

                    HGFLightnessColorSpace ColorSpaceConverter(DEFAULT_GAMMA_FACTOR, 16);

                    do
                        {
                        pConverter->ConvertToValue(pRawData,(void*)Value);

                        // HChkSebG !!!! HGFLightnessColorSpace !!!!
                        LightnessValue = ColorSpaceConverter.ConvertFromRGB (Value[0],
                                                                             Value[1],
                                                                             Value[2]);

                        pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                        }
                    while((pRawData = GetNextResamplingPixel()) != 0);
                    }
                else
                    {
                    //8 bits histogram
                    HRPPixelTypeV24R8G8B8 ConvertedPixelType;
                    pConverter = ConvertedPixelType.GetConverterFrom(pSrcPixelType);

                    HGFLightnessColorSpace ColorSpaceConverter(DEFAULT_GAMMA_FACTOR, 8);

                    do
                        {
                        pConverter->ConvertToValue(pRawData,(void*)Value);

                        // HChkSebG !!!! HGFLightnessColorSpace !!!!
                        LightnessValue = ColorSpaceConverter.ConvertFromRGB(Value[0],
                                                                            Value[1],
                                                                            Value[2]);

                        pHistogram->IncrementEntryCount((uint32_t)(min(LightnessValue * Scale, MaxFrequencyValue)), 1, 0);

                        }
                    while((pRawData = GetNextResamplingPixel()) != 0);
                    }
                }
            }
        TerminateResamplingPixel();
        }
    }

//-----------------------------------------------------------------------------
// public
// ComputeLightnessHistogram
//-----------------------------------------------------------------------------

void HRABitmap::ComputeGrayscaleHistogram(HRAHistogramOptions* pio_pOptions)
    {
    HPRECONDITION(pio_pOptions != 0);
    HPRECONDITION(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::GRAYSCALE);

    HFCPtr<HRPPixelType> pSrcPixelType;

    // test if we must use the source pixel type or a pixel type replacer
    if(pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
        GetSurfaceDescriptor(&pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer(), &pSrcPixelType);
    else
        GetSurfaceDescriptor(0, &pSrcPixelType);

    HFCPtr<HRPPixelConverter> pConverter;
    HFCPtr<HRPPixelType>      pPixelType;
    HAutoPtr<HRABitmapEditor> pEditor;

    // allocate an editor
    if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
        {
        HVEShape PhysicalRegion(*pio_pOptions->GetSamplingOptions().GetRegionToScan());
        PhysicalRegion.Intersect(*GetEffectiveShape());
        PhysicalRegion.ChangeCoordSys(GetPhysicalCoordSys());

        if (!PhysicalRegion.IsEmpty())
            pEditor = (HRABitmapEditor*)CreateEditor(PhysicalRegion, HFC_READ_ONLY);
        }
    else
        pEditor = (HRABitmapEditor*)CreateEditor(HFC_READ_ONLY);

    if (pEditor != 0)
        {
        Byte PixelsInterval = (Byte)(100 / pio_pOptions->GetSamplingOptions().GetPixelsToScan());

        // go to the first pixel
        const void* pRawData = GetFirstResamplingPixel(pEditor, PixelsInterval);
        if(pRawData != 0)
            {
            // a variable that contain each composite value converted
            // 32 is an arbitrary number larger enough to contain any kind of
            // composite value
            Byte Value[32];

            HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();
            HRPPixelTypeV8Gray8 ConvertedPixelType;

            pConverter = ConvertedPixelType.GetConverterFrom(pSrcPixelType);
            do
                {
                pConverter->ConvertToValue(pRawData,(void*)Value);
                pHistogram->IncrementEntryCount(*Value, 1, 0);

                }
            while((pRawData = GetNextResamplingPixel()) != 0);
            }
        TerminateResamplingPixel();
        }
    }

//-----------------------------------------------------------------------------
// public
// ComputeLightnessHistogram
//-----------------------------------------------------------------------------

void HRABitmap::ComputeRGBHistogram(HRAHistogramOptions* pio_pOptions)
    {
    HPRECONDITION(pio_pOptions != 0);
    HPRECONDITION(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::RGB);

    HFCPtr<HRPPixelType> pSrcPixelType;

    // test if we must use the source pixel type or a pixel type replacer
    if(pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
        GetSurfaceDescriptor(&pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer(), &pSrcPixelType);
    else
        GetSurfaceDescriptor(0, &pSrcPixelType);

    HFCPtr<HRPPixelConverter> pConverter;
    HFCPtr<HRPPixelType>      pPixelType;
    HAutoPtr<HRABitmapEditor> pEditor;

    // allocate an editor
    if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
        {
        HVEShape PhysicalRegion(*pio_pOptions->GetSamplingOptions().GetRegionToScan());
        PhysicalRegion.Intersect(*GetEffectiveShape());
        PhysicalRegion.ChangeCoordSys(GetPhysicalCoordSys());

        if (!PhysicalRegion.IsEmpty())
            pEditor = (HRABitmapEditor*)CreateEditor(PhysicalRegion, HFC_READ_ONLY);
        }
    else
        pEditor = (HRABitmapEditor*)CreateEditor(HFC_READ_ONLY);

    if (pEditor != 0)
        {
        Byte PixelsInterval = (Byte)(100 / pio_pOptions->GetSamplingOptions().GetPixelsToScan());

        // go to the first pixel
        const void* pRawData = GetFirstResamplingPixel(pEditor, PixelsInterval);
        if(pRawData != 0)
            {
            // a variable that contain each composite value converted
            // 32 is an arbitrary number larger enough to contain any kind of
            // composite value
            Byte Value[32];

            HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();
            HRPPixelTypeV24R8G8B8 ConvertedPixelType;

            pConverter = ConvertedPixelType.GetConverterFrom(pSrcPixelType);
            do
                {
                pConverter->ConvertToValue(pRawData,(void*)Value);

                pHistogram->IncrementEntryCount(*(Value    ), 1, 0);
                pHistogram->IncrementEntryCount(*(Value + 1), 1, 1);
                pHistogram->IncrementEntryCount(*(Value + 2), 1, 2);

                }
            while((pRawData = GetNextResamplingPixel()) != 0);
            }
        TerminateResamplingPixel();
        }
    }

//-----------------------------------------------------------------------------
// public
// ComputeLightnessHistogram
//-----------------------------------------------------------------------------

void HRABitmap::ComputeRGBAHistogram(HRAHistogramOptions* pio_pOptions)
    {
    HPRECONDITION(pio_pOptions != 0);
    HPRECONDITION(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::RGBA);

    HFCPtr<HRPPixelType> pSrcPixelType;

    // test if we must use the source pixel type or a pixel type replacer
    if(pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
        GetSurfaceDescriptor(&pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer(), &pSrcPixelType);
    else
        GetSurfaceDescriptor(0, &pSrcPixelType);

    HFCPtr<HRPPixelConverter> pConverter;
    HFCPtr<HRPPixelType>      pPixelType;
    HAutoPtr<HRABitmapEditor> pEditor;

    // allocate an editor
    if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
        {
        HVEShape PhysicalRegion(*pio_pOptions->GetSamplingOptions().GetRegionToScan());
        PhysicalRegion.Intersect(*GetEffectiveShape());
        PhysicalRegion.ChangeCoordSys(GetPhysicalCoordSys());

        if (!PhysicalRegion.IsEmpty())
            pEditor = (HRABitmapEditor*)CreateEditor(PhysicalRegion, HFC_READ_ONLY);
        }
    else
        pEditor = (HRABitmapEditor*)CreateEditor(HFC_READ_ONLY);

    if (pEditor != 0)
        {
        Byte PixelsInterval = (Byte)(100 / pio_pOptions->GetSamplingOptions().GetPixelsToScan());

        // go to the first pixel
        const void* pRawData = GetFirstResamplingPixel(pEditor, PixelsInterval);
        if(pRawData != 0)
            {
            // a variable that contain each composite value converted
            // 32 is an arbitrary number larger enough to contain any kind of
            // composite value
            Byte Value[32];

            HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();
            HRPPixelTypeV32R8G8B8A8 ConvertedPixelType;

            pConverter = ConvertedPixelType.GetConverterFrom(pSrcPixelType);
            do
                {
                pConverter->ConvertToValue(pRawData,(void*)Value);

                pHistogram->IncrementEntryCount(*(Value    ), 1, 0);
                pHistogram->IncrementEntryCount(*(Value + 1), 1, 1);
                pHistogram->IncrementEntryCount(*(Value + 2), 1, 2);
                pHistogram->IncrementEntryCount(*(Value + 2), 1, 3);

                }
            while((pRawData = GetNextResamplingPixel()) != 0);
            }
        TerminateResamplingPixel();
        }
    }
//-----------------------------------------------------------------------------
// public
// MakeEmpty - Free all unnecessary memory in the Bitmap
//-----------------------------------------------------------------------------
void HRABitmap::MakeEmpty()
    {
    // Memory manager stuff
    if (GetPool() != 0 && GetPool()->IsMemoryMgrEnabled())
        {
        m_pPacket->SetBufferOwnership(false);
        GetPool()->FreeMemory(m_pPacket->GetBufferAddress(), m_pPacket->GetBufferSize());
        }

    m_pPacket->SetBuffer(0, 0);
    m_pPacket->SetDataSize(0);
    m_pPacket->SetBufferOwnership(false);
    m_pSurfaceDescriptor = 0;
    }

// ----------------------------------------------------------------------------
// public
// SetPacket
// ----------------------------------------------------------------------------
void HRABitmap::SetPacket(const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    m_pSurfaceDescriptor = 0;

    m_pPacket = pi_rpPacket;
    }

//-----------------------------------------------------------------------------
// public
// MakeEmpty - Free all unnecessary memory in the Butmap
//-----------------------------------------------------------------------------
bool HRABitmap::IsEmpty() const
    {
    return (m_pPacket->GetBufferSize() == 0);
    }

// ----------------------------------------------------------------------------
// private
// HRABitmap::DeepDelete - Deep deletes allocated data
// ----------------------------------------------------------------------------
void HRABitmap::DeepDelete()
    {
    }

// ----------------------------------------------------------------------------
// public
// GetCodec
// ----------------------------------------------------------------------------
const HFCPtr<HCDCodec>& HRABitmap::GetCodec() const
    {
    return m_pPacket->GetCodec();
    }

// ----------------------------------------------------------------------------
// public
// SetPool / GetPool - MemoryManager
// ----------------------------------------------------------------------------
void HRABitmap::SetPool(HPMPool* pi_pPool)
    {
    m_pPool = pi_pPool;
    }
HPMPool* HRABitmap::GetPool() const
    {
    return m_pPool;
    }