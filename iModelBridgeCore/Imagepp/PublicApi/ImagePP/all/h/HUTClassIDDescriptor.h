//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTClassIDDescriptor.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTClassIDDescriptor
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HPMClassKey.h"
#include <ImagePP/all/h/ImagePPMessages.xliff.h>

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
    HFCClassDescriptor(ImagePPMessages::StringId const& pi_ID, Utf8CP pi_ClassCode);
    HFCClassDescriptor(const HFCClassDescriptor& pi_rObj);

    const Utf8String& GetClassCode()  const;
    ImagePPMessages::StringId const& GetStringID() const;

private:

    ImagePPMessages::StringId m_ID;
    Utf8String m_ClassCode;

    // Disable unusable operator= and default constructor.
    HFCClassDescriptor();
    HFCClassDescriptor& operator=(const HFCClassDescriptor& pi_rObj);
    };

inline const Utf8String& HFCClassDescriptor::GetClassCode () const
    {
    return m_ClassCode;
    }

//-----------------------------------------------------------------------------
// Inline standard constructor
//-----------------------------------------------------------------------------

inline HFCClassDescriptor::HFCClassDescriptor(ImagePPMessages::StringId const& pi_ID, Utf8CP pi_ClassCode)
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

    IMAGEPP_EXPORT Utf8String GetClassLabelCodec           (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelCodec           (const HCDCodec&    pi_rCodec) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelPixelType       (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelPixelType       (const HRPPixelType&
                                                 pi_rPixelType) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelTransfoModel    (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelTransfoModel    (const HGF2DTransfoModel&
                                                 pi_rTransfoModel) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelFilter          (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelFilter          (const HRPFilter&   pi_rFilter) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelSLO       (const HRFScanlineOrientation&
                                           pi_rScanlineO) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelBlockType       (const HRFBlockType&
                                                 pi_rBlockT) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelEncodingType    (const HRFEncodingType&
                                                 pi_rEncodingT) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelResampling      (const HRFDownSamplingMethod&
                                                 pi_rSampling) const;
    IMAGEPP_EXPORT Utf8String GetClassLabelGeoRef          (const HRFGeoreferenceFormat&
                                                 pi_rGeoRef) const;
    IMAGEPP_EXPORT const Utf8String& GetClassCodeCodec     (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT const Utf8String& GetClassCodePixelType (HCLASS_ID          pi_ClassID) const;
    const Utf8String& GetClassCodeTransfoModel     (HCLASS_ID          pi_ClassID) const;
    const Utf8String& GetClassCodeFilter           (HCLASS_ID          pi_ClassID) const;
    IMAGEPP_EXPORT const Utf8String& GetNotfoundString() const;

private:
    typedef map<HCLASS_ID , HFCClassDescriptor, less<HCLASS_ID>, allocator<std::pair<const HCLASS_ID, HFCClassDescriptor> > > IDDescriptorMap;
    IDDescriptorMap     m_CodecClassIDDescriptorMap;
    IDDescriptorMap     m_PixelClassIDDescriptorMap;
    IDDescriptorMap     m_TransfoClassIDDescriptorMap;
    IDDescriptorMap     m_FilterClassIDDescriptorMap;

    const Utf8String m_NotFound;

    // Disable operator= and copy constructor
    HUTClassIDDescriptor                        (const HUTClassIDDescriptor& pi_rObj);
    HUTClassIDDescriptor&       operator =      (const HUTClassIDDescriptor& pi_rObj);


    };
END_IMAGEPP_NAMESPACE

