//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABitmapRLE.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRABitmapRLE
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRABitmapRLE.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGFLightnessColorSpace.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRASamplingOptions.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HVE2DRectangle.h>

#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGSMemoryRLESurfaceDescriptor.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HRABitmapEditor.h>
#include <Imagepp/all/h/HGFScanLines.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HCDPacket.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HGFTolerance.h>
#include <Imagepp/all/h/HRPMessages.h>
#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePPInternal/gra/HRAImageNode.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>



HPM_REGISTER_CLASS(HRABitmapRLE, HRABitmapBase)

HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRABitmapRLE, HRABitmapBase, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRABitmapRLE, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_END_MESSAGE_MAP()


//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct BitmapRleSourceNode : ImageSourceNode
{
public:
    static RefCountedPtr<BitmapRleSourceNode> Create(HRABitmapRLE& bitmap, HFCPtr<HGF2DCoordSys>& pPhysicalCoordSys, HFCPtr<HRPPixelType>& pPixelType)
        {
        return new BitmapRleSourceNode(bitmap, pPhysicalCoordSys, pPixelType);
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  07/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    BitmapRleSourceNode(HRABitmapRLE& bitmap, HFCPtr<HGF2DCoordSys>& pPhysicalCoordSys, HFCPtr<HRPPixelType>& pPixelType)
        :ImageSourceNode(pPixelType),    
         m_bitmap(bitmap)
        {
        BeAssert(pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));

        uint64_t width64, height64;
        m_bitmap.GetSize(&width64, &height64);
        m_physicalExtent = HGF2DExtent(0, 0, (double)width64, (double)height64, pPhysicalCoordSys);

        BeAssert(width64 <= ULONG_MAX && height64 <= ULONG_MAX);
        m_width = (uint32_t)width64;
        m_height = (uint32_t)height64;
        }

    virtual ~BitmapRleSourceNode(){}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _GetRegion(HRAImageSurfacePtr& pOutSurface, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator) override
        {
        // Bitmap do not care about the asked region.  It returns a ref to itself, without copying memory.  If extra clamping pixels
        // are required it is assumed that the transformNode sampler will do it.
        outOffset.x = outOffset.y = 0;

        HRAPacketRleSurfacePtr pSurface = HRAPacketRleSurface::CreateSurface(m_width, m_height, GetPixelType(), m_height);

        HFCPtr<HRABitmapRLE> pBitmapRle(&m_bitmap); // HRABitmapRLE is always an HFCPtr.
        pSurface->AppendStrip(pBitmapRle);

        pOutSurface = pSurface;
        
        return IMAGEPP_STATUS_Success;
        }      

    virtual HGF2DExtent const& _GetPhysicalExtent() const override { return m_physicalExtent; }
    virtual HFCPtr<HGF2DCoordSys>& _GetPhysicalCoordSys() override { return const_cast<HFCPtr<HGF2DCoordSys>&>(m_physicalExtent.GetCoordSys()); }
    
private:
    HRABitmapRLE&  m_bitmap;
    HGF2DExtent m_physicalExtent;
    uint32_t    m_width;
    uint32_t    m_height;
    size_t      m_bytesPerRow;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRABitmapRLE> HRABitmapRLE::Create(uint32_t                       pi_WidthPixels,
                                          uint32_t                       pi_HeightPixels,
                                          const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                                          const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                                          const HFCPtr<HRPPixelType>&    pi_rpPixel,
                                          bool                           pi_Resizable)
    {
    return new HRABitmapRLE(pi_WidthPixels, pi_HeightPixels, pi_pModelCSp_CSl, pi_rpLogicalCoordSys, pi_rpPixel, pi_Resizable);
    }

//-----------------------------------------------------------------------------
// public
// Default constructor. Create a HRABitmapRLE of 1 pixel height
//-----------------------------------------------------------------------------
HRABitmapRLE::HRABitmapRLE(bool pi_Resizable)
    : HRABitmapBase(new HRPPixelTypeI1R8G8B8,
                    pi_Resizable)
    {
    m_pPacketRLE = new HCDPacketRLE(1,1);
    }

//-----------------------------------------------------------------------------
// public
// HRABitmapRLE::HRABitmapRLE - Full featured constructor.
//-----------------------------------------------------------------------------
HRABitmapRLE::HRABitmapRLE(uint32_t                       pi_WidthPixels,
                           uint32_t                       pi_HeightPixels,
                           const HGF2DTransfoModel*       pi_pModelCSp_CSl,
                           const HFCPtr<HGF2DCoordSys>&   pi_rpLogicalCoordSys,
                           const HFCPtr<HRPPixelType>&    pi_rpPixel,
                           bool                          pi_Resizable)
    : HRABitmapBase(pi_WidthPixels,
                    pi_HeightPixels,
                    pi_pModelCSp_CSl,
                    pi_rpLogicalCoordSys,
                    pi_rpPixel,
                    8,
                    pi_Resizable)
    {
    HPRECONDITION(pi_rpPixel != 0);
    HPRECONDITION(pi_rpPixel->IsCompatibleWith(HRPPixelTypeI1R8G8B8::CLASS_ID)   ||
                  pi_rpPixel->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8::CLASS_ID));

// We should not create HRABitmapRLE with the RLE pixelType.
//                                                                                    ||
//                  pi_rpPixel->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID)   ||
//                  pi_rpPixel->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));

    m_pPacketRLE = new HCDPacketRLE(pi_WidthPixels, pi_HeightPixels);
    }

