//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCNodeIterator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> inline methods for class HFCNodeIterator
//:>-----------------------------------------------------------------------------

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Destructor.
-----------------------------------------------------------------------------*/
inline HFCNodeIterator::~HFCNodeIterator()
    {
    // decrease theiterator ref count
    m_pNode->m_IteratorCount--;
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HFCNodeIterator& HFCNodeIterator::operator=(const HFCNodeIterator& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        m_pNode = pi_rObj.m_pNode;
        m_Itr = pi_rObj.m_Itr;
        }

    return *this;
    }

/**----------------------------------------------------------------------------
 Return the first child node.

 @return const HFCPtr<HFCNode>& The first child node, 0 if no node found.

 @see GetNextNode
 @see GetNode
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFCNode>& HFCNodeIterator::GetFirstNode() const
    {
    if ((m_Itr = m_pNode->m_ChildList.begin()) == m_pNode->m_ChildList.end())
        {
        m_pResult = 0;
        }
    else
        m_pResult = *m_Itr;

    return m_pResult;
    }

/**----------------------------------------------------------------------------
 Return the next child node.

 @note A call to GetFirstNode() must be call to start iteration.

 @return const HFCPtr<HFCNode>& The next child node, 0 if the no node found.

 @see GetFirstNode
 @see GetNode
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFCNode>& HFCNodeIterator::GetNextNode() const
    {
    HPRECONDITION(m_pResult != 0);

    m_Itr++;
    if (m_Itr == m_pNode->m_ChildList.end())
        {
        m_pResult = 0;
        }
    else
        m_pResult = *m_Itr;

    return m_pResult;
    }

/**----------------------------------------------------------------------------
 Return the current child node.

 @return const HFCPtr<HFCNode>& The current child node, 0 if no node found.

 @see GetFirstNode
 @see GetNextNode
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFCNode>& HFCNodeIterator::GetNode() const
    {
    return m_pResult;
    }

END_IMAGEPP_NAMESPACE