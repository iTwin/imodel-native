//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSException.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HPSException
//---------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//      HPSException                    (Info struct : -)
//            HPSTypeMismatchException    (Info struct : HPSTypeMismatchExInfo)
//            HPSOutOfRangeException        (Info struct : HPSOutOfRangeExInfo)
//            HPSAlreadyDefinedException    (Info struct : HPSAlreadyDefinedExInfo)
//----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HPAException.h>

//----------------------------------------------------------------------------
// Class HPSException
//----------------------------------------------------------------------------
class HPSException : public HPAException
    {
    HDECLARE_CLASS_ID(6220, HPAException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HPSException(HFCPtr<HPANode>   pi_pOffendingNode);

    HPSException(ExceptionID       pi_ExceptionID,
                 HFCPtr<HPANode>   pi_pOffendingNode,
                 bool               pi_CreateExInfo = true);
    virtual        ~HPSException();
    HPSException& operator=(const HPSException& pi_rObj);
    HPSException(const HPSException& pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HPSTypeMismatchException
//----------------------------------------------------------------------------
class HPSTypeMismatchException  : public HPSException
    {
    HDECLARE_CLASS_ID(6221, HPSException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:

    enum ExpectedType
        {
        STRING,
        NUMBER,
        OBJECT,
        OBJECT_OR_NUMBER,
        INTEGER
        };

    // Primary methods.
    // Contructor and destructor.
    HPSTypeMismatchException(HFCPtr<HPANode>    pi_pOffendingNode,
                             ExpectedType        pi_ExpectedType);

    HPSTypeMismatchException(ExceptionID        pi_ExceptionID,
                             HFCPtr<HPANode>    pi_pOffendingNode,
                             ExpectedType        pi_ExpectedType);
    virtual        ~HPSTypeMismatchException();
    HPSTypeMismatchException& operator=(const HPSTypeMismatchException& pi_rObj);
    HPSTypeMismatchException(const HPSTypeMismatchException& pi_rObj);
    };


//----------------------------------------------------------------------------
// Class HPSOutOfRangeException
//----------------------------------------------------------------------------
class HPSOutOfRangeException : public HPSException
    {
    HDECLARE_CLASS_ID(6222, HPSException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:

    // Primary methods.
    // Contructor and destructor.
    HPSOutOfRangeException(HFCPtr<HPANode>         pi_pOffendingNode,
                           double                pi_Lower,
                           double              pi_Upper);
    virtual        ~HPSOutOfRangeException();
    HPSOutOfRangeException& operator=(const HPSOutOfRangeException& pi_rObj);
    HPSOutOfRangeException(const HPSOutOfRangeException& pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HPSAlreadyDefinedException
//----------------------------------------------------------------------------
class HPSAlreadyDefinedException : public HPSException
    {
    HDECLARE_CLASS_ID(6223, HPSException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:

    // Primary methods.
    // Contructor and destructor.
    HPSAlreadyDefinedException(HFCPtr<HPANode>        pi_pOffendingNode,
                               const WString&       pi_rName);
    virtual        ~HPSAlreadyDefinedException();
    HPSAlreadyDefinedException& operator=(const HPSAlreadyDefinedException& pi_rObj);
    HPSAlreadyDefinedException(const HPSAlreadyDefinedException& pi_rObj);
    };

