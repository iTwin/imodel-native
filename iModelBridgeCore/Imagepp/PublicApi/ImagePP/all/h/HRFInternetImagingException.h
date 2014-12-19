//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImagingException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetImagingException
//
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//      HRFInternetImagingException                    (Info struct : HRFInternetImagingExInfo)
//          HRFInternetImagingConnectionException    (Info struct : HRFInternetImagingConnectionExInfo)
//          HRFInternetImagingServerException        (Info struct : HRFInternetImagingServerExInfo)
//          HRFIIHandlerException                   (Info struct : HRFIIHandlerExInfo)
//-----------------------------------------------------------------------------

#pragma once

#include "HRFException.h"

//----------------------------------------------------------------------------------
//Exception information struct for HRFInternetImagingConnectionException
//----------------------------------------------------------------------------------
struct HRFInternetImagingConnectionExInfo : public HFCFileExInfo
    {
    unsigned short    m_ErrorType; //see HRFInternetImagingConnectionException::ErrorType
    };

//----------------------------------------------------------------------------------
//Exception information struct for HRFIIHandlerException
//----------------------------------------------------------------------------------
struct HRFIIHandlerExInfo : public HFCFileExInfo
    {
    unsigned short    m_HandlerID; //see HRFIIHandlerException::HandlerID
    };


//----------------------------------------------------------------------------------
//Exception information struct for HRFInternetImagingServerException
//----------------------------------------------------------------------------------
struct HRFInternetImagingServerExInfo : public HFCFileExInfo
    {
    uint32_t        m_Class;
    uint32_t        m_Number;
    WString         m_Object;
    WString         m_Message;
    };

//------------------------------------------------------------------------------
// HRFInternetImagingException - Base Exception Class
//------------------------------------------------------------------------------
class HRFInternetImagingException : public HFCFileException
    {
    HDECLARE_CLASS_ID(6150, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HRFInternetImagingException(ExceptionID            pi_ExceptionID,
                                const WString&      pi_rFileName,
                                bool                pi_CreateInfoStruct = true);
    virtual       ~HRFInternetImagingException();
    HRFInternetImagingException& operator=(const HRFInternetImagingException& pi_rObj);
    HRFInternetImagingException(const HRFInternetImagingException& pi_rObj);
    };


//------------------------------------------------------------------------------
// HRFInternetImagingConnectionException - Connection Exception
//------------------------------------------------------------------------------
class HRFInternetImagingConnectionException : public HRFInternetImagingException
    {
    HDECLARE_CLASS_ID(6151, HRFInternetImagingException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:

    enum ErrorType
        {
        CANNOT_CONNECT  = 0,
        CONNECTION_LOST,
        UNKNOWN
        };

    // Primary methods.
    // Contructor and destructor.
    HRFInternetImagingConnectionException(const WString& pi_rFileName,
                                          ErrorType         pi_ErrorType);
    virtual        ~HRFInternetImagingConnectionException();
    HRFInternetImagingConnectionException& operator=(const HRFInternetImagingConnectionException& pi_rObj);
    HRFInternetImagingConnectionException(const HRFInternetImagingConnectionException& pi_rObj);

    _HDLLg ErrorType   GetErrorType();
    };

//------------------------------------------------------------------------------
// HRFInternetImagingServerException - Server Exception
//------------------------------------------------------------------------------
class HRFInternetImagingServerException : public HRFInternetImagingException
    {
    HDECLARE_CLASS_ID(6152, HRFInternetImagingException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:

    // Primary methods.
    // Contructor and destructor.
    HRFInternetImagingServerException(const WString&    pi_rFileName,
                                      uint32_t          pi_Class,
                                      uint32_t          pi_Number,
                                      const WString&    pi_rObject,
                                      const WString&    pi_rMessage);
    virtual       ~HRFInternetImagingServerException();
    HRFInternetImagingServerException& operator=(const HRFInternetImagingServerException& pi_rObj);
    HRFInternetImagingServerException(const HRFInternetImagingServerException& pi_rObj);

    _HDLLg uint32_t   GetClass()  const;
    _HDLLg uint32_t   GetNumber() const;
    _HDLLg WString     GetObject() const;
    _HDLLg WString     GetMessageText() const;
    };


//------------------------------------------------------------------------------
// HRFIIHandlerException - Handler Exception
//------------------------------------------------------------------------------
class HRFIIHandlerException : public HRFInternetImagingException
    {
    HDECLARE_CLASS_ID(6153, HRFInternetImagingException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:

    enum HandlerID
        {
        TRANSFORM = 0,
        TILE,
        ROI,
        RESOLUTION,
        MAX_SIZE,
        IIP_VERSION,
        HIP_WORLD_ID,
        HIP_VERSION,
        HIP_TILE_SIZE,
        HIP_TILE,
        HIP_REDIRECT,
        HIP_PIXEL_TYPE,
        HIP_PAGE_NUMBER,
        HIP_IMAGE_SIZE,
        HIP_FILE_TYPE,
        HIP_FILE_INFO,
        HIP_FILE,
        HIP_FILE_COMPRESSION,
        HIP_BACKGROUND,
        HIP_PALETTE,
        ERROR_HANDLER,
        COMP_GROUP,
        COLOR_SPACE,
        ATTRIBUTES,
        ATTRIBUTE_NAMES
        };

    // Primary methods.
    // Contructor and destructor.
    HRFIIHandlerException(ExceptionID    pi_ExceptionID,
                          const WString& pi_rFileName,
                          HandlerID      pi_HandlerID);
    virtual                ~HRFIIHandlerException();
    HRFIIHandlerException& operator=(const HRFIIHandlerException& pi_rObj);
    HRFIIHandlerException(const HRFIIHandlerException& pi_rObj);
    };


