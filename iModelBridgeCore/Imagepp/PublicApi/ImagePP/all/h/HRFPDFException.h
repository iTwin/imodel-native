//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPDFException.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    HRFPDFException (const WString&              pi_rFileName, const ErrorCode             pi_ErrorCode);
    virtual ~HRFPDFException       ();
    const ErrorCode    GetErrorCode                        () const;
    ExceptionID  GetExceptionIDForCode(ErrorCode pi_ErrorCode) const;
    HRFPDFException (const HRFPDFException&     pi_rObj); 
    virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
protected:
    ErrorCode m_ErrorCode;
    static const ErrorCode s_ErrorMask = 0x0000FFFF;
    };
END_IMAGEPP_NAMESPACE
