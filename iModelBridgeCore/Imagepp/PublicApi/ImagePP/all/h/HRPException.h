//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPException.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the HRF exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//      HRPException    (Info struct : -)
//----------------------------------------------------------------------------

#pragma once

#include "HFCException.h"


//----------------------------------------------------------------------------
// Class HRPException
//----------------------------------------------------------------------------
class _HDLLg HRPException : public HFCException
    {
    HDECLARE_CLASS_ID(6300, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public :

    HRPException();
    HRPException(ExceptionID pi_ExceptionID);
    virtual ~HRPException();

    HRPException(const HRPException& pi_rObj);
    HRPException&   operator=(const HRPException& pi_rObj);
    };


