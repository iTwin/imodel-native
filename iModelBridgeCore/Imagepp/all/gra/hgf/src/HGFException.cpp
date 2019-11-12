//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HGFException.h>



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
Utf8String HGFException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    Utf8PrintfString message("%s - [%s]", GetRawMessageFromResource(pi_rsID).c_str(), pi_rsID.m_str);
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
Utf8String HGFmzGCoordException::GetExceptionMessage() const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HGFmzGCoordException()).c_str(), m_StatusCode);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), ImagePPExceptions::HGFmzGCoordException().m_str);
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
Utf8String HGFmzGCoordException::GetErrorText() const
    {
    WString errorStr;
    GeoCoordinates::BaseGCS::GetErrorMessage (errorStr, m_StatusCode);

    return Utf8String(errorStr);
    }
