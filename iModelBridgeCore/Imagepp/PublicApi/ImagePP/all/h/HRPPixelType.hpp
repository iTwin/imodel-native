//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelType.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HRPPixelType
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns number of index bits (0 if no index)
//-----------------------------------------------------------------------------
inline unsigned short HRPPixelType::CountIndexBits() const
    {
    return(m_IndexBits);
    }

//-----------------------------------------------------------------------------
// Returns to total size required to store all channels of this pixel.
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelType::CountPixelRawDataBits() const
    {
    uint32_t IndexBits;

    if(m_IndexBits)
        {
        IndexBits = m_IndexBits;
        }
    else
        {
        const HRPChannelOrg* pChannelOrg = (const HRPChannelOrg*) &(m_PixelPalette.GetChannelOrg());
        IndexBits = pChannelOrg->CountPixelCompositeValueBits();
        }

    return IndexBits;
    }

//-----------------------------------------------------------------------------
// Returns the channel organization of this pixel type
//-----------------------------------------------------------------------------
inline const HRPChannelOrg& HRPPixelType::GetChannelOrg() const
    {
    return(m_ChannelOrg);
    }

//-----------------------------------------------------------------------------
// Returns the palette of this pixel type
//-----------------------------------------------------------------------------
inline const HRPPixelPalette& HRPPixelType::GetPalette() const
    {
    return(m_PixelPalette);
    }

//-----------------------------------------------------------------------------
// GetDefaultRawData
//-----------------------------------------------------------------------------
inline const void* HRPPixelType::GetDefaultRawData() const
    {
    return m_pDefaultRawData.get();
    }
END_IMAGEPP_NAMESPACE
