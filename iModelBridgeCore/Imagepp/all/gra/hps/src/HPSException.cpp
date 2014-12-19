//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPSParser
//---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HPSException.h>
#include <Imagepp/all/h/HPAnode.h>

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HPSException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HPSTypeMismatchException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HPSOutOfRangeException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HPSAlreadyDefinedException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSException::HPSException(HFCPtr<HPANode> pi_pOffendingNode)
    : HPAException(HPS_EXCEPTION, pi_pOffendingNode)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSException::HPSException(ExceptionID        pi_ExceptionID,
                                  HFCPtr<HPANode>    pi_pOffendingNode,
                                  bool                pi_CreateExInfo)
    : HPAException(pi_ExceptionID, pi_pOffendingNode, pi_CreateExInfo)
    {
    HPRECONDITION((pi_ExceptionID >= HPS_BASE) && (pi_ExceptionID < HPS_SEPARATOR_ID));
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HPSException::HPSException(const HPSException& pi_rObj)
    : HPAException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPSException::~HPSException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HPSException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPAException::FormatExceptionMessage(pio_rMessage);
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HPSException& HPSException::operator=(const HPSException& pi_rObj)
    {
    if (this != &pi_rObj)
        HPAException::operator=(pi_rObj);

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSTypeMismatchException::HPSTypeMismatchException(HFCPtr<HPANode>    pi_pOffendingNode,
                                                          ExpectedType        pi_ExpectedType)
    : HPSException(HPS_TYPE_MISMATCH_EXCEPTION, pi_pOffendingNode, false)
    {
    m_pInfo = new HPSTypeMismatchExInfo();
    ((HPSTypeMismatchExInfo*)m_pInfo)->m_pOffendingNode = pi_pOffendingNode;
    ((HPSTypeMismatchExInfo*)m_pInfo)->m_ExpectedType   = (unsigned short)pi_ExpectedType;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSTypeMismatchException::HPSTypeMismatchException(ExceptionID        pi_ExceptionID,
                                                          HFCPtr<HPANode>    pi_pOffendingNode,
                                                          ExpectedType      pi_ExpectedType)
    : HPSException(pi_ExceptionID, pi_pOffendingNode, false)
    {
    m_pInfo = new HPSTypeMismatchExInfo();
    ((HPSTypeMismatchExInfo*)m_pInfo)->m_pOffendingNode = pi_pOffendingNode;
    ((HPSTypeMismatchExInfo*)m_pInfo)->m_ExpectedType   = (unsigned short)pi_ExpectedType;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HPSTypeMismatchException::HPSTypeMismatchException(const HPSTypeMismatchException& pi_rObj)
    : HPSException((ExceptionID)pi_rObj.GetID(), HFCPtr<HPANode>(), false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPSTypeMismatchExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPSTypeMismatchException::~HPSTypeMismatchException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HPSTypeMismatchException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPAException::FormatExceptionMessage(pio_rMessage);
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HPSTypeMismatchException& HPSTypeMismatchException::operator=(const HPSTypeMismatchException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPSTypeMismatchExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSOutOfRangeException::HPSOutOfRangeException(HFCPtr<HPANode>    pi_pOffendingNode,
                                                      double            pi_Lower,
                                                      double           pi_Upper)
    : HPSException(HPS_OUT_OF_RANGE_EXCEPTION, pi_pOffendingNode, false)
    {
    m_pInfo = new HPSOutOfRangeExInfo();
    ((HPSOutOfRangeExInfo*)m_pInfo)->m_pOffendingNode = pi_pOffendingNode;
    ((HPSOutOfRangeExInfo*)m_pInfo)->m_Lower            = pi_Lower;
    ((HPSOutOfRangeExInfo*)m_pInfo)->m_Upper            = pi_Upper;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HPSOutOfRangeException::HPSOutOfRangeException(const HPSOutOfRangeException& pi_rObj)
    : HPSException((ExceptionID)pi_rObj.GetID(), HFCPtr<HPANode>(), false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPSOutOfRangeExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPSOutOfRangeException::~HPSOutOfRangeException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HPSOutOfRangeException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPAException::FormatExceptionMessage(pio_rMessage);
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HPSOutOfRangeException& HPSOutOfRangeException::operator=(const HPSOutOfRangeException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPSOutOfRangeExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSAlreadyDefinedException::HPSAlreadyDefinedException(HFCPtr<HPANode>    pi_pOffendingNode,
                                                              const WString&    pi_rName)
    : HPSException(HPS_ALREADY_DEFINED_EXCEPTION, pi_pOffendingNode, false)
    {
    m_pInfo = new HPSAlreadyDefinedExInfo();
    ((HPSAlreadyDefinedExInfo*)m_pInfo)->m_pOffendingNode = pi_pOffendingNode;
    ((HPSAlreadyDefinedExInfo*)m_pInfo)->m_Name              = pi_rName;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HPSAlreadyDefinedException::HPSAlreadyDefinedException(const HPSAlreadyDefinedException& pi_rObj)
    : HPSException((ExceptionID)pi_rObj.GetID(), HFCPtr<HPANode>(), false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPSAlreadyDefinedExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPSAlreadyDefinedException::~HPSAlreadyDefinedException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HPSAlreadyDefinedException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPAException::FormatExceptionMessage(pio_rMessage);
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HPSAlreadyDefinedException& HPSAlreadyDefinedException::operator=(const HPSAlreadyDefinedException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HPSAlreadyDefinedExInfo)
        }

    return *this;
    }
