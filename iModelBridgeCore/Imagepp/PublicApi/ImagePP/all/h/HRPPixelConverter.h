//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelConverter.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelConverter
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

class HRPPixelType;

class HNOVTABLEINIT HRPPixelConverter : public HFCShareableObject<HRPPixelConverter>
    {
public:

    virtual         ~HRPPixelConverter();

    // Get PixelType
    const HRPPixelType*
    GetDestinationPixelType () const;
    const HRPPixelType*
    GetSourcePixelType      () const;

    // Conversion methods
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData) const;

    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount) const;

    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const;

    virtual void    Compose(const void* pi_pSourceRawData,
                            void* pio_pDestRawData) const;

    virtual void    Compose(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount) const;

    virtual void    ConvertToValue(const void* pi_pSourceRawData,
                                   void* pio_pDestRawData) const;

    virtual const short*
    GetLostChannels() const;

    // Pixel type methods
    virtual void    SetDestinationPixelType(const HRPPixelType* pi_pDestPixelType);
    virtual void    SetSourcePixelType(const HRPPixelType* pi_pSourcePixelType);

    virtual HRPPixelConverter*
    AllocateCopy() const = 0;

protected:


    // Primary methods

    HRPPixelConverter(const HRPPixelType* pi_pSourcePixelType,
                      const HRPPixelType* pi_pDestPixelType);
    HRPPixelConverter();
    HRPPixelConverter(const HRPPixelConverter& pi_rObj);
    HRPPixelConverter&
    operator=(const HRPPixelConverter& pi_rObj);

    virtual void    Update();

private:

    // Attributes

    // Source and destination pixel types
    const HRPPixelType*    m_pSourcePixelType;
    const HRPPixelType*    m_pDestPixelType;

    // Methods
    void            DeepDelete();
    void            DeepCopy(const HRPPixelConverter& pi_rObj);
    };

#include "HRPPixelConverter.hpp"
