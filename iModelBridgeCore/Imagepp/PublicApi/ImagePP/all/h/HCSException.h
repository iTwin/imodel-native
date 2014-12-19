//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSException.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation of the HCS exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//        HFCDeviceException    (Info struct : HFCDeviceExInfo)
//            HCSException    (Info struct : HFCDeviceExInfo)
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"

//----------------------------------------------------------------------------
// Class HCSException
//----------------------------------------------------------------------------
class HCSException : public HFCDeviceException
    {
    HDECLARE_CLASS_ID(6070, HFCDeviceException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HCSException(ExceptionID    pi_ExceptionID,
                 const WString& pi_rDeviceName);

    HCSException(const WString& pi_rDeviceName);
    virtual       ~HCSException();
    HCSException& operator=(const HCSException& pi_rObj);
    HCSException(const HCSException& pi_rObj);
    };