//-----------------------------------------------------------------------------
// public
// HRABitmapRLE::~HRABitmapRLE - Destructor
//-----------------------------------------------------------------------------
HRABitmapRLE::~HRABitmapRLE()
    {
    DeepDelete();

    // Memory Manager stuff
    m_pSurfaceDescriptor = 0;

    m_pPacketRLE = 0;
    }

//-----------------------------------------------------------------------------
// public
// Clone - Store and Log are not used.
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRABitmapRLE::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HRABitmapRLE(*this);
    }
//-----------------------------------------------------------------------------
// public
// Clone - Store and Log are not used.
//-----------------------------------------------------------------------------
HPMPersistentObject* HRABitmapRLE::Clone () const
    {
    return new HRABitmapRLE(*this);
    }


//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRABitmapRLE::Clear()
    {
    HRAClearOptions ClearOptions;
    Clear(ClearOptions);
    }

//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRABitmapRLE::Clear(const HRAClearOptions& pi_rOptions)
    {
    // patch to be sure it works with the editor even if the raster is not
    // in a HFCPtr somewhere
    IncrementRef();
        {
        HAutoPtr<HRABitmapEditor> pEditor;
        HFCPtr<HVEShape> pShape;
        const HVEShape* pClearShape = 0;

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
            if (pi_rOptions.HasShape())
                {
                if (pi_rOptions.HasApplyRasterClipping())
                    {
                    pShape = new HVEShape(*GetEffectiveShape());
                    pShape->Intersect(*pi_rOptions.GetShape());
                    pClearShape = pShape;
                    }
                else
                    pClearShape = pi_rOptions.GetShape();

                pEditor = (HRABitmapEditor*)CreateEditor(*pClearShape, HFC_WRITE_ONLY);
                }
            else
                pEditor = (HRABitmapEditor*)CreateEditor(HFC_WRITE_ONLY);
            }

        HASSERT(pEditor != 0);

        HRAEditor* pSurfaceEditor = pEditor->GetSurfaceEditor();

        HPRECONDITION(pSurfaceEditor != 0);
        HPRECONDITION(pSurfaceEditor->GetSurface().GetSurfaceDescriptor() != 0);
        HPRECONDITION(pSurfaceEditor->GetSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

        HFCPtr<HRPPixelType> pPixelTypeRLE1 = ((const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pSurfaceEditor->GetSurface().GetSurfaceDescriptor())->GetPixelType();

        // We assume the converter will return RLE run
        HPOSTCONDITION(pPixelTypeRLE1 != 0 &&
                       (pPixelTypeRLE1->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pPixelTypeRLE1->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID)));

        HFCPtr<HRPPixelConverter> pConverter(GetPixelType()->GetConverterTo(pPixelTypeRLE1));
        unsigned short OutData[4];

        if (pi_rOptions.GetRawDataValue() != 0)
            pConverter->Convert(pi_rOptions.GetRawDataValue(), &OutData);
        else
            pConverter->Convert(GetPixelType()->GetDefaultRawData(), &OutData);

        OutData[0] = OutData[0] ? 0 : 1; // TR 159986: must clear with state(0,1) not RLE run.
        pSurfaceEditor->Clear(OutData, GetCurrentTransaction());

        Updated(pClearShape);   // if we clear by scanline, pClearShape == 0
        }

    DecrementRef();
    }

//-----------------------------------------------------------------------------
// protected
// HRABitmapRLE::HRABitmapRLE - Copy constructor
//-----------------------------------------------------------------------------
HRABitmapRLE::HRABitmapRLE(const HRABitmapRLE& pi_rBitmap)
    : HRABitmapBase(pi_rBitmap)
    {
    // Perform initialization of the object
    DeepCopy(pi_rBitmap);
    }

