//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecFactory.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodec
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecFactory.h>
#include <Imagepp/all/h/HCDCodecBMPRLE4.h>
#include <Imagepp/all/h/HCDCodecBMPRLE8.h>
#include <Imagepp/all/h/HCDCodecCCITTRLE.h>
#include <Imagepp/all/h/HCDCodecCRL8.h>
#include <Imagepp/all/h/HCDCodecECW.h>
#include <Imagepp/all/h/HCDCodecErMapperSupported.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecFlashpixOLDForMSI10.h>
#include <Imagepp/all/h/HCDCodecFLIRLE8.h>
#include <Imagepp/all/h/HCDCodecJBIG.h>
#include <Imagepp/all/h/HCDCodecLRDRLE.h>
#include <Imagepp/all/h/HCDCodecLZW.h>
#include <Imagepp/all/h/HCDCodecRLE8.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecTGARLE.h>
#include <Imagepp/all/h/HCDCodecPCX.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecCCITTFax4.h>
#include <Imagepp/all/h/HCDCodecFPXSingleColor.h>
#include <Imagepp/all/h/HCDCodecHMRGIF.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecJPEG2000.h>
#include <Imagepp/all/h/HCDCodecJPEGAlpha.h>
#include <Imagepp/all/h/HCDCodecZlib.h>


#define REGISTER_CODEC(pi_ClassName) \
    static struct pi_ClassName##Creator : public HCDCodecFactory::HCDCodecCreator \
        { \
        virtual HFCPtr<HCDCodec> Create() const override {return new pi_ClassName; } \
        } pi_ClassName##CreatorInstance; \
    m_CodecCreators.insert(CodecCreatorMap::value_type(pi_ClassName::CLASS_ID, &pi_ClassName##CreatorInstance));

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HCDCodecFactory::HCDCodecFactory()
    {
    REGISTER_CODEC(HCDCodecIdentity)
    REGISTER_CODEC(HCDCodecBMPRLE4)
    REGISTER_CODEC(HCDCodecBMPRLE8)
    REGISTER_CODEC(HCDCodecCCITTRLE)    
    REGISTER_CODEC(HCDCodecCRL8)
    REGISTER_CODEC(HCDCodecECW)
    REGISTER_CODEC(HCDCodecErMapperSupported)
    REGISTER_CODEC(HCDCodecFlashpix)
    REGISTER_CODEC(HCDCodecFlashpixOLDForMSI10)
    REGISTER_CODEC(HCDCodecFLIRLE8)
#ifdef JBIG_SUPPORT
    REGISTER_CODEC(HCDCodecJBIG)
#endif
    REGISTER_CODEC(HCDCodecLRDRLE)
    REGISTER_CODEC(HCDCodecLZW)
    REGISTER_CODEC(HCDCodecRLE8)
    REGISTER_CODEC(HCDCodecHMRCCITT)
    REGISTER_CODEC(HCDCodecTGARLE)
    
    REGISTER_CODEC(HCDCodecPCX)
    REGISTER_CODEC(HCDCodecCCITTFax4)
    REGISTER_CODEC(HCDCodecFPXSingleColor)
    //Abstract REGISTER_CODEC(HCDCodecGIF)
        REGISTER_CODEC(HCDCodecHMRGIF)
    //Abstract REGISTER_CODEC(HCDCodecPackBits)
        REGISTER_CODEC(HCDCodecHMRPackBits)
    //Abstract REGISTER_CODEC(HCDCodecRLE1)
    REGISTER_CODEC(HCDCodecHMRRLE1)
    REGISTER_CODEC(HCDCodecIJG)
    // these are internal to HCDCodecIJG
    //     REGISTER_CODEC(HCDCodecIJG_8bits)
    //     REGISTER_CODEC(HCDCodecIJG_12bits)
    REGISTER_CODEC(HCDCodecJPEG2000)
    REGISTER_CODEC(HCDCodecJPEGAlpha)
    //Abstract REGISTER_CODEC(HCDCodecDeflate)
        REGISTER_CODEC(HCDCodecZlib)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HCDCodecFactory::~HCDCodecFactory()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HCDCodecFactory& HCDCodecFactory::GetInstance()
    {
    static HCDCodecFactory s_factory;
    return s_factory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HCDCodec> HCDCodecFactory::Create(const HCLASS_ID& pi_rClassKey) const
    {
    HPRECONDITION(m_CodecCreators.find(pi_rClassKey) != m_CodecCreators.end());

    CodecCreatorMap::const_iterator itr = m_CodecCreators.find(pi_rClassKey);
    if(itr == m_CodecCreators.end())
        return NULL;

    return itr->second->Create();
    }
