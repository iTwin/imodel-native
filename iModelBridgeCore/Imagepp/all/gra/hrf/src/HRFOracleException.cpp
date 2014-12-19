//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFOracleException.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation for HRFOracleException
//-----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRFOracleException.h>

typedef map<HRFOracleException::ErrorCode, ExceptionID>     ErrorMap;
typedef ErrorMap::value_type                                ErrorMapItem;

//-----------------------------------------------------------------------------
// Mapping that relates common Oracle library errors to HMR exceptions ID
// system.
//-----------------------------------------------------------------------------
static const ErrorMap::value_type s_ErrorMappingArray[] =
    {
    ErrorMapItem(12154, HRF_AUTHENTICATION_INVALID_SERVICE_EXCEPTION), // Original Oracle Msg: "TNS:could not resolve the connect identifier specified"
    ErrorMapItem(1017, HRF_AUTHENTICATION_INVALID_LOGIN_EXCEPTION), // Original Oracle Msg: "invalid username/password; logon denied"
    };

static const ErrorMap s_ErrorMap(s_ErrorMappingArray,
                                 s_ErrorMappingArray +
                                 sizeof(s_ErrorMappingArray)/sizeof(s_ErrorMappingArray[0]));


HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFOracleException)

HRFOracleException::HRFOracleException  (const WString&      pi_rFileName,
                                                const WString&      pi_rOriginalErrorMsg,
                                                const ErrorCode     pi_ErrorCode)
    :   HRFException(GetExceptionIDForCode(pi_ErrorCode), pi_rFileName, false)
    {
    m_pInfo = new HRFOracleErrorExInfo(pi_rFileName, pi_rOriginalErrorMsg, pi_ErrorCode);

    if (GetID() == HRF_ORACLE_EXCEPTION)
        {
        GENERATE_FORMATTED_EXCEPTION_MSG();
        }
    }

HRFOracleException::~HRFOracleException()
    {

    }

HRFOracleException& HRFOracleException::operator=(const HRFOracleException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFOracleErrorExInfo)
        }

    return *this;
    }


HRFOracleException::HRFOracleException(const HRFOracleException& pi_rObj)
    : HRFException(static_cast<ExceptionID>(pi_rObj.GetID()), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFOracleErrorExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }

//-----------------------------------------------------------------------------
// Static Private
// Returns the appropriate HMR exception ID for the specified error code
// according to the mapping of Oracle errors to HMR errors. If no appropriate
// exception ID is found for the specified code, the general HRF_ORACLE_EXCEPTION
// is returned.
//-----------------------------------------------------------------------------
ExceptionID  HRFOracleException::GetExceptionIDForCode(ErrorCode pi_ErrorCode)
    {
    ErrorMap::const_iterator HMRErrorIt = s_ErrorMap.find(pi_ErrorCode);

    if (HMRErrorIt != s_ErrorMap.end())
        return HMRErrorIt->second;
    else
        return HRF_ORACLE_EXCEPTION;
    }


void HRFOracleException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(0 != m_pInfo);
    HPRECONDITION(GetID() == HRF_ORACLE_EXCEPTION);

    const HRFOracleErrorExInfo* pExInfo = static_cast<const HRFOracleErrorExInfo*>(m_pInfo);

    wostringstream ErrorCodeSS;
    if (NO_ERROR_CODE != pExInfo->m_ErrorCode)
        ErrorCodeSS << pExInfo->m_ErrorCode;
    else
        ErrorCodeSS << L"?";

    FORMAT_EXCEPTION_MSG(pio_rMessage,
                         ErrorCodeSS.str().c_str(),
                         pExInfo->m_ErrorMsg.c_str());
    }