//-----------------------------------------------------------------------------
// protected
// HRABitmapRLE::operator= - Assignment operator
//-----------------------------------------------------------------------------
HRABitmapRLE& HRABitmapRLE::operator=(const HRABitmapRLE& pi_rBitmap)
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
//-----------------------------------------------------------------------------
HRARasterEditor* HRABitmapRLE::CreateEditor (HFCAccessMode pi_Mode)
    {
    return new HRABitmapEditor(this, pi_Mode);
    }

HRARasterEditor* HRABitmapRLE::CreateEditor(const HVEShape& pi_rShape,
                                            HFCAccessMode   pi_Mode)
    {
    return new HRABitmapEditor(this, pi_rShape, pi_Mode);
    }

HRARasterEditor* HRABitmapRLE::CreateEditor(const HGFScanLines& pi_rScanLines,
                                            HFCAccessMode       pi_Mode)
    {
    return new HRABitmapEditor(this, pi_rScanLines, pi_Mode);
    }

HRARasterEditor* HRABitmapRLE::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return new HRABitmapEditor(this, pi_Mode);
    }

//-----------------------------------------------------------------------------
// public
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRABitmapRLE::GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short CountUsed = HRABitmapBase::GetRepresentativePalette(pio_pRepPalParms);

    return CountUsed;
    }

