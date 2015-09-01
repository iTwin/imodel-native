//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCNode.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

/**----------------------------------------------------------------------------
 Constructor of this class.

 @param pi_rpParent Parent of this node. This parameter can be set to 0

 @note If the parent is set to 0, this node will be the root
-----------------------------------------------------------------------------*/
inline HFCNode::HFCNode(const HFCPtr<HFCNode>& pi_rpParent)
    : m_pParent(pi_rpParent),
      m_IteratorCount(0)
    {
    }

/**----------------------------------------------------------------------------
 Destructor.
-----------------------------------------------------------------------------*/
inline HFCNode::~HFCNode()
    {
    }

/**----------------------------------------------------------------------------
 This method check if the node has a parent.

 @return bool true if the node has a parent, false otherwise.
-----------------------------------------------------------------------------*/
inline bool HFCNode::HasParent() const
    {
    return m_pParent != 0;
    }

/**----------------------------------------------------------------------------
 Set a new parent to the node.

 @param pi_rpParent Parent of this node. This parameter can be set to 0

 @note If the parent is set to 0, this node will be the root
-----------------------------------------------------------------------------*/
inline void HFCNode::SetParent(const HFCPtr<HFCNode>& pi_rpParent)
    {
    m_pParent = pi_rpParent;
    }

/**----------------------------------------------------------------------------
 Get the parent of the node.

 @return const HFCPtr<HFCNode>& The parent of the node.
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFCNode>& HFCNode::GetParent() const
    {
    return m_pParent;
    }

/**----------------------------------------------------------------------------
 Count the number of node into the child list.

 @return UInt32 The number of node into the child list.
-----------------------------------------------------------------------------*/
inline size_t HFCNode::CountChild() const
    {
    return m_ChildList.size();
    }

/**----------------------------------------------------------------------------
 Get the number of HFCNodeIterator on this node.

 @return UInt32
-----------------------------------------------------------------------------*/
inline uint32_t HFCNode::CountIterator() const
    {
    return m_IteratorCount;
    }

/**----------------------------------------------------------------------------
 Get the child list of the node.

 @return HFCNode::ChildList The node list of this node.

 @see HFCNode::ChildList
-----------------------------------------------------------------------------*/
inline const HFCNode::ChildList& HFCNode::GetChildList() const
    {
    return m_ChildList;
    }


END_IMAGEPP_NAMESPACE