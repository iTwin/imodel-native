//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetAttributesHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetAttributesHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetBinaryHandler.h"

class HRFInternetAttributesHandler : public HRFInternetBinaryHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetAttributesHandler();
    virtual         ~HRFInternetAttributesHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);
    
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromString      (WStringCR pi_rAttributeName, WStringCR pi_rValue);
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromVectorDouble(WStringCR pi_rAttributeName, vector<double> pi_rValue);
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromDouble      (WStringCR pi_rAttributeName, double pi_rValue);
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromVectorByte  (WStringCR pi_rAttributeName, vector<Byte> pi_rValue);
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromVectorUShort(WStringCR pi_rAttributeName, vector<unsigned short> pi_rValue);
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromULong32     (WStringCR pi_rAttributeName, uint32_t pi_rValue);
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromVectorChar  (WStringCR pi_rAttributeName, vector<char> pi_rValue);
    static HFCPtr<HPMGenericAttribute> DecodeHPMAttributeFromMatrix      (WStringCR pi_rAttributeName, HFCMatrix<4,4, double> pi_rValue);

protected:
    // XML parser callbacks
    static void StartElement        (void*         pi_pUserData,
                                     const char*  pi_pName,
                                     const char** pi_ppAttributes);
    static void CharacterDataHandler(void*         pi_pUserData,
                                     const char*  pi_pString,
                                     int           pi_StringLength);
    static void EndElement          (void*         pi_pUserData,
                                     const char*  pi_pName);
    };
