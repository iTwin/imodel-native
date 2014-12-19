//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCConnection.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCConnection
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCConnection.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//
// Note : pi_rServer can be empty
//-----------------------------------------------------------------------------
HFCConnection::HFCConnection(const WString& pi_rServer)
    : m_ConnectionState(false),
      m_Server(pi_rServer)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCConnection::~HFCConnection()
    {
    }

