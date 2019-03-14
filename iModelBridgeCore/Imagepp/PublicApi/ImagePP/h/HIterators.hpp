//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HIterators.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

template <typename ContainerType, typename ElementIteratorType, typename IndexIteratorType, typename ElementType>
void IndexedElementRandomIterator<ContainerType, ElementIteratorType, IndexIteratorType, ElementType>::Increment ()
    {
    HPRECONDITION(IndexIteratorType() != m_IndexIter);
    HPRECONDITION(m_IndexIter < m_ContainerInfo.m_IndexEnd);
    ++m_IndexIter;
    }

template <typename ContainerType, typename ElementIteratorType, typename IndexIteratorType, typename ElementType>
void IndexedElementRandomIterator<ContainerType, ElementIteratorType, IndexIteratorType, ElementType>::Decrement ()
    {
    HPRECONDITION(IndexIteratorType() != m_IndexIter);
    HPRECONDITION(m_IndexIter > m_ContainerInfo.m_IndexBegin);
    --m_IndexIter;
    }

END_IMAGEPP_NAMESPACE