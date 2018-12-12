//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGFSpatialIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// HGFIndexNode Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::HGFIndexNode(bool propagateDataDown)
    : CONTAINER()
    {

    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_NbObjects = -1;
    m_delayedDataPropagation = propagateDataDown;
    m_pParentNode = 0;

    m_nodeHeader.m_SplitTreshold = 10000;
    m_nodeHeader.m_IsLeaf = true;
    m_nodeHeader.m_IsUnSplitSubLevel = false;
    m_nodeHeader.m_IsBranched = false;
    m_nodeHeader.m_level = 0;
    m_nodeHeader.m_balanced = false;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 4;
    m_nodeHeader.m_contentExtentDefined = false;
    m_nodeHeader.m_totalCountDefined = true;
    m_nodeHeader.m_totalCount = 0;

    HDEBUGCODE(m_unspliteable = false;)
    HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)


    HINVARIANTS;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::HGFIndexNode(size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        bool balanced, bool propagatesDataDown, bool dummyParam2, bool dummyParam3)
    : CONTAINER()
    {

    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_NbObjects = -1;
    m_delayedDataPropagation = !propagatesDataDown;
    m_pParentNode = 0;

    m_nodeHeader.m_SplitTreshold = pi_SplitTreshold;
    m_nodeHeader.m_IsLeaf = true;
    m_nodeHeader.m_IsUnSplitSubLevel = false;
    m_nodeHeader.m_IsBranched = false;
    m_nodeHeader.m_level = 0;
    m_nodeHeader.m_balanced = balanced;
    m_nodeHeader.m_nodeExtent = pi_rExtent;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 4;
    m_nodeHeader.m_contentExtentDefined = false;
    m_nodeHeader.m_totalCountDefined = true;
    m_nodeHeader.m_totalCount = 0;

    HDEBUGCODE(m_unspliteable = false;)
    HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)


    // Do not allow NULL sized extent ...
    if (!(ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent) > ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)))
        ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + MAX (fabs(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)) * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0, 1.0)));
    if (!(ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent) > ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)))
        ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + MAX (fabs(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)) * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0, 1.0)));

    HINVARIANTS;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::HGFIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const SELFNODEPTR& pi_rpParentNode)
    : CONTAINER(),
      m_pParentNode(pi_rpParentNode)
    {
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 4;
    m_delayedDataPropagation = !pi_rpParentNode->PropagatesDataDown();


    m_nodeHeader.m_IsBranched = false;

    m_nodeHeader.m_IsUnSplitSubLevel = false;


    m_nodeHeader.m_level = pi_rpParentNode->GetLevel() + 1;
    m_nodeHeader.m_balanced = pi_rpParentNode->IsBalanced();
    m_nodeHeader.m_SplitTreshold = pi_SplitTreshold;
    m_nodeHeader.m_IsLeaf = true;
    m_nodeHeader.m_nodeExtent = pi_rExtent;
    m_nodeHeader.m_contentExtentDefined = false;
    m_nodeHeader.m_totalCountDefined = true;
    m_nodeHeader.m_totalCount = 0;

    HDEBUGCODE(m_unspliteable = false;)
    HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)

    // Call soft invariant as the parent is most probably reconfiguring
    this->ValidateInvariantsSoft();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::HGFIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const SELFNODEPTR& pi_rpParentNode,
        bool IsUnsplitSubLevel)
    : CONTAINER(),
      m_pParentNode(pi_rpParentNode)
    {
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 4;
    m_delayedDataPropagation = !pi_rpParentNode->PropagatesDataDown();

    m_NbObjects = -1;

    m_nodeHeader.m_IsBranched = false;

    m_nodeHeader.m_IsUnSplitSubLevel = IsUnsplitSubLevel;

    m_nodeHeader.m_level = pi_rpParentNode->GetLevel() + 1;
    m_nodeHeader.m_balanced = pi_rpParentNode->IsBalanced();
    m_nodeHeader.m_SplitTreshold = pi_SplitTreshold;
    m_nodeHeader.m_IsLeaf = true;
    m_nodeHeader.m_nodeExtent = pi_rExtent;
    m_nodeHeader.m_contentExtentDefined = false;
    m_nodeHeader.m_totalCountDefined = true;
    m_nodeHeader.m_totalCount = 0;

    HDEBUGCODE(m_unspliteable = false;)
    HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)

    // Call soft invariant as the parent is most probably reconfiguring
    this->ValidateInvariantsSoft();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::HGFIndexNode (const HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>& pi_rNode)
    : CONTAINER(pi_rNode),
      m_pParentNode(0),
      m_ListOfObjects(pi_rNode.m_ListOfObjects),
    {
    // A copied node may not have a parent
    HPRECONDITION(pi_rNode.m_pParentNode == 0);

    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_NbObjects = -1;
    m_delayedDataPropagation = !pi_rNode->PropagatesDataDown();

    m_nodeHeader.m_numberOfSubNodesOnSplit = 4;


    m_nodeHeader.m_IsUnSplitSubLevel = pi_rNode.m_IsUnSplitSubLevel;
    m_nodeHeader.m_IsBranched = pi_rNode.m_nodeHeader.m_IsBranched;


    m_nodeHeader.m_SplitTreshold = pi_rNode.GetSplitTreshold();
    m_nodeHeader.m_IsLeaf = pi_rNode.IsLeaf();
    m_nodeHeader.m_level = pi_rNode.GetLevel();
    m_nodeHeader.m_balanced = pi_rNode.IsBalanced();
    m_nodeHeader.m_nodeExtent = pi_rNode.m_nodeHeader.m_nodeExtent;
    m_nodeHeader.m_contentExtent = pi_rNode.m_nodeHeader.m_contentExtent;
    m_nodeHeader.m_contentExtentDefined = pi_rNode.m_nodeHeader.m_contentExtentDefined;
    m_nodeHeader.m_totalCountDefined = pi_rNode.m_nodeHeader.m_totalCountDefined;
    m_nodeHeader.m_totalCount = pi_rNode.m_nodeHeader.m_totalCount;


    HDEBUGCODE(m_unspliteable = pi_rNode.m_unspliteable;)
    HDEBUGCODE(m_parentOfAnUnspliteableNode = pi_rNode.m_parentOfAnUnspliteableNode;)

    // Check if there are subnodes
    if (!m_nodeHeader.m_IsLeaf)
        {
        if (pi_rNode.m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit = new Node (pi_rNode.m_SubNodeNoSplit, this, m_filter);
        else
            {
            m_nodeHeader.m_IsBranched = true;

            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                m_apSubNodes[indexNode] = this->CloneChild();
                }
            }
        }

    HINVARIANTS;
    }




