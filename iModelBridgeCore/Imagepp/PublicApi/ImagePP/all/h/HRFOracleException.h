//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
    HRFOracleException     (const Utf8String&              pi_rFileName, const Utf8String& pi_rOriginalErrorMsg, 
                            const ErrorCode             pi_ErrorCode = NO_ERROR_CODE);
    virtual             ~HRFOracleException    ();
    const ErrorCode    GetErrorCode                       () const;
    Utf8StringCR    GetOriginalErrorMsg                       () const;
    HRFOracleException                   (const HRFOracleException&     pi_rObj); 
    virtual Utf8String GetExceptionMessage() const override;
    virtual HFCException* Clone() const override ; 
    virtual void ThrowMyself() const override {throw *this;} 
protected:       
        ErrorCode m_ErrorCode;
        Utf8String         m_ErrorMsg;
private:
    static ExceptionID GetExceptionIDForCode(ErrorCode pi_ErrorCode);
    };
END_IMAGEPP_NAMESPACE
