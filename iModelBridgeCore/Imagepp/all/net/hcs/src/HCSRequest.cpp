//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSRequest.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSRequest
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSRequest.h>


//-----------------------------------------------------------------------------
// public
// Constructor with a string and a separator
//
// UNICODE Note : pi_rRequest and pi_rSeparator must be in UTF8
//-----------------------------------------------------------------------------
HCSRequest::HCSRequest(const WString& pi_rRequest,
                       const WString& pi_rSeparator)
    {
    HPRECONDITION(!pi_rRequest.empty());

    // find the separator in the request string to build
    // a key and a value.  If none is found, the whole
    // request is the key
    WString::size_type Pos;
    if ((!pi_rSeparator.empty()) &&
        (Pos = pi_rRequest.find_first_of(pi_rSeparator)) != WString::npos)
        {
        m_Key   = pi_rRequest.substr(0, Pos);
        m_Value = pi_rRequest.substr(Pos + pi_rSeparator.size());
        }
    else
        {
        m_Key = pi_rRequest;
        }
    }


//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCSRequest::HCSRequest(const HCSRequest& pi_rObj)
    : m_Key  (pi_rObj.GetKey()),
      m_Value(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCSRequest::~HCSRequest()
    {
    }


//-----------------------------------------------------------------------------
// public
// Copy operator
//-----------------------------------------------------------------------------
HCSRequest& HCSRequest::operator=(const HCSRequest& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        m_Key   = pi_rObj.GetKey();
        m_Value = pi_rObj;
        }

    return *this;
    }

