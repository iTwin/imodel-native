//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>

#include <ImagePP/all/h/HRFException.h>

//-----------------------------------------------------------------------------
// Class HRFException
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFException::HRFException(const WString&    pi_rFileName)
    : HFCFileException(pi_rFileName)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFException::HRFException(const HRFException&     pi_rObj) : HFCFileException(pi_rObj)
 {
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFException::~HRFException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HRFException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    return  HFCFileException::_BuildMessage(pi_rsID);
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFFileParameterException::HRFFileParameterException(const WString& pi_rFileName, const WString& pi_rParamName)
    : HRFException(pi_rFileName)
    {
        m_ParameterName = pi_rParamName;
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFFileParameterException::HRFFileParameterException(const HRFFileParameterException&     pi_rObj) : HRFException(pi_rObj)
 {
     m_ParameterName = pi_rObj.m_ParameterName;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFFileParameterException::~HRFFileParameterException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HRFFileParameterException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    WPrintfString rawMessage = L"";
    if(HasFilenameInMessageString())
        {
        rawMessage = WPrintfString(GetRawMessageFromResource(pi_rsID).c_str(), m_FileName.c_str(), m_ParameterName.c_str());
        }
    else
        {
        rawMessage = WPrintfString(GetRawMessageFromResource(pi_rsID).c_str(), m_ParameterName.c_str());
        }

    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message; 
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
WStringCR HRFFileParameterException::GetParameterName() const
    {
    return m_ParameterName;
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFChildFileException::HRFChildFileException(const WString&    pi_rParentFileName,
                                                    const WString&    pi_rChildFileName)
    : HRFException(pi_rParentFileName)
    {
    m_ChildFileName = pi_rChildFileName; 
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFChildFileException::HRFChildFileException(const HRFChildFileException&     pi_rObj) : HRFException(pi_rObj)
 {
     m_ChildFileName = pi_rObj.m_ChildFileName;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFChildFileException::~HRFChildFileException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
WStringCR HRFChildFileException::GetChildFileName() const
    {
    return m_ChildFileName;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFCannotOpenChildFileException::HRFCannotOpenChildFileException(const WString&    pi_rParentFileName,
                                                    const WString&    pi_rChildFileName)
    : HRFChildFileException(pi_rParentFileName,pi_rChildFileName)
    {
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFCannotOpenChildFileException::HRFCannotOpenChildFileException(const HRFCannotOpenChildFileException&     pi_rObj) : HRFChildFileException(pi_rObj)
 {
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFCannotOpenChildFileException::~HRFCannotOpenChildFileException()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HRFCannotOpenChildFileException::Clone() const
    {
    return new HRFCannotOpenChildFileException(*this);;
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HRFCannotOpenChildFileException::GetExceptionMessage() const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HRFCannotOpenChildFile()).c_str(), m_ChildFileName.c_str());
    WString exceptionName(ImagePPExceptions::HRFCannotOpenChildFile().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFTiffErrorException::HRFTiffErrorException(const WString&        pi_rFileName, const HTIFFError&    pi_rErrorInfo)
    : HRFException(pi_rFileName),
      m_ErrorInfo(pi_rErrorInfo)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFTiffErrorException::HRFTiffErrorException(const HRFTiffErrorException&     pi_rObj) 
    : HRFException(pi_rObj), m_ErrorInfo(pi_rObj.m_ErrorInfo)
 {
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFTiffErrorException::~HRFTiffErrorException()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HRFTiffErrorException::Clone() const
    {
    return new HRFTiffErrorException(*this);
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HRFTiffErrorException::GetExceptionMessage() const
    {
    WString HTIFFErrorMsg;

    //Get the detailed HTIFF error message from the localized resource and format it.
    m_ErrorInfo.GetErrorMsg(HTIFFErrorMsg);

    WPrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HRFTiffError()).c_str(), HTIFFErrorMsg.c_str());
    WString exceptionName(ImagePPExceptions::HRFTiffError().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());

    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const HTIFFError HRFTiffErrorException::GetError() const
    {
        return m_ErrorInfo;
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFChildFileParameterException::HRFChildFileParameterException(const WString& pi_rFileName,
                                                                      const WString& pi_rChildFileName,
                                                                      const WString& pi_rParameterName)
    : HRFChildFileException(pi_rFileName, pi_rChildFileName)
    {
    m_ParameterName = pi_rParameterName;  
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFChildFileParameterException::HRFChildFileParameterException(const HRFChildFileParameterException&     pi_rObj) : HRFChildFileException(pi_rObj)
 {
     m_ParameterName = pi_rObj.m_ParameterName;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFChildFileParameterException::~HRFChildFileParameterException()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HRFChildFileParameterException::Clone() const
    {
    return new HRFChildFileParameterException(*this);;
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HRFChildFileParameterException::GetExceptionMessage() const
    {
    WPrintfString rawMessage = (GetRawMessageFromResource(ImagePPExceptions::HRFChildFileParameter()).c_str(), 
        m_ChildFileName.c_str(), m_ParameterName.c_str());

    WString exceptionName(ImagePPExceptions::HRFChildFileParameter().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());

    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
WStringCR HRFChildFileParameterException::GetParameterName() const
    {
    return m_ParameterName;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFInvalidNewFileDimensionException::HRFInvalidNewFileDimensionException(const WString& pi_rFileName,
                                                                                uint64_t      pi_WidthLimit,
                                                                                uint64_t      pi_HeightLimit)
    : HRFException(pi_rFileName)
    {
    m_WidthLimit  = pi_WidthLimit;
    m_HeightLimit = pi_HeightLimit;
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HRFInvalidNewFileDimensionException::HRFInvalidNewFileDimensionException(const HRFInvalidNewFileDimensionException&     pi_rObj) : HRFException(pi_rObj)
 {
     m_WidthLimit = pi_rObj.m_WidthLimit;
     m_HeightLimit = pi_rObj.m_HeightLimit;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFInvalidNewFileDimensionException::~HRFInvalidNewFileDimensionException()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HRFInvalidNewFileDimensionException::Clone() const
    {
    return new HRFInvalidNewFileDimensionException(*this);;
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HRFInvalidNewFileDimensionException::GetExceptionMessage() const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HRFInvalidNewFileDimension()).c_str(), m_WidthLimit, m_HeightLimit);
    WString exceptionName(ImagePPExceptions::HRFInvalidNewFileDimension().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const uint64_t HRFInvalidNewFileDimensionException::GetWidthLimit() const
    {
    return m_WidthLimit;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const uint64_t HRFInvalidNewFileDimensionException::GetHeightLimit() const
    {
    return m_HeightLimit;
    }