/**----------------------------------------------------------------------------
 Destroyer
-----------------------------------------------------------------------------*/
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::~HGFIndexNode()
    {
    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 @bsimethod                                         AlainRobert
-----------------------------------------------------------------------------*/
// template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER> SELFNODEPTR HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Clone () const
// {
//     return new HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>(GetSplitTreshold(), GetExtent(), IsBalanced(), true, false, false);
// }
// template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER> SELFNODEPTR HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Clone (const EXTENT& newNodeExtent) const
// {
//     return new HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>(GetSplitTreshold(), newNodeExtent, IsBalanced(), true, false, false);
// }
// template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER> SELFNODEPTR HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::CloneChild (const EXTENT& newNodeExtent) const
// {
//     return new HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>(GetSplitTreshold(), newNodeExtent, const_cast<HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>* >(this));
// }
// template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER> SELFNODEPTR HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::CloneUnsplitChild (const EXTENT& newNodeExtent) const
// {
//     HFCPtr<HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>>  toto = const_cast<HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>* >(this);
//     return new HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>(GetSplitTreshold(), GetExtent(), toto, true);
// }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Load() const
    {
    HPRECONDITION (!IsLoaded());

    // The default implementation does nothing as the default implementation assumes it is always loaded
    // If this is not the case for the container ancester class the descendant must override the present
    // method

    m_loaded = true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
    void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Unload() const
	{
    // The default implementation does nothing as the default implementation assumes it is always loaded
    // If this is not the case for the container ancester class the descendant must override the present
    // method

    m_loaded = false;
	}


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER> 
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::IsLoaded() const
    {
    return m_loaded;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::IsDestroyed() const
    {
    return m_destroyed;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Destroy()
    {
    // The very first step is to sever the parent node relation
    // This will prevent the propagation of undue events towards the parent.
    m_pParentNode = NULL;

    // If there are child nodes they must be destroyed first.
    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            m_pSubNodeNoSplit->Destroy();
            }
        else
            {
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                {
                m_apSubNodes[indexNodes]->Destroy();
                }
            }
        }

    m_destroyed = true;

    this->clear();
    SetDirty (false);
    m_loaded = false;


    HINVARIANTS;

    return true;
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::SetDirty (bool dirty) const // Intentionaly const ... only mutable field changed
    {
    if (!IsLoaded())
        Load();

    m_NbObjects = -1;

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::AdviseSubNodeIDChanged(const SELFNODEPTR& p_subNode)
    {
    if (!IsLoaded())
        Load();

    // Nothing changed
    if (!m_nodeHeader.m_IsLeaf)
        {
        SetDirty(true);
        }
    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::SetParentNode(const SELFNODEPTR& pi_rpParentNode)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    // The node must be orphan(parentless)
    HPRECONDITION(m_pParentNode == 0);

    m_pParentNode = pi_rpParentNode;

    SetDirty(true);

    if (m_pParentNode != NULL)
        m_pParentNode->AdviseSubNodeIDChanged(REALTHIS());

    HINVARIANTS;


    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
const SELFNODEPTR& HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetParentNode() const
    {
    if (!IsLoaded())
        Load();

    return m_pParentNode;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::SetSubNodes(SELFNODEPTR pi_apSubNodes[], size_t numSubNodes)
    {
    HINVARIANTS;

    if (!IsLoaded())
        Load();

    // Indicate node is not a leaf anymore
    m_nodeHeader.m_IsLeaf = false;
    m_nodeHeader.m_IsBranched = true;

    // Table must be provided and each subnode pointer point to an existing node
    HPRECONDITION(pi_apSubNodes != NULL);

    size_t indexNode;
    for (indexNode = 0 ; indexNode < numSubNodes; indexNode++)
        {
        HASSERT(pi_apSubNodes[indexNode] != 0);
        m_apSubNodes[indexNode] = pi_apSubNodes[indexNode];
        }
    for (indexNode = 0 ; indexNode < numSubNodes; indexNode++)
        {
        m_apSubNodes[indexNode]->SetParentNode(REALTHIS());
        }


    SetDirty(true);

    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::SetSplitTreshold(size_t pi_SplitTreshold)
    {
    if (!IsLoaded())
        Load();
    HINVARIANTS;

    // Set new value
    m_nodeHeader.m_SplitTreshold = pi_SplitTreshold;

    // Check if there are sub-nodes
    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->SetSplitTreshold(GetSplitTreshold());
        else
            {
            // Set treshold for all sub-nodes

            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                m_apSubNodes[indexNode]->SetSplitTreshold(GetSplitTreshold());
                }
            }
        }
    else
        {
        // The node is a leaf ... check if the current number of objects is greater than new treshold
        if (this->size() >= GetSplitTreshold())
            {
            // Treshold attained ... we must split
            SplitNode();
            }
        }
    SetDirty(true);

    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
size_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetSplitTreshold() const
    {
    // HINVARIANTS; // We do not call invariants for simple accessors as they are extensively called within reorganising methods
    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_SplitTreshold);
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
size_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetNumberOfSubNodesOnSplit() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_numberOfSubNodesOnSplit);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::IsBalanced() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_balanced);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Balance(size_t depth)
    {
    this->ValidateInvariantsSoft();

    if (!IsLoaded())
        Load();

    if (!m_nodeHeader.m_balanced)
        {
        m_nodeHeader.m_balanced = true;

        if (m_nodeHeader.m_IsLeaf)
            {
            if (m_nodeHeader.m_level < depth)
                PushNodeDown (depth);
            }
        else
            {
            if (m_pSubNodeNoSplit != NULL)
                m_pSubNodeNoSplit->Balance (depth);
            else
                {
                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    m_apSubNodes[indexNode]->Balance (depth);
                    }
                }
            }
        }
    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Unbalance()
    {


    if (!IsLoaded())
        Load();

    this->ValidateInvariantsSoft();

    m_nodeHeader.m_balanced = false;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->Unbalance ();
        else
            {
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                m_apSubNodes[indexNode]->Unbalance();
                }
            }
        }

    this->ValidateInvariantsSoft();

    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::PropagatesDataDown() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    // No need to load the node as this parameter is not stored
    return(!m_delayedDataPropagation);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::SetPropagateDataDown(bool propagate)
    {
    HINVARIANTS;
    // No need to load the node as this parameter is not stored
    m_delayedDataPropagation = !propagate;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::PropagateDataDownImmediately(bool propagateRecursively)
    {
    // We do not call invariants because this method is called during transformation of node.
    if (!IsLoaded())
        Load();


    // For a leaf nothing need be done
    if (!m_nodeHeader.m_IsLeaf && (this->size() > 0))
        {

        if (m_pSubNodeNoSplit != NULL)
            {
            size_t numberSpatial = this->size();
            SPATIAL* spatialArray = new SPATIAL[numberSpatial];
            for (size_t indexSpatial = 0; indexSpatial < numberSpatial ; indexSpatial++)
                spatialArray[indexSpatial] = this->operator[](indexSpatial);
            this->clear();

            // We copy the whole content to this sub-node
            m_pSubNodeNoSplit->AddArrayUnconditional (spatialArray, numberSpatial);

            // The total count remains unchanged!

            delete spatialArray;
            }
        else
            {

            size_t numberSpatial = this->size();
            SPATIAL* spatialArray[8];
            SPATIAL* spatialArrayReturned = new SPATIAL[numberSpatial];
            size_t spatialArrayReturnedNumber = 0;
            size_t spatialArrayNumber[8];
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                {
                spatialArray[indexNodes] = new SPATIAL[numberSpatial];
                spatialArrayNumber[indexNodes] = 0;
                }

            for (size_t indexSpatial = 0; indexSpatial < numberSpatial ; indexSpatial++)
                {
                bool addedToNode = false;
                for (size_t indexNodes = 0; !addedToNode && indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                    {
                    if (SpatialOp<SPATIAL, POINT, EXTENT>::IsSpatialInExtent2D (this->operator[](indexSpatial), m_apSubNodes[indexNodes]->GetNodeExtent()))
                        {
                        spatialArray[indexNodes][spatialArrayNumber[indexNodes]] = this->operator[](indexSpatial);
                        spatialArrayNumber[indexNodes]++;
                        addedToNode = true;
                        }
                    }

                if (!addedToNode)
                    {
                    spatialArrayReturned[spatialArrayReturnedNumber] = this->operator[](indexSpatial);
                    spatialArrayReturnedNumber++;
                    }
                }
            this->clear();

            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                {
                if (spatialArrayNumber[indexNodes] > 0)
                    m_apSubNodes[indexNodes]->AddArrayUnconditional(spatialArray[indexNodes], spatialArrayNumber[indexNodes]);
                }

            for (size_t indexSpatial = 0; indexSpatial < spatialArrayReturnedNumber ; indexSpatial++)
                {
                this->push_back(spatialArrayReturned[indexSpatial]);
                }


            delete [] spatialArrayReturned;

            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                delete [] spatialArray[indexNodes];

            // The total count remains unchanged!


            }
        }

    // As a result of previous operations it is possible that delayed split be invoked for the present node ...
    if (m_DelayedSplitRequested)
        SplitNode();

    if (!IsLeaf() && propagateRecursively)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            m_pSubNodeNoSplit->PropagateDataDownImmediately (propagateRecursively);
            }
        else
            {
            for (size_t i = 0 ; i < m_nodeHeader.m_numberOfSubNodesOnSplit; ++ i)
                {
                m_apSubNodes[i]->PropagateDataDownImmediately (propagateRecursively);
                }
            }
        }
    HINVARIANTS;

    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
size_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetLevel() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_level);
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
size_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetDepth() const
    {
    HINVARIANTS;

    if (!IsLoaded())
        Load();

    if (m_nodeHeader.m_balanced)
        {
        if (IsLeaf())
            return m_nodeHeader.m_level;
        else if (m_pSubNodeNoSplit != NULL)
            return m_pSubNodeNoSplit->GetDepth();
        else
            return m_apSubNodes[0]->GetDepth();
        }
    else
        {
        // Unbalanced
        if (m_nodeHeader.m_IsLeaf)
            return m_nodeHeader.m_level;
        else if (m_pSubNodeNoSplit != NULL)
            return m_pSubNodeNoSplit->GetDepth();
        else
            {
            size_t maxDepth = m_nodeHeader.m_level;
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                maxDepth = MAX(m_apSubNodes[indexNode]->GetDepth(), maxDepth);
                }
            return maxDepth;
            }
        }
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::IncreaseLevel()
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    m_nodeHeader.m_level++;

    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->IncreaseLevel();
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                m_apSubNodes[indexNode]->IncreaseLevel();
                }
            }
        }
    HINVARIANTS;

    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::IsLeaf() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_IsLeaf);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
