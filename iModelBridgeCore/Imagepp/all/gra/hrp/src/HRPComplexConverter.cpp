//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPComplexConverter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPComplexConverter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPComplexConverter.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRPComplexConverter::HRPComplexConverter(const HRPPixelType* pi_pSourcePixelType,
                                         const HRPPixelType* pi_pDestPixelType)
    :  HRPPixelConverter(pi_pSourcePixelType, pi_pDestPixelType),
       m_PixelTypeRGB(),
       m_PixelTypeRGBA(),
       m_LastConversionPixelCount(0)
    {
    HPRECONDITION(pi_pSourcePixelType != 0);
    HPRECONDITION(pi_pDestPixelType != 0);

    Update();
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPComplexConverter::HRPComplexConverter(const HRPComplexConverter& pi_rObj)
    :  HRPPixelConverter(pi_rObj),
       m_PixelTypeRGB(),
       m_PixelTypeRGBA(),
       m_LastConversionPixelCount(0)
    {
    Update();
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRPComplexConverter::~HRPComplexConverter()
    {
    }


//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPComplexConverter::Convert(const void* pi_pSourceRawData,
                                  void*       pio_pDestRawData,
                                  size_t      pi_PixelsCount) const
    {
    if (pi_PixelsCount > m_LastConversionPixelCount)
        {
        m_pTransientConversionDataBuffer = new Byte[pi_PixelsCount * m_TransientNbBytesPerPixel];
        m_LastConversionPixelCount = pi_PixelsCount;
        }

    m_pConverterToStandard->Convert(pi_pSourceRawData,
                                    m_pTransientConversionDataBuffer.get(),
                                    pi_PixelsCount);

    m_pConverterFromStandard->Convert(m_pTransientConversionDataBuffer.get(),
                                      pio_pDestRawData,
                                      pi_PixelsCount);
    }

//-----------------------------------------------------------------------------
// public
// Compose
//-----------------------------------------------------------------------------
void HRPComplexConverter::Compose(const void* pi_pSourceRawData,
                                  void*       pio_pDestRawData,
                                  size_t      pi_PixelsCount) const
    {
    if(m_Alpha)
        {
        if (pi_PixelsCount > m_LastConversionPixelCount)
            {
            m_pTransientConversionDataBuffer = new Byte[pi_PixelsCount *
                                                          m_TransientNbBytesPerPixel];
            m_LastConversionPixelCount = pi_PixelsCount;
            }

        m_pConverterToStandard->Convert(pi_pSourceRawData,
                                        m_pTransientConversionDataBuffer.get(),
                                        pi_PixelsCount);

        m_pConverterFromStandard->Compose(m_pTransientConversionDataBuffer.get(),
                                          pio_pDestRawData,
                                          pi_PixelsCount);
        }
    else
        {
        Convert(pi_pSourceRawData, pio_pDestRawData, pi_PixelsCount);
        }
    }

//-----------------------------------------------------------------------------
// public
// AllocateCopy
//-----------------------------------------------------------------------------
HRPPixelConverter* HRPComplexConverter::AllocateCopy() const
    {
    return(new HRPComplexConverter(*this));
    }


//-----------------------------------------------------------------------------
// private
// Has16bitsChannel
//-----------------------------------------------------------------------------
bool HRPComplexConverter::Has16bitsChannel(const HRPPixelType* pPixelType) const
    {
    const HRPChannelOrg& channelOrg = pPixelType->GetChannelOrg();

    for(uint32_t i(0); i < channelOrg.CountChannels(); i++)
        {
        if(channelOrg.GetChannelPtr(i)->GetSize() == 16)
            {
            return true;
            }
        }

    return false;
    }


//-----------------------------------------------------------------------------
// private
// Update
//-----------------------------------------------------------------------------
void HRPComplexConverter::Update()
    {
    // is there alpha in the source pixel type
    if((GetSourcePixelType()->GetChannelOrg()).GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        {
        // if yes, use RGBA as the intermediate standard
        m_Alpha = true;

        if(Has16bitsChannel(GetSourcePixelType()) && Has16bitsChannel(GetDestinationPixelType()))
            {
            // 16 bits per channel
            m_pConverterToStandard = GetSourcePixelType()->GetConverterTo(&m_PixelTypeV64RGBA);
            m_pConverterFromStandard = GetDestinationPixelType()->GetConverterFrom(&m_PixelTypeV64RGBA);
            }
        else
            {
            // 8 bits per channel
            m_pConverterToStandard = GetSourcePixelType()->GetConverterTo(&m_PixelTypeRGBA);
            m_pConverterFromStandard = GetDestinationPixelType()->GetConverterFrom(&m_PixelTypeRGBA);
            }
        }
    else
        {
        // otherwise, use simple RGB
        m_Alpha = false;

        if(Has16bitsChannel(GetSourcePixelType()) && Has16bitsChannel(GetDestinationPixelType()))
            {
            // 16 bits per channel
            m_pConverterToStandard = GetSourcePixelType()->GetConverterTo(&m_PixelTypeV48RGB);
            m_pConverterFromStandard = GetDestinationPixelType()->GetConverterFrom(&m_PixelTypeV48RGB);
            }
        else
            {
            // 8 bits per channel
            m_pConverterToStandard = GetSourcePixelType()->GetConverterTo(&m_PixelTypeRGB);
            m_pConverterFromStandard = GetDestinationPixelType()->GetConverterFrom(&m_PixelTypeRGB);
            }
        }

    m_TransientNbBytesPerPixel = (Byte)ceil(m_pConverterToStandard->
                                              GetDestinationPixelType()->
                                              CountPixelRawDataBits() / 8.0);

    HASSERT(m_pConverterToStandard != 0);
    HASSERT(m_pConverterFromStandard != 0);
    }
