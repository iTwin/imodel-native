//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPDFException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation for HRFPDFException
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFPDFFile.h>

#if defined(IPP_HAVE_PDF_SUPPORT) 

#include <Imagepp/all/h/HRFPDFException.h>

#include "HRFPDFLibInterface.h"

typedef map<HRFPDFException::ErrorCode, ExceptionID>        ErrorMap;
typedef ErrorMap::value_type                                ErrorMapItem;

HRFPDFException::HRFPDFException(const WString&          pi_rFileName,
                                        const ErrorCode         pi_ErrorCode)
    :   HRFException(pi_rFileName)
    {
    m_ErrorCode = pi_ErrorCode & s_ErrorMask;
    }

HRFPDFException::~HRFPDFException()
{

}
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFPDFException::HRFPDFException(const HRFPDFException&     pi_rObj) : HRFException(pi_rObj)
 {
     m_ErrorCode =pi_rObj.m_ErrorCode;
 }
//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const HRFPDFException::ErrorCode HRFPDFException::GetErrorCode() const
    {
    return m_ErrorCode;
    }
//-----------------------------------------------------------------------------
// Mapping that relates common PDF library errors to HMR exceptions ID
// system.
//-----------------------------------------------------------------------------
static const ErrorMap::value_type s_ErrorMappingArray[] =
    {
    ErrorMapItem(pdErrNeedPassword, ImagePPExceptions::HRFPDFNeedPassword()), //pdErrNeedPassword
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
ExceptionID  HRFPDFException::GetExceptionIDForCode(ErrorCode pi_ErrorCode) const
    {
    ErrorMap::const_iterator HMRErrorIt = s_ErrorMap.find(pi_ErrorCode);

    if (HMRErrorIt != s_ErrorMap.end())
        return HMRErrorIt->second;
    else
        return ImagePPExceptions::HRFPDFGeneric();
    }


WString HRFPDFException::GetExceptionMessage() const
{
    ImagePPExceptions::StringId exceptionID = GetExceptionIDForCode(m_ErrorCode);
    WString message = GetRawMessageFromResource(exceptionID);
    if(exceptionID == ImagePPExceptions::HRFPDFGeneric())
        {
        // Get acrobat's descriptive string for error code
        static const uint32_t ERROR_STR_MAX_LENGTH = 250;
        vector<char> ErrorString(ERROR_STR_MAX_LENGTH + 1, '\0');

        ASGetErrorString(m_ErrorCode, &ErrorString[0], ERROR_STR_MAX_LENGTH);
        HASSERT(strlen(&ErrorString[0]) > 0);
        string ErrCodeMess(ErrorString.begin(), ErrorString.end());              
        
        WChar TempBuffer[2048]; 
        BeStringUtilities::Snwprintf(TempBuffer, message.c_str(), m_ErrorCode,
                             WString(ErrCodeMess.c_str(),false).c_str()); 
        message = WString(TempBuffer); 
        }

    WString exceptionName(exceptionID.m_str, true/*isUtf8*/);
    WPrintfString formatedMessage(L"%ls - [%ls]", message.c_str(), exceptionName.c_str());
    return formatedMessage;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HRFPDFException::Clone() const
    {
    return new HRFPDFException(*this);;
    }

#endif //IPP_HAVE_PDF_SUPPORT
