//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//      HGFException                    (Info struct : -)
//          HGFmzGCoordException        (Info struct : HGFmzGCoordExInfo)
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"

/*----------------------------------------------------------------------------+
|Class HGFException
|
|Description: Exception thrown when an error occurred in HGF.
|
+----------------------------------------------------------------------------*/
class _HDLLg HGFException : public HFCException
    {
    HDECLARE_CLASS_ID(6011, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()
public:
    // Primary methods.
    // Contructor and destructor.
    HGFException(ExceptionID    pi_ExceptionID);
    virtual        ~HGFException();
    HGFException& operator=(const HGFException& pi_rObj);
    HGFException(const HGFException& pi_rObj);
    };


/*----------------------------------------------------------------------------+
|Class HGFmzGCoordException
|
|Description: Exception thrown when error occurred in mzGCoord wrapper API.
|
+----------------------------------------------------------------------------*/
class _HDLLg HGFmzGCoordException : public HGFException
    {
    HDECLARE_CLASS_ID(6021, HGFException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()
public:
    // Primary methods.
    // Contructor and destructor.
    HGFmzGCoordException(int32_t       pi_StatusCode);
    virtual        ~HGFmzGCoordException();
    HGFmzGCoordException& operator=(const HGFmzGCoordException& pi_rObj);
    HGFmzGCoordException(const HGFmzGCoordException& pi_rObj);
    WString GetErrorText() const;
    };