const EXTENT& HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetNodeExtent() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_nodeExtent);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
const EXTENT& HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetContentExtent() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_contentExtent);
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::SetNodeExtent(const EXTENT& extent)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    m_nodeHeader.m_nodeExtent = extent;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
uint64_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetCount() const
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (!m_nodeHeader.m_totalCountDefined)
        {
        uint64_t count = this->size();
        if (!IsLeaf())
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                count += m_pSubNodeNoSplit->GetCount();
                }
            else
                {
                for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit(); ++ i)
                    {
                    count += m_apSubNodes[i]->GetCount();
                    }
                }
            }
        m_nodeHeader.m_totalCount = count;
        m_nodeHeader.m_totalCountDefined = true;
        }

    HINVARIANTS;

    return m_nodeHeader.m_totalCount;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
size_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetSplitDepth() const
    {
    if (!IsLoaded())
        Load();

    size_t deepestSplitLevel = m_nodeHeader.m_level;

    if (m_nodeHeader.m_IsUnSplitSubLevel)
        return 0;

    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit == NULL)
            {
            for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit(); ++ i)
                {
                deepestSplitLevel = MAX (deepestSplitLevel, m_apSubNodes[i]->GetSplitDepth());
                }
            }
        }
    return deepestSplitLevel;
    }

//=======================================================================================
// @bsimethod                                                   Richard.Bois 02/13
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER> 
    SELFNODEPTR HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetSubNodeNoSplit() const
    {
	return m_pSubNodeNoSplit;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::IsEmpty() const
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_nodeHeader.m_totalCountDefined)
        return (m_nodeHeader.m_totalCount == 0);

    if (this->size() > 0)
        return false;

    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            return m_pSubNodeNoSplit->IsEmpty();
            }
        else
            {
            for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit(); ++ i)
                {
                if (!m_apSubNodes[i]->IsEmpty())
                    return false;
                }
            }
        }
    HINVARIANTS;

    return true;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::AddConditional (const SPATIAL& pi_rpSpatialObject, bool ExtentFixed)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();


    // Check is spatial extent is in node ...
    if (SpatialOp<SPATIAL, POINT, EXTENT>::IsSpatialInExtent2D (pi_rpSpatialObject, GetNodeExtent()))
        {
        return Add (pi_rpSpatialObject);
        }
    // The spatial object is not in extent ... check if we can increase extent (not extent fixed, no parent and no sub-nodes)
    else if (!ExtentFixed && GetParentNode() == NULL && IsLeaf())
        {
        // We can increase the extent ... do it
        m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(GetNodeExtent(), SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));

        // We maintain the extent square
        if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
            }
        else
            {
            ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
            }

        return Add (pi_rpSpatialObject);
        }

    HINVARIANTS;

    return false;
    }




