//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HGFException.h>
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

//---------------------------------------------------------------------------
// methods for class HGFException
//---------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HGFException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HGFmzGCoordException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGFException::HGFException(ExceptionID        pi_ExceptionID)
    : HFCException(pi_ExceptionID)
    {
    HPRECONDITION((pi_ExceptionID >= HGF_BASE) && (pi_ExceptionID < HGF_SEPARATOR_ID));
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HGFException::HGFException(const HGFException& pi_rObj)
    : HFCException(pi_rObj)
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
void HGFException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HGFException& HGFException::operator=(const HGFException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        }
    return *this;
    }


//-----------------------------------------------------------------------------
// public
// ConstructorHGFmzGCoordException
//-----------------------------------------------------------------------------
HGFmzGCoordException::HGFmzGCoordException(int32_t pi_StatusCode)
    : HGFException(HGF_MZ_G_COORD_EXCEPTION)
    {
    m_pInfo = new HGFmzGCoordExInfo;
    ((HGFmzGCoordExInfo*)m_pInfo)->m_StatusCode = pi_StatusCode;
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HGFmzGCoordException::HGFmzGCoordException(const HGFmzGCoordException& pi_rObj)
    : HGFException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HGFmzGCoordExInfo)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGFmzGCoordException::~HGFmzGCoordException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HGFmzGCoordException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HGFmzGCoordException& HGFmzGCoordException::operator=(const HGFmzGCoordException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HGFmzGCoordExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred from baseGeoCoord plugIn.
//-----------------------------------------------------------------------------
WString HGFmzGCoordException::GetErrorText() const
    {
    WString errorStr;
    GCSServices->_GetErrorMessage (errorStr, ((HGFmzGCoordExInfo*)m_pInfo)->m_StatusCode);

    return errorStr;
    }