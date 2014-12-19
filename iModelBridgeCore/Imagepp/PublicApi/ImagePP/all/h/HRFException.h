//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation of the HRF exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//  HFCFileException                            (Info struct : HFCFileExInfo)
//        HRFException                            (Info struct : HFCFileExInfo)
//            HRFWMSException                        (Info struct : HRFWMSExInfo)
//          HRFChildFileException               (Info struct : HRFChildFileExInfo)
//              HRFChildFileParameterException  (Info struct : HRFChildFileParamExInfo)
//          HRFFileParameterException           (Info struct : HRFFileParameterExInfo)
//      HRFTiffErrorException                   (Info struct : HRFTiffErrorExInfo)
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"
#include "HTIFFUtils.h"

//----------------------------------------------------------------------------------
//Exception information struct for HRFTiffErrorException
//----------------------------------------------------------------------------------
struct HRFTiffErrorExInfo : public HFCFileExInfo
    {
    HTIFFError m_ErrorInfo;
    };

//---------------------------------------------------------HRFException-------------------
// Class HRFException
//----------------------------------------------------------------------------
class HRFException : public HFCFileException
    {
    HDECLARE_CLASS_ID(6100, HFCFileException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    _HDLLg HRFException            (const WString&          pi_rFileName);
    _HDLLg HRFException            (ExceptionID             pi_ExceptionID,
                                    const WString&          pi_rFileName,
                                    bool                   pi_CreateExInfo = true);
    _HDLLg virtual         ~HRFException();
    _HDLLg HRFException& operator=        (const HRFException&     pi_rObj);
    _HDLLg HRFException                   (const HRFException&     pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HRFWMSException
//----------------------------------------------------------------------------
class HRFWMSException : public HRFException
    {
    HDECLARE_CLASS_ID(6101, HRFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HRFWMSException(ExceptionID    pi_ExceptionID,
                    const WString& pi_rFileName);
    HRFWMSException(const HRFWMSException& pi_rObj);
    virtual           ~HRFWMSException();
    HRFWMSException& operator=(const HRFWMSException& pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HRFInvalidParamValueException
//----------------------------------------------------------------------------
class HRFFileParameterException : public HRFException
    {
    HDECLARE_CLASS_ID(6102, HRFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HRFFileParameterException(ExceptionID    pi_ExceptionID,
                              const WString& pi_rFileName,
                              const WString& pi_rParamName);
    HRFFileParameterException(const HRFFileParameterException& pi_rObj);
    virtual        ~HRFFileParameterException();
    HRFFileParameterException& operator=(const HRFFileParameterException& pi_rObj);
    };


//----------------------------------------------------------------------------
// Class HRFChildFileException
//----------------------------------------------------------------------------
class HRFChildFileException : public HRFException
    {
    HDECLARE_CLASS_ID(6103, HRFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HRFChildFileException(ExceptionID     pi_ExceptionId);

    HRFChildFileException(ExceptionID     pi_ExceptionId,
                          const WString& pi_rParentFileName,
                          const WString& pi_rChildFileName);
    HRFChildFileException(const HRFChildFileException& pi_rObj);
    virtual        ~HRFChildFileException();
    HRFChildFileException& operator=(const HRFChildFileException& pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HRFChildFileParameterException
//----------------------------------------------------------------------------
class HRFChildFileParameterException : public HRFChildFileException
    {
    HDECLARE_CLASS_ID(6104, HRFChildFileException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HRFChildFileParameterException(ExceptionID    pi_ExceptionID,
                                   const WString& pi_rParentFileName,
                                   const WString& pi_rChildFileName,
                                   const WString& pi_rParamName);
    HRFChildFileParameterException(const HRFChildFileParameterException& pi_rObj);
    virtual        ~HRFChildFileParameterException();
    HRFChildFileParameterException& operator=(const HRFChildFileParameterException& pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HRFTiffErrorException
//----------------------------------------------------------------------------
class HRFTiffErrorException : public HRFException
    {
    HDECLARE_CLASS_ID(6105, HRFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HRFTiffErrorException(const WString&    pi_rFileName,
                          const HTIFFError& pi_rErrInfo);
    HRFTiffErrorException(const HRFTiffErrorException& pi_rObj);
    virtual        ~HRFTiffErrorException();
    HRFTiffErrorException& operator=(const HRFTiffErrorException& pi_rObj);
    };

//-----------------------------------------------------------------------------
// HRFInvalidNewFileDimensionException class
//-----------------------------------------------------------------------------
class HRFInvalidNewFileDimensionException : public HRFException
    {
    HDECLARE_CLASS_ID(6108, HRFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public :
    HRFInvalidNewFileDimensionException(ExceptionID    pi_ExceptionID,
                                        const WString& pi_rFileName,
                                        uint64_t      pi_WidthLimit,
                                        uint64_t      pi_HeightLimit);

    virtual                  ~HRFInvalidNewFileDimensionException();
    HRFInvalidNewFileDimensionException(const HRFInvalidNewFileDimensionException& pi_rObj);
    HRFInvalidNewFileDimensionException& operator=(const HRFInvalidNewFileDimensionException& pi_rObj);
    };
