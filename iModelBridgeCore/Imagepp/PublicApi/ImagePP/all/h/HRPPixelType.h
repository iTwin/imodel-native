//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelType.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelType
//-----------------------------------------------------------------------------
// Pixel type definition.  It is composed of a set of channel descriptions
// with a palette definition.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPPixelPalette.h"
#include "HPMPersistentObject.h"
#include "HMGMessageSender.h"

class HRPPixelType1BitInterface;
class HRPPixelConverter;
class HRPQuantizedPalette;

// Factors used when computing the luminance for an RGB triplet.
// used in the conversion to grayscale. These factors come from:
//
// ITU-R Recommendation  BT.709, Basic Parameter Values for the HDTV Standard
// for the Studio and for International Programme Exchange (1990),
// [formerly CCIR Rec.  709], ITU, 1211  Geneva  20, Switzerland.
#define REDFACTOR   (0.2125)
#define GREENFACTOR (0.7154)
#define BLUEFACTOR  (0.0721)

class HRPPixelType : public HPMPersistentObject,
    public HPMShareableObject<HRPPixelType>,
    public HMGMessageSender
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1004)

public:
    // Primary methods
    virtual         ~HRPPixelType();

    // operators
    virtual bool   operator==(const HRPPixelType& pi_rObj) const;
    virtual bool   operator!=(const HRPPixelType& pi_rObj) const;

    // Not a const methos because it returns a non const pointer to this.
    virtual HRPPixelType1BitInterface*
    Get1BitInterface ();

    virtual HFCPtr<HRPPixelConverter>
    GetConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    _HDLLg virtual HFCPtr<HRPPixelConverter>
    GetConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;

    const HRPPixelPalette&
    GetPalette() const;

    const HRPChannelOrg&
    GetChannelOrg() const;

    virtual HRPQuantizedPalette*
    CreateQuantizedPalette(uint32_t pi_MaxEntries) const;

    _HDLLg HRPPixelPalette&
    LockPalette();

    _HDLLg void            UnlockPalette();

    // Please do not override this method.
    uint32_t CountPixelRawDataBits() const;

    // Other methods
    virtual unsigned short CountIndexBits()        const;
    virtual unsigned short CountValueBits()        const = 0;

    _HDLLg const void*        GetDefaultRawData() const;
    _HDLLg void            SetDefaultRawData(const void* pi_pValue);

    _HDLLg virtual void    SetDefaultCompositeValue(const void* pi_pValue);

    virtual uint32_t FindNearestEntryInPalette(const void* pi_pValue) const;

protected:
    // member
    static HFCExclusiveKey s_ConverterAccess;


    // primary methods
    HRPPixelType();
    HRPPixelType(const HRPChannelOrg& pi_rChannelOrg,
                 unsigned short pi_IndexBits);
    HRPPixelType(const HRPChannelOrg& pi_rChannelOrg,
                 unsigned short pi_IndexBits,
                 const HRPChannelOrg& pi_rPaletteChannelOrg);
    HRPPixelType(const HRPPixelPalette& pi_rPixelPalette);
    HRPPixelType(const HRPPixelType& pi_rObj);


    virtual const HRPPixelConverter*
    HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const = 0;

    virtual const HRPPixelConverter*
    HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const = 0;

private:

    // operator= DISABLED
    HRPPixelType&   operator=(const HRPPixelType& pi_rObj);

    void           DeepDelete();
    void           DeepCopy(const HRPPixelType& pi_rObj);
    HFCPtr<HRPPixelConverter>
    CreateConverter(const HRPPixelConverter*  pi_pConverter,
                    const HRPPixelType* pi_pPixelType) const;

    // greater than 0 if pixels are indices, equal to 0 if pixels are values.
    unsigned short                 m_IndexBits;

    HRPPixelPalette                 m_PixelPalette;

    HRPChannelOrg                   m_ChannelOrg;

    bool                           m_HasUserDefaultRawData;
    HArrayAutoPtr<Byte>            m_pDefaultRawData;

    HFCExclusiveKey                 m_PaletteAccess;
    };

#include "HRPPixelType.hpp"
