//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeFactory.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeFactory
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeFactory.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32A8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32CMYK.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8Mask.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

HFC_IMPLEMENT_SINGLETON(HRPPixelTypeFactory)


//-----------------------------------------------------------------------------
// public
// Default Constructor.
//-----------------------------------------------------------------------------
HRPPixelTypeFactory::HRPPixelTypeFactory ()
    {
    Register(new HRPPixelTypeI1R8G8B8);
    Register(new HRPPixelTypeI1R8G8B8RLE);
    Register(new HRPPixelTypeI1R8G8B8A8);
    Register(new HRPPixelTypeI1R8G8B8A8RLE);
    //not working Register(new HRPPixelTypeI2R8G8B8);
    Register(new HRPPixelTypeI4R8G8B8);
    Register(new HRPPixelTypeI4R8G8B8A8);
    Register(new HRPPixelTypeI8R8G8B8);
    Register(new HRPPixelTypeI8R8G8B8A8);
    Register(new HRPPixelTypeI8VA8R8G8B8);    
    Register(new HRPPixelTypeI8Gray8);
    Register(new HRPPixelTypeV1Gray1);
    Register(new HRPPixelTypeV1GrayWhite1);
    Register(new HRPPixelTypeV8Gray8);
    Register(new HRPPixelTypeV8GrayWhite8);
    Register(new HRPPixelTypeV16Gray16);
    Register(new HRPPixelTypeV16Int16);
    Register(new HRPPixelTypeV16B5G5R5);
    Register(new HRPPixelTypeV16R5G6B5);
    Register(new HRPPixelTypeV16PRGray8A8);
    Register(new HRPPixelTypeV24B8G8R8);
    Register(new HRPPixelTypeV24PhotoYCC);
    Register(new HRPPixelTypeV24R8G8B8);
    Register(new HRPPixelTypeV32A8R8G8B8);
    Register(new HRPPixelTypeV32PR8PG8PB8A8);
    Register(new HRPPixelTypeV32PRPhotoYCCA8);
    Register(new HRPPixelTypeV32R8G8B8A8);
    Register(new HRPPixelTypeV32B8G8R8X8);
    Register(new HRPPixelTypeV32R8G8B8X8);
    Register(new HRPPixelTypeV32CMYK);
    Register(new HRPPixelTypeV48R16G16B16);
    Register(new HRPPixelTypeV64R16G16B16A16);
    Register(new HRPPixelTypeV64R16G16B16X16);
    Register(new HRPPixelTypeV32Float32);
    Register(new HRPPixelTypeV96R32G32B32);    
    Register(new HRPPixelTypeI8R8G8B8Mask);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeFactory::~HRPPixelTypeFactory()
    {
    }

//-----------------------------------------------------------------------------
// public
// CreatePixelType
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRPPixelTypeFactory::Create(  const HRPChannelOrg& pi_rChannelOrg,
                                                   unsigned short pi_IndexBits) const
    {
    // parse the list to find a similar channel org with the same index bits
    for(PixelTypeMap::const_iterator itr = m_PixelTypes.begin(); itr != m_PixelTypes.end(); ++itr)
        {
        HRPPixelType const& pixelType = *itr->second;
        if(pixelType.CountIndexBits() == pi_IndexBits && pixelType.GetChannelOrg() == pi_rChannelOrg)
            {
            // Found one! return a plain copy.
            return (HRPPixelType*)pixelType.Clone();            
            }
        }

    return NULL;
    }


//-----------------------------------------------------------------------------
// public
// CreatePixelType
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRPPixelTypeFactory::Create(const HRPPixelPalette& pi_rPalette) const
    {
    // parse the list to find a similar channel org with the same index bits
    for(PixelTypeMap::const_iterator itr = m_PixelTypes.begin(); itr != m_PixelTypes.end(); ++itr)
        {
        HRPPixelType const& pixelType = *itr->second;
        if(pixelType.CountIndexBits() != 0 &&
           pixelType.GetPalette().GetMaxEntries() == pi_rPalette.GetMaxEntries() &&
           pixelType.GetPalette().GetChannelOrg() == pi_rPalette.GetChannelOrg())
            {
            HFCPtr<HRPPixelType> pPixelType = (HRPPixelType*)pixelType.Clone(); 

            // copy the palette
            HRPPixelPalette& rPalette = pPixelType->LockPalette();
            rPalette = pi_rPalette;
            pPixelType->UnlockPalette();
            
            return pPixelType;
            }
        }

    return NULL;
    }

//-----------------------------------------------------------------------------
// public
// CreatePixelType
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRPPixelTypeFactory::Create(const HCLASS_ID& pi_rClassKey) const
    {
    HPRECONDITION(m_PixelTypes.find(pi_rClassKey) != m_PixelTypes.end());

    PixelTypeMap::const_iterator itr = m_PixelTypes.find(pi_rClassKey);
    if(itr == m_PixelTypes.end())
        return NULL;
    
    // create the appropriate pixel type
    HFCPtr<HRPPixelType> pPixelType = (HRPPixelType*)itr->second->Clone();

    // PARAM ->>> const void* pi_pPalette
    // Do not restore. It is unsafe! 
    // If you have a palette buffer you must be able to create a HRPPixelPalette 
    // which will properly defined channelOrg. Then create from this HRPPixelPalette.
#if 0
    // test if there is a palette, and if the palette may contain entry.
    if(pi_pPalette != 0 && pPixelType->CountIndexBits() != 0)
        {
        HRPPixelPalette& rPalette = pPixelType->LockPalette();

        // set the palette entries
        void* pRawData = rPalette.GetCompositeValue(0);
        memcpy(pRawData, pi_pPalette, rPalette.CountUsedEntries() * ((pPixelType->GetChannelOrg().CountPixelCompositeValueBits() + 7) / 8));

        pPixelType->UnlockPalette();
        }
#endif

    return pPixelType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRPPixelTypeFactory::Register(HFCPtr<HRPPixelType> pPixelType)
    {
    HPRECONDITION(m_PixelTypes.find(pPixelType->GetClassID()) == m_PixelTypes.end());
    HPRECONDITION("Update HRPPixelType::MAX_PIXEL_BITS" || pPixelType->CountPixelRawDataBits() <= HRPPixelType::MAX_PIXEL_BITS);

    m_PixelTypes.insert(PixelTypeMap::value_type(pPixelType->GetClassID(), pPixelType));
    }