//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
size_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::AddArrayConditional (SPATIAL* spatialArray, size_t startSpatialIndex, size_t countSpatial, bool ExtentFixed)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();

    // If nothing to be added ... get out right away
    if (startSpatialIndex >= countSpatial)
        return countSpatial;

    // If node is not extent limited ...
    if (!ExtentFixed && GetParentNode() == NULL && IsLeaf())
        {

        size_t endSpatialIndex;
        if (countSpatial - startSpatialIndex + this->size() >= GetSplitTreshold())
            {
            // Not all points will be held by node ...
            endSpatialIndex = GetSplitTreshold() - this->size() + startSpatialIndex;
            }
        else
            {
            endSpatialIndex = countSpatial;
            }
        for (size_t currIndex = startSpatialIndex ; currIndex < endSpatialIndex; currIndex++)
            {
            // We can increase the extent ... do it
            m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(GetNodeExtent(), SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(spatialArray[currIndex]));

            // We maintain the extent square
            if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
                {
                ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent));
                }
            else
                {
                ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent));
                }

            }
        AddArrayUnconditional (&(spatialArray[startSpatialIndex]), endSpatialIndex);

        if (endSpatialIndex < countSpatial)
            return AddArrayConditional(spatialArray, endSpatialIndex, countSpatial, true);
        else
            return countSpatial;

        }
    else
        {
        // Extent is limited ...
        size_t lastSpatialIndexInExtent = startSpatialIndex;
        while ((lastSpatialIndexInExtent < countSpatial) &&
               (SpatialOp<SPATIAL, POINT, EXTENT>::IsSpatialInExtent2D (spatialArray[lastSpatialIndexInExtent], m_nodeHeader.m_nodeExtent)))
            {
            lastSpatialIndexInExtent++;
            }

        if (lastSpatialIndexInExtent > startSpatialIndex)
            {
            AddArrayUnconditional (&(spatialArray[startSpatialIndex]), lastSpatialIndexInExtent - startSpatialIndex);
            }

        HINVARIANTS;

        return lastSpatialIndexInExtent;
        }

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::AddArrayUnconditional(SPATIAL* spatialArray, size_t countSpatial)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();

    // The total count increases by countSpatial whatever the path selected below
    m_nodeHeader.m_totalCount += countSpatial;

    // All points must be fully contained in node extent

    // Check if the threshold amount of objects is attained
    if ((IsLeaf() || m_nodeHeader.m_IsUnSplitSubLevel) && (this->size() + countSpatial >= GetSplitTreshold()))
        {
        // There are too much objects ... need to split current node
        SplitNode();
        }
    else if (m_delayedDataPropagation && (this->size() + countSpatial >= GetSplitTreshold()))
        {
        PropagateDataDownImmediately(false);
        }

    for (size_t indexSpatial = 0 ; indexSpatial < countSpatial ; indexSpatial++)
        {
        if (!m_nodeHeader.m_contentExtentDefined)
            {
            m_nodeHeader.m_contentExtent = SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(spatialArray[indexSpatial]);
            m_nodeHeader.m_contentExtentDefined = true;
            }
        else
            {
            m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(spatialArray[indexSpatial]));
            }
        }

    // Check if node is still a leaf ...
    if (IsLeaf() || (m_delayedDataPropagation && (this->size() + countSpatial < m_nodeHeader.m_SplitTreshold)))
        {
        if (this->size() + countSpatial >= this->capacity())
            this->reserve (this->size() + countSpatial + GetSplitTreshold() / 10);

        // It is a leaf ... we add reference in list
        for (size_t indexSpatial = 0 ; indexSpatial < countSpatial ; indexSpatial++)
            {
            push_back (spatialArray[indexSpatial]);
            }
//        this->push_back(spatialArray, countSpatial);

        }
    else
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            m_pSubNodeNoSplit->AddArrayUnconditional (spatialArray, countSpatial);
            }
        else
            {
            size_t startIndex = 0;
            while (startIndex < countSpatial)
                {
                HDEBUGCODE (size_t previousCount = startIndex);
                for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    startIndex = m_apSubNodes[indexNode]->AddArrayConditional(spatialArray, startIndex, countSpatial, true);
                    }
                HDEBUGCODE(HPRECONDITION(previousCount < startIndex));
                }
            }


        }

    SetDirty(true);

    HINVARIANTS;

    return true;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Add(const SPATIAL& pi_rpSpatialObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();

    // The total count increases by 1
    m_nodeHeader.m_totalCount += 1;

    // The object must be fully contained in node extent
//    HPRECONDITION(SpatialOp<SPATIAL, POINT, EXTENT>::IsSpatialInExtent2D(pi_rpSpatialObject, GetExtent()));


    // Check if the threshold amount of objects is attained
    if ((IsLeaf() || m_nodeHeader.m_IsUnSplitSubLevel) && (this->size() + 1 >= GetSplitTreshold()))
        {
        // There are too much objects ... need to split current node
        SplitNode();
        }
    else if (m_delayedDataPropagation && (this->size() + 1 >= GetSplitTreshold()))
        {
        PropagateDataDownImmediately(false);
        }

    if (!m_nodeHeader.m_contentExtentDefined)
        {
        m_nodeHeader.m_contentExtent = SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(pi_rpSpatialObject);
        m_nodeHeader.m_contentExtentDefined = true;
        }
    else
        {
        m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));
        }


    // Check if node is still a leaf ...
    if (IsLeaf() || m_delayedDataPropagation)
        {
        if (this->size() + 1 >= this->capacity())
            this->reserve (this->size() + GetSplitTreshold() / 10);

        // It is a leaf ... we add reference in list
        this->push_back(pi_rpSpatialObject);
        }
    else
        {
        // Attempt to add in one of the subnodes
        bool Added = false;
        if (m_pSubNodeNoSplit != NULL)
            {
            m_pSubNodeNoSplit->Add (pi_rpSpatialObject);
            Added = true;
            }
        else
            {
            for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit() && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (SpatialOp<SPATIAL, POINT, EXTENT>::IsSpatialInExtent2D (pi_rpSpatialObject, m_apSubNodes[i]->GetNodeExtent()))
                    {
                    // The object is contained ... we add to subnode
                    m_apSubNodes[i]->Add(pi_rpSpatialObject);
                    Added = true;
                    }
                }
            }

        // Check if the object was not added in a subnode ...
        if (!Added)
            {
            // The object was not added, evidently because it is too large ...
            // We add it to current node.
            if (this->size()+ 1 >= this->capacity())
                this->reserve (this->size() + GetSplitTreshold() / 10);

            this->push_back(pi_rpSpatialObject);
            }
        }
    SetDirty(true);

    HINVARIANTS;

    return true;
    }


