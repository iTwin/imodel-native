//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSRequestGroup.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSRequestGroup
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSRequestGroup.h>
#include <Imagepp/all/h/HCSRequest.h>


//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HCSRequestGroup::HCSRequestGroup(const WString& pi_rRequestSeparator,
                                 const WString& pi_rValueSeparator)
    : m_RequestSeparator(pi_rRequestSeparator),
      m_ValueSeparator(pi_rValueSeparator)
    {
    HPRECONDITION(!m_RequestSeparator.empty());
    HPRECONDITION(!m_ValueSeparator.empty());
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSRequestGroup::~HCSRequestGroup()
    {
    // delete the pointers in the list.  The actual internal list object will
    // be destroyed afterwards when the list is destroyed.
    for (RequestList::const_iterator Itr = m_RequestList.begin();
         Itr != m_RequestList.end();
         Itr++)
        delete (*Itr);
    }


//-----------------------------------------------------------------------------
// public
// Insert a request from a string
//-----------------------------------------------------------------------------
void HCSRequestGroup::Insert(const WString& pi_rRequest)
    {
    HPRECONDITION(!pi_rRequest.empty());

    // parse each token separated by the request separator
    WString::size_type StartPos = 0;
    WString::size_type EndPos   = string::npos;
    WString::size_type Length;
    while (StartPos != WString::npos)
        {
        // find the next separator from the current start pos
        EndPos = pi_rRequest.find_first_of(m_RequestSeparator, StartPos);

        // build the request if any
        if (EndPos != StartPos)
            {
            Length = (EndPos != WString::npos ? EndPos - StartPos : EndPos);

            HAutoPtr<HCSRequest> pRequest(new HCSRequest(WString(pi_rRequest, StartPos, Length), m_ValueSeparator));

            // insert in this group
            Insert(pRequest.release());
            }

        // setup for the next iteration
        StartPos = (EndPos != WString::npos ? EndPos + 1 : EndPos);
        }
    }

