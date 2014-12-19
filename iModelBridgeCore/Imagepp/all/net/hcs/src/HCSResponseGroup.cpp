//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSResponseGroup.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSResponseGroup
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSResponseGroup.h>
#include <Imagepp/all/h/HCSResponse.h>

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSResponseGroup::HCSResponseGroup()
    {
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
HCSResponseGroup::~HCSResponseGroup()
    {
    // delete the pointers in the list.  The actual internal list object will
    // be destroyed afterwards when the list is destroyed.
    for (ResponseList::const_iterator Itr = m_ResponseList.begin();
         Itr != m_ResponseList.end();
         Itr++)
        delete (*Itr);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HCSResponseGroup::Insert(HCSResponse* pi_pResponse)
    {
    HPRECONDITION(pi_pResponse != 0);

    // add the Response in the list
    m_ResponseList.push_back(pi_pResponse);
    }