//-----------------------------------------------------------------------------
// public
// ComputeHistogram
//-----------------------------------------------------------------------------
void HRABitmapRLE::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                    bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    if (pi_ForceRecompute || (GetHistogram() == 0) || (GetHistogram() != 0 && !GetHistogram()->CanBeUsedInPlaceOf(*pio_pOptions)))
        {
        // Options are native build RLE histogram
        if(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::NATIVE &&
           pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() == 0 &&
           (pio_pOptions->GetPixelType() == 0 || pio_pOptions->GetPixelType()->GetID() == GetPixelType()->GetID()))
            {
            ComputeHistogramRLE(pio_pOptions);

            if (pio_pOptions->GetSamplingOptions().GetRegionToScan() == 0)
                SetHistogram(pio_pOptions); // set native histogram into HRARaster
            }
        else
            {
            // First get Native histogram
            HRAHistogramOptions NativeHisto(GetPixelType(), HRPHistogram::NATIVE);

            // Preserve region only. We don't care about anything else.
            // Support of pixel interval will only slow down processing so we do the real thing.
            // If pixel interval cause problems because it returns too much entries we should adjust the result using the pixel interval.
            if(pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
                NativeHisto.GetSamplingOptions().SetRegionToScan(pio_pOptions->GetSamplingOptions().GetRegionToScan());

            // This call will reuse already computed native histogram if available
            ComputeHistogram(&NativeHisto);

            HFCPtr<HRPPixelType> pSrcPixelType;
            // test if we must use the source pixel type or a pixel type replacer
            if(pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
                GetSurfaceDescriptor(&pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer(), &pSrcPixelType);
            else
                GetSurfaceDescriptor(0, &pSrcPixelType);

            HASSERT(pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pSrcPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));

            // Adjust native histogram to whatever caller asked.
            HFCPtr<HRPHistogram> pHistogram = pio_pOptions->GetHistogram();

            if(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::LIGHTNESS)
                {
                uint32_t              MaxFrequencyValue = pHistogram->GetEntryFrequenciesSize(0) - 1;
                double              Scale = (double)(MaxFrequencyValue) / 100.0 + HGLOBAL_EPSILON;

                HRPPixelTypeV24R8G8B8 PixelType;
                HFCPtr<HRPPixelConverter> pConverter(pSrcPixelType->GetConverterTo(&PixelType));
                Byte                    RGBValue[3];

                // LightnessValue *= Scale;
                // LightnessValue  = MIN(LightnessValue, MaxFrequencyValue);
                // pHistogram->IncrementEntryCount((UInt32)LightnessValue, 1, 0);
                HGFLightnessColorSpace ColorSpaceConverter(DEFAULT_GAMMA_FACTOR, 8);

                // Index 0
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(0), RGBValue);
                double LightnessValue = ColorSpaceConverter.ConvertFromRGB(RGBValue[0],RGBValue[1],RGBValue[2]);
                pHistogram->IncrementEntryCount((uint32_t)(MIN(LightnessValue * Scale, MaxFrequencyValue)), NativeHisto.GetHistogram()->GetEntryCount(0, 0), 0);

                // Index 1
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(1), RGBValue);
                LightnessValue = ColorSpaceConverter.ConvertFromRGB(RGBValue[0],RGBValue[1],RGBValue[2]);
                pHistogram->IncrementEntryCount((uint32_t)(MIN(LightnessValue * Scale, MaxFrequencyValue)), NativeHisto.GetHistogram()->GetEntryCount(1, 0), 0);
                }
            else if(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::GRAYSCALE)
                {
                HRPPixelTypeV8Gray8 PixelVal;
                HFCPtr<HRPPixelConverter> pConverter(pSrcPixelType->GetConverterTo(&PixelVal));
                Byte                    IndexValue;

                // Index 0
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(0), &IndexValue);
                pHistogram->IncrementEntryCount(IndexValue, NativeHisto.GetHistogram()->GetEntryCount(0, 0), 0);

                // Index 1
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(1), &IndexValue);
                pHistogram->IncrementEntryCount(IndexValue, NativeHisto.GetHistogram()->GetEntryCount(1, 0), 0);
                }
            else if(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::RGB)
                {
                HRPPixelTypeV24R8G8B8 PixelVal;
                HFCPtr<HRPPixelConverter> pConverter(pSrcPixelType->GetConverterTo(&PixelVal));
                Byte                    Value[3];

                // Index 0
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(0), Value);
                pHistogram->IncrementEntryCount(Value[0], NativeHisto.GetHistogram()->GetEntryCount(0, 0), 0);
                pHistogram->IncrementEntryCount(Value[1], NativeHisto.GetHistogram()->GetEntryCount(0, 0), 1);
                pHistogram->IncrementEntryCount(Value[2], NativeHisto.GetHistogram()->GetEntryCount(0, 0), 2);

                // Index 1
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(1), Value);
                pHistogram->IncrementEntryCount(Value[0], NativeHisto.GetHistogram()->GetEntryCount(1, 0), 0);
                pHistogram->IncrementEntryCount(Value[1], NativeHisto.GetHistogram()->GetEntryCount(1, 0), 1);
                pHistogram->IncrementEntryCount(Value[2], NativeHisto.GetHistogram()->GetEntryCount(1, 0), 2);

                }
            else if (pio_pOptions->GetSamplingColorSpace() == HRPHistogram::RGBA)
                {
                HRPPixelTypeV32R8G8B8A8 PixelVal;
                HFCPtr<HRPPixelConverter> pConverter(pSrcPixelType->GetConverterTo(&PixelVal));
                Byte                    Value[4];

                // Index 0
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(0), Value);
                pHistogram->IncrementEntryCount(Value[0], NativeHisto.GetHistogram()->GetEntryCount(0, 0), 0);
                pHistogram->IncrementEntryCount(Value[1], NativeHisto.GetHistogram()->GetEntryCount(0, 0), 1);
                pHistogram->IncrementEntryCount(Value[2], NativeHisto.GetHistogram()->GetEntryCount(0, 0), 2);
                pHistogram->IncrementEntryCount(Value[3], NativeHisto.GetHistogram()->GetEntryCount(0, 0), 3);

                // Index 1
                pConverter->Convert(pSrcPixelType->GetPalette().GetCompositeValue(1), Value);
                pHistogram->IncrementEntryCount(Value[0], NativeHisto.GetHistogram()->GetEntryCount(1, 0), 0);
                pHistogram->IncrementEntryCount(Value[1], NativeHisto.GetHistogram()->GetEntryCount(1, 0), 1);
                pHistogram->IncrementEntryCount(Value[2], NativeHisto.GetHistogram()->GetEntryCount(1, 0), 2);
                pHistogram->IncrementEntryCount(Value[3], NativeHisto.GetHistogram()->GetEntryCount(1, 0), 3);
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
void HRABitmapRLE::DeepCopy(const HRABitmapRLE& pi_rBitmap)
    {
    m_pPacketRLE = (HCDPacketRLE*)pi_rBitmap.m_pPacketRLE->Clone();
    }

//-----------------------------------------------------------------------------
// public
// InitSize
//-----------------------------------------------------------------------------
void HRABitmapRLE::InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels)
    {
    HASSERT(pi_WidthPixels <= ULONG_MAX);
    HASSERT(pi_HeightPixels <= ULONG_MAX);

    m_pPacketRLE = new HCDPacketRLE((uint32_t)pi_WidthPixels, (uint32_t)pi_HeightPixels);
    m_pSurfaceDescriptor = 0;
    HRABitmapBase::InitSize(pi_WidthPixels, pi_HeightPixels);
    }

