//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// methods for HCD exception
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDException.h>
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDException::HCDException()
    : HFCException()
    {
    }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDException::~HCDException()
    {
    }
//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HCDException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", GetRawMessageFromResource(pi_rsID).c_str(), exceptionName.c_str());
    return message;
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HCDException::HCDException(const HCDException&     pi_rObj) : HFCException(pi_rObj)
 {
 }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDIJLErrorException::HCDIJLErrorException(short pi_IJLErrorCode)
    : HCDException()
    {
    m_IJLErrorCode = pi_IJLErrorCode;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDIJLErrorException::~HCDIJLErrorException()
    {
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HCDIJLErrorException::HCDIJLErrorException(const HCDIJLErrorException&     pi_rObj) : HCDException(pi_rObj)
 {
     m_IJLErrorCode = pi_rObj.m_IJLErrorCode;
 }
 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HCDIJLErrorException::Clone() const
    {
    return new HCDIJLErrorException(*this);;
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HCDIJLErrorException::GetExceptionMessage() const
    {
     return HCDException::_BuildMessage(ImagePPExceptions::HCDIJLError());
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const short HCDIJLErrorException::GetErrorCode() const
    {
    return m_IJLErrorCode;
    }
