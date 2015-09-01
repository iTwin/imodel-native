//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTClassIDDescriptor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTClassIDDescriptor
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HPMClassKey.h"
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

BEGIN_IMAGEPP_NAMESPACE
class HCDCodec;
class HRPPixelType;
class HGF2DTransfoModel;
class HRPFilter;
class HRFScanlineOrientation;
class HRFBlockType;
class HRFEncodingType;
class HRFDownSamplingMethod;
class HRFGeoreferenceFormat;

//-----------------------------------------------------------------------------
// Class : HFCClassDescriptor
//-----------------------------------------------------------------------------

class HFCClassDescriptor
    {
public:
    HFCClassDescriptor(ImagePPMessages::StringId const& pi_ID, WCharCP pi_ClassCode);
    HFCClassDescriptor(const HFCClassDescriptor& pi_rObj);

    const WString& GetClassCode()  const;
    ImagePPMessages::StringId const& GetStringID() const;

private:

    ImagePPMessages::StringId m_ID;
    WString m_ClassCode;

    // Disable unusable operator= and default constructor.
    HFCClassDescriptor();
    HFCClassDescriptor& operator=(const HFCClassDescriptor& pi_rObj);
    };

inline const WString& HFCClassDescriptor::GetClassCode () const
    {
    return m_ClassCode;
    }

//-----------------------------------------------------------------------------
// Inline standard constructor
//-----------------------------------------------------------------------------

inline HFCClassDescriptor::HFCClassDescriptor(ImagePPMessages::StringId const& pi_ID, WCharCP pi_ClassCode)
    {
    m_ID = pi_ID;
    m_ClassCode  = pi_ClassCode;
    }

//-----------------------------------------------------------------------------
//  Inline copy constructor required by the map.
//-----------------------------------------------------------------------------

inline HFCClassDescriptor::HFCClassDescriptor(const HFCClassDescriptor& pi_rObj)
    {
    m_ID = pi_rObj.m_ID;
    m_ClassCode  = pi_rObj.m_ClassCode;
    }
//-----------------------------------------------------------------------------
//  Inline Get the ID.
//-----------------------------------------------------------------------------

inline ImagePPMessages::StringId const& HFCClassDescriptor::GetStringID() const
    {
    return m_ID;
    }

//-----------------------------------------------------------------------------
// Class : HUTClassIDDescriptor
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------

class HUTClassIDDescriptor
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HUTClassIDDescriptor)

public:
    HUTClassIDDescriptor();
    ~HUTClassIDDescriptor();

    IMAGEPP_EXPORT WString GetClassLabelCodec           (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT WString GetClassLabelCodec           (const HCDCodec&    pi_rCodec) const;
    IMAGEPP_EXPORT WString GetClassLabelPixelType       (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT WString GetClassLabelPixelType       (const HRPPixelType&
                                                 pi_rPixelType) const;
    IMAGEPP_EXPORT WString GetClassLabelTransfoModel    (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT WString GetClassLabelTransfoModel    (const HGF2DTransfoModel&
                                                 pi_rTransfoModel) const;
    IMAGEPP_EXPORT WString GetClassLabelFilter          (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT WString GetClassLabelFilter          (const HRPFilter&   pi_rFilter) const;
    IMAGEPP_EXPORT WString GetClassLabelSLO       (const HRFScanlineOrientation&
                                           pi_rScanlineO) const;
    IMAGEPP_EXPORT WString GetClassLabelBlockType       (const HRFBlockType&
                                                 pi_rBlockT) const;
    IMAGEPP_EXPORT WString GetClassLabelEncodingType    (const HRFEncodingType&
                                                 pi_rEncodingT) const;
    IMAGEPP_EXPORT WString GetClassLabelResampling      (const HRFDownSamplingMethod&
                                                 pi_rSampling) const;
    IMAGEPP_EXPORT WString GetClassLabelGeoRef          (const HRFGeoreferenceFormat&
                                                 pi_rGeoRef) const;
    IMAGEPP_EXPORT const WString& GetClassCodeCodec     (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT const WString& GetClassCodePixelType (HCLASS_ID          pi_ClassID) const;
    const WString& GetClassCodeTransfoModel     (HCLASS_ID          pi_ClassID) const;
    const WString& GetClassCodeFilter           (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT const WString& GetNotfoundString() const;

private:
    typedef map<HCLASS_ID , HFCClassDescriptor, less<HCLASS_ID>, allocator<HFCClassDescriptor> > IDDescriptorMap;
    IDDescriptorMap     m_CodecClassIDDescriptorMap;
    IDDescriptorMap     m_PixelClassIDDescriptorMap;
    IDDescriptorMap     m_TransfoClassIDDescriptorMap;
    IDDescriptorMap     m_FilterClassIDDescriptorMap;

    const WString m_NotFound;

    // Disable operator= and copy constructor
    HUTClassIDDescriptor                        (const HUTClassIDDescriptor& pi_rObj);
    HUTClassIDDescriptor&       operator =      (const HUTClassIDDescriptor& pi_rObj);


    };
END_IMAGEPP_NAMESPACE