/**----------------------------------------------------------------------------
 Removes a reference to spatial object in node.
 The spatial object extent must be included (contained) in node extent

 @param Pointer to spatial object to remove reference to in the index
-----------------------------------------------------------------------------*/
// template<class POINT, class EXTENT, class CONTAINER, SELFNODEPTR, NODEHEADER>
//    bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Clear(HFCPtr<HVEShape> pi_shapeToClear)
// {
//
//    HINVARIANTS;
//    if (!IsLoaded())
//        Load();
//
//    bool SomeRemoved = false;
//
//    // Obtain the shape extent
//    HGF2DExtent shapeExtent = pi_ShapeToClear->GetExtent();
//    // Convert shape into an EXTENT
//    EXTENT shapeExtent2 = ExtentOp<EXTENT>::Create(shapeExtent.GetXMin(),
//                                                   shapeExtent.GetYMin(),
//                                                   0.0,
//                                                   shapeExtent.GetXMax(),
//                                                   shapeExtent.GetYMax(),
//                                                   0.0);
//
//
//    // Check if node is a leaf ...
//    if (!IsLeaf())
//    {
//
//        if (m_pSubNodeNoSplit != NULL)
//        {
//            SomeRemoved = m_pSubNodeNoSplit->Clear (pi_shapeToClear);
//        }
//        else
//        {
//
//           for (int i = 0 ; i < GetNumberOfSubNodesOnSplit() && !Removed ; ++ i)
//           {
//               // Check if extent of node overlap shape.
//               // Obtain the extent of the shape
//                // Check if object is contained in this sub-node
//                if (ExtentOp<POINT, EXTENT>::Overlap(m_apSubNodes[i]->GetExtent(), shapeExtent))
//                {
//                    // The object is contained ... we add to subnode
//                    SomeRemoved = m_apSubNodes[i]->Clear(pi_ShapeToClear) || SomeRemoved;
//                }
//            }
//        }
//    }
//
//    for (size_t currentIndex = 0 ; currentIndex < this->size(); currentIndex++)
//    {
//        // Check if current object is in shape
//
//        if (SpatialOp<SPATIAL, POINT, EXTENT>::SpatialInShape(this->operator[](currentIndex), pi_shapeToClear))
//        {
//            // We have found it... erase it
//            this->erase(currentIndex);
//            SomeRemoved = true;
//        }
//    }
//
//
//
//    if (SomeRemoved)
//       SetDirty(true);
//
//    return(SomeRemoved);
// }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
bool HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::Remove(const SPATIAL& pi_rpSpatialObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    bool Removed = false;

    // Check if node is still a leaf ...
    if (!IsLeaf())
        {

        if (m_pSubNodeNoSplit != NULL)
            {
            Removed = m_pSubNodeNoSplit->Remove (pi_rpSpatialObject);
            }
        else
            {
            for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit() && !Removed ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (SpatialOp<SPATIAL, POINT, EXTENT>::IsSpatialInExtent2D(pi_rpSpatialObject, m_apSubNodes[i]->GetNodeExtent()))
                    {
                    // The object is contained ... we add to subnode
                    Removed = m_apSubNodes[i]->Remove(pi_rpSpatialObject);
                    }
                }
            }
        }

    // Check if the object was not removed in a subnode ...
    if (!Removed)
        {
        // The object was not removed, evidently because it is too large ...
        CONTAINER::iterator itr = this->begin();

        for (; !Removed && itr != this->end() ; itr++)
            {
            // Check if current object is the one to remove ...
            if (*itr == pi_rpSpatialObject)
                {
                // We have found it... erase it
                this->erase(itr);
                Removed = true;
                }
            }
        }


    if (Removed)
        {
        // The total count has changed
        m_nodeHeader.m_totalCount --;
        SetDirty(true);
        }

    HINVARIANTS;


    return(Removed);
    }






//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
size_t HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::GetIn(const EXTENT& pi_rExtent,
        list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // Check if coordinate falls inside node extent
    if (ExtentOp<EXTENT>::Overlap(GetNodeExtent(), pi_rExtent))
        {
        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)
        if (!IsLeaf())
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                m_pSubNodeNoSplit->GetIn (pi_rExtent, pio_rListOfObjects);
                }
            else
                {
                for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit() ; ++ i)
                    {
                    m_apSubNodes[i]->GetIn (pi_rExtent, pio_rListOfObjects);
                    }
                }

            }

        // Search in present list of objects for current node
        CONTAINER::const_iterator itr = this->begin();
        for ( ; itr != this->end() ; ++itr)
            {
            // Check if point is in extent of object
            if (ExtentOp<EXTENT>::Overlap(pi_rExtent, SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(*itr)))
                {
                // The point falls inside extent of object .. we add a reference to the list
                pio_rListOfObjects.push_back(*itr);
                }
            }
        }

    HINVARIANTS;


    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }




