//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelType.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPComplexConverter.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPQuantizedPalette.h>
#include <ImagePP/all/h/HRPMessages.h>

HPM_REGISTER_ABSTRACT_CLASS(HRPPixelType, HPMPersistentObject)


HFCExclusiveKey HRPPixelType::s_ConverterAccess;

//-----------------------------------------------------------------------------
// Default Constructor.
//-----------------------------------------------------------------------------
HRPPixelType::HRPPixelType()
    : m_PixelPalette(),
      m_ChannelOrg()
    {
    m_IndexBits = 0;

    uint32_t NumberOfBytes = (CountPixelRawDataBits() + 7) / 8;
    m_pDefaultRawData = new Byte[NumberOfBytes];
    memset(m_pDefaultRawData, 0, NumberOfBytes);
    }

//-----------------------------------------------------------------------------
// Constructor from channel organisation and number of index bits.  Will
// create a pixel palette internally
//-----------------------------------------------------------------------------
HRPPixelType::HRPPixelType(const HRPChannelOrg& pi_rChannelOrg,
                           unsigned short pi_IndexBits)
    : m_PixelPalette((pi_IndexBits ? 2 << (pi_IndexBits-1) : 0),pi_rChannelOrg),
      m_ChannelOrg(pi_rChannelOrg)
    {
    m_IndexBits = pi_IndexBits;

    uint32_t NumberOfBytes = (CountPixelRawDataBits() + 7) / 8;
    m_pDefaultRawData = new Byte[NumberOfBytes];
    memset(m_pDefaultRawData, 0, NumberOfBytes);
    }

//-----------------------------------------------------------------------------
// Constructor from channel organisation and number of index bits.  Will
// create a pixel palette internally, using the distinct palette
// channelorg.
//
// Condition (unverified): The channels in the palette org must be in the
// pixeltype original orgnanization.
//-----------------------------------------------------------------------------
HRPPixelType::HRPPixelType(const HRPChannelOrg& pi_rChannelOrg,
                           unsigned short pi_IndexBits,
                           const HRPChannelOrg& pi_rPaletteChannelOrg)
    : m_PixelPalette((pi_IndexBits ? 2 << (pi_IndexBits-1) : 0),pi_rPaletteChannelOrg),
      m_ChannelOrg(pi_rChannelOrg)
    {
    uint32_t ValueBits = MAX(pi_rChannelOrg.CountPixelCompositeValueBits() - pi_rPaletteChannelOrg.CountPixelCompositeValueBits(), 0);
    m_IndexBits = (unsigned short)(pi_IndexBits + ValueBits);

    uint32_t NumberOfBytes = (m_IndexBits + 7) / 8;
    m_pDefaultRawData = new Byte[NumberOfBytes];
    memset(m_pDefaultRawData, 0, NumberOfBytes);
    }

