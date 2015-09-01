//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCNode.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HFCNode
//:>---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE
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
    HDECLARE_BASECLASS_ID (HFCNodeId_Base);

    // data type
    typedef list< HFCPtr<HFCNode> > ChildList;

    // Primary methods
    IMAGEPP_EXPORT                             HFCNode     (const HFCPtr<HFCNode>& pi_rpParent);
    IMAGEPP_EXPORT virtual                     ~HFCNode    ();

    virtual bool               HasParent   () const;
    virtual void                SetParent   (const HFCPtr<HFCNode>& pi_rpParent);
    virtual const HFCPtr<HFCNode>&
    GetParent   () const;

    IMAGEPP_EXPORT virtual bool               IsAChildOf(const HFCPtr<HFCNode>& pi_rpParent) const;

    IMAGEPP_EXPORT virtual uint32_t            GetLevel    () const;

    IMAGEPP_EXPORT virtual bool               AddChild    (const HFCPtr<HFCNode>& pi_rpNode);
    IMAGEPP_EXPORT virtual bool               RemoveChild (const HFCPtr<HFCNode>& pi_rpNode);
    IMAGEPP_EXPORT virtual void                RemoveAllChild();
    IMAGEPP_EXPORT virtual size_t              CountChild() const;

    IMAGEPP_EXPORT virtual HFCNodeIterator*    CreateIterator() const;
    IMAGEPP_EXPORT virtual uint32_t            CountIterator() const;

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

END_IMAGEPP_NAMESPACE

#include "HFCNode.hpp"
