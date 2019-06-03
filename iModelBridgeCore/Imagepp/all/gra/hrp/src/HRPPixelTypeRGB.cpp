//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeRGB
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRPPixelTypeRGB.h>
#include <ImagePP/all/h/HRPChannelOrgRGB.h>
#include <ImagePP/all/h/HRPPixelConverter.h>
#include <ImagePP/all/h/HRPQuantizedPaletteR8G8B8.h>
#include <ImagePP/all/h/HRPPaletteOctreeR8G8B8.h>
#include <ImagePP/all/h/HRPQuantizedPaletteR8G8B8A8.h>
#include <ImagePP/all/h/HRPPaletteOctreeR8G8B8A8.h>



HPM_REGISTER_ABSTRACT_CLASS(HRPPixelTypeRGB, HRPPixelType)


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeRGB::HRPPixelTypeRGB()
    : HRPPixelType(HRPChannelOrgRGB(0,
                                    1,
                                    2,
                                    3,
                                    HRPChannelType::UNUSED,
                                    HRPChannelType::VOID_CH,
                                    0),
                   0)
    {
    }


//-----------------------------------------------------------------------------
// Constructor for straight RGB
//-----------------------------------------------------------------------------
HRPPixelTypeRGB::HRPPixelTypeRGB(uint16_t pi_BitsRed,
                                 uint16_t pi_BitsGreen,
                                 uint16_t pi_BitsBlue,
                                 uint16_t pi_BitsExtra,
                                 uint16_t pi_IndexBits,
                                 bool   pi_IsBitsExtraAlpha)
    : HRPPixelType(HRPChannelOrgRGB(pi_BitsRed,
                                    pi_BitsGreen,
                                    pi_BitsBlue,
                                    pi_BitsExtra,
                                    HRPChannelType::UNUSED,
                                    HRPChannelType::VOID_CH,
                                    0,
                                    pi_IsBitsExtraAlpha),
                   pi_IndexBits)
    {
    }

//-----------------------------------------------------------------------------
// Constructor for straight RGB, with a partial palette
//-----------------------------------------------------------------------------
HRPPixelTypeRGB::HRPPixelTypeRGB(uint16_t pi_BitsRed,
                                 uint16_t pi_BitsGreen,
                                 uint16_t pi_BitsBlue,
                                 uint16_t pi_BitsAlpha,
                                 uint16_t pi_IndexBits,
                                 uint16_t pi_IndexBitsRed,
                                 uint16_t pi_IndexBitsGreen,
                                 uint16_t pi_IndexBitsBlue,
                                 uint16_t pi_IndexBitsAlpha)
    : HRPPixelType(HRPChannelOrgRGB(pi_BitsRed,
                                    pi_BitsGreen,
                                    pi_BitsBlue,
                                    pi_BitsAlpha,
                                    HRPChannelType::UNUSED,
                                    HRPChannelType::VOID_CH,
                                    0),
                   pi_IndexBits,
                   HRPChannelOrgRGB(pi_IndexBitsRed,
                                    pi_IndexBitsGreen,
                                    pi_IndexBitsBlue,
                                    pi_IndexBitsAlpha,
                                    HRPChannelType::UNUSED,
                                    HRPChannelType::VOID_CH,
                                    0))
    {
    // Channels in palette must be in original channel organization
    HASSERT(pi_IndexBits > 0);
    HASSERT(pi_IndexBitsRed   <= pi_BitsRed);
    HASSERT(pi_IndexBitsGreen <= pi_BitsGreen);
    HASSERT(pi_IndexBitsBlue  <= pi_BitsBlue);
    HASSERT(pi_IndexBitsAlpha <= pi_BitsAlpha);
    }

//-----------------------------------------------------------------------------
// Constructor for RGB channels plus a 4th channel.
// Useful for creating RGBA or RGBU pixel types.
//-----------------------------------------------------------------------------
HRPPixelTypeRGB::HRPPixelTypeRGB(uint16_t             pi_BitsRed,
                                 uint16_t             pi_BitsGreen,
                                 uint16_t             pi_BitsBlue,
                                 uint16_t             pi_BitsAlpha,
                                 HRPChannelType::ChannelRole pi_RoleChannel5,
                                 HRPChannelType::DataType    pi_DataTypeChannel5,
                                 uint16_t             pi_BitsChannel5,
                                 uint16_t             pi_IndexBits)
    : HRPPixelType(HRPChannelOrgRGB(pi_BitsRed,
                                    pi_BitsGreen,
                                    pi_BitsBlue,
                                    pi_BitsAlpha,
                                    pi_RoleChannel5,
                                    pi_DataTypeChannel5,
                                    pi_BitsChannel5),
                   pi_IndexBits)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeRGB::HRPPixelTypeRGB(const HRPPixelTypeRGB& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeRGB::~HRPPixelTypeRGB()
    {
    }

//-----------------------------------------------------------------------------
// CreateQuantizedPalette
//-----------------------------------------------------------------------------
HRPQuantizedPalette* HRPPixelTypeRGB::CreateQuantizedPalette(uint32_t pi_MaxEntries) const
    {
    HRPQuantizedPalette* pQuantizedPalette = 0;

    // return an appropriate quantized palette
    if(CountIndexBits() == 0)
        {
        // there is no index in the pixel type, then no quantized palette object associated
        pQuantizedPalette = 0;
        }
    else
        {
        if (CountValueBits() == 0 &&
            GetPalette().GetChannelOrg().CountPixelCompositeValueBits() == 32 &&
            GetPalette().GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            pQuantizedPalette = new HRPQuantizedPaletteR8G8B8A8((uint16_t)pi_MaxEntries, 8);
            }
        else
            {
            pQuantizedPalette = new HRPQuantizedPaletteR8G8B8((uint16_t)pi_MaxEntries, 8);
            }
        }

    return pQuantizedPalette;
    }

//-----------------------------------------------------------------------------
// FindNearestEntryInPalette
//-----------------------------------------------------------------------------
uint32_t HRPPixelTypeRGB::FindNearestEntryInPalette(const void* pi_pValue) const
    {
    HPRECONDITION(CountIndexBits() != 0);

    if (GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        {
        HRPPaletteOctreeR8G8B8A8 Octree;

        const HRPPixelPalette& rPalette = GetPalette();

        for(uint32_t EntryIndex = 0; EntryIndex < rPalette.CountUsedEntries(); EntryIndex++)
            Octree.AddCompositeValue(rPalette.GetCompositeValue(EntryIndex), (Byte)EntryIndex);

        return(Octree.GetIndex(((Byte*)pi_pValue)[0], ((Byte*)pi_pValue)[1], ((Byte*)pi_pValue)[2], ((Byte*)pi_pValue)[3]));
        }
    else
        {
        HRPPaletteOctreeR8G8B8 Octree;

        const HRPPixelPalette& rPalette = GetPalette();

        for(uint32_t EntryIndex = 0; EntryIndex < rPalette.CountUsedEntries(); EntryIndex++)
            Octree.AddCompositeValue(rPalette.GetCompositeValue(EntryIndex), (Byte)EntryIndex);

        return(Octree.GetIndex(((Byte*)pi_pValue)[0], ((Byte*)pi_pValue)[1], ((Byte*)pi_pValue)[2]));
        }
    }