//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::AdviseDelayedSplitRequested() const
    {
    HASSERT (m_pSubNodeNoSplit != NULL);

    // Check if current node is an unsplit sub-node
    if (m_nodeHeader.m_IsUnSplitSubLevel)
        {
        GetParentNode()->AdviseDelayedSplitRequested();
        }
    else
        {
        m_DelayedSplitRequested = true;
        }

    HINVARIANTS;


    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::SplitNode(bool propagateSplit)
    {
    HINVARIANTS;

    if (!IsLoaded())
        Load();

    size_t deepestLevelNumber = m_nodeHeader.m_level;


    // The process is completely different if the present node is an unplit sub-level ...
    // If this is the case then we want to split the unsplit owner parent instead.
    // Unfortunately we may currently be in a process that requires that the node structure not be changed ...
    // We delay the split in this case.
    if (m_nodeHeader.m_IsUnSplitSubLevel)
        {
        HASSERT (GetParentNode() != NULL);
//        HASSERT (IsBalanced());

        // Advise parent that a delayed split is in order
        GetParentNode()->AdviseDelayedSplitRequested();

        // We end the process there ... split will occur eventually at the parent's discretion
        return;
        }


    // The split process is organised differently if there are sub-levels non-split under the parent node
    if (m_pSubNodeNoSplit != NULL)
        {
        // In this case we must regather all the points back to current level then split...
        HFCPtr<HGFIndexNode <SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER> > pCurrentNode = m_pSubNodeNoSplit;
        while (pCurrentNode != NULL)
            {
            deepestLevelNumber = pCurrentNode->m_nodeHeader.m_level;
            CopyVector<CONTAINER>::Copy(this, &*pCurrentNode);

            // Note that we do not need to update the Z mins and Max even though we are quadtree because
            // The copied upon node is a parent and already contains the whole Z extent.

            // Dig down to next level
            pCurrentNode = pCurrentNode->m_pSubNodeNoSplit;
            }

        // Destroy nodes

        m_pSubNodeNoSplit->Destroy();
        m_pSubNodeNoSplit = NULL;
        m_nodeHeader.m_IsLeaf = true;
        }

    // The node must be a leaf
    HPRECONDITION(IsLeaf());

    // The position of this is very important ... the delayed split must be desactivated the woonest possible.
    m_DelayedSplitRequested = false;

    double Width = ExtentOp<EXTENT>::GetWidth(GetNodeExtent());
    double Height = ExtentOp<EXTENT>::GetHeight(GetNodeExtent());
    double Thickness = ExtentOp<EXTENT>::GetThickness(GetNodeExtent());

    // Make sure that the nodes can be split. Sometimes, the width or height is very close to zero when max and min are subtracted as a result
    // of the limit of the double of representing 15 digits maximum. If this representation limit is attained
    // then we will not split and mark the node as "unspliteable"
    // We only check the size of one extent
    if ((HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                                 ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                 HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent))) ||
        (HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                 ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                                 HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent))))
        {
        // Values would be virtually equal ... we will not split
        HDEBUGCODE(m_unspliteable = true;)
        return;
        }

    if (m_nodeHeader.m_numberOfSubNodesOnSplit == 4)
        {

        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        }
    else
        {
        HPRECONDITION (Thickness > 0.0);

        if (HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2),
                                    HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent)))
            {
            // Values would be virtually equal ... we will not split
            HDEBUGCODE(m_unspliteable = true;)
            return;
            }


        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2)));

        m_apSubNodes[4] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2),
                                                                    ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[5] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2),
                                                                    ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[6] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2),
                                                                    ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[7] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2),
                                                                    ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                                                                    ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2),
                                                                    ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        }

    // Indicate node is not a leaf anymore
    m_nodeHeader.m_IsLeaf = false;
    m_nodeHeader.m_IsBranched = true;



    if (this->size() > 0)
        {
        PropagateDataDownImmediately(false);
        }


    SetDirty(true);

    if (IsBalanced() && propagateSplit)
        {
        if (GetParentNode() != NULL)
            {
            size_t targetLevel = MAX (GetLevel() + 1, deepestLevelNumber);
            if (targetLevel > GetLevel() + 1)
                {
                // In this case sub-nodes must be pushed down
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    m_apSubNodes[indexNodes]->PushNodeDown (targetLevel);
                    }
                }
            GetParentNode()->PropagateSplitNode(REALTHIS(), targetLevel);
            }
        }


    HINVARIANTS;

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::PushNodeDown(size_t targetLevel)
    {
    this->ValidateInvariantsSoft();

    if (!IsLoaded())
        Load();

    // The total count remains the same


    //  HASSERT (IsBalanced());

    // Check if level is already attained
    if (m_nodeHeader.m_level >= targetLevel)
        return;

    // Make sure conditions are acceptable
    if (!m_nodeHeader.m_IsLeaf)
        {
        // The node is not a leaf ... we ask its descendents to push themselves
        if (m_pSubNodeNoSplit != NULL)
            {
            m_pSubNodeNoSplit->PushNodeDown (targetLevel);
            }
        else
            {
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                {
                m_apSubNodes[indexNodes]->PushNodeDown (targetLevel);
                }
            }
        }
    else
        {
        // The node is a leaf and its level is insufficiant
        m_pSubNodeNoSplit = this->CloneUnsplitChild(EXTENT(m_nodeHeader.m_nodeExtent));

        // We copy the whole content to this sub-node
        m_pSubNodeNoSplit->reserve (this->size());

#if (0)
        for (size_t indexSpatial = 0 ; indexSpatial < this->size() ; indexSpatial++)
            {
            m_pSubNodeNoSplit->push_back (this->operator[](indexSpatial));
            }
#else
        CopyVector<CONTAINER>::Copy(m_pSubNodeNoSplit, this);
#endif

        m_pSubNodeNoSplit->m_nodeHeader.m_contentExtent = m_nodeHeader.m_contentExtent;
        m_pSubNodeNoSplit->m_nodeHeader.m_contentExtentDefined = m_nodeHeader.m_contentExtentDefined;
        m_pSubNodeNoSplit->m_nodeHeader.m_totalCount = m_pSubNodeNoSplit->size();

        m_nodeHeader.m_IsLeaf = false;

        this->clear();



        // Check if the new node is deep enough
        if (m_pSubNodeNoSplit->GetLevel() < targetLevel)
            m_pSubNodeNoSplit->PushNodeDown(targetLevel);
        }


    SetDirty(true);

    this->ValidateInvariantsSoft();

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER>
void HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>::PropagateSplitNode(SELFNODEPTR initiator,
        size_t quadTreeDepth)
    {
    HINVARIANTS;

    // Check if current Node is leaf
    if (IsLeaf())
        {
        // It is a leaf .. check if the depth level is sufficient
        if (GetLevel() < quadTreeDepth)
            {
            // Not deep enough ... split
            // Important note ... this will result in a new propagation event
            // SplitNode(false);
            PushNodeDown (quadTreeDepth);
            }
        // Not abolutely sure about this condition due to the various possible propagations that can occur at the same time
        // HASSERT (GetLevel() == quadTreeDepth);
        }


    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            if (m_pSubNodeNoSplit != initiator)
                m_pSubNodeNoSplit->PropagateSplitNode(REALTHIS(), quadTreeDepth);
            }
        else
            {
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                {
                if (m_apSubNodes[indexNodes] != initiator)
                    m_apSubNodes[indexNodes]->PropagateSplitNode(REALTHIS(), quadTreeDepth);
                }
            }
        }

    // Propagate to parent
    if (GetParentNode() != NULL)
        {
        if (GetParentNode() != initiator)
            GetParentNode()->PropagateSplitNode(REALTHIS(), quadTreeDepth);
        }


    HINVARIANTS;

    }



