//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFOracleException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation for HRFOracleException
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFOracleException.h>

typedef map<HRFOracleException::ErrorCode, ImagePPExceptions::StringId> ErrorMap;
typedef ErrorMap::value_type                                            ErrorMapItem;

//-----------------------------------------------------------------------------
// Mapping that relates common Oracle library errors to HMR exceptions ID
// system.
//-----------------------------------------------------------------------------
static const ErrorMap::value_type s_ErrorMappingArray[] =
    {
    ErrorMapItem(12154, ImagePPExceptions::HRFOracleCouldNotResolverIndetifier()), // Original Oracle Msg: "TNS:could not resolve the connect identifier specified"
    ErrorMapItem(1017, ImagePPExceptions::HRFOracleInvalidLoginInformation()), // Original Oracle Msg: "invalid username/password; logon denied" 
    };

static const ErrorMap s_ErrorMap(s_ErrorMappingArray,
                                 s_ErrorMappingArray +
                                 sizeof(s_ErrorMappingArray)/sizeof(s_ErrorMappingArray[0]));


HRFOracleException::HRFOracleException  (const WString&      pi_rFileName, const WString&      pi_rOriginalErrorMsg,
                                         const ErrorCode     pi_ErrorCode) :   HRFException(pi_rFileName)
    {
    m_ErrorMsg = pi_rOriginalErrorMsg;
    m_ErrorCode =  pi_ErrorCode;
    }

HRFOracleException::~HRFOracleException()
    {

    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFOracleException::HRFOracleException(const HRFOracleException&     pi_rObj) : HRFException(pi_rObj)
    {
    m_ErrorMsg =pi_rObj.m_ErrorMsg;
    m_ErrorCode =pi_rObj.m_ErrorCode;
    }
//-----------------------------------------------------------------------------
// Static Private
// Returns the appropriate HMR exception ID for the specified error code
// according to the mapping of Oracle errors to HMR errors. If no appropriate
// exception ID is found for the specified code, the general HRF_ORACLE_EXCEPTION
// is returned.
//-----------------------------------------------------------------------------
ImagePPExceptions::StringId HRFOracleException::GetExceptionIDForCode(ErrorCode pi_ErrorCode)
    {
    ErrorMap::const_iterator HMRErrorIt = s_ErrorMap.find(pi_ErrorCode);

    if (HMRErrorIt != s_ErrorMap.end())
        return HMRErrorIt->second;
    else
        return ImagePPExceptions::HRFOracleGeneric(); //HRF oracle exception
    }

//-----------------------------------------------------------------------------
// public
// Get the exception message
//-----------------------------------------------------------------------------
WString HRFOracleException::GetExceptionMessage() const
    {
    wostringstream ErrorCodeSS;
    if (NO_ERROR_CODE != m_ErrorCode)
        ErrorCodeSS << m_ErrorCode;
    else
        ErrorCodeSS << L"?";

    ImagePPExceptions::StringId exceptionID = GetExceptionIDForCode(m_ErrorCode);
    WPrintfString rawMessage(GetRawMessageFromResource(exceptionID).c_str(), ErrorCodeSS.str().c_str(), m_ErrorMsg.c_str());

    BeStringUtilities::Utf8ToWChar(exceptionNameWChar, exceptionID.m_str, 100);

    WString exceptionName(exceptionID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }
//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const HRFOracleException::ErrorCode HRFOracleException::GetErrorCode() const
    {
    return m_ErrorCode;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR HRFOracleException::GetOriginalErrorMsg() const
{
    return m_ErrorMsg;
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HRFOracleException::Clone() const
    {
    return new HRFOracleException(*this);;
    }
