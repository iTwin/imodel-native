//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAException.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPAException
//---------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//      HPAException                    (Info struct : HPAExInfo)
//            HPAGenericException            (Info struct : HPAGenericExInfo)
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"
class HPANode;

//----------------------------------------------------------------------------
// Class HPAException
//----------------------------------------------------------------------------
class _HDLLu HPAException : public HFCException
    {
    HDECLARE_CLASS_ID(6200, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HPAException(HFCPtr<HPANode>&  pi_rpOffendingNode);

    HPAException(ExceptionID        pi_ExceptionID,
                 bool               pi_CreateInfoStruct = true);

    HPAException(ExceptionID        pi_ExceptionID,
                 HFCPtr<HPANode>&  pi_rpOffendingNode,
                 bool                pi_CreateInfoStruct = true);
    virtual        ~HPAException();
    HPAException& operator=(const HPAException& pi_rObj);
    HPAException(const HPAException& pi_rObj);

    WString         MakeErrorMsg() const;
    WString         GetErrorText() const;
    };

//----------------------------------------------------------------------------
// Class HPAGenericException
//----------------------------------------------------------------------------
class _HDLLu HPAGenericException : public HPAException
    {
    HDECLARE_CLASS_ID(6201, HPAException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HPAGenericException(HFCPtr<HPANode>&    pi_rpOffendingNode,
                        const WString&        pi_rMessage);
    HPAGenericException(const HPAException* pi_pObj);

    virtual        ~HPAGenericException();
    HPAGenericException& operator=(const HPAGenericException& pi_rObj);
    HPAGenericException(const HPAGenericException& pi_rObj);
    };

