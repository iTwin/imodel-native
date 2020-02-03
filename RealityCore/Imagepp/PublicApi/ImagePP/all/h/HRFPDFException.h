//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFPDFException
//-----------------------------------------------------------------------------
#pragma once

#include "HRFException.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFPDFException : public HRFException
    {
public:
    typedef uint32_t     ErrorCode;
    HRFPDFException (const Utf8String&              pi_rFileName, const ErrorCode             pi_ErrorCode);
    virtual ~HRFPDFException       ();
    const ErrorCode    GetErrorCode                        () const;
    ExceptionID  GetExceptionIDForCode(ErrorCode pi_ErrorCode) const;
    HRFPDFException (const HRFPDFException&     pi_rObj); 
    virtual Utf8String GetExceptionMessage() const override;
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
protected:
    ErrorCode m_ErrorCode;
    static const ErrorCode s_ErrorMask = 0x0000FFFF;
    };
END_IMAGEPP_NAMESPACE
