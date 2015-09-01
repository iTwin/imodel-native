//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelType.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_IMAGEPP_NAMESPACE
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
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPPixelTypeId_Base)

public:
    // The max size needed to hold one pixel in bits and bytes(rounded up).
    enum { 
            MAX_PIXEL_BITS  = 96,     // HRPPixelTypeV96R32G32B32
            MAX_PIXEL_BYTES = (MAX_PIXEL_BITS + 7) / 8
         };

    // Primary methods
    virtual         ~HRPPixelType();

    // Not a const methos because it returns a non const pointer to this.
    virtual HRPPixelType1BitInterface*          Get1BitInterface ();

    virtual HFCPtr<HRPPixelConverter>           GetConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const;

    IMAGEPP_EXPORT virtual HFCPtr<HRPPixelConverter>    GetConverterTo(const HRPPixelType* pi_pPixelTypeTo) const;

    const HRPPixelPalette&                      GetPalette() const;

    const HRPChannelOrg&
    GetChannelOrg() const;

    virtual HRPQuantizedPalette*                CreateQuantizedPalette(uint32_t pi_MaxEntries) const;

    IMAGEPP_EXPORT HRPPixelPalette&                     LockPalette();

    IMAGEPP_EXPORT void                                 UnlockPalette();

    //! Based on the same input, will both pixeltypes output the same value?
    //! Basically an equal operator but without the rawData and others states, metadatas...
    IMAGEPP_EXPORT bool HasSamePixelInterpretation(HRPPixelType const& obj) const;

    // Please do not override this method.
    uint32_t                                   CountPixelRawDataBits() const;

    // Other methods
    virtual unsigned short                      CountIndexBits()        const;
    virtual unsigned short                      CountValueBits()        const = 0;

    IMAGEPP_EXPORT const void*        GetDefaultRawData() const;
    IMAGEPP_EXPORT void            SetDefaultRawData(const void* pi_pValue);

    IMAGEPP_EXPORT virtual void    SetDefaultCompositeValue(const void* pi_pValue);

    virtual uint32_t                           FindNearestEntryInPalette(const void* pi_pValue) const;

    bool HasConverterFrom(HRPPixelType const& from) const {return HasConverterFrom(&from) != NULL;}
    bool HasConverterTo(HRPPixelType const& to) const {return HasConverterTo(&to) != NULL;}

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


    virtual const HRPPixelConverter*            HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const = 0;
    virtual const HRPPixelConverter*            HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const = 0;

private:

    // operator= DISABLED
    HRPPixelType&   operator=(const HRPPixelType& pi_rObj);

    //*** Diasabled because they use m_pDefaultRawData which we usually want to ignore.
    //    For now, use HasSamePixelInterpretation() that will ignore rawData.
    bool   operator==(const HRPPixelType& pi_rObj) const;
    bool   operator!=(const HRPPixelType& pi_rObj) const;

    void                             DeepDelete();
    void                             DeepCopy(const HRPPixelType& pi_rObj);
    HFCPtr<HRPPixelConverter>        CreateConverter(const HRPPixelConverter*  pi_pConverter,
                                                     const HRPPixelType* pi_pPixelType) const;

    // greater than 0 if pixels are indices, equal to 0 if pixels are values.
    unsigned short                  m_IndexBits;

    HRPPixelPalette                 m_PixelPalette;

    HRPChannelOrg                   m_ChannelOrg;

    HArrayAutoPtr<Byte>             m_pDefaultRawData;

    HFCExclusiveKey                 m_PaletteAccess;
    };
END_IMAGEPP_NAMESPACE

#include "HRPPixelType.hpp"
