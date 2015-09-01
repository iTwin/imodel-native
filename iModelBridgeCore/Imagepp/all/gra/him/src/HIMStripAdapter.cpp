//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMStripAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Methods for class HIMStripAdapter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMStripAdapter.h>

#include <Imagepp/all/h/HIMStripAdapterIterator.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HGSSurfaceDescriptor.h>


HPM_REGISTER_CLASS(HIMStripAdapter, HRAImageView)


//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HIMStripAdapter::HIMStripAdapter()
    :   HRAImageView(HFCPtr<HRARaster>())
    {
    m_StripWidth  = 0;
    m_StripHeigth = 0;
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------

HIMStripAdapter::HIMStripAdapter(const HFCPtr<HRARaster>&    pi_rpSource,
                                 const Byte*               pi_pRGBBackgroundColor,
                                 double                     pi_QualityFactor,
                                 size_t                      pi_MaxSizeInBytes)
    :   HRAImageView(pi_rpSource)
    {
    HPRECONDITION(pi_pRGBBackgroundColor != 0);
    HASSERT(pi_QualityFactor > 0.0 && pi_QualityFactor <= 1.0);

    // We have to do this because we're gonna be sticked
    // inside some messaging induced by our call to
    // GetRepresentativePalette().
    IncrementRef();

    m_MaxSizeInBytes = pi_MaxSizeInBytes;
    m_QualityFactor  = pi_QualityFactor;
    m_ApplyClipping  = false;
    m_StripWidth     = 0;
    m_StripHeigth    = 0;

    HFCPtr<HRPPixelType> pPixelType = pi_rpSource->GetPixelType();

    SetBackgroundColor(pPixelType, pi_rpSource, pi_pRGBBackgroundColor);

    // To match the previous IncrementRef()
    DecrementRef();
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------

HIMStripAdapter::HIMStripAdapter(const HFCPtr<HRARaster>&    pi_rpSource,
                                 const HFCPtr<HRPPixelType>& pi_rpStripPixelType,
                                 double                     pi_QualityFactor,
                                 size_t                      pi_MaxSizeInBytes)
    :   HRAImageView(pi_rpSource)
    {
    HASSERT(pi_QualityFactor > 0.0 && pi_QualityFactor <= 1.0);

    m_MaxSizeInBytes = pi_MaxSizeInBytes;
    m_QualityFactor  = pi_QualityFactor;
    m_ApplyClipping  = false;
    m_StripWidth      = 0;
    m_StripHeigth     = 0;

    m_pInputBitmapExample = HRABitmap::Create (1,
                                           1,
                                           0,
                                           pi_rpSource->GetCoordSys(),
                                           pi_rpStripPixelType,     // Clone?
                                           32);
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------

HIMStripAdapter::HIMStripAdapter(   const HFCPtr<HRARaster>&    pi_rpSource,
                                    const Byte*               pi_pRGBBackgroundColor,
                                    uint32_t                    pi_StripWidth,
                                    uint32_t                    pi_StripHeigth)
    :   HRAImageView(pi_rpSource)
    {
    HPRECONDITION(pi_pRGBBackgroundColor != 0);
    HPRECONDITION(pi_StripWidth > 0 && pi_StripHeigth > 0);

    // We have to do this because we're gonna be sticked
    // inside some messaging induced by our call to
    // GetRepresentativePalette().
    IncrementRef();

    m_QualityFactor = 1.0; // Always use FULL Quality.

    HFCPtr<HRPPixelType> pPixelType = pi_rpSource->GetPixelType();
    uint32_t BitmapBits = SetBackgroundColor(pPixelType, pi_rpSource, pi_pRGBBackgroundColor);

    m_StripWidth      = pi_StripWidth;
    m_StripHeigth     = pi_StripHeigth;
    m_MaxSizeInBytes  = ((BitmapBits * pi_StripWidth) + 7) / 8;
    m_MaxSizeInBytes *= pi_StripHeigth;
    m_ApplyClipping   = false;

    // To match the previous IncrementRef()
    DecrementRef();
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HIMStripAdapter::HIMStripAdapter(const HIMStripAdapter& pi_rObj)
    :   HRAImageView(pi_rObj)
    {
    m_pInputBitmapExample = pi_rObj.m_pInputBitmapExample;
    m_MaxSizeInBytes      = pi_rObj.m_MaxSizeInBytes;
    m_QualityFactor       = pi_rObj.m_QualityFactor;
    m_ApplyClipping       = pi_rObj.m_ApplyClipping;
    m_StripWidth          = pi_rObj.m_StripWidth;
    m_StripHeigth         = pi_rObj.m_StripHeigth;
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HIMStripAdapter::~HIMStripAdapter()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HIMStripAdapter::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HIMStripAdapter(*this);
    }
//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HPMPersistentObject* HIMStripAdapter::Clone () const
    {
    return new HIMStripAdapter(*this);
    }

//-----------------------------------------------------------------------------
// public
// CreateIterator
//-----------------------------------------------------------------------------
HRARasterIterator* HIMStripAdapter::CreateIterator (const HRAIteratorOptions& pi_rOptions) const
    {
    HRARasterIterator*  pIterator;

    // Return an iterator on the reference
    ((HIMStripAdapter*)this)->IncrementRef(); // Wrap temporary HFCPtr creation with an artificial addref
    pIterator = new HIMStripAdapterIterator(
        HFCPtr<HIMStripAdapter>((HIMStripAdapter*)this), pi_rOptions);
    ((HIMStripAdapter*)this)->DecrementRef();

    return pIterator;
    }

//-----------------------------------------------------------------------------
// public
// GetInputBitmapExample
//-----------------------------------------------------------------------------
HFCPtr<HRABitmap> HIMStripAdapter::GetInputBitmapExample() const
    {
    return m_pInputBitmapExample;
    }


//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

HFCPtr<HRPPixelType> HIMStripAdapter::GetPixelType() const
    {
    return m_pInputBitmapExample->GetPixelType();
    }



//-----------------------------------------------------------------------------
// private
// SetBackgroundColor
//-----------------------------------------------------------------------------

uint32_t HIMStripAdapter::SetBackgroundColor (const HFCPtr<HRPPixelType>& pio_pPixelType,
                                            const HFCPtr<HRARaster>&    pi_pSource,
                                            const Byte*               pi_pRGBBackgroundColor)
    {
    Byte BitmapBits;

    if((pio_pPixelType->GetClassID() == HRPPixelTypeI1R8G8B8::CLASS_ID) ||
       (pio_pPixelType->GetClassID() == HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
        {
        const HRPPixelPalette& rPalette = pio_pPixelType->GetPalette();

        if(memcmp(pi_pRGBBackgroundColor, rPalette.GetCompositeValue(0), 3) == 0 ||
           memcmp(pi_pRGBBackgroundColor, rPalette.GetCompositeValue(1), 3) == 0)
            BitmapBits = 1;
        else
            BitmapBits = 8;
        }
    else if(pio_pPixelType->GetClassID() == HRPPixelTypeI1R8G8B8A8::CLASS_ID ||
            pio_pPixelType->GetClassID() == HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID)
        {
        // test if we have a total transparent color or the same RGB value opaque as the background color
        const HRPPixelPalette& rPalette = pio_pPixelType->GetPalette();

        if( ((Byte*)rPalette.GetCompositeValue(0))[3] == 0x00 ||
            ((Byte*)rPalette.GetCompositeValue(1))[3] == 0x00 ||
            (((Byte*)rPalette.GetCompositeValue(0))[3] == 0xff &&
             memcmp(pi_pRGBBackgroundColor, rPalette.GetCompositeValue(0), 3) == 0) ||
            (((Byte*)rPalette.GetCompositeValue(1))[3] == 0xff &&
             memcmp(pi_pRGBBackgroundColor, rPalette.GetCompositeValue(1), 3) == 0))
            BitmapBits = 1;
        else if(((Byte*)rPalette.GetCompositeValue(0))[3] == 0xff ||
                ((Byte*)rPalette.GetCompositeValue(1))[3] == 0xff)
            BitmapBits = 8;
        else
            BitmapBits = 24;
        }
    else if(pio_pPixelType->GetClassID() == HRPPixelTypeV1Gray1::CLASS_ID ||
            pio_pPixelType->GetClassID() == HRPPixelTypeV1GrayWhite1::CLASS_ID)
        {
        Byte BlackColor[3];
        memset(BlackColor, 0x00, 3);

        Byte WhiteColor[3];
        memset(WhiteColor, 0xff, 3);

        if(memcmp(pi_pRGBBackgroundColor, BlackColor, 3) == 0 ||
           memcmp(pi_pRGBBackgroundColor, WhiteColor, 3) == 1 )
            BitmapBits = 1;
        else
            BitmapBits = 8;
        }
    else if(pio_pPixelType->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID ||
            pio_pPixelType->GetClassID() == HRPPixelTypeV8GrayWhite8::CLASS_ID)
        {
        // test if the color is a gray, if yes, go with 8 bits
        if(pi_pRGBBackgroundColor[0] == pi_pRGBBackgroundColor[1] &&
           pi_pRGBBackgroundColor[1] == pi_pRGBBackgroundColor[2])
            BitmapBits = 8;
        else
            BitmapBits = 24;
        }
    else if(pio_pPixelType->GetClassID() == HRPPixelTypeI8R8G8B8::CLASS_ID)
        {
        // test if the background color is part of the source palette
        const HRPPixelPalette& rPalette = pio_pPixelType->GetPalette();

        uint32_t Index;
        for(Index = 0; Index < 256; Index++)
            {
            Byte* pValue = (Byte*)rPalette.GetCompositeValue(Index);

            if( pValue[0] == pi_pRGBBackgroundColor[0] &&
                pValue[1] == pi_pRGBBackgroundColor[1] &&
                pValue[2] == pi_pRGBBackgroundColor[2])
                break;
            }

        if(Index == 256)
            BitmapBits = 24;
        else
            BitmapBits = 8;
        }
    else
        {
        if(pio_pPixelType->CountPixelRawDataBits() < 8)
            BitmapBits = 8;
        else
            BitmapBits = 24;
        }

    // compute the bitmap example
    switch(BitmapBits)
        {
        case 1:
            {
            m_pInputBitmapExample = HRABitmap::Create (1, 1, 0, pi_pSource->GetCoordSys(), new HRPPixelTypeI1R8G8B8(), 32);
            HFCPtr<HRPPixelType> pPixelType(m_pInputBitmapExample->GetPixelType());
            HRPPixelPalette& rPalette = pPixelType->LockPalette();
            rPalette.SetCompositeValue(0, pi_pRGBBackgroundColor);
            rPalette.LockEntry(0);
            pPixelType->UnlockPalette();
            Byte RawData = 0;
            pPixelType->SetDefaultRawData(&RawData);
            HRARepPalParms RepPalParms(pPixelType);
            pi_pSource->GetRepresentativePalette(&RepPalParms);
            }
        break;

        case 8:
            {
            m_pInputBitmapExample = HRABitmap::Create (1, 1, 0, pi_pSource->GetCoordSys(), new HRPPixelTypeI8R8G8B8(), 32);
            HFCPtr<HRPPixelType> pPixelType(m_pInputBitmapExample->GetPixelType());
            HRPPixelPalette& rPalette = pPixelType->LockPalette();
            rPalette.SetCompositeValue(0, pi_pRGBBackgroundColor);
            rPalette.LockEntry(0);
            pPixelType->UnlockPalette();
            Byte RawData = 0;
            pPixelType->SetDefaultRawData(&RawData);
            HRARepPalParms RepPalParms(pPixelType);
            pi_pSource->GetRepresentativePalette(&RepPalParms);
            }
        break;

        case 24:
        default:
            {
            m_pInputBitmapExample = HRABitmap::Create (1, 1, 0, pi_pSource->GetCoordSys(), new HRPPixelTypeV24B8G8R8(), 32);
            HFCPtr<HRPPixelType> pPixelType(m_pInputBitmapExample->GetPixelType());
            HFCPtr<HRPPixelType> pPixelTypeRGB(new HRPPixelTypeV24B8G8R8());
            HFCPtr<HRPPixelConverter> pConverter = pPixelTypeRGB->GetConverterTo(pPixelType);
            Byte BackgroundRawData[3];
            pConverter->Convert(pi_pRGBBackgroundColor, BackgroundRawData);
            pPixelType->SetDefaultRawData(BackgroundRawData);
            }
        break;
        }

    return BitmapBits;
    }


//-----------------------------------------------------------------------------
// public
// IsStoredRaster
//-----------------------------------------------------------------------------
bool HIMStripAdapter::IsStoredRaster () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HIMStripAdapter::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    HRADrawOptions Options(pi_Options);

    HFCPtr<HVEShape> pRegionToDraw;
    if (Options.GetShape() != 0)
        pRegionToDraw = new HVEShape(*Options.GetShape());
    else
        pRegionToDraw = new HVEShape(*GetEffectiveShape());
    pRegionToDraw->ChangeCoordSys(GetCoordSys());

    // set the effective coordsys
    if (Options.GetReplacingCoordSys() == 0)
        Options.SetReplacingCoordSys(GetCoordSys());

    // test if there is a clip region in the destination
    HFCPtr<HGSRegion> pClipRegion(pio_destSurface.GetRegion());
    if (pClipRegion != 0)
        {
        // if yes, intersect it with the destination
        HFCPtr<HVEShape> pSurfaceShape(pClipRegion->GetShape());

        pSurfaceShape->ChangeCoordSys(Options.GetReplacingCoordSys());
        pSurfaceShape->SetCoordSys(GetCoordSys());

        pRegionToDraw->Intersect(*pSurfaceShape);
        }
    else
        {
        // Create a rectangular clip region to stay
        // inside the destination surface.
        HVEShape DestSurfaceShape(0.0, 0.0, pio_destSurface.GetSurfaceDescriptor()->GetWidth(), pio_destSurface.GetSurfaceDescriptor()->GetHeight(), pio_destSurface.GetSurfaceCoordSys());
        DestSurfaceShape.ChangeCoordSys(Options.GetReplacingCoordSys());
        DestSurfaceShape.SetCoordSys(GetCoordSys());

        pRegionToDraw->Intersect(DestSurfaceShape);
        }


    HAutoPtr<HRARasterIterator> pIterator(CreateIterator(HRAIteratorOptions(pRegionToDraw, pio_destSurface.GetSurfaceCoordSys())));
    HASSERT(pIterator != 0);

    if (pIterator != 0)
        {
        HFCPtr<HRARaster> pCurrentStrip = (*pIterator)();
        while (pCurrentStrip != 0)
            {
            HRADrawOptions Options2(pi_Options);

            HFCPtr<HVEShape> pClip(new HVEShape(*pRegionToDraw));
            pClip->Intersect(*pCurrentStrip->GetEffectiveShape());
            Options2.SetShape(pClip);

            pCurrentStrip->Draw(pio_destSurface, Options2);

            pCurrentStrip = pIterator->Next();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HIMStripAdapter::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    BeAssert(false);
    return IMAGEPP_STATUS_NoImplementation;
    // It doesn't make sense to adapt the source that way to produce pixels. The only good reason to use a strip adapter is to make iteration on the source.
    // The current usage of the StripAdapter is to compute a representative palette. We should provide a way to compute a representative palette not requiring a StripAdapter.
    // return GetSource()->BuildCopyToContext(imageNode, options);
    }
