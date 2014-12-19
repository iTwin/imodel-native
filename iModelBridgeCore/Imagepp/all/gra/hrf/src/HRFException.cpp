//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <ImagePP/all/h/HRFException.h>

//-----------------------------------------------------------------------------
// Class HRFException
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFWMSException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFFileParameterException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFChildFileException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFTiffErrorException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFChildFileParameterException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFInvalidNewFileDimensionException)


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFException::HRFException(const WString&    pi_rFileName)
    : HFCFileException(HRF_EXCEPTION, pi_rFileName)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFException::HRFException(ExceptionID    pi_ExceptionID,
                                  const WString& pi_rFileName,
                                  bool          pi_CreateExInfo)
    : HFCFileException(pi_ExceptionID, pi_rFileName, pi_CreateExInfo)
    {
    HPRECONDITION((pi_ExceptionID >= HRF_BASE) && (pi_ExceptionID < HRF_SEPARATOR_ID));

    if ((GetID() == HRF_INVALID_SISTER_FILE_EXCEPTION) ||
        (GetID() == HRF_INVALID_TRANSFO_FOR_SISTER_FILE_EXCEPTION) ||
        (GetID() == HRF_SISTER_FILE_SLO_NOT_SUPPORTED_EXCEPTION) ||
        (GetID() == HRF_ERS_UNMATCH_REG_SPACE_COORD_TYPE_EXCEPTION) ||
        (GetID() == HRF_ERS_UNMATCH_REG_COORD_TYPE_EXCEPTION))
        {
        // NOTE: Bad!!! Is calling a virtual function inside the constructor.
        // Won't do what is expected for classes who enter here and that inherit
        // from HRFException reimplementing FormatExceptionMessage(...). Call
        // to FormatExceptionMessage should be moved to a member function
        // like GetExceptionMessage.
        GENERATE_FORMATTED_EXCEPTION_MSG()
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFException::HRFException(const HRFException& pi_rObj)
    : HFCFileException(pi_rObj)
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
void HRFException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    if ((GetID() == HRF_INVALID_SISTER_FILE_EXCEPTION) ||
        (GetID() == HRF_INVALID_TRANSFO_FOR_SISTER_FILE_EXCEPTION) ||
        (GetID() == HRF_SISTER_FILE_SLO_NOT_SUPPORTED_EXCEPTION) ||
        (GetID() == HRF_ERS_UNMATCH_REG_SPACE_COORD_TYPE_EXCEPTION) ||
        (GetID() == HRF_ERS_UNMATCH_REG_COORD_TYPE_EXCEPTION))
        {
        FORMAT_EXCEPTION_MSG(pio_rMessage, ((HFCFileExInfo*)m_pInfo)->m_FileName.c_str())
        }
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFException& HRFException::operator=(const HRFException& pi_rObj)
    {
    if (this != &pi_rObj)
        HFCFileException::operator=(pi_rObj);

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFWMSException::HRFWMSException(ExceptionID    pi_ExceptionID,
                                 const WString& pi_rFileName)
    : HRFException(pi_ExceptionID, pi_rFileName, false)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFWMSException::HRFWMSException(const HRFWMSException& pi_rObj)
    : HRFException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFWMSException::~HRFWMSException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFWMSException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFWMSException& HRFWMSException::operator=(const HRFWMSException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFFileParameterException::HRFFileParameterException(ExceptionID    pi_ExceptionID,
                                                            const WString& pi_rFileName,
                                                            const WString& pi_rParamName)
    : HRFException(pi_ExceptionID, pi_rFileName, false)
    {
    m_pInfo = new HRFFileParameterExInfo;
    ((HRFFileParameterExInfo*)m_pInfo)->m_FileName      = pi_rFileName;
    ((HRFFileParameterExInfo*)m_pInfo)->m_ParameterName = pi_rParamName;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFFileParameterException::HRFFileParameterException(const HRFFileParameterException& pi_rObj)
    : HRFException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFFileParameterExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
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
void HRFFileParameterException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);

    if ((GetID() == HRF_SISTER_FILE_INVALID_PARAM_VALUE_EXCEPTION) ||
        (GetID() == HRF_SISTER_FILE_MISSING_PARAMETER_EXCEPTION) ||
        (GetID() == HRF_SISTER_FILE_MISSING_PARAMETER_GROUP_EXCEPTION) ||
        (GetID() == HRF_ERS_DATA_FOUND_OUTSIDE_DATASET_HEADER_EXCEPTION))
        {
        FORMAT_EXCEPTION_MSG(pio_rMessage,
                             ((HRFFileParameterExInfo*)m_pInfo)->m_FileName.c_str(),
                             ((HRFFileParameterExInfo*)m_pInfo)->m_ParameterName.c_str())
        }
    else
        {
        FORMAT_EXCEPTION_MSG(pio_rMessage, ((HRFFileParameterExInfo*)m_pInfo)->m_ParameterName.c_str())
        }
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFFileParameterException& HRFFileParameterException::operator=(const HRFFileParameterException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFFileParameterExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFChildFileException::HRFChildFileException(ExceptionID pi_ExceptionId)
    : HRFException(pi_ExceptionId, L"", false)
    {
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFChildFileException::HRFChildFileException(ExceptionID     pi_ExceptionId,
                                                    const WString&    pi_rParentFileName,
                                                    const WString&    pi_rChildFileName)
    : HRFException(pi_ExceptionId, pi_rParentFileName, false)
    {
    m_pInfo = new HRFChildFileExInfo;
    ((HRFChildFileExInfo*)m_pInfo)->m_FileName      = pi_rParentFileName;
    ((HRFChildFileExInfo*)m_pInfo)->m_ChildFileName = pi_rChildFileName;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFChildFileException::HRFChildFileException(const HRFChildFileException& pi_rObj)
    : HRFException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFChildFileExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
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
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFChildFileException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage,
                         ((HRFChildFileExInfo*)m_pInfo)->m_ChildFileName.c_str())
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFChildFileException& HRFChildFileException::operator=(const HRFChildFileException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFChildFileExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFTiffErrorException::HRFTiffErrorException(const WString&        pi_rFileName,
                                                    const HTIFFError&    pi_rErrorInfo)
    : HRFException(HRF_TIFF_ERROR_EXCEPTION, pi_rFileName, false)
    {
    m_pInfo = new HRFTiffErrorExInfo;
    ((HRFTiffErrorExInfo*)m_pInfo)->m_FileName  = pi_rFileName;
    ((HRFTiffErrorExInfo*)m_pInfo)->m_ErrorInfo = pi_rErrorInfo;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFTiffErrorException::HRFTiffErrorException(const HRFTiffErrorException& pi_rObj)
    : HRFException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFTiffErrorExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFTiffErrorException::~HRFTiffErrorException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFTiffErrorException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);

    WString HTIFFErrorMsg;

    //Get the detailed HTIFF error message from the localized resource and format it.
    ((HRFTiffErrorExInfo*)m_pInfo)->m_ErrorInfo.GetErrorMsg(HTIFFErrorMsg);

    //Format the exception message with the detailed message
    FORMAT_EXCEPTION_MSG(pio_rMessage, HTIFFErrorMsg.c_str())
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFTiffErrorException& HRFTiffErrorException::operator=(const HRFTiffErrorException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFTiffErrorExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFChildFileParameterException::HRFChildFileParameterException(ExceptionID    pi_ExceptionID,
                                                                      const WString& pi_rFileName,
                                                                      const WString& pi_rChildFileName,
                                                                      const WString& pi_rParameterName)
    : HRFChildFileException(pi_ExceptionID)
    {
    m_pInfo = new HRFChildFileParamExInfo;
    ((HRFChildFileParamExInfo*)m_pInfo)->m_FileName      = pi_rFileName;
    ((HRFChildFileParamExInfo*)m_pInfo)->m_ChildFileName = pi_rChildFileName;
    ((HRFChildFileParamExInfo*)m_pInfo)->m_ParameterName = pi_rParameterName;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFChildFileParameterException::HRFChildFileParameterException(const HRFChildFileParameterException& pi_rObj)
    : HRFChildFileException((ExceptionID)pi_rObj.GetID())
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFChildFileParamExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFChildFileParameterException::~HRFChildFileParameterException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFChildFileParameterException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage,
                         ((HRFChildFileParamExInfo*)m_pInfo)->m_ChildFileName.c_str(),
                         ((HRFChildFileParamExInfo*)m_pInfo)->m_ParameterName.c_str())
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFChildFileParameterException& HRFChildFileParameterException::operator=(const HRFChildFileParameterException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFChildFileParamExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFInvalidNewFileDimensionException::HRFInvalidNewFileDimensionException(ExceptionID    pi_ExceptionID,
                                                                                const WString& pi_rFileName,
                                                                                uint64_t      pi_WidthLimit,
                                                                                uint64_t      pi_HeightLimit)
    : HRFException(pi_ExceptionID, pi_rFileName, false)
    {
    m_pInfo = new HRFInvalidNewFileDimExInfo;
    ((HRFInvalidNewFileDimExInfo*)m_pInfo)->m_FileName    = pi_rFileName;
    ((HRFInvalidNewFileDimExInfo*)m_pInfo)->m_WidthLimit  = pi_WidthLimit;
    ((HRFInvalidNewFileDimExInfo*)m_pInfo)->m_HeightLimit = pi_HeightLimit;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFInvalidNewFileDimensionException::HRFInvalidNewFileDimensionException(const HRFInvalidNewFileDimensionException& pi_rObj)
    : HRFException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFInvalidNewFileDimExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFInvalidNewFileDimensionException::~HRFInvalidNewFileDimensionException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFInvalidNewFileDimensionException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage,
                         ((HRFInvalidNewFileDimExInfo*)m_pInfo)->m_WidthLimit,
                         ((HRFInvalidNewFileDimExInfo*)m_pInfo)->m_HeightLimit)
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFInvalidNewFileDimensionException& HRFInvalidNewFileDimensionException::operator=(const HRFInvalidNewFileDimensionException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFInvalidNewFileDimExInfo)
        }

    return *this;
    }