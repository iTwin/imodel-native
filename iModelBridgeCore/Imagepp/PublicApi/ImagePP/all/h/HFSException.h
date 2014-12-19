//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSException.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//      HFSException                    (Info struct : -)
//          HFSInvalidPathException        (Info struct : -)
//----------------------------------------------------------------------------

#pragma once

#include "HFCException.h"

//----------------------------------------------------------------------------
// Class HFSException
//----------------------------------------------------------------------------
class _HDLLu HFSException : public HFCException
    {
    HDECLARE_CLASS_ID(5050, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()
public:
    // Primary methods.
    // Contructor and destructor.
    HFSException();
    HFSException(ExceptionID    pi_ExceptionID);
    virtual ~HFSException();
    HFSException& operator=(const HFSException& pi_rObj);
    HFSException(const HFSException& pi_rObj);
    };

//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException                                    (Info struct : -)
//      HFSException                                (Info struct : HFCFileExInfo)
//            HFSHIBPInvalidResponseException            (Info struct : HFSHIBPInvalidResponseExInfo)
//            HFSHIBPErrorException                    (Info struct : HFSHIBPErrorExInfo)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Class HFSInvalidPathException
//----------------------------------------------------------------------------
class _HDLLu HFSInvalidPathException : public HFSException
    {
    HDECLARE_CLASS_ID(5051, HFSException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()
public:
    // Primary methods.
    // Contructor and destructor.
    HFSInvalidPathException(const WString& pi_rPath);
    HFSInvalidPathException(ExceptionID pi_ExceptionID, const WString& pi_rPath);
    virtual        ~HFSInvalidPathException();
    HFSInvalidPathException& operator=(const HFSInvalidPathException& pi_rObj);
    HFSInvalidPathException(const HFSInvalidPathException& pi_rObj);
    };


/**---------------------------------------------------------------------------
Class to use when the request respond was invalid
----------------------------------------------------------------------------*/
class HFSHIBPInvalidResponseException : public HFSException
    {
    HDECLARE_CLASS_ID(5070, HFSException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HFSHIBPInvalidResponseException(const WString& pi_rRequest);
    virtual        ~HFSHIBPInvalidResponseException();
    HFSHIBPInvalidResponseException& operator=(const HFSHIBPInvalidResponseException& pi_rObj);
    HFSHIBPInvalidResponseException(const HFSHIBPInvalidResponseException& pi_rObj);
    };

/**---------------------------------------------------------------------------
Class to use when the protocol HIBP return an error
----------------------------------------------------------------------------*/
class HFSHIBPErrorException : public HFSException
    {
    HDECLARE_CLASS_ID(5071, HFSException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HFSHIBPErrorException(const WString& pi_rErrorMsg);
    virtual        ~HFSHIBPErrorException();
    HFSHIBPErrorException& operator=(const HFSHIBPErrorException& pi_rObj);
    HFSHIBPErrorException(const HFSHIBPErrorException& pi_rObj);
    };