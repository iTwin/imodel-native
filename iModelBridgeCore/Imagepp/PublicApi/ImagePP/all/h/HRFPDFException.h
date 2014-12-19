//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPDFException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFPDFException
//-----------------------------------------------------------------------------
#pragma once

#include "HRFException.h"

class HRFPDFException : public HRFException
    {
    HDECLARE_CLASS_ID(6109, HRFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    typedef uint32_t     ErrorCode;

    HRFPDFException        (const WString&              pi_rFileName,
                            const ErrorCode             pi_ErrorCode);

    virtual             ~HRFPDFException       ();

    HRFPDFException&    operator=              (const HRFPDFException&      pi_rObj);
    HRFPDFException        (const HRFPDFException&      pi_rObj);

private:

    static ExceptionID  GetExceptionIDForCode  (ErrorCode                   pi_ErrorCode);

    //----------------------------------------------------------------------------------
    // Exception information struct for HRFPDFException
    //----------------------------------------------------------------------------------
    struct HRFPDFErrorExInfo : public HFCFileExInfo
        {
        HRFPDFErrorExInfo      (const WString&              pi_rFileName = L"",
                                const ErrorCode             pi_ErrorCode = 0)
            :   HFCFileExInfo(pi_rFileName),
                m_ErrorCode(pi_ErrorCode)
            {};

        virtual         ~HRFPDFErrorExInfo ()
            {};

        ErrorCode       m_ErrorCode;
        };

    static const ErrorCode s_ErrorMask = 0x0000FFFF;

    };
