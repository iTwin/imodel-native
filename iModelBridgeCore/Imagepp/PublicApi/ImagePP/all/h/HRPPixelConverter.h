//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelConverter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelConverter
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;

class HNOVTABLEINIT HRPPixelConverter : public HFCShareableObject<HRPPixelConverter>
    {
public:

    virtual         ~HRPPixelConverter();

    // Get PixelType
    const HRPPixelType* GetDestinationPixelType () const;
    const HRPPixelType* GetSourcePixelType      () const;
   
    // Convert to destination pixeltype. If destination pixel size is not byte aligned, remaining bits will be padded with zero.
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const = 0;
       
    // Compose(Alpha Blend) source and destination. If destination pixel size is not byte aligned, remaining bits will be padded with zero.
    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const;

    // One pixel convert/compose. Kept for back compatibility.
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const {Convert(pi_pSourceRawData, pio_pDestRawData, 1);}
    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData) const {Compose(pi_pSourceRawData, pio_pDestRawData, 1);}

    //&&Backlog Seems to used only by filtering to restore alpha. To be remove after the move to HRAImageOp.
    virtual void ConvertLostChannel(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount, const bool* pi_pChannelsMask) const;
    virtual const short* GetLostChannels() const;

    // Pixel type methods
    virtual void SetDestinationPixelType(const HRPPixelType* pi_pDestPixelType);
    virtual void SetSourcePixelType(const HRPPixelType* pi_pSourcePixelType);

    virtual HRPPixelConverter* AllocateCopy() const = 0;

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
END_IMAGEPP_NAMESPACE

#include "HRPPixelConverter.hpp"
