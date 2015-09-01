//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMTranslucentImageCreator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMTranslucentImageCreator.h>

#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRAPixelTypeReplacer.h>
#include <Imagepp/all/h/HRPFunctionFilters.h>
#include <Imagepp/all/h/HIMFilteredImage.h>


//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HIMTranslucentImageCreator::HIMTranslucentImageCreator(const HFCPtr<HRARaster>& pi_pSource,
                                                       Byte pi_DefaultAlpha)
    :   m_Ranges(),
        m_pSourceRaster(pi_pSource)
    {
    InitObject(pi_DefaultAlpha);
    }


//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HIMTranslucentImageCreator::~HIMTranslucentImageCreator()
    {
    }


//-----------------------------------------------------------------------------
// private
// InitObject
//-----------------------------------------------------------------------------
void HIMTranslucentImageCreator::InitObject(Byte pi_AlphaValue)
    {
    m_DefaultAlpha = pi_AlphaValue;
    m_TranslucencyMethod = REPLACE;
    }

//-----------------------------------------------------------------------------
// public
// GetDefaultAlpha
//-----------------------------------------------------------------------------
Byte HIMTranslucentImageCreator::GetDefaultAlpha() const
    {
    return m_DefaultAlpha;
    }

//-----------------------------------------------------------------------------
// public
// SetDefaultAlpha
//-----------------------------------------------------------------------------
void HIMTranslucentImageCreator::SetDefaultAlpha(Byte pi_DefaultValue)
    {
    m_DefaultAlpha = pi_DefaultValue;
    }

//-----------------------------------------------------------------------------
// public
// AddAlphaRange
//-----------------------------------------------------------------------------
void HIMTranslucentImageCreator::AddAlphaRange(const HRPAlphaRange& pi_rRange)
    {
    m_Ranges.push_back(pi_rRange);
    }

//-----------------------------------------------------------------------------
// public
// GetAlphaRanges
//-----------------------------------------------------------------------------
const ListHRPAlphaRange& HIMTranslucentImageCreator::GetAlphaRanges() const
    {
    return m_Ranges;
    }

//-----------------------------------------------------------------------------
// public
// RemoveAllRanges
//-----------------------------------------------------------------------------
void HIMTranslucentImageCreator::RemoveAllRanges()
    {
    m_Ranges.clear();
    }


//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithChannel
//-----------------------------------------------------------------------------
void HIMTranslucentImageCreator::SetTranslucencyMethod(TranslucencyMethod pi_Method)
    {
    m_TranslucencyMethod = pi_Method;
    }


//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithChannel
//-----------------------------------------------------------------------------
HIMTranslucentImageCreator::TranslucencyMethod HIMTranslucentImageCreator::GetTranslucencyMethod() const
    {
    return m_TranslucencyMethod;
    }


/** ---------------------------------------------------------------------------
    Create a translucent raster using the current settings. If possible, a
    pixeltype replacer will be created. Otherwise, a filtered image will
    be returned.

    If the pi_pAlphaValues parameter is used, it must contain as much alpha
    entries (bytes) as the source raster's palette contains entries. Also,
    the source raster must have a palette with V24RGB or V32RGBA organization.
    ---------------------------------------------------------------------------
*/
HRARaster* HIMTranslucentImageCreator::CreateTranslucentRaster(Byte const* pi_pAlphaValues) const
    {
    HRARaster* pTranslucentImage = 0;

    HFCPtr<HRPPixelType> pSrcPixelType(m_pSourceRaster->GetPixelType());

    if(pSrcPixelType->CountIndexBits() != 0 &&
       pSrcPixelType->CountValueBits() == 0 &&
       (pSrcPixelType->GetChannelOrg() == HRPPixelTypeV24R8G8B8().GetChannelOrg() ||
        pSrcPixelType->GetChannelOrg() == HRPPixelTypeV32R8G8B8A8().GetChannelOrg()))
        {
        // We can create a pixeltype replacer

        uint32_t MaxEntries = pSrcPixelType->GetPalette().GetMaxEntries();

        // create a palette
        HRPPixelPalette Palette(MaxEntries, HRPPixelTypeV32R8G8B8A8().GetChannelOrg());

        // add the entries of the palette
        const HRPPixelPalette& rSrcPalette = pSrcPixelType->GetPalette();

        if (pi_pAlphaValues != 0)
            {
            // The alpha values for the palette were specified

            if (pSrcPixelType->GetChannelOrg() == HRPPixelTypeV32R8G8B8A8().GetChannelOrg())
                {
                uint32_t Value;
                Byte* pColor = (Byte*)(&Value);

                // There are already alpha values in the palette.
                // We don't remove or diminish the values, only raise them.
                for(uint32_t EntryIndex = 0; EntryIndex < MaxEntries; EntryIndex++)
                    {
                    Value = *((uint32_t*)(rSrcPalette.GetCompositeValue(EntryIndex)));
                    if (pColor[3] > pi_pAlphaValues[EntryIndex])
                        pColor[3] = pi_pAlphaValues[EntryIndex];
                    Palette.AddEntry(&Value);
                    }
                }
            else
                {
                Byte* pSrcValue;
                Byte  pNewValue[4];
                // No original alpha, set the specified ones.
                for(uint32_t EntryIndex = 0; EntryIndex < MaxEntries; EntryIndex++)
                    {
                    pSrcValue = (Byte*)(rSrcPalette.GetCompositeValue(EntryIndex));
                    pNewValue[0] = pSrcValue[0];
                    pNewValue[1] = pSrcValue[1];
                    pNewValue[2] = pSrcValue[2];
                    pNewValue[3] = pi_pAlphaValues[EntryIndex];
                    Palette.AddEntry(pNewValue);
                    }
                }
            }
        else
            {
            // No specified alpha values. Use the alpha cubes.

            for(uint32_t EntryIndex = 0; EntryIndex < MaxEntries; EntryIndex++)
                {
                Byte* pSrcValue;
                Byte  pNewValue[4];

                pSrcValue = (Byte*)(rSrcPalette.GetCompositeValue(EntryIndex));

                pNewValue[0] = pSrcValue[0];
                pNewValue[1] = pSrcValue[1];
                pNewValue[2] = pSrcValue[2];
                pNewValue[3] = m_DefaultAlpha;

                // check if the color is defined in the range
                for(uint32_t RangeIndex = 0; RangeIndex < m_Ranges.size(); RangeIndex++)
                    {
                    if(m_Ranges[RangeIndex].IsIn(pSrcValue[0], pSrcValue[1], pSrcValue[2]))
                        {
                        // if yes, change the default alpha value
                        pNewValue[3] = m_Ranges[RangeIndex].GetAlphaValue();
                        break;
                        }
                    }

                Palette.AddEntry(pNewValue);
                }
            }

        HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(Palette);
        if (pPixelType != 0)
            pTranslucentImage = new HRAPixelTypeReplacer(m_pSourceRaster, pPixelType);
        }

    if (pTranslucentImage == 0 && pi_pAlphaValues == 0)
        {
        // case where there is no palette (by value or a complex image)
        HFCPtr<HRPFilter> pFilter;

        // verify the translucency method of the owning image to generate the correct filter
        if (m_TranslucencyMethod == REPLACE)
            pFilter = new HRPAlphaReplacer(m_DefaultAlpha, m_Ranges);
        else
            pFilter = new HRPAlphaComposer(m_DefaultAlpha, m_Ranges);

        // create a filtered image
        pTranslucentImage = new HIMFilteredImage(m_pSourceRaster, pFilter);
        }

    return pTranslucentImage;
    }
