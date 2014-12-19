//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPDFException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation for HRFPDFException
//-----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRFPDFException.h>

#include "HRFPDFLibInterface.h"

typedef map<HRFPDFException::ErrorCode, ExceptionID>        ErrorMap;
typedef ErrorMap::value_type                                ErrorMapItem;


HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFPDFException)

HRFPDFException::HRFPDFException(const WString&          pi_rFileName,
                                        const ErrorCode         pi_ErrorCode)
    :   HRFException(GetExceptionIDForCode(pi_ErrorCode& s_ErrorMask), pi_rFileName, false)
    {
    m_pInfo = new HRFPDFErrorExInfo(pi_rFileName, pi_ErrorCode & s_ErrorMask);

    if (GetID() == HRF_PDF_EXCEPTION)
        {
        GENERATE_FORMATTED_EXCEPTION_MSG();
        }
    }

HRFPDFException::~HRFPDFException()
    {

    }

HRFPDFException& HRFPDFException::operator=(const HRFPDFException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFPDFErrorExInfo)
        }

    return *this;
    }


HRFPDFException::HRFPDFException(const HRFPDFException& pi_rObj)
    : HRFException(static_cast<ExceptionID>(pi_rObj.GetID()), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFPDFErrorExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }

//-----------------------------------------------------------------------------
// Mapping that relates common PDF library errors to HMR exceptions ID
// system.
//-----------------------------------------------------------------------------
static const ErrorMap::value_type s_ErrorMappingArray[] =
    {
    ErrorMapItem(pdErrNeedPassword, HRF_AUTHENTICATION_INVALID_LOGIN_EXCEPTION), //pdErrNeedPassword
    };

static const ErrorMap s_ErrorMap(s_ErrorMappingArray,
                                 s_ErrorMappingArray +
                                 sizeof(s_ErrorMappingArray)/sizeof(s_ErrorMappingArray[0]));



//-----------------------------------------------------------------------------
// Static Private
// Returns the appropriate HMR exception ID for the specified error code
// according to the mapping of PDF errors to HMR errors. If no appropriate
// exception ID is found for the specified code, the general HRF_PDF_EXCEPTION
// is returned.
//-----------------------------------------------------------------------------
ExceptionID  HRFPDFException::GetExceptionIDForCode(ErrorCode pi_ErrorCode)
    {
    ErrorMap::const_iterator HMRErrorIt = s_ErrorMap.find(pi_ErrorCode);

    if (HMRErrorIt != s_ErrorMap.end())
        return HMRErrorIt->second;
    else
        return HRF_PDF_EXCEPTION;
    }


void HRFPDFException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(0 != m_pInfo);
    HPRECONDITION(GetID() == HRF_PDF_EXCEPTION);

    const HRFPDFErrorExInfo* pErrorInfoEx = static_cast<const HRFPDFErrorExInfo*>(m_pInfo);

    // Get acrobat's descriptive string for error code
    static const uint32_t ERROR_STR_MAX_LENGTH = 250;
    vector<char> ErrorString(ERROR_STR_MAX_LENGTH + 1, '\0');

    ASGetErrorString(pErrorInfoEx->m_ErrorCode, &ErrorString[0], ERROR_STR_MAX_LENGTH);
    HASSERT(strlen(&ErrorString[0]) > 0);
    string ErrCodeMess(ErrorString.begin(), ErrorString.end());
    FORMAT_EXCEPTION_MSG(pio_rMessage,
                         pErrorInfoEx->m_ErrorCode,
                         WString(ErrCodeMess.c_str(),false).c_str());
    }