//-----------------------------------------------------------------------------
// Constructor from a palette that will be copied internally
//-----------------------------------------------------------------------------
HRPPixelType::HRPPixelType(const HRPPixelPalette& pi_rPixelPalette)
    : m_PixelPalette(pi_rPixelPalette),
      m_ChannelOrg(pi_rPixelPalette.GetChannelOrg())
    {
    m_IndexBits = (unsigned short)(log((double)m_PixelPalette.GetMaxEntries()) / log(2.0));

    uint32_t NumberOfBytes = (CountPixelRawDataBits() + 7) / 8;
    m_pDefaultRawData = new Byte[NumberOfBytes];
    memset(m_pDefaultRawData, 0, NumberOfBytes);
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRPPixelType::HRPPixelType(const HRPPixelType& pi_rObj)
    : m_PixelPalette(pi_rObj.m_PixelPalette),
      m_ChannelOrg(pi_rObj.m_ChannelOrg)
    {
    m_IndexBits = pi_rObj.m_IndexBits;

    uint32_t NumberOfBytes = (CountPixelRawDataBits() + 7) / 8;
    m_pDefaultRawData = new Byte[NumberOfBytes];
    memcpy(m_pDefaultRawData, pi_rObj.m_pDefaultRawData, NumberOfBytes);
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HRPPixelType::~HRPPixelType()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// Deletes everything owned by the object
//-----------------------------------------------------------------------------
void HRPPixelType::DeepDelete()
    {
    }

//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another pixel type, doing a deep copy.
//-----------------------------------------------------------------------------
/*
***DISABLED***
Not used and not fully implemented in the HRPPixelType object hierarchy.
HRPPixelType& HRPPixelType::operator=(const HRPPixelType& pi_rObj)
{
    if (this != &pi_rObj) {
        DeepDelete();
        DeepCopy(pi_rObj);
    }
    return *this;
}
***DISABLED***
*/

//-----------------------------------------------------------------------------
// CreateQuantizedPalette.
//-----------------------------------------------------------------------------
HRPQuantizedPalette* HRPPixelType::CreateQuantizedPalette(uint32_t pi_MaxEntries) const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// Copies everything owned by the object
//-----------------------------------------------------------------------------
void HRPPixelType::DeepCopy(const HRPPixelType& pi_rObj)
    {
    m_PixelPalette = pi_rObj.m_PixelPalette;
    m_IndexBits = pi_rObj.m_IndexBits;
    m_ChannelOrg = pi_rObj.m_ChannelOrg;

    uint32_t NumberOfBytes = (pi_rObj.CountPixelRawDataBits() + 7) / 8;
    m_pDefaultRawData = new Byte[NumberOfBytes];
    memcpy(m_pDefaultRawData, pi_rObj.m_pDefaultRawData, NumberOfBytes);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRPPixelType::HasSamePixelInterpretation(HRPPixelType const& obj) const
    {
    if (GetClassID() != obj.GetClassID())
        return false;

    // Indexed pixeltype must have the same palette.
    if (CountIndexBits() > 0)
        return m_PixelPalette == obj.m_PixelPalette;

    return true;
    }

//-----------------------------------------------------------------------------
// Equal To operator
//-----------------------------------------------------------------------------
bool HRPPixelType::operator==(const HRPPixelType& pi_rObj) const
    {
    bool Result = false;

    if (GetClassID() == pi_rObj.GetClassID())
        {
        if (CountPixelRawDataBits() == pi_rObj.CountPixelRawDataBits())
            {
            uint32_t NumberOfBytes = (CountPixelRawDataBits() + 7) / 8;
            Result = memcmp(m_pDefaultRawData, pi_rObj.m_pDefaultRawData, NumberOfBytes) == 0;
            }

        // Must have the same structure and the same palette
        Result = (Result &&
                  m_PixelPalette == pi_rObj.m_PixelPalette);
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Not Equal To operator
//-----------------------------------------------------------------------------
bool HRPPixelType::operator!=(const HRPPixelType& pi_rObj) const
    {
    return(!operator==(pi_rObj));
    }

//-----------------------------------------------------------------------------
// FindNearestENtryInPalette
//-----------------------------------------------------------------------------
uint32_t HRPPixelType::FindNearestEntryInPalette(const void* pi_pValue) const
    {
    // default case, not supported
    return 0;
    }

//-----------------------------------------------------------------------------
// Get1BitInterface
//-----------------------------------------------------------------------------
HRPPixelType1BitInterface* HRPPixelType::Get1BitInterface()
    {
    return NULL;
    }

//-----------------------------------------------------------------------------
// GetConverterFrom
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelConverter> HRPPixelType::GetConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HPRECONDITION(pi_pPixelTypeFrom != 0);

    const HRPPixelConverter* pConverter;
    HFCPtr<HRPPixelConverter> pNewConverter;

    // we scan the internal list of converters associated to the current pixel
    // type and the other one to find an appropriate converter
    if(pConverter = HasConverterFrom(pi_pPixelTypeFrom))
        pNewConverter = CreateConverter(pConverter,pi_pPixelTypeFrom);
    else if(pConverter = pi_pPixelTypeFrom->HasConverterTo((const HRPPixelType*)this))
        pNewConverter = CreateConverter(pConverter,pi_pPixelTypeFrom);
    else
        pNewConverter = new HRPComplexConverter(pi_pPixelTypeFrom, this);

    return pNewConverter;
    }

//-----------------------------------------------------------------------------
// GetConverterTo
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelConverter> HRPPixelType::GetConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HPRECONDITION(pi_pPixelTypeTo != 0);

    const HRPPixelConverter* pConverter;
    HFCPtr<HRPPixelConverter> pNewConverter;

    // we scan the internal list of converters associated to the current pixel
    // type and the other one to find an appropriate converter
    if(pConverter = HasConverterTo(pi_pPixelTypeTo))
        pNewConverter = pi_pPixelTypeTo->CreateConverter(pConverter,this);
    else if(pConverter = pi_pPixelTypeTo->HasConverterFrom((const HRPPixelType*)this))
        pNewConverter = pi_pPixelTypeTo->CreateConverter(pConverter,this);
    else
        pNewConverter = new HRPComplexConverter(this, pi_pPixelTypeTo);

    return pNewConverter;
    }

//-----------------------------------------------------------------------------
// public
// LockPalette
//-----------------------------------------------------------------------------
HRPPixelPalette& HRPPixelType::LockPalette()
    {
    m_PaletteAccess.ClaimKey();

    return m_PixelPalette;
    }

//-----------------------------------------------------------------------------
// public
// UnlockPalette
//-----------------------------------------------------------------------------
void HRPPixelType::UnlockPalette()
    {
    m_PaletteAccess.ReleaseKey();

    // Object modified
    SetModificationState();

    // notify rasters that a palette change possibly occured
    Propagate(HRPPaletteChangedMsg());
    }

//-----------------------------------------------------------------------------
// SetDefaultRawData
//-----------------------------------------------------------------------------
void HRPPixelType::SetDefaultRawData(const void* pi_pValue)
    {
    HPRECONDITION(pi_pValue != 0);

    uint32_t NumberOfBytes = (CountPixelRawDataBits() + 7) / 8;
    memcpy(m_pDefaultRawData, pi_pValue, NumberOfBytes);

    // initialize the extra bits to 0
    Byte NumberOfBits = (Byte)(CountPixelRawDataBits() % 8);
    if (NumberOfBits > 0)
        {
        Byte Mask = 0xFF & (0xFF << (8 - NumberOfBits));
        m_pDefaultRawData[NumberOfBytes - 1] = m_pDefaultRawData[NumberOfBytes - 1] & Mask;
        }
    }

//-----------------------------------------------------------------------------
// CreateConverter
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelConverter> HRPPixelType::CreateConverter(const HRPPixelConverter*  pi_pConverter,
                                                        const HRPPixelType* pi_pPixelType) const
    {
    HRPPixelConverter* pNewConverter = pi_pConverter->AllocateCopy();

    HASSERT(pNewConverter != 0);

    pNewConverter->SetSourcePixelType(pi_pPixelType);
    pNewConverter->SetDestinationPixelType(this);

    return(pNewConverter);
    }


//-----------------------------------------------------------------------------
// SetDefaultCompositeValue
//-----------------------------------------------------------------------------
void HRPPixelType::SetDefaultCompositeValue(const void* pi_pValue)
    {
    HPRECONDITION(pi_pValue != 0);

    if(m_IndexBits != 0)
        {
        uint32_t EntryIndex = FindNearestEntryInPalette(pi_pValue);
        uint32_t PixelRawDataBits = CountPixelRawDataBits();
        if ((PixelRawDataBits % 8) != 0)
            {
            HPRECONDITION(PixelRawDataBits < 8);

            // shift the index
            Byte Index = (Byte)(EntryIndex << (8 - PixelRawDataBits));
            SetDefaultRawData(&Index);
            }
        else
            SetDefaultRawData(&EntryIndex);
        }
    else
        {
        SetDefaultRawData(pi_pValue);
        }
    }

