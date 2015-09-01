//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCNodeIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Methods for class HFCNodeIterator
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCNodeIterator.h>

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor of this class.

 @param pi_rpNode The node to iterate. Cannot be 0
-----------------------------------------------------------------------------*/
HFCNodeIterator::HFCNodeIterator(const HFCPtr<HFCNode>& pi_rpNode)
    : m_pNode(pi_rpNode)
    {
    HPRECONDITION(pi_rpNode != 0);
    m_pResult = 0;

    // increase the iterator ref count
    m_pNode->m_IteratorCount++;
    }

/**----------------------------------------------------------------------------
 Copy constructor.

 @param pi_rObj
-----------------------------------------------------------------------------*/
HFCNodeIterator::HFCNodeIterator(const HFCNodeIterator& pi_rObj)
    : m_pNode(pi_rObj.m_pNode),
      m_Itr(pi_rObj.m_Itr)
    {
    // increase the iterator ref count
    m_pNode->m_IteratorCount++;
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------
