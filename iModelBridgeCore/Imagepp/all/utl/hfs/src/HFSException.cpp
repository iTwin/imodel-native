//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/src/HFSException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFSException.h>

//:>-----------------------------------------------------------------------------
//:> methods for HFS exception classes.
//:>-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFSException::HFSException()
    : HFCException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HFSException::HFSException(const HFSException&     pi_rObj) : HFCException(pi_rObj)
 {
 }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFSException::~HFSException()
{
}

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HFSException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", GetRawMessageFromResource(pi_rsID).c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFSInvalidPathException::HFSInvalidPathException(const WString& pi_rPath) : HFSException()
    {
    m_Path = pi_rPath;   
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HFSInvalidPathException::HFSInvalidPathException(const HFSInvalidPathException&     pi_rObj) : HFSException(pi_rObj)
 {
     m_Path = pi_rObj.m_Path;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFSInvalidPathException::~HFSInvalidPathException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HFSInvalidPathException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
{
    WPrintfString rawMessage(GetRawMessageFromResource(pi_rsID).c_str(), m_Path.c_str());
    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
}

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
WStringCR HFSInvalidPathException::GetPath() const
    {
    return m_Path;
    }

