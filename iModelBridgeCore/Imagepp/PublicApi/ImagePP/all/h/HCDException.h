//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the HRF exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//      HCDException                (Info struct : -)
//          HCDIJLErrorException    (Info struct : HCDIJLErrorExInfo)
//----------------------------------------------------------------------------

#pragma once

#include "HFCException.h"

//----------------------------------------------------------------------------
// Class HCDException
//----------------------------------------------------------------------------
class HCDException : public HFCException
    {
    HDECLARE_CLASS_ID(6400, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()
public:
    // Primary methods.
    // Contructor and destructor.
    HCDException();
    HCDException(ExceptionID pi_ExceptionID);
    virtual        ~HCDException();
    HCDException& operator=(const HCDException& pi_rObj);
    HCDException(const HCDException& pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HCDException
//----------------------------------------------------------------------------
class HCDIJLErrorException : public HCDException
    {
    HDECLARE_CLASS_ID(6401, HCDException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()
public:
    // Primary methods.
    // Contructor and destructor.
    HCDIJLErrorException(short pi_IJLErrorCode);
    virtual        ~HCDIJLErrorException();
    HCDIJLErrorException& operator=(const HCDIJLErrorException& pi_rObj);
    HCDIJLErrorException(const HCDIJLErrorException& pi_rObj);
    };
