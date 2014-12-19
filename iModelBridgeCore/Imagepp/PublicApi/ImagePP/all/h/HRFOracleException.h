//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFOracleException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFOracleException
//-----------------------------------------------------------------------------
#pragma once

#include "HRFException.h"

class HRFOracleException : public HRFException
    {
    HDECLARE_CLASS_ID(6110, HRFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    typedef int32_t     ErrorCode;
    static const ErrorCode
    NO_ERROR_CODE = -1;

    HRFOracleException     (const WString&              pi_rFileName,
                            const WString&              pi_rOriginalErrorMsg,
                            const ErrorCode             pi_ErrorCode = NO_ERROR_CODE);

    virtual             ~HRFOracleException    ();

    HRFOracleException& operator=              (const HRFOracleException&   pi_rObj);
    HRFOracleException     (const HRFOracleException&   pi_rObj);

private:
    static ExceptionID  GetExceptionIDForCode  (ErrorCode                   pi_ErrorCode);

    //----------------------------------------------------------------------------------
    // Exception information struct for HRFOracleException
    //----------------------------------------------------------------------------------
    struct HRFOracleErrorExInfo : public HFCFileExInfo
        {
        HRFOracleErrorExInfo       (const WString&          pi_rFileName = L"",
                                    const WString&          pi_rErrorMsg = L"",
                                    const ErrorCode         pi_ErrorCode = 0)
            :   HFCFileExInfo(pi_rFileName),
                m_ErrorMsg(pi_rErrorMsg),
                m_ErrorCode(pi_ErrorCode)
            {};

        virtual         ~HRFOracleErrorExInfo ()
            {};

        ErrorCode m_ErrorCode;
        WString         m_ErrorMsg;
        };
    };
