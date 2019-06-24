//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFException.h>

//-----------------------------------------------------------------------------
// Class HRFException
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFException::HRFException(const Utf8String&    pi_rFileName)
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
Utf8String HRFException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    return  HFCFileException::_BuildMessage(pi_rsID);
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFFileParameterException::HRFFileParameterException(const Utf8String& pi_rFileName, const Utf8String& pi_rParamName)
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
Utf8String HRFFileParameterException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    Utf8String rawMessage;
    if(HasFilenameInMessageString())
        {
        rawMessage = Utf8PrintfString(GetRawMessageFromResource(pi_rsID).c_str(), m_FileName.c_str(), m_ParameterName.c_str());
        }
    else
        {
        rawMessage = Utf8PrintfString(GetRawMessageFromResource(pi_rsID).c_str(), m_ParameterName.c_str());
        }

    Utf8String exceptionName(pi_rsID.m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
    return message; 
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
Utf8StringCR HRFFileParameterException::GetParameterName() const
    {
    return m_ParameterName;
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFChildFileException::HRFChildFileException(const Utf8String&    pi_rParentFileName,
                                                    const Utf8String&    pi_rChildFileName)
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
Utf8StringCR HRFChildFileException::GetChildFileName() const
    {
    return m_ChildFileName;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFCannotOpenChildFileException::HRFCannotOpenChildFileException(const Utf8String&    pi_rParentFileName,
                                                    const Utf8String&    pi_rChildFileName)
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
Utf8String HRFCannotOpenChildFileException::GetExceptionMessage() const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HRFCannotOpenChildFile()).c_str(), m_ChildFileName.c_str());
    Utf8String exceptionName(ImagePPExceptions::HRFCannotOpenChildFile().m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFTiffErrorException::HRFTiffErrorException(const Utf8String&        pi_rFileName, const HTIFFError&    pi_rErrorInfo)
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
Utf8String HRFTiffErrorException::GetExceptionMessage() const
    {
    Utf8String HTIFFErrorMsg;

    //Get the detailed HTIFF error message from the localized resource and format it.
    m_ErrorInfo.GetErrorMsg(HTIFFErrorMsg);

    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HRFTiffError()).c_str(), HTIFFErrorMsg.c_str());
    Utf8String exceptionName(ImagePPExceptions::HRFTiffError().m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());

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
HRFChildFileParameterException::HRFChildFileParameterException(const Utf8String& pi_rFileName,
                                                                      const Utf8String& pi_rChildFileName,
                                                                      const Utf8String& pi_rParameterName)
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
Utf8String HRFChildFileParameterException::GetExceptionMessage() const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HRFChildFileParameter()).c_str(), m_ChildFileName.c_str(), m_ParameterName.c_str());

    Utf8String exceptionName(ImagePPExceptions::HRFChildFileParameter().m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());

    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
Utf8StringCR HRFChildFileParameterException::GetParameterName() const
    {
    return m_ParameterName;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFInvalidNewFileDimensionException::HRFInvalidNewFileDimensionException(const Utf8String& pi_rFileName,
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
Utf8String HRFInvalidNewFileDimensionException::GetExceptionMessage() const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HRFInvalidNewFileDimension()).c_str(), m_WidthLimit, m_HeightLimit);
    Utf8String exceptionName(ImagePPExceptions::HRFInvalidNewFileDimension().m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
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
