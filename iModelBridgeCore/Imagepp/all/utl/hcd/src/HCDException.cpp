//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// methods for HCD exception
//-----------------------------------------------------------------------------
#include <ImageppInternal.h>

#include <ImagePP/all/h/HCDException.h>
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
Utf8String HCDException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    Utf8String exceptionName(pi_rsID.m_str);
    Utf8PrintfString message("%s - [%s]", GetRawMessageFromResource(pi_rsID).c_str(), exceptionName.c_str());
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
HCDIJLErrorException::HCDIJLErrorException(int16_t pi_IJLErrorCode)
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
Utf8String HCDIJLErrorException::GetExceptionMessage() const
    {
     return HCDException::_BuildMessage(ImagePPExceptions::HCDIJLError());
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const int16_t HCDIJLErrorException::GetErrorCode() const
    {
    return m_IJLErrorCode;
    }
