//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSResponseGroup.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSResponseGroup
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline HCSResponseGroup::IteratorHandle HCSResponseGroup::StartIteration() const
    {
    return (IteratorHandle) (new LocalIterator((ResponseList*)&m_ResponseList));
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const HCSResponse* HCSResponseGroup::Iterate(HCSResponseGroup::IteratorHandle pi_Handle) const
    {
    LocalIterator* pItr = (LocalIterator*)pi_Handle;

    return  (++(pItr->m_Itr) == pItr->m_pList->end()) ? 0 : *(pItr->m_Itr);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const HCSResponse* HCSResponseGroup::GetElement(HCSResponseGroup::IteratorHandle pi_Handle) const
    {
    LocalIterator* pItr = (LocalIterator*)pi_Handle;

    return  (pItr->m_Itr == pItr->m_pList->end()) ? 0 : *(pItr->m_Itr);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline void HCSResponseGroup::StopIteration(HCSResponseGroup::IteratorHandle pi_Handle) const
    {
    delete ((LocalIterator*)pi_Handle);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline uint32_t HCSResponseGroup::CountElements() const
    {
    return (uint32_t)m_ResponseList.size();
    }