//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSResponse.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSResponse
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSResponse.h>

//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------

// Specify a grow size of 1 to insure that we do not have superflous memory.
static const size_t s_GrowSize = 1;

// Specify a max size of 1 in order to destroy the buffer when there is no data
// in the response.  (This insures that we keep the minimum memory space for the
// response data).
static const size_t s_MaxSize  = 1;


//-----------------------------------------------------------------------------
// public
// Default contructor - build an empty response
//-----------------------------------------------------------------------------
HCSResponse::HCSResponse()
    : m_Buffer(s_GrowSize, s_MaxSize)
    {
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSResponse::HCSResponse(const Byte* pi_pData, size_t pi_DataSize)
    : m_Buffer(s_GrowSize, s_MaxSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    // add the data to our buffer
    AppendResponse(pi_pData, pi_DataSize);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSResponse::HCSResponse(const WString& pi_rResponse)
    : m_Buffer(s_GrowSize, s_MaxSize)
    {
    HPRECONDITION(!pi_rResponse.empty());

    // add the data to our buffer
    AppendResponse(pi_rResponse);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSResponse::HCSResponse(const HCSResponse& pi_rObj)
    : m_Buffer(s_GrowSize, s_MaxSize)
    {
    // add the data to our buffer
    if (pi_rObj.GetDataSize() > 0)
        AppendResponse(pi_rObj);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSResponse::~HCSResponse()
    {
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSResponse& HCSResponse::operator=(const HCSResponse& pi_rResponse)
    {
    if (this != &pi_rResponse)
        {
        // Clear the buffer
        m_Buffer.Clear();

        // add the data to our buffer
        if (pi_rResponse.GetDataSize() > 0)
            AppendResponse(pi_rResponse);
        }

    return (*this);
    }