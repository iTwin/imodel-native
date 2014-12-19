//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFactory.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HCDCodec.h>

//=======================================================================================
//! HCDCodecFactory
// @bsiclass 
//=======================================================================================
class HCDCodecFactory
{
public:
    _HDLLu HFCPtr<HCDCodec> Create(const HCLASS_ID& pi_rClassKey) const;

    _HDLLu static HCDCodecFactory& GetInstance();

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