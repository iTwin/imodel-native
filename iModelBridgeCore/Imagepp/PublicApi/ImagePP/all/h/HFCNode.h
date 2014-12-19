//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCNode.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HFCNode
//:>---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a node. A node can have a child list. A HFCNodeIterator
    must be se to scan child list of an HFCNode. A node with child list is a tree.
    A node with no child is a leaf. A node with no parent is a root node.

    @see HFCPtr
    @see HFCNodeIterator
    -----------------------------------------------------------------------------
*/
class HFCNodeIterator;

class HFCNode : public HFCShareableObject<HFCNode>
    {
public:
    HDECLARE_BASECLASS_ID (1340);

    // data type
    typedef list< HFCPtr<HFCNode> > ChildList;

    // Primary methods
    _HDLLu                             HFCNode     (const HFCPtr<HFCNode>& pi_rpParent);
    _HDLLu virtual                     ~HFCNode    ();

    virtual bool               HasParent   () const;
    virtual void                SetParent   (const HFCPtr<HFCNode>& pi_rpParent);
    virtual const HFCPtr<HFCNode>&
    GetParent   () const;

    _HDLLu virtual bool               IsAChildOf(const HFCPtr<HFCNode>& pi_rpParent) const;

    _HDLLu virtual uint32_t            GetLevel    () const;

    _HDLLu virtual bool               AddChild    (const HFCPtr<HFCNode>& pi_rpNode);
    _HDLLu virtual bool               RemoveChild (const HFCPtr<HFCNode>& pi_rpNode);
    _HDLLu virtual void                RemoveAllChild();
    _HDLLu virtual size_t              CountChild() const;

    _HDLLu virtual HFCNodeIterator*    CreateIterator() const;
    _HDLLu virtual uint32_t            CountIterator() const;

    virtual const ChildList&           GetChildList() const;

protected:

private:

    friend class HFCNodeIterator;

    // Members
    ChildList                   m_ChildList;
    HFCPtr<HFCNode>             m_pParent;
    uint32_t                   m_IteratorCount;

    // optimization
    mutable HFCPtr<HFCNode>     m_pResult;

    // Disabled methods
    HFCNode();
    HFCNode(const HFCNode&);
    HFCNode& operator=(const HFCNode&);
    };

#include "HFCNode.hpp"
