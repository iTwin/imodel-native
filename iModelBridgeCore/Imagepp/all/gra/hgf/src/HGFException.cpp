//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGFException.h>



//---------------------------------------------------------------------------
// methods for class HGFException
//---------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGFException::HGFException()
    : HFCException()
    {
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HGFException::HGFException(const HGFException&     pi_rObj) : HFCException(pi_rObj)
 {
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGFException::~HGFException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HGFException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    WString exceptionName(pi_rsID.m_str, true /*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", GetRawMessageFromResource(pi_rsID).c_str(), exceptionName.c_str());
    return message;
    }
//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HGFmzGCoordException::HGFmzGCoordException(int32_t pi_StatusCode)
    :HGFException()
    {
    m_StatusCode = pi_StatusCode;
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HGFmzGCoordException::HGFmzGCoordException(const HGFmzGCoordException&     pi_rObj) : HGFException(pi_rObj)
 {
     m_StatusCode = pi_rObj.m_StatusCode;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGFmzGCoordException::~HGFmzGCoordException()
    {
    }
 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HGFmzGCoordException::Clone() const
    {
    return new HGFmzGCoordException(*this);
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HGFmzGCoordException::GetExceptionMessage() const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HGFmzGCoordException()).c_str(), m_StatusCode);
    WString exceptionName(ImagePPExceptions::HGFmzGCoordException().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }
//-----------------------------------------------------------------------------
// public
// Return the exception synchro object type
//-----------------------------------------------------------------------------
const int32_t HGFmzGCoordException::GetStatusCode() const
{
    return m_StatusCode;
}
//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred from baseGeoCoord plugIn.
//-----------------------------------------------------------------------------
WString HGFmzGCoordException::GetErrorText() const
    {
    WString errorStr;
    GCSServices->_GetErrorMessage (errorStr, m_StatusCode);

    return errorStr;
    }
