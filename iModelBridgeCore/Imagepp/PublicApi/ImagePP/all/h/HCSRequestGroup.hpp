//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSRequestGroup.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSRequestGroup
//-----------------------------------------------------------------------------

#include "HCSRequest.h"

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline uint32_t HCSRequestGroup::CountElements() const
    {
    return (uint32_t)m_RequestList.size();
    }


//-----------------------------------------------------------------------------
// public
// Insert an already set up request
//-----------------------------------------------------------------------------
inline void HCSRequestGroup::Insert(HCSRequest* pi_pRequest)
    {
    HPRECONDITION(pi_pRequest != 0);

    // add the request in the list
    m_RequestList.push_back(pi_pRequest);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline HCSRequestGroup::IteratorHandle HCSRequestGroup::StartIteration() const
    {
    return (IteratorHandle) (new LocalIterator((RequestList*)&m_RequestList));
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const HCSRequest*
HCSRequestGroup::Iterate(HCSRequestGroup::IteratorHandle pi_Handle) const
    {
    LocalIterator* pItr = (LocalIterator*)pi_Handle;

    return  (++(pItr->m_Itr) == pItr->m_pList->end()) ? 0 : *(pItr->m_Itr);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const HCSRequest*
HCSRequestGroup::GetElement(HCSRequestGroup::IteratorHandle pi_Handle) const
    {
    LocalIterator* pItr = (LocalIterator*)pi_Handle;

    return  (pItr->m_Itr == pItr->m_pList->end()) ? 0 : *(pItr->m_Itr);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline void HCSRequestGroup::StopIteration(HCSRequestGroup::IteratorHandle pi_Handle) const
    {
    delete ((LocalIterator*)pi_Handle);
    }


