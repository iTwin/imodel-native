//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPComplexConverter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPComplexConverter
//-----------------------------------------------------------------------------
// Complex converter definition.
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelConverter.h"
#include "HRPPixelTypeV24R8G8B8.h"
#include "HRPPixelTypeV32R8G8B8A8.h"
#include "HRPPixelTypeV48R16G16B16.h"
#include "HRPPixelTypeV64R16G16B16A16.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPComplexConverter : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    HRPComplexConverter();

    HRPComplexConverter(const HRPPixelType* pi_pSourcePixelType,
                        const HRPPixelType* pi_pDestPixelType);

    HRPComplexConverter(const HRPComplexConverter& pi_rObj);

    virtual         ~HRPComplexConverter();

    virtual void    Convert(    const void* pi_pSourceRawData,
                                void* pio_pDestRawData,
                                size_t pi_PixelsCount) const override; 

    virtual void    Compose(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount) const override;

    virtual HRPPixelConverter* AllocateCopy() const override;

protected:

    virtual void    Update() override;

private:

    bool            Has16bitsChannel(const HRPPixelType* pPixelType) const;

    // private attributes

    HFCPtr<HRPPixelConverter>
    m_pConverterToStandard;

    HFCPtr<HRPPixelConverter>
    m_pConverterFromStandard;

    HRPPixelTypeV24R8G8B8
    m_PixelTypeRGB;

    HRPPixelTypeV32R8G8B8A8
    m_PixelTypeRGBA;

    HRPPixelTypeV48R16G16B16
    m_PixelTypeV48RGB;

    HRPPixelTypeV64R16G16B16A16
    m_PixelTypeV64RGBA;

    bool           m_Alpha;

    //Number of bytes per pixel for the transient pixel type.
    Byte          m_TransientNbBytesPerPixel;

    //Buffer for the transient conversion.
    mutable HAutoPtr<Byte> m_pTransientConversionDataBuffer;
    mutable size_t           m_LastConversionPixelCount;
    };
END_IMAGEPP_NAMESPACE

//#include "HRPComplexConverter.hpp"
