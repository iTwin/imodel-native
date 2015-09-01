//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPSParser
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPSException.h>
#include <Imagepp/all/h/HPAnode.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSException::HPSException(HFCPtr<HPANode> pi_pOffendingNode)
    : HPAException(pi_pOffendingNode)
    {
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HPSException::HPSException(const HPSException&     pi_rObj) : HPAException(pi_rObj)
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
WString HPSException::_BuildMessage(const ImagePPExceptions::StringId& rsID) const
    {
    return HPAException::_BuildMessage(rsID);
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSTypeMismatchException::HPSTypeMismatchException(HFCPtr<HPANode>    pi_pOffendingNode, ExpectedType        pi_ExpectedType)
    : HPSException(pi_pOffendingNode)
    {
    m_ExpectedType   = pi_ExpectedType;
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HPSTypeMismatchException::HPSTypeMismatchException(const HPSTypeMismatchException&     pi_rObj) : HPSException(pi_rObj)
 {
     m_ExpectedType = pi_rObj.m_ExpectedType;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPSTypeMismatchException::~HPSTypeMismatchException()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HPSTypeMismatchException::Clone() const
    {
    return new HPSTypeMismatchException(*this);
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HPSTypeMismatchException::GetExceptionMessage() const
    {
    return HPSException::_BuildMessage(ImagePPExceptions::HPSTypeMismatch());
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const HPSTypeMismatchException::ExpectedType HPSTypeMismatchException::GetExpectedType() const
    {
    return m_ExpectedType;
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSOutOfRangeException::HPSOutOfRangeException(HFCPtr<HPANode>    pi_pOffendingNode,
                                                      double            pi_Lower,
                                                      double           pi_Upper)
    : HPSException(pi_pOffendingNode)
    {
    m_Lower            = pi_Lower;
    m_Upper            = pi_Upper; 
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HPSOutOfRangeException::HPSOutOfRangeException(const HPSOutOfRangeException&     pi_rObj) : HPSException(pi_rObj)
 {
     m_Lower = pi_rObj.m_Lower;
     m_Upper = pi_rObj.m_Upper;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPSOutOfRangeException::~HPSOutOfRangeException()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HPSOutOfRangeException::Clone() const
    {
    return new HPSOutOfRangeException(*this);
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HPSOutOfRangeException::GetExceptionMessage() const
    {
    return HPSException::_BuildMessage(ImagePPExceptions::HPSOutOfRange());
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const double HPSOutOfRangeException::GetLower() const
    {
    return m_Lower;
    }
//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const double HPSOutOfRangeException::GetUpper() const
    {
    return m_Upper;
    }
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HPSAlreadyDefinedException::HPSAlreadyDefinedException(HFCPtr<HPANode>    pi_pOffendingNode,
                                                              const WString&    pi_rName)
    : HPSException(pi_pOffendingNode)
    {
    m_Name              = pi_rName;   
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HPSAlreadyDefinedException::HPSAlreadyDefinedException(const HPSAlreadyDefinedException&     pi_rObj) : HPSException(pi_rObj)
 {
     m_Name = pi_rObj.m_Name;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HPSAlreadyDefinedException::~HPSAlreadyDefinedException()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HPSAlreadyDefinedException::Clone() const
    {
    return new HPSAlreadyDefinedException(*this);
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HPSAlreadyDefinedException::GetExceptionMessage() const
    {
    return HPAException::_BuildMessage(ImagePPExceptions::HPSAlreadyDefined());
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
WStringCR HPSAlreadyDefinedException::GetName() const
    {
    return m_Name;
    }