//-----------------------------------------------------------------------------
// public
// GetAdditionalSize
//-----------------------------------------------------------------------------
size_t HRABitmapRLE::GetAdditionalSize() const
    {
    size_t DataSize = m_pPacketRLE->GetBufferSize();

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
        size_t BytesPerRow = ((uint32_t)Width * pPixelType->CountPixelRawDataBits() + 8 - 1) / 8;
        BytesPerRow = (BytesPerRow * 8 + 7) / 8;
        size_t RawDataSize = BytesPerRow * (uint32_t)Height;

        // Compute the ESTIMATED compressed size (e.g., NOT THE MAX)
        HCDCodecHMRRLE1 RleCode;
        DataSize = (size_t)(RleCode.GetEstimatedCompressionRatio() * RawDataSize);
        }

    return DataSize;
    }

// ----------------------------------------------------------------------------
// public
// GetSurfaceDescriptor
// ----------------------------------------------------------------------------
HFCPtr<HGSSurfaceDescriptor> HRABitmapRLE::GetSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
                                                                HFCPtr<HRPPixelType>* po_ppOutputPixelType) const
    {
    HFCPtr<HGSSurfaceDescriptor> pDescriptor;

    if((pi_ppReplacingPixelType != 0 && *pi_ppReplacingPixelType != 0) ||
       po_ppOutputPixelType != 0)
        {
        pDescriptor = CreateSurfaceDescriptor(pi_ppReplacingPixelType, po_ppOutputPixelType);
        }
    else if(m_pSurfaceDescriptor == 0)
        {
        pDescriptor = CreateSurfaceDescriptor(pi_ppReplacingPixelType, po_ppOutputPixelType);
        m_pSurfaceDescriptor = pDescriptor;
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
void HRABitmapRLE::Updated(const HVEShape* pi_pModifiedContent)
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
// GetCodec
// ----------------------------------------------------------------------------
const HFCPtr<HCDCodec>& HRABitmapRLE::GetCodec() const
    {
    return (const HFCPtr<HCDCodec>&)m_pPacketRLE->GetCodec();
    }

// ----------------------------------------------------------------------------
// public
// GetPacket
// ----------------------------------------------------------------------------
const HFCPtr<HCDPacketRLE>& HRABitmapRLE::GetPacket() const
    {
    // test if there is already a buffer allocated
    if (IsEmpty())
        {
        uint64_t Width;
        uint64_t Height;
        GetSize(&Width, &Height);

        HASSERT(Width <= ULONG_MAX);
        HASSERT(Height <= ULONG_MAX);

        uint32_t RunsPerLine = ((uint32_t)Width / 32767) * 2 + 1;

        HArrayAutoPtr<unsigned short> pTemplateLine(new unsigned short[RunsPerLine]);
        uint32_t PixelCount = (uint32_t)Width;
        unsigned short* pTLineItr = pTemplateLine;
        for(; PixelCount > 32767; PixelCount-=32767)
            {
            *pTLineItr++ = 32767;
            *pTLineItr++ = 0;
            }
        *pTLineItr = (unsigned short)PixelCount;    // remaining blacks.

        // packet is owner of buffers.
        m_pPacketRLE->SetBufferOwnership(true);

        for(uint32_t i=0; i < Height; ++i)
            {
            unsigned short* pLine = new unsigned short[RunsPerLine];
            memcpy(pLine, pTemplateLine, RunsPerLine*sizeof(unsigned short));
            m_pPacketRLE->SetLineBuffer(i, (Byte*)pLine, RunsPerLine*sizeof(unsigned short), RunsPerLine*sizeof(unsigned short));
            }
        }

    return m_pPacketRLE;
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRABitmapRLE::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    // If the Raster is empty, skip it
    if (IsEmpty())
        return;

    HRADrawOptions Options(pi_Options);

    // set the effective coordsys
    if (Options.GetReplacingCoordSys() == 0)
        // case when there is no replacing coordsys
        Options.SetReplacingCoordSys(GetCoordSys());

    // get the destination descriptor
    HFCPtr<HGSSurfaceDescriptor> pDstDescriptor(pio_destSurface.GetSurfaceDescriptor());

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
    pClipShape->ChangeCoordSys(pio_destSurface.GetSurfaceCoordSys());

    // store the old clip region
    HFCPtr<HGSRegion> pOldClipRegion(pio_destSurface.GetRegion());

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
        HVEShape DestSurfaceShape(0.0, 0.0, pDstDescriptor->GetWidth(), pDstDescriptor->GetHeight(), pio_destSurface.GetSurfaceCoordSys());
        HFCPtr<HGFTolerance> pTol (new HGFTolerance(GetSurfaceDescriptor()->GetWidth() / 2.0 - DEFAULT_PIXEL_TOLERANCE,
                                                    GetSurfaceDescriptor()->GetHeight() / 2.0 - DEFAULT_PIXEL_TOLERANCE,
                                                    GetSurfaceDescriptor()->GetWidth() / 2.0 + DEFAULT_PIXEL_TOLERANCE,
                                                    GetSurfaceDescriptor()->GetHeight() / 2.0 + DEFAULT_PIXEL_TOLERANCE,
                                                    DestSurfaceShape.GetCoordSys()));

        DestSurfaceShape.SetStrokeTolerance(pTol);

        pClipShape->Intersect(DestSurfaceShape);
        }

    // Not sure it is actually needed but just in case. Original code from HRABitmap.
    if ((m_pPacketRLE->GetCodec() != 0) /*&&
        m_pPacketRLE->GetCodec()->IsCompatibleWith(HCDCodecImage::CLASS_ID)*/)
        {
        // SOMETIMES. THE SOURCE COMPRESSED DATA IS INCOMPLETE!, WE MUST CLIP
        HFCPtr<HCDCodecHMRRLE1> const& pCodecImage = m_pPacketRLE->GetCodec();

        uint64_t WidthPixels;
        uint64_t HeightPixels;
        GetSize(&WidthPixels, &HeightPixels);

        if (pCodecImage->GetHeight() < HeightPixels)
            {
            HFCPtr<HVEShape> pShape(new HVEShape(0, 0, static_cast<double>(pCodecImage->GetWidth()),static_cast<double>(pCodecImage->GetHeight()),  GetPhysicalCoordSys()));
            pShape->ChangeCoordSys(GetCoordSys());
            pShape->SetCoordSys(Options.GetReplacingCoordSys());
            pShape->ChangeCoordSys(pio_destSurface.GetSurfaceCoordSys());

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
            pio_destSurface.SetRegion(pTheRegion);
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
        HFCPtr<HGF2DTransfoModel> pTransfoModel = pio_destSurface.GetSurfaceCoordSys()->GetTransfoModelTo (Options.GetReplacingCoordSys());

        if (GetTransfoModel() != 0)
            {
            pTransfoModel = pTransfoModel->ComposeInverseWithInverseOf(*GetTransfoModel());
            }

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
            StretchWithHGS(pio_destSurface, Options, pTransfoModel);
        else
            WarpWithHGS(pio_destSurface, Options, pTransfoModel);

        // set the old clip region back
        if (Clip)
            pio_destSurface.SetRegion(pOldClipRegion);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRABitmapRLE::_BuildCopyToContext(ImageTransformNodeR parentNode, HRACopyToOptionsCR options)
    {
    // Get the effective logical and physical coordsys
    HFCPtr<HGF2DCoordSys> pEffLogCS(GetCoordSys());
    HFCPtr<HGF2DCoordSys> pEffPhyCS(GetPhysicalCoordSys());
    if (NULL != options.GetReplacingCoordSys().GetPtr())
        {
        pEffLogCS = options.GetReplacingCoordSys();

        HFCPtr<HGF2DTransfoModel> pPhysicalToLogical(GetPhysicalCoordSys()->GetTransfoModelTo(GetCoordSys()));
        HFCPtr<HGF2DTransfoModel> pSimplModel(pPhysicalToLogical->CreateSimplifiedModel());
        if (NULL != pSimplModel)
            pPhysicalToLogical = pSimplModel;

        pEffPhyCS = new HGF2DCoordSys(*pPhysicalToLogical, pEffLogCS);
        }

    //Image Node and Image sample use the pixel type to differentiate between 1bit and Rle data. Unlike HRABitmap[RLE] that use pixeltype and codec.
    HFCPtr<HRPPixelType> pRlePixelType = ImageNode::TransformToRleEquivalent(options.GetReplacingPixelType() != NULL ? options.GetReplacingPixelType() : GetPixelType());
    if (pRlePixelType == NULL)
        {
        BeAssert(!"Incompatible replacing pixelType");
        return COPYFROM_STATUS_IncompatiblePixelTypeReplacer;
        }

    // Validate that we intersect with copyRegion.
    HDEBUGCODE
        (
        HVEShape myPhysicalExtent(GetPhysicalExtent());
        myPhysicalExtent.SetCoordSys(pEffPhyCS);

        BeAssert(options.GetShape()->HasIntersect(myPhysicalExtent));
    );

    RefCountedPtr<BitmapRleSourceNode> pSource = BitmapRleSourceNode::Create(*this, pEffPhyCS, pRlePixelType);
     
    return parentNode.LinkTo(*pSource);
    }

//-----------------------------------------------------------------------------
// private
// CreateSurfaceDescriptor
//-----------------------------------------------------------------------------
HFCPtr<HGSSurfaceDescriptor> HRABitmapRLE::CreateSurfaceDescriptor(const HFCPtr<HRPPixelType>* pi_ppReplacingPixelType,
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

    // we have an RLE1
    HASSERT(pSrcPixelType->CountPixelRawDataBits() == 1);

    // Transform I1RGB to I1RGBRLE
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

    // be sure that the buffer has been allocated
    GetPacket();

    // create the descriptor
    HFCPtr<HGSSurfaceDescriptor> pDescriptor(
        new HGSMemoryRLESurfaceDescriptor((uint32_t)Width,
                                          (uint32_t)Height,
                                          pSrcPixelType,
                                          m_pPacketRLE,
                                          HGFSLO::HGF_UPPER_LEFT_HORIZONTAL));

    pDescriptor->SetOffsets(m_XPosInRaster,
                            m_YPosInRaster);

    return pDescriptor;
    }

//-----------------------------------------------------------------------------
// Notification for palette change
//-----------------------------------------------------------------------------
bool HRABitmapRLE::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRABitmapRLE* HRABitmapRLE::_AsHRABitmapRleP()  
    {
    return this; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HCDPacket> HRABitmapRLE::GetContiguousBuffer() const
    {
    if (NULL == m_pPacketRLE.GetPtr())
        return NULL;

    size_t bufSize = m_pPacketRLE->GetBufferSize();
    HArrayAutoPtr<Byte> pContiguousBuffer(new Byte[bufSize]);

    if (NULL == pContiguousBuffer)
        return NULL;

    HFCPtr<HCDCodecHMRRLE1> pCodec = new HCDCodecHMRRLE1;
    if (NULL == pCodec.GetPtr())
        return NULL;

    // Line indexesTable is not enabled
    BeAssert(false == pCodec->HasLineIndexesTable());

    HFCPtr<HCDPacket> pPacket(new HCDPacket(pCodec.GetPtr(), pContiguousBuffer.release(), bufSize, bufSize));
    if (NULL == pPacket.GetPtr())
        return NULL;

    pPacket->SetBufferOwnership(true);

    uint64_t width, height;
    GetSize(&width, &height);
    size_t pos = 0;
    Byte* pDst = pPacket->GetBufferAddress();
    // Make contiguous buffer
    for (uint32_t line = 0; line < height; ++line)
        {
        memcpy(&pDst[pos], m_pPacketRLE->GetLineBuffer(line), m_pPacketRLE->GetLineDataSize(line));
        pos += m_pPacketRLE->GetLineDataSize(line);
        }

    return pPacket;
    }

//-----------------------------------------------------------------------------
// Compute histogram on an RLE surface
//-----------------------------------------------------------------------------
void HRABitmapRLE::ComputeHistogramRLE(HRAHistogramOptions* pio_pOptions)
    {
    HPRECONDITION(m_pPacketRLE != 0);
    HPRECONDITION(pio_pOptions != 0);
    HPRECONDITION(pio_pOptions->GetSamplingColorSpace() == HRPHistogram::NATIVE);
    HPRECONDITION(pio_pOptions->GetSamplingOptions().GetSrcPixelTypeReplacer() == 0);
    HPRECONDITION(pio_pOptions->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) ||
                  pio_pOptions->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
                  pio_pOptions->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8::CLASS_ID) ||
                  pio_pOptions->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8::CLASS_ID));

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

                unsigned short* pRun = (unsigned short*)m_pPacketRLE->GetLineBuffer(RunPosY);
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
                    uint32_t CurrentRunLen = (uint32_t)MIN(PixelFromRun, RunLen);
                    pio_pOptions->GetHistogram()->IncrementEntryCount((RunState ? 1 : 0), CurrentRunLen);

                    RunLen -= CurrentRunLen;
                    pRun++;
                    RunState = !RunState;
                    }

                while (RunLen > 0)
                    {
                    pio_pOptions->GetHistogram()->IncrementEntryCount((RunState ? 1 : 0), *pRun);
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

            for (uint32_t i = 0; i < Height; i++)
                {
                pRun = (unsigned short*)m_pPacketRLE->GetLineBuffer(i);
                PixelCount = (uint32_t)Width;
                RunState = false;
                while (PixelCount > 0)
                    {
                    pio_pOptions->GetHistogram()->IncrementEntryCount((RunState ? 1 : 0), *pRun);
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
// MakeEmpty - Free all unnecessary memory in the Bitmap
//-----------------------------------------------------------------------------
void HRABitmapRLE::MakeEmpty()
    {
    m_pPacketRLE->ClearAll();
    m_pPacketRLE->SetBufferOwnership(false);
    m_pSurfaceDescriptor = 0;
    }

//-----------------------------------------------------------------------------
// public
// MakeEmpty - Free all unnecessary memory in the Bitmap
//-----------------------------------------------------------------------------
bool HRABitmapRLE::IsEmpty() const
    {
    return m_pPacketRLE->GetBufferSize() == 0;
    }

// ----------------------------------------------------------------------------
// private
// HRABitmap::DeepDelete - Deep deletes allocated data
// ----------------------------------------------------------------------------
void HRABitmapRLE::DeepDelete()
    {
    }

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
template<typename SinkNode_T>
struct BitmapImageSurfaceIterator : public ImageSurfaceIterator
{
public:
    BitmapImageSurfaceIterator(SinkNode_T& sink, HFCInclusiveGrid const& grid)
        :m_bitmap(sink.GetBitmap()),
         m_grid(grid)
        {
        HRAPacketRleSurfacePtr pSurface = HRAPacketRleSurface::CreateSurface(sink.GetWidth(), sink.GetHeight(), sink.GetPixelType(), sink.GetHeight());
        HFCPtr<HRABitmapRLE> pBitmapRle(&m_bitmap); // HRABitmapRLE is always an HFCPtr.
        pSurface->AppendStrip(pBitmapRle);

        SetCurrent(*pSurface, PixelOffset64(0, 0));
        }

    virtual ~BitmapImageSurfaceIterator()
        {
        HVEShape rect((double)m_grid.GetXMin(), (double)m_grid.GetYMin(), (double)m_grid.GetXMax()+ 1.0, (double)m_grid.GetYMax()+ 1.0, m_bitmap.GetPhysicalCoordSys());
        m_bitmap.Updated(&rect);
        }
    
    virtual bool _Next() override {Invalidate(); return false;}

    HRABitmapRLE& m_bitmap;
    HFCInclusiveGrid m_grid;
};

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct BitmapRleSinkNode : public ImageSinkNode
{
public:
    typedef BitmapImageSurfaceIterator<BitmapRleSinkNode> Iterator;

    static ImageSinkNodePtr Create(HRABitmapRLE& raster, HVEShape const& sinkShape, HFCPtr<HGF2DCoordSys> pPhysicalCoordSys, HFCPtr<HRPPixelType>& pPixelType)
        {
        uint64_t width64, height64;
        raster.GetSize(&width64, &height64);
        return new BitmapRleSinkNode(raster, sinkShape, pPixelType, HGF2DExtent(0, 0, (double)width64, (double)height64, pPhysicalCoordSys));
        }

    HRABitmapRLE& GetBitmap() { return m_raster; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

protected:
    BitmapRleSinkNode(HRABitmapRLE& raster, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pPixelType, HGF2DExtent const& physicalExtent)
        :ImageSinkNode(sinkShape, pPixelType, physicalExtent),
        m_raster(raster)
        {
        BeAssert(pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));

        uint64_t width64, height64;
        m_raster.GetSize(&width64, &height64);
        m_width = (uint32_t)width64;
        m_height = (uint32_t)height64;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImageSurfaceIterator* _GetImageSurfaceIterator(HFCInclusiveGrid& strip) override
        {
        // We are using input pixeltype instead of the bitmap pixeltype because we may have a replacing pixeltype in the sinkNode.
        BeAssert(GetPixelType()->CountPixelRawDataBits() == m_raster.GetPixelType()->CountPixelRawDataBits());
        BeAssert(GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));

        HFCInclusiveGrid physicalGrid;
        physicalGrid.InitFromLenght(0, 0, m_width, m_height);

        HFCInclusiveGrid effectiveGrid;
        effectiveGrid.InitFromIntersectionOf(physicalGrid, strip);

        return new Iterator(*this, effectiveGrid);
        }

    //! Native block size of the sink.
    virtual uint32_t _GetBlockSizeX() override { return m_width; }
    virtual uint32_t _GetBlockSizeY() override { return m_height; }

    HRABitmapRLE&         m_raster;
    uint32_t              m_width;
    uint32_t              m_height;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSinkNodePtr HRABitmapRLE::_GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType)
    {
    status = IMAGEPP_STATUS_Success;

    //Image Node and Image sample use the pixel type to differentiate between 1bit and Rle data. Unlike HRABitmap[RLE] that use pixeltype and codec.
    HFCPtr<HRPPixelType> pRlePixelType = ImageNode::TransformToRleEquivalent(pReplacingPixelType ? pReplacingPixelType : GetPixelType());
    if (pRlePixelType == NULL)
        {
        BeAssert(!"Incompatible replacing pixelType");
        status = COPYFROM_STATUS_IncompatiblePixelTypeReplacer;
        return NULL;
        }

    return BitmapRleSinkNode::Create(*this, sinkShape, GetPhysicalCoordSys(), pRlePixelType);
    }
