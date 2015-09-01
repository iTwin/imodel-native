//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelTypeFactory.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPPixelTypeFactory
//-----------------------------------------------------------------------------
// Pixel type definition.  It is composed of a set of channel descriptions
// with a palette definition.
//-----------------------------------------------------------------------------
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "HFCMacros.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;
class HRPChannelOrg;
class HRPPixelPalette;

class HRPPixelTypeFactory
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRPPixelTypeFactory)

public:

    // PixelType creation
    IMAGEPP_EXPORT HFCPtr<HRPPixelType> Create(const HRPChannelOrg& pi_rChannelOrg, unsigned short pi_IndexBits) const;
    IMAGEPP_EXPORT HFCPtr<HRPPixelType> Create(const HRPPixelPalette& pi_rPixelPalette) const;
    IMAGEPP_EXPORT HFCPtr<HRPPixelType> Create(const HCLASS_ID& pi_rClassKey) const;
    
private:
    typedef map<HCLASS_ID, HFCPtr<HRPPixelType> > PixelTypeMap;

    HRPPixelTypeFactory();
    ~HRPPixelTypeFactory();

    // disabled
    HRPPixelTypeFactory(const HRPPixelTypeFactory& pi_rObj);
    HRPPixelTypeFactory& operator=(const HRPPixelTypeFactory& pi_rObj);

    void Register(HFCPtr<HRPPixelType> pPixeltype);    

    PixelTypeMap  m_PixelTypes;
    };
END_IMAGEPP_NAMESPACE