/**----------------------------------------------------------------------------
 PRIVATE METHOD
 This method inserts a new root that contains the old root in order
 to attempt inclusion of given extent. The root is pushed down by one level.
-----------------------------------------------------------------------------*/
template<typename POINT, typename EXTENT> EXTENT ComputeExtentForPushRootDown(const EXTENT& pi_rObjectExtent, const EXTENT& RootExtent)
    {
    // Calculate center of current root extent
    POINT RootExtentCenter = PointOp<POINT>::Create((ExtentOp<EXTENT>::GetXMax(RootExtent) + ExtentOp<EXTENT>::GetXMin(RootExtent)) / 2.0,
                                                    (ExtentOp<EXTENT>::GetYMax(RootExtent) + ExtentOp<EXTENT>::GetYMin(RootExtent)) / 2.0,
                                                    (ExtentOp<EXTENT>::GetZMax(RootExtent) + ExtentOp<EXTENT>::GetZMin(RootExtent)) / 2.0);

    // Calculate center of spatial object
    POINT ObjectExtentCenter = PointOp<POINT>::Create((ExtentOp<EXTENT>::GetXMax(pi_rObjectExtent) + ExtentOp<EXTENT>::GetXMin(pi_rObjectExtent)) / 2.0,
                                                      (ExtentOp<EXTENT>::GetYMax(pi_rObjectExtent) + ExtentOp<EXTENT>::GetYMin(pi_rObjectExtent)) / 2.0,
                                                      (ExtentOp<EXTENT>::GetZMax(pi_rObjectExtent) + ExtentOp<EXTENT>::GetZMin(pi_rObjectExtent)) / 2.0);


    // Find out what sub-node of new root will current root be
    // and create subnodes appropriately
    EXTENT NewRootExtent;
    if (PointOp<POINT>::GetZ(ObjectExtentCenter) > PointOp<POINT>::GetZ(RootExtentCenter))
        {
        if (PointOp<POINT>::GetX(ObjectExtentCenter) > PointOp<POINT>::GetX(RootExtentCenter))
            {
            if (PointOp<POINT>::GetY(ObjectExtentCenter) > PointOp<POINT>::GetY(RootExtentCenter))
                {

                // Oldroot becomes number 2
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent) + ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent) + ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent) + ExtentOp<EXTENT>::GetThickness(RootExtent));
                }
            else
                {
                // Oldroot becomes number 0
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent) - ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent) + ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent) + ExtentOp<EXTENT>::GetThickness(RootExtent));
                }
            }
        else
            {
            if (PointOp<POINT>::GetY(ObjectExtentCenter) > PointOp<POINT>::GetY(RootExtentCenter))
                {
                // Oldroot becomes number 3
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent) - ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent) + ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent) + ExtentOp<EXTENT>::GetThickness(RootExtent));
                }
            else
                {
                // Oldroot becomes number 1
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent) - ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent) - ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent) + ExtentOp<EXTENT>::GetThickness(RootExtent));
                }
            }
        }
    else
        {
        if (PointOp<POINT>::GetX(ObjectExtentCenter) > PointOp<POINT>::GetX(RootExtentCenter))
            {
            if (PointOp<POINT>::GetY(ObjectExtentCenter) > PointOp<POINT>::GetY(RootExtentCenter))
                {

                // Oldroot becomes number 2
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent) - ExtentOp<EXTENT>::GetThickness(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent) + ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent) + ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent));
                }
            else
                {
                // Oldroot becomes number 0
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent) - ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent) - ExtentOp<EXTENT>::GetThickness(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent) + ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent));
                }
            }
        else
            {
            if (PointOp<POINT>::GetY(ObjectExtentCenter) > PointOp<POINT>::GetY(RootExtentCenter))
                {
                // Oldroot becomes number 3
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent) - ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent) - ExtentOp<EXTENT>::GetThickness(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent) + ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent));
                }
            else
                {
                // Oldroot becomes number 1
                NewRootExtent = ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(RootExtent) - ExtentOp<EXTENT>::GetWidth(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMin(RootExtent) - ExtentOp<EXTENT>::GetHeight(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMin(RootExtent) - ExtentOp<EXTENT>::GetThickness(RootExtent),
                                                         ExtentOp<EXTENT>::GetXMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetYMax(RootExtent),
                                                         ExtentOp<EXTENT>::GetZMax(RootExtent));
                }
            }
        }

    return NewRootExtent;

    }






//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// HGFSpatialIndex Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::HGFSpatialIndex(size_t pi_SplitTreshold, bool balanced, bool propagatesDataDown, HFCPtr<NODE> exampleNode)
    {
    m_propagatesDataDown = propagatesDataDown;
    m_indexHeader.m_SplitTreshold = pi_SplitTreshold;
    m_indexHeader.m_HasMaxExtent = false;
    m_indexHeader.m_balanced = balanced;
    m_indexHeader.m_numberOfSubNodesOnSplit = 4;
//    m_indexHeader.m_contentMinZ = 0;
//    m_indexHeader.m_contentMaxZ = 0;

    m_pExampleNode = exampleNode;
    m_pExampleNode->SetSplitTreshold(pi_SplitTreshold);
    HASSERT(m_pExampleNode->IsBalanced() == balanced);
    HASSERT(m_pExampleNode->PropagatesDataDown() == propagatesDataDown);

    m_indexHeaderDirty = true;

    HINVARIANTS;
    }






