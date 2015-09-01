//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFactory.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HCDCodec.h>

BEGIN_IMAGEPP_NAMESPACE
//=======================================================================================
//! HCDCodecFactory
// @bsiclass 
//=======================================================================================
class HCDCodecFactory
{
public:
    IMAGEPP_EXPORT HFCPtr<HCDCodec> Create(const HCLASS_ID& pi_rClassKey) const;

    IMAGEPP_EXPORT static HCDCodecFactory& GetInstance();

private:
    struct HCDCodecCreator
        {
        virtual HFCPtr<HCDCodec> Create() const = 0;
        };

    typedef std::map<HCLASS_ID, HCDCodecCreator*> CodecCreatorMap;

    HCDCodecFactory();
    ~HCDCodecFactory();

    // disabled
    HCDCodecFactory(const HCDCodecFactory& pi_rObj);
    HCDCodecFactory& operator=(const HCDCodecFactory& pi_rObj);

    CodecCreatorMap  m_CodecCreators;
};

END_IMAGEPP_NAMESPACE