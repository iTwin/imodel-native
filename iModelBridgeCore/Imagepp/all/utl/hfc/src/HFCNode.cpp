//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCNode.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Methods for class HFCNode
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCNode.h>
#include <Imagepp/all/h/HFCNodeIterator.h>

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Verify if the node is a descendant of the parent.

 @note This method is recursive, cannot be inline

 @param pi_rpParent A parent node.

 @return bool true if the is a child of the parent, false otherwise.
-----------------------------------------------------------------------------*/
bool HFCNode::IsAChildOf(const HFCPtr<HFCNode>& pi_rpParent) const
    {
    HPRECONDITION(pi_rpParent != 0);

    if (m_pParent != 0)
        return (pi_rpParent == m_pParent? true : IsAChildOf(m_pParent));
    else
        return false;
    }

/**----------------------------------------------------------------------------
 Get the level of this node.

 @note This method is recursive, cannot be inline.

 @return UInt32 The level of the node.

 @note The level of the root is 0.
-----------------------------------------------------------------------------*/
uint32_t HFCNode::GetLevel() const
    {
    if (HasParent())
        return 1 + GetParent()->GetLevel();
    else
        return 0;
    }

/**----------------------------------------------------------------------------
 Add a new node into this node. The node must be not exist into the child list

 @param pi_rpNode   The node to add.

 @return bool true If the node doesn't exist into the child list,
               false otherwise
-----------------------------------------------------------------------------*/
bool HFCNode::AddChild(const HFCPtr<HFCNode>& pi_rpNode)
    {
    HPRECONDITION(pi_rpNode != 0);
    HWARNING(CountIterator() == 0, L"HFCNode::AddChild() : At least one iterator was active");

    // search the node into the child list
    bool NodeFound = false;
    ChildList::const_iterator Itr(m_ChildList.begin());
    while (Itr != m_ChildList.end() && !NodeFound)
        {
        NodeFound = (*Itr == pi_rpNode);
        Itr++;
        }

    // we can insert the node if the node was not found
    if (!NodeFound)
        m_ChildList.push_back(pi_rpNode);

    return !NodeFound;
    }

/**----------------------------------------------------------------------------
 Remove a node from this node

 @param pi_rpNode       The node to remove.

 @note This method delete the node tree.

 @return bool true if the node exist into the child list,
               false otherwise

-----------------------------------------------------------------------------*/
bool HFCNode::RemoveChild(const HFCPtr<HFCNode>&   pi_rpNode)
    {
    HPRECONDITION(pi_rpNode != 0);

    HWARNING(CountIterator() == 0, L"HFCNode::RemoveChild() : At least one iterator was active");

    ChildList::iterator Itr(m_ChildList.begin());
    bool NodeFound = false;
    while(Itr != m_ChildList.end() && !NodeFound)
        {
        if ((NodeFound = (*Itr == pi_rpNode)))
            {
            pi_rpNode->SetParent(0);        // remove the parent for this node
            pi_rpNode->RemoveAllChild();    // remove all child for this node
            m_ChildList.erase(Itr);         // remove the node from the child list
            break;
            }
        Itr++;
        }

    return NodeFound;
    }

/**----------------------------------------------------------------------------
 Remove all node into the child list

 @note This method is recursive.
       This method remove the node tree for each child.
-----------------------------------------------------------------------------*/
void HFCNode::RemoveAllChild()
    {
    HWARNING(CountIterator() == 0, L"HFCNode::RemoveAllChild() : At least one iterator was active");

    ChildList::iterator Itr(m_ChildList.begin());
    while (Itr != m_ChildList.end())
        {
        (*Itr)->SetParent(0);       // remove the parent of this node
        (*Itr)->RemoveAllChild();   // remove all child for this node
        Itr++;
        }
    m_ChildList.clear();            // clear the child list
    }

/**----------------------------------------------------------------------------
 Create an iterator to the node.

 @return HFCNodeIterator*
-----------------------------------------------------------------------------*/
HFCNodeIterator* HFCNode::CreateIterator() const
    {
    HPRECONDITIONSHAREABLE();

    return new HFCNodeIterator(const_cast<HFCNode*>(this));
    }


//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------