//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::~HGFSpatialIndex()
    {
    // Make sure the example node does not attempt storage ...
    m_pExampleNode->Destroy();
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
size_t HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetSplitTreshold() const
    {
    HINVARIANTS;

    return(m_indexHeader.m_SplitTreshold);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::PropagatesDataDown() const
    {
    HINVARIANTS;

    return(m_propagatesDataDown);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
void HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::SetPropagateDataDown(bool propagate)
    {
    m_propagatesDataDown = propagate;
    if (m_pRootNode != NULL)
        {
        m_pRootNode->SetPropagateDataDown (propagate);
        }

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
void HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::PropagateDataDownImmediately()
    {
    if (m_pRootNode != NULL)
        {
        m_pRootNode->PropagateDataDownImmediately(true);
        }
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::IsBalanced() const
    {
    HINVARIANTS;

    return(m_indexHeader.m_balanced);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
void HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::Balance()
    {
    HINVARIANTS;

    if (!m_indexHeader.m_balanced)
        {
        m_indexHeader.m_balanced = true;

        m_indexHeaderDirty = true;

        if (m_pRootNode != NULL)
            {

            size_t depth = m_pRootNode->GetDepth();
            m_pRootNode->Balance(depth);
            }
        }

    HINVARIANTS;

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
size_t HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetNumberOfSubNodesOnSplit() const
    {
    HINVARIANTS;

    return m_indexHeader.m_numberOfSubNodesOnSplit;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
void HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::SetSplitTreshold(size_t pi_SplitTreshold)
    {
    HINVARIANTS;
    HPRECONDITION(pi_SplitTreshold >= 2);

    m_indexHeader.m_SplitTreshold = pi_SplitTreshold;

    m_indexHeaderDirty = true;

    // Check if there is a root node
    if (m_pRootNode != 0)
        {
        // There is a root node ... we set the treshold
        m_pRootNode->SetSplitTreshold(m_SplitTreshold);
        }

    }


/**----------------------------------------------------------------------------
 PROTECTED
 Gets the limiting outter extent (maximum extent) if the spatial index
 was created with a limiting maximum extent. Prior to calling this method
 It must be verified that the spatial effectively has a maximum extent using
 the HasMaxExtent() method.

 @return Returns the maximum extent of spatial index.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
EXTENT HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetMaxExtent() const
    {
    HPRECONDITION (HasMaxExtent());

    return m_indexHeader.m_MaxExtent;
    }

/**----------------------------------------------------------------------------
 PROTECTED
 Returns the root node of the spatial index.
 
 @return Root node.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER> 
    HFCPtr<NODE> HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetRootNode() const
{
	return m_pRootNode;
}

/**----------------------------------------------------------------------------
 PROTECTED
 Gets the effective limiting outter extent.

 @return Returns the extent of spatial index.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
EXTENT HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetIndexExtent() const
    {
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        if (m_indexHeader.m_HasMaxExtent)
            return GetMaxExtent();
        else
            return EXTENT();
        }
    else
        return m_pRootNode->GetNodeExtent();

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
EXTENT HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetContentExtent() const
    {
    if (m_pRootNode == NULL)
        return EXTENT();

    return m_pRootNode->GetContentExtent();
    }



/**----------------------------------------------------------------------------
 PROTECTED
 Indicates if the index has a limiting outter extent (maximum extent).
 In order to have a limited extent the spatial index must have been created with
 a limiting maximum extent.

 @return true if the spatial index has a maximum extent.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::HasMaxExtent() const
    {
    return m_indexHeader.m_HasMaxExtent;
    }




//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER> bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::Add(const SPATIAL& pi_rpSpatialObject)
    {
    HINVARIANTS;


    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = m_pExampleNode->Clone(m_indexHeader.m_MaxExtent);
        else
            m_pRootNode = m_pExampleNode->Clone(SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    if (!m_pRootNode->AddConditional (pi_rpSpatialObject, m_indexHeader.m_HasMaxExtent))
        {
        // Item could not be added

        // Make sure that the node extent contains the object extent
        while (!SpatialOp<SPATIAL, POINT, EXTENT>::IsSpatialInExtent2D(pi_rpSpatialObject, m_pRootNode->GetNodeExtent()))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_indexHeader.m_HasMaxExtent)
                return false;

            // The extent is not contained... we must create a new node
            PushRootDown(SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));
            }


        // The root node contains the spatial object ... add it
        m_pRootNode->Add(pi_rpSpatialObject);
        }

    HINVARIANTS;

    return true;
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::AddArray(SPATIAL* spatialArray, size_t countOfSpatials)
    {
    HINVARIANTS;


    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = m_pExampleNode->Clone(m_indexHeader.m_MaxExtent);
        else
            m_pRootNode = m_pExampleNode->Clone(SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(spatialArray[0]));
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    size_t numberOfPointsAdded = 0;

    while (numberOfPointsAdded < countOfSpatials)
        {
        numberOfPointsAdded = m_pRootNode->AddArrayConditional (spatialArray, numberOfPointsAdded, countOfSpatials, m_indexHeader.m_HasMaxExtent);

        if (numberOfPointsAdded < countOfSpatials)
            {
            // Not all Item could not be added

            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_indexHeader.m_HasMaxExtent)
                return false;

            // The extent is not contained... we must create a new node
            PushRootDown(SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(spatialArray[numberOfPointsAdded]));
            }
        }

    HINVARIANTS;

    return true;
    }


/**----------------------------------------------------------------------------
 Clears the indicated area of all objects. Objects must be completely included in the
 specified shape to be removed.

 @return true if object was removed and false otherwise.

-----------------------------------------------------------------------------*/
//template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER> bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::Clear(HFCPtr<HVEShape> pi_shapeToClear)
//{
//    if (m_pRootNode == NULL)
//        return false;
//
//    return m_pRootNode->Clear(pi_shapeToClear);
//}

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::Remove(const SPATIAL& pi_rpSpatialObject)
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return false;

    return m_pRootNode->Remove(pi_rpSpatialObject);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
size_t HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetIn(const EXTENT& pi_rExtent,
                                                                         list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return 0;

    return m_pRootNode->GetIn(pi_rExtent, pio_rListOfObjects);
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER>
size_t HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetDepth() const
    {

    if (m_pRootNode == NULL)
        return 0;

    return m_pRootNode->GetDepth();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER> uint64_t HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::GetCount() const
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return 0;

    return m_pRootNode->GetCount();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER> bool HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::IsEmpty() const
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return true;


    return m_pRootNode->IsEmpty();
    }



/**----------------------------------------------------------------------------
 PRIVATE METHOD
 This method inserts a new root that contains the old root in order
 to attempt inclusion of given extent. The root is pushed down by one level.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER> void HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>::PushRootDown(const EXTENT& pi_rObjectExtent)
    {
    HINVARIANTS;



    int fullDepth = 0;
    if (IsBalanced())
        {
        fullDepth = (int)m_pRootNode->GetDepth();
        }

    // The new node will be twice the sizes of current root node
    EXTENT RootExtent = m_pRootNode->GetNodeExtent();

    // Calculate center of current root extent
    POINT RootExtentCenter = PointOp<POINT>::Create((ExtentOp<EXTENT>::GetXMax(RootExtent) + ExtentOp<EXTENT>::GetXMin(RootExtent)) / 2.0,
                                                    (ExtentOp<EXTENT>::GetYMax(RootExtent) + ExtentOp<EXTENT>::GetYMin(RootExtent)) / 2.0,
                                                    (ExtentOp<EXTENT>::GetZMax(RootExtent) + ExtentOp<EXTENT>::GetZMin(RootExtent)) / 2.0);


    EXTENT NewRootExtent = ComputeExtentForPushRootDown<POINT, EXTENT>(pi_rObjectExtent, RootExtent);

    // The Z extent must be adjusted when we are a quadtree ... this serves as a container of the Z content extent instead of a
    // node fixed extent.
    if (GetNumberOfSubNodesOnSplit() == 4)
        {
        ExtentOp<EXTENT>::SetZMin(NewRootExtent, ExtentOp<EXTENT>::GetZMin(RootExtent));
        ExtentOp<EXTENT>::SetZMax(NewRootExtent, ExtentOp<EXTENT>::GetZMax(RootExtent));
        }

    // Create new node
    HFCPtr<NODE> pNewRootNode = m_pExampleNode->Clone(NewRootExtent);

    // Split new rootnode
    pNewRootNode->SplitNode(false);

    // Replace the appropriate node by current root node
    for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
        {
        if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D (pNewRootNode->m_apSubNodes[indexNode]->GetNodeExtent(), RootExtentCenter))
            {
            pNewRootNode->m_apSubNodes[indexNode]->Destroy();
            pNewRootNode->m_apSubNodes[indexNode] = m_pRootNode;
            pNewRootNode->m_apSubNodes[indexNode]->IncreaseLevel();
            pNewRootNode->m_apSubNodes[indexNode]->SetParentNode (&*pNewRootNode);
            break;
            }
        }

    pNewRootNode->m_nodeHeader.m_contentExtent = m_pRootNode->m_nodeHeader.m_contentExtent;
    pNewRootNode->m_nodeHeader.m_contentExtentDefined = m_pRootNode->m_nodeHeader.m_contentExtentDefined;
    pNewRootNode->m_nodeHeader.m_totalCount = m_pRootNode->m_nodeHeader.m_totalCount;
    pNewRootNode->m_nodeHeader.m_totalCountDefined = m_pRootNode->m_nodeHeader.m_totalCountDefined;




    // Set new root as root
    m_pRootNode = pNewRootNode;

    // If is balanced ... propagate a depth change (this will generate subnodes to the new nodes)
    if (IsBalanced())
        {
        m_pRootNode->PropagateSplitNode (NULL, fullDepth + 1);
        }

    HINVARIANTS;

    m_pRootNode->ValidateInvariants();
    }
