//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFOracleException.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFOracleException
//-----------------------------------------------------------------------------
#pragma once

#include "HRFException.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFOracleException : public HRFException
    {
public:
    typedef int32_t     ErrorCode;
    static const ErrorCode
    NO_ERROR_CODE = -1;
    HRFOracleException     (const WString&              pi_rFileName, const WString& pi_rOriginalErrorMsg, 
                            const ErrorCode             pi_ErrorCode = NO_ERROR_CODE);
    virtual             ~HRFOracleException    ();
    const ErrorCode    GetErrorCode                       () const;
    WStringCR    GetOriginalErrorMsg                       () const;
    HRFOracleException                   (const HRFOracleException&     pi_rObj); 
    virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override ; 
    virtual void ThrowMyself() const override {throw *this;} 
protected:       
        ErrorCode m_ErrorCode;
        WString         m_ErrorMsg;
private:
    static ExceptionID GetExceptionIDForCode(ErrorCode pi_ErrorCode);
    };
END_IMAGEPP_NAMESPACE
