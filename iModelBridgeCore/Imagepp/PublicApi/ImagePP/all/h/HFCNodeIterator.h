//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCNodeIterator.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> Class : HFCNodeIterator
//:>---------------------------------------------------------------------------

#pragma once

#include "HFCNode.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a node iterator. This iterator can must be use to scan
    enties into a node that contain child.

    @see HFCNode
    -----------------------------------------------------------------------------
*/
class HFCNodeIterator
    {
public:
    HDECLARE_BASECLASS_ID (1341);

    HFCNodeIterator(const HFCPtr<HFCNode>& pi_rpNode);
    HFCNodeIterator(const HFCNodeIterator& pi_rObj);
    virtual ~HFCNodeIterator();

    HFCNodeIterator& operator=(const HFCNodeIterator& pi_rObj);

    const HFCPtr<HFCNode>& GetFirstNode() const;
    const HFCPtr<HFCNode>& GetNextNode() const;
    const HFCPtr<HFCNode>& GetNode() const;
protected:

    mutable HFCPtr<HFCNode> m_pResult;

private:

    // members
    HFCPtr<HFCNode>         m_pNode;
    mutable HFCNode::ChildList::const_iterator
    m_Itr;

    // disabled method
    HFCNodeIterator();
    };


#include "HFCNodeIterator.hpp"

