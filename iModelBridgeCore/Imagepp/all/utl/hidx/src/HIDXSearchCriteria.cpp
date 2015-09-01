//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hidx/src/HIDXSearchCriteria.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXSearchCriteria
//-----------------------------------------------------------------------------
// General class for search criterias.
//-----------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIDXSearchCriteria.h>
#include <Imagepp/all/h/HIDXCriteria.h>



//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HIDXSearchCriteria::HIDXSearchCriteria()
    {
    }


//-----------------------------------------------------------------------------
// Constructor with a first criteria
//-----------------------------------------------------------------------------
HIDXSearchCriteria::HIDXSearchCriteria(void const* pi_Index, HIDXCriteria* pi_pNewCriteria)
    {
    HASSERT(pi_Index != 0);
    HASSERT(pi_pNewCriteria != 0);
    HASSERT(m_Criterias.find(pi_Index) == m_Criterias.end());

    m_Criterias.insert(CriteriaMap::value_type(pi_Index, pi_pNewCriteria));
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HIDXSearchCriteria::~HIDXSearchCriteria()
    {
    // We have to destroy the criterias.
    CriteriaMap::const_iterator Itr(m_Criterias.begin());

    while (Itr != m_Criterias.end())
        {
        delete (*Itr).second;
        ++Itr;
        }
    }


//-----------------------------------------------------------------------------
// Give new criteria (possessed by SearchCriteria)
//-----------------------------------------------------------------------------
void HIDXSearchCriteria::AddCriteria(void const* pi_Index, HIDXCriteria* pi_pNewCriteria)
    {
    HASSERT(pi_Index != 0);
    HASSERT(pi_pNewCriteria != 0);
    HASSERT(m_Criterias.find(pi_Index) == m_Criterias.end());

    m_Criterias.insert(CriteriaMap::value_type(pi_Index, pi_pNewCriteria));
    }


//-----------------------------------------------------------------------------
// Retrieve a criteria
//-----------------------------------------------------------------------------
HIDXCriteria* HIDXSearchCriteria::GetCriteria(void const* pi_Index) const
    {
    HASSERT(pi_Index != 0);

    CriteriaMap::const_iterator Itr(m_Criterias.find(pi_Index));

    // 2 returns to save an assignment
    if (Itr == m_Criterias.end())
        return 0;
    else
        return (*Itr).second;
    }
