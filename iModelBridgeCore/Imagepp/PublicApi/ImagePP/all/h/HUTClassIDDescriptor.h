//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTClassIDDescriptor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HUTClassIDDescriptor
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HPMClassKey.h"

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
    HFCClassDescriptor(uint32_t pi_ID, WString pi_ClassCode);
    HFCClassDescriptor(const HFCClassDescriptor& pi_rObj);

    const WString& GetClassLabel() const;
    const WString& GetClassCode()  const;
    uint32_t GetStringID() const;

private:

    uint32_t m_ID;
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

inline HFCClassDescriptor::HFCClassDescriptor(uint32_t pi_ID, WString pi_ClassCode)
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

inline uint32_t HFCClassDescriptor::GetStringID() const
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
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HUTClassIDDescriptor)

public:
    HUTClassIDDescriptor();
    ~HUTClassIDDescriptor();

    _HDLLg WString GetClassLabelCodec           (HCLASS_ID          pi_ClassID) const;
    _HDLLg WString GetClassLabelCodec           (const HCDCodec&    pi_rCodec) const;
    _HDLLg WString GetClassLabelPixelType       (HCLASS_ID          pi_ClassID) const;
    _HDLLg WString GetClassLabelPixelType       (const HRPPixelType&
                                                 pi_rPixelType) const;
    _HDLLg WString GetClassLabelTransfoModel    (HCLASS_ID          pi_ClassID) const;
    _HDLLg WString GetClassLabelTransfoModel    (const HGF2DTransfoModel&
                                                 pi_rTransfoModel) const;
    _HDLLg WString GetClassLabelFilter          (HCLASS_ID          pi_ClassID) const;
    _HDLLg WString GetClassLabelFilter          (const HRPFilter&   pi_rFilter) const;
    _HDLLg WString GetClassLabelSLO       (const HRFScanlineOrientation&
                                           pi_rScanlineO) const;
    _HDLLg WString GetClassLabelBlockType       (const HRFBlockType&
                                                 pi_rBlockT) const;
    _HDLLg WString GetClassLabelEncodingType    (const HRFEncodingType&
                                                 pi_rEncodingT) const;
    _HDLLg WString GetClassLabelResampling      (const HRFDownSamplingMethod&
                                                 pi_rSampling) const;
    _HDLLg WString GetClassLabelGeoRef          (const HRFGeoreferenceFormat&
                                                 pi_rGeoRef) const;
    _HDLLg const WString& GetClassCodeCodec     (HCLASS_ID          pi_ClassID) const;
    _HDLLg const WString& GetClassCodePixelType (HCLASS_ID          pi_ClassID) const;
    const WString& GetClassCodeTransfoModel     (HCLASS_ID          pi_ClassID) const;
    const WString& GetClassCodeFilter           (HCLASS_ID          pi_ClassID) const;
    _HDLLg const WString& GetNotfoundString() const;

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

