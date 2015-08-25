//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMPointIndex.hpp $
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

// SMPointIndexNode Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode(size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        HFCPtr<HPMCountLimitedPool<POINT> > pool,
        HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
        ISMPointIndexFilter<POINT, EXTENT>* filter,
        bool balanced,
        bool propagateDataDown,         
        CreatedNodeMap*                      createdNodeMap)
        : CONTAINER()
    //: HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<SMPointIndexNode<POINT, EXTENT> >, SMPointNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, balanced, propagateDataDown, false, false)
    {
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_NbObjects = -1;
    m_delayedDataPropagation = !propagateDataDown;
    m_pParentNode = 0;
    m_isParentNodeSet = true;
    m_isGenerating = false;
    m_nodeHeader.m_SplitTreshold = pi_SplitTreshold;
    m_nodeHeader.m_IsLeaf = true;
    m_nodeHeader.m_IsUnSplitSubLevel = false;
    m_nodeHeader.m_IsBranched = false;
    m_nodeHeader.m_level = 0;
    m_nodeHeader.m_balanced = balanced;
    m_nodeHeader.m_nodeExtent = pi_rExtent;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 8;
    m_nodeHeader.m_apSubNodeID.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_apSubNodes.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_nodeHeader.m_contentExtentDefined = false;
    m_nodeHeader.m_totalCountDefined = true;
    m_nodeHeader.m_totalCount = 0;
    m_nodeHeader.m_arePoints3d = false;

    for (size_t nodeInd = 0; nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS; nodeInd++)
        {
        m_nodeHeader.m_apAreNeighborNodesStitched[nodeInd] = false;
        }

    HDEBUGCODE(m_unspliteable = false;)
        HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)


        // Do not allow NULL sized extent ...
        if (!(ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent) > ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)))
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + max(fabs(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent)) * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0, 1.0)));
    if (!(ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent) > ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)))
        ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + max(fabs(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent)) * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0, 1.0)));

    HINVARIANTS;
    SetStore (store);
    SetPool (pool);
    m_NbObjects = -1;

    m_filter = filter; 
    m_createdNodeMap = createdNodeMap;
    m_nodeHeader.m_filtered = false;
    m_nodeHeader.m_nbFaceIndexes = 0;
	setNbPointsUsedForMeshIndex(0);
    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;
    m_nodeHeader.m_numberOfMeshComponents = 0;
    m_nodeHeader.m_meshComponents = nullptr;
    for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }
    
    this->ValidateInvariantsSoft();

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& pi_rpParentNode)
    //: HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<SMPointIndexNode<POINT, EXTENT> >, SMPointNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, &*pi_rpParentNode)
    : CONTAINER(), m_pParentNode(pi_rpParentNode)
    {
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 8;
    m_nodeHeader.m_apSubNodeID.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_apSubNodes.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_delayedDataPropagation = !pi_rpParentNode->PropagatesDataDown();
    m_isParentNodeSet = true;
    m_isGenerating = pi_rpParentNode->m_isGenerating;

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
    m_nodeHeader.m_arePoints3d = false;

    for (size_t nodeInd = 0; nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS; nodeInd++)
        {
        m_nodeHeader.m_apAreNeighborNodesStitched[nodeInd] = false;
        }

    HDEBUGCODE(m_unspliteable = false;)
        HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)

        // Call soft invariant as the parent is most probably reconfiguring
        this->ValidateInvariantsSoft();
    SetStore (static_cast<SMPointTileStore<POINT, EXTENT>*>(&*pi_rpParentNode->GetStore()));
    SetPool (pi_rpParentNode->GetPool());

    m_filter = pi_rpParentNode->GetFilter();
    m_nodeHeader.m_filtered = false;
    m_nodeHeader.m_nbFaceIndexes = 0;
	setNbPointsUsedForMeshIndex(0);
    m_nodeHeader.m_numberOfMeshComponents = 0;
    m_nodeHeader.m_meshComponents = nullptr;
    m_nodeHeader.m_arePoints3d = pi_rpParentNode->m_nodeHeader.m_arePoints3d;
    if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
    else SetNumberOfSubNodesOnSplit(8);

    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;
    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }
    
    this->ValidateInvariantsSoft();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& pi_rpParentNode,
        bool IsUnsplitSubLevel)
        :CONTAINER(), m_pParentNode(pi_rpParentNode)
   // : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<SMPointIndexNode<POINT, EXTENT> >, SMPointNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, &*pi_rpParentNode, IsUnsplitSubLevel)
    {
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 8;
    m_nodeHeader.m_apSubNodeID.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_apSubNodes.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_delayedDataPropagation = !pi_rpParentNode->PropagatesDataDown();
    m_isParentNodeSet = true;
    m_isGenerating = pi_rpParentNode->m_isGenerating;
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
    m_nodeHeader.m_arePoints3d = false;

    for (size_t nodeInd = 0; nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS; nodeInd++)
        {
        m_nodeHeader.m_apAreNeighborNodesStitched[nodeInd] = false;
        }

    HDEBUGCODE(m_unspliteable = false;)
        HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)

        // Call soft invariant as the parent is most probably reconfiguring
        this->ValidateInvariantsSoft();
    m_NbObjects = -1;
    SetStore (static_cast<SMPointTileStore<POINT, EXTENT>*>(&*pi_rpParentNode->GetStore()));
    SetPool (pi_rpParentNode->GetPool());

    m_filter = pi_rpParentNode->GetFilter();
    m_nodeHeader.m_filtered = false;
    m_nodeHeader.m_nbFaceIndexes = 0;
	setNbPointsUsedForMeshIndex(0);
    m_nodeHeader.m_numberOfMeshComponents = 0;
    m_nodeHeader.m_meshComponents = nullptr;    
    m_nodeHeader.m_arePoints3d = pi_rpParentNode->m_nodeHeader.m_arePoints3d;
    if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
    else SetNumberOfSubNodesOnSplit(8);

    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;
    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }
   
    this->ValidateInvariantsSoft();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode (const SMPointIndexNode<POINT, EXTENT>& pi_rNode)
    //: HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<SMPointIndexNode<POINT, EXTENT> >, SMPointNodeHeader<EXTENT> >(&*pi_rNode)
    : CONTAINER(pi_rNode),
    m_pParentNode(0),
    m_ListOfObjects(pi_rNode.m_ListOfObjects)
    {
    // A copied node may not have a parent
    HPRECONDITION(pi_rNode.GetParentNodePtr() == 0);

    m_isParentNodeSet = true;
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_NbObjects = -1;
    m_delayedDataPropagation = !pi_rNode->PropagatesDataDown();
    m_isGenerating = pi_rNode->m_isGenerating;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 8;
    m_nodeHeader.m_apSubNodesID.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);


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
    m_nodeHeader.m_arePoints3d = false;

    for (size_t nodeInd = 0; nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS; nodeInd++)
        {
        m_apAreNeighborNodesStitched[nodeInd] = false;
        }

    HDEBUGCODE(m_unspliteable = pi_rNode.m_unspliteable;)
        HDEBUGCODE(m_parentOfAnUnspliteableNode = pi_rNode.m_parentOfAnUnspliteableNode;)

        // Check if there are subnodes
        if (!m_nodeHeader.m_IsLeaf)
            {
            if (pi_rNode.m_pSubNodeNoSplit != NULL && !pi_rNode.m_pSubNodeNoSplit->IsVirtualNode())
                m_pSubNodeNoSplit = new Node(pi_rNode.m_SubNodeNoSplit, this, m_filter);
            else if (pi_rNode.m_pSubNodeNoSplit == NULL)
                {
                m_nodeHeader.m_IsBranched = true;

                for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    m_apSubNodes[indexNode] = this->CloneChild();
                    }
                }
            else m_pSubNodeNoSplit = this->CloneUnsplitChildVirtual();
            }

    //NEEDS_WORK_SM Does we need to have clone neighbor
    assert(!"Not implemented yet");

    HINVARIANTS;

    SetStore (static_cast<SMPointTileStore<POINT, EXTENT>*>(&*pi_rNode->GetStore()));
    SetPool (pi_rNode->GetPool());
    m_NbObjects = -1;

    m_filter = pi_rNode->GetFilter();
    m_nodeHeader.m_filtered = false;
    m_nodeHeader.m_nbFaceIndexes = 0;
	setNbPointsUsedForMeshIndex(0);
    m_nodeHeader.m_numberOfMeshComponents = 0;
    m_nodeHeader.m_meshComponents = nullptr;
    m_nodeHeader.m_arePoints3d = pi_rNode->m_nodeHeader.m_arePoints3d;
    if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
    else SetNumberOfSubNodesOnSplit(8);

    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;
    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }    

    this->ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
//NEEDS_WORK_SM: This one calls a parent constructor that doesn't have an implementation?
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode(const SMPointIndexNode& pi_rNode,
        const HFCPtr<SMPointIndexNode>& pi_rpParentNode)
    //: HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<SMPointIndexNode<POINT, EXTENT> >, SMPointNodeHeader<EXTENT> >(&*pi_rNode, &*pi_rpParentNode)
    {
    assert(!"template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode(const SMPointIndexNode& pi_rNode,const HFCPtr<SMPointIndexNode>& pi_rpParentNode)");
   /* SetStore (static_cast<SMPointTileStore<POINT, EXTENT>*>(&*pi_rNode->GetStore()));
    SetPool (pi_rNode->GetPool());

    m_filter = pi_rpParentNode->GetFilter();
    m_nodeHeader.m_filtered = false;
    m_nodeHeader.m_nbFaceIndexes = 0;
	setNbPointsUsedForMeshIndex(0);
    m_nodeHeader.m_numberOfMeshComponents = 0;
    m_nodeHeader.m_meshComponents = nullptr;
    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;
    m_nodeHeader.m_arePoints3d = pi_rpParentNode->m_nodeHeader.m_arePoints3d;

    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }    

    this->ValidateInvariantsSoft();*/
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode(HPMBlockID blockID,
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > parent,
        HFCPtr<HPMCountLimitedPool<POINT> > pool,
        HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
        ISMPointIndexFilter<POINT, EXTENT>* filter,
        bool balanced,
        bool propagateDataDown, 
        CreatedNodeMap* createdNodeMap)
        : CONTAINER()
   // : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<SMPointIndexNode<POINT, EXTENT> >, SMPointNodeHeader<EXTENT> >(propagateDataDown)
    {
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_NbObjects = -1;
    m_delayedDataPropagation = propagateDataDown;
    m_pParentNode = 0;
    m_isParentNodeSet = false;
    m_isGenerating = false;
    m_nodeHeader.m_SplitTreshold = 10000;
    m_nodeHeader.m_IsLeaf = true;
    m_nodeHeader.m_IsUnSplitSubLevel = false;
    m_nodeHeader.m_IsBranched = false;
    m_nodeHeader.m_level = 0;
    m_nodeHeader.m_balanced = false;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 8;
    m_nodeHeader.m_apSubNodeID.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_apSubNodes.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_nodeHeader.m_contentExtentDefined = false;
    m_nodeHeader.m_totalCountDefined = true;
    m_nodeHeader.m_totalCount = 0;
    m_nodeHeader.m_arePoints3d = false;

    for (size_t nodeInd = 0; nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS; nodeInd++)
        {
        m_nodeHeader.m_apAreNeighborNodesStitched[nodeInd] = false;
        }

    HDEBUGCODE(m_unspliteable = false;)
        HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)

        HINVARIANTS;
    HPRECONDITION (blockID.IsValid());
    SetStore (store);
    SetPool (pool);

    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;

    m_loaded = false;
    m_NbObjects = -1;
    m_filter = filter; 
    m_createdNodeMap = createdNodeMap;

    m_storeBlockID = blockID;
    m_discarded = true;
    SetParentNodePtr(parent);     
    m_nodeHeader.m_balanced = balanced;
    HPMCountLimitedPoolItem::SetDirty (false);

    this->ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::SMPointIndexNode(HPMBlockID blockID,        
        HFCPtr<HPMCountLimitedPool<POINT> > pool,
        HFCPtr<SMPointTileStore<POINT, EXTENT> > store,
        ISMPointIndexFilter<POINT, EXTENT>* filter,
        bool balanced,
        bool propagateDataDown, 
        CreatedNodeMap* createdNodeMap)
        : CONTAINER()
    //: HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<SMPointIndexNode<POINT, EXTENT> >, SMPointNodeHeader<EXTENT> >(propagateDataDown)
    {
    m_loaded = true;
    m_destroyed = false;
    m_DelayedSplitRequested = false;
    m_NbObjects = -1;
    m_delayedDataPropagation = propagateDataDown;
    m_pParentNode = 0;
    m_isParentNodeSet = false;
    m_isGenerating = false;
    m_nodeHeader.m_SplitTreshold = 10000;
    m_nodeHeader.m_IsLeaf = true;
    m_nodeHeader.m_IsUnSplitSubLevel = false;
    m_nodeHeader.m_IsBranched = false;
    m_nodeHeader.m_level = 0;
    m_nodeHeader.m_balanced = false;
    m_nodeHeader.m_numberOfSubNodesOnSplit = 8;
    m_nodeHeader.m_apSubNodeID.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_apSubNodes.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    m_nodeHeader.m_contentExtentDefined = false;
    m_nodeHeader.m_totalCountDefined = true;
    m_nodeHeader.m_totalCount = 0;
    m_nodeHeader.m_arePoints3d = false;

    for (size_t nodeInd = 0; nodeInd < MAX_NUM_NEIGHBORNODE_POSITIONS; nodeInd++)
        {
        m_nodeHeader.m_apAreNeighborNodesStitched[nodeInd] = false;
        }

    HDEBUGCODE(m_unspliteable = false;)
        HDEBUGCODE(m_parentOfAnUnspliteableNode = false;)

        HINVARIANTS;
    HPRECONDITION (blockID.IsValid());

    SetStore (store);
    SetPool (pool);

    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;

    m_loaded = false;
    m_NbObjects = -1;
    m_filter = filter;
    m_createdNodeMap = createdNodeMap;

    m_storeBlockID = blockID;
    m_discarded = true;    
    m_nodeHeader.m_balanced = balanced;
    HPMCountLimitedPoolItem::SetDirty (false);

    this->ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> SMPointIndexNode<POINT, EXTENT>::~SMPointIndexNode()
    {  
    HINVARIANTS;
    if (!IsDestroyed())
        {           
        // Unload self ... this should result in discard 
        Unload();

        if (!Discarded() && IsDirty())
            {
            // This is a bit too late to discard as we are during destruction ... virtual overload of Discard is not accessible if inherited
            // Usually inheritance will not exist at this time ...
            Discard();
            }
        }
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::Clone () const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), GetNodeExtent(), GetPool(), dynamic_cast<SMPointTileStore<POINT, EXTENT>* >(&*(GetStore())), GetFilter(), IsBalanced(), PropagatesDataDown(),  m_createdNodeMap);
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::Clone (const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, GetPool(), dynamic_cast<SMPointTileStore<POINT, EXTENT>* >(&*(GetStore())), GetFilter(), IsBalanced(), PropagatesDataDown(),  m_createdNodeMap);
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::CloneChild (const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMPointIndexNode<POINT, EXTENT>*>(this));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::CloneUnsplitChild (const EXTENT& newNodeExtent) const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<SMPointIndexNode<POINT, EXTENT>*>(this), true);
    return pNewNode;
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::CloneUnsplitChildVirtual() const
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMIndexNodeVirtual<POINT, EXTENT, SMPointIndexNode<POINT, EXTENT>>(const_cast<SMPointIndexNode<POINT, EXTENT>*>(this));
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::CreateNewChildNode(HPMBlockID blockID)
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(blockID, this, m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(m_store)), m_filter, m_nodeHeader.m_balanced, !(m_delayedDataPropagation), m_createdNodeMap);
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID)
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(blockID, m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(m_store)), m_filter, m_nodeHeader.m_balanced, !(m_delayedDataPropagation), m_createdNodeMap);
    return pNewNode;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::Load() const
    {
    HPRECONDITION (!IsLoaded());

    if (0 == static_cast<SMPointTileStore<POINT, EXTENT>*>(&*m_store)->LoadHeader (&m_nodeHeader, m_storeBlockID))
        {
        // Something went wrong
        throw HFCUnknownException();
        }
        
    size_t nbPointsUsedForMeshIndex = (size_t)ceil((double)m_nodeHeader.m_nbFaceIndexes * sizeof(int32_t) / (double)sizeof(POINT));

    setNbPointsUsedForMeshIndex(nbPointsUsedForMeshIndex);

    SMPointIndexNode<POINT, EXTENT>* UNCONSTTHIS =  const_cast<SMPointIndexNode<POINT, EXTENT>* >(this);

    UNCONSTTHIS->m_count = UNCONSTTHIS->m_store->GetBlockDataCount (m_storeBlockID);
    HDEBUGCODE(UNCONSTTHIS->m_countLoaded = true;);              

    // If there are sub-nodes we must create them
    if (!UNCONSTTHIS->m_nodeHeader.m_IsLeaf)
        {
        if (!UNCONSTTHIS->m_nodeHeader.m_IsBranched)
            {
            assert(UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID.m_integerInitialized == true && UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID.m_alternateID == 0);
            
            CreatedNodeMap::iterator nodeIter(m_createdNodeMap->find(UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID.m_integerID));

            if (nodeIter == m_createdNodeMap->end())
                {                
                UNCONSTTHIS->m_pSubNodeNoSplit = UNCONSTTHIS->CreateNewChildNode(UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID);//new SMPointIndexNode<POINT, EXTENT>(UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID, UNCONSTTHIS, UNCONSTTHIS->m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)), UNCONSTTHIS->m_filter, UNCONSTTHIS->m_nodeHeader.m_balanced, !(UNCONSTTHIS->m_delayedDataPropagation), UNCONSTTHIS->m_mesher2_5d, UNCONSTTHIS->m_mesher3d, UNCONSTTHIS->m_createdNodeMap);
                m_createdNodeMap->insert(std::pair<__int64, HFCPtr<SMPointIndexNode<POINT, EXTENT>>>(UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID.m_integerID, UNCONSTTHIS->m_pSubNodeNoSplit));                
                }
            else
                {
                UNCONSTTHIS->m_pSubNodeNoSplit = nodeIter->second;                
                }

            UNCONSTTHIS->m_pSubNodeNoSplit->m_nodeHeader.m_IsUnSplitSubLevel = true;
            }
        else
            {
            UNCONSTTHIS->m_apSubNodes.resize(UNCONSTTHIS->m_nodeHeader.m_numberOfSubNodesOnSplit);
            // ATTENTION! DO NOT CALL GetNumberOfSubNodesOnSplit() FUNCTION AS IT WILL CALL Load() RESULTING
            // INTO AN INFINITE LOOP
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                assert(UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode].m_integerInitialized == true && UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode].m_alternateID == 0);

                CreatedNodeMap::iterator nodeIter(m_createdNodeMap->find(UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode].m_integerID));

                if (nodeIter == m_createdNodeMap->end())
                    {                
                    UNCONSTTHIS->m_apSubNodes[indexNode] = UNCONSTTHIS->CreateNewChildNode(UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode]); //new SMPointIndexNode<POINT, EXTENT>(UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode], UNCONSTTHIS, UNCONSTTHIS->m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)), UNCONSTTHIS->m_filter, UNCONSTTHIS->m_nodeHeader.m_balanced, !(UNCONSTTHIS->m_delayedDataPropagation), UNCONSTTHIS->m_mesher2_5d, UNCONSTTHIS->m_mesher3d, UNCONSTTHIS->m_createdNodeMap);
                    m_createdNodeMap->insert(std::pair<__int64, HFCPtr<SMPointIndexNode<POINT, EXTENT>>>(UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode].m_integerID, UNCONSTTHIS->m_apSubNodes[indexNode]));                
                    }
                else
                    {
                    UNCONSTTHIS->m_apSubNodes[indexNode] = nodeIter->second;
                    }
                
                UNCONSTTHIS->m_apSubNodes[indexNode]->m_nodeHeader.m_IsUnSplitSubLevel = false;
                }
            }
        }
   
     for (size_t neighborPosIndex = 0; neighborPosIndex < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosIndex++)
        {            
        for (size_t neigborIndex = 0; neigborIndex < UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex].size(); neigborIndex++)
            {            
            assert(UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neigborIndex].m_integerInitialized == true && UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neigborIndex].m_alternateID == 0);
            assert(UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neigborIndex].IsValid());

            CreatedNodeMap::iterator nodeIter(m_createdNodeMap->find(UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neigborIndex].m_integerID));            

            if (nodeIter == m_createdNodeMap->end())
                {                  
                UNCONSTTHIS->m_apNeighborNodes[neighborPosIndex].push_back(UNCONSTTHIS->CreateNewNode(UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neigborIndex]));//new SMPointIndexNode<POINT, EXTENT>(UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neigborIndex], UNCONSTTHIS->m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)), UNCONSTTHIS->m_filter, UNCONSTTHIS->m_nodeHeader.m_balanced, !(UNCONSTTHIS->m_delayedDataPropagation), UNCONSTTHIS->m_mesher2_5d, UNCONSTTHIS->m_mesher3d, UNCONSTTHIS->m_createdNodeMap));
                m_createdNodeMap->insert(std::pair<__int64, HFCPtr<SMPointIndexNode<POINT, EXTENT>>>(UNCONSTTHIS->m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neigborIndex].m_integerID, UNCONSTTHIS->m_apNeighborNodes[neighborPosIndex].back()));                
                }
            else
                {
                UNCONSTTHIS->m_apNeighborNodes[neighborPosIndex].push_back(nodeIter->second);
                }                        
            }
        }

    if (!IsParentSet())
        {
        if (UNCONSTTHIS->m_nodeHeader.m_parentNodeID != IDTMFile::GetNullNodeID())
            {             
            CreatedNodeMap::iterator nodeIter(m_createdNodeMap->find(UNCONSTTHIS->m_nodeHeader.m_parentNodeID.m_integerID));            

            if (nodeIter == m_createdNodeMap->end())
                {
                UNCONSTTHIS->SetParentNodePtr(UNCONSTTHIS->CreateNewNode(UNCONSTTHIS->m_nodeHeader.m_parentNodeID.m_integerID));//new SMPointIndexNode<POINT, EXTENT>(UNCONSTTHIS->m_nodeHeader.m_parentNodeID.m_integerID, UNCONSTTHIS->m_pool, static_cast<SMPointTileStore<POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)), UNCONSTTHIS->m_filter, UNCONSTTHIS->m_nodeHeader.m_balanced, !(UNCONSTTHIS->m_delayedDataPropagation), UNCONSTTHIS->m_mesher2_5d, UNCONSTTHIS->m_mesher3d, UNCONSTTHIS->m_createdNodeMap));                
                m_createdNodeMap->insert(std::pair<__int64, HFCPtr<SMPointIndexNode<POINT, EXTENT>>>(UNCONSTTHIS->m_nodeHeader.m_parentNodeID.m_integerID, UNCONSTTHIS->GetParentNodePtr()));                
                }
            else
                {
                UNCONSTTHIS->SetParentNodePtr(nodeIter->second);
                }
            }
        }
        
    m_loaded = true;

    // Validate invariants
    ValidateInvariantsSoft();

    // Validate sub-nodes invariants
    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->ValidateInvariantsSoft();
        else
            {
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                m_apSubNodes[indexNode]->ValidateInvariantsSoft();
                }
            }
        }
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::Unload() const
    {
    if (IsLoaded())
        {
        SMPointIndexNode<POINT, EXTENT>* UNCONSTTHIS =  const_cast<SMPointIndexNode<POINT, EXTENT>* >(this);

        // If there are sub-nodes we must create them
        if (HasRealChildren())
            {
            if (UNCONSTTHIS->m_pSubNodeNoSplit != NULL && !UNCONSTTHIS->m_pSubNodeNoSplit->IsVirtualNode())
                {
                UNCONSTTHIS->m_nodeHeader.m_IsBranched = false;

                UNCONSTTHIS->m_pSubNodeNoSplit->Unload();
                }
            else if (UNCONSTTHIS->m_pSubNodeNoSplit == NULL)
                {
                UNCONSTTHIS->m_nodeHeader.m_IsBranched = true;

                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    if (UNCONSTTHIS->m_apSubNodes[indexNode] != NULL) UNCONSTTHIS->m_apSubNodes[indexNode]->Unload();
                    }
                }
            }

        
        for (size_t indexNode = 0 ; indexNode < IDTMFile::NeighborNodesTable::MAX_QTY; indexNode++)
            {
            if (UNCONSTTHIS->m_apNeighborNodes[indexNode].size() > 0)
                {
                UNCONSTTHIS->m_apNeighborNodes[indexNode].clear();
                }
            }

        if (!Discarded() && IsDirty())
            {
            Discard();
            }

        // If there are sub-nodes we must create them
        if (!UNCONSTTHIS->m_nodeHeader.m_IsLeaf)
            {
            if (UNCONSTTHIS->m_pSubNodeNoSplit != NULL)
                {
                UNCONSTTHIS->m_pSubNodeNoSplit->SetParentNodePtr(0); 

                UNCONSTTHIS->m_pSubNodeNoSplit = NULL;
                }
            else
                {
                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    UNCONSTTHIS->m_apSubNodes[indexNode]->SetParentNodePtr(0); 
                    UNCONSTTHIS->m_apSubNodes[indexNode] = NULL;
                    }
                }
            }

        m_loaded = false;
        }

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::IsLoaded() const
    {
    return m_loaded;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::IsDestroyed() const
    {
    return m_destroyed;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Destroy()
    {
    this->ValidateInvariantsSoft();

    // The very first step is to sever the parent node relation
    // This will prevent the propagation of undue events towards the parent.
    m_pParentNode = NULL;

    // If there are child nodes they must be destroyed first.
    if (HasRealChildren())
        {
        if (IsParentOfARealUnsplitNode())
            {
            m_pSubNodeNoSplit->Destroy();
            }
        else
            {
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                {
                if (m_apSubNodes[indexNodes] != 0)
                    {
                    m_apSubNodes[indexNodes]->Destroy();
                    }
                }
            }
        }

    /*NEEDS_WORK_SM Does we need that?
    for (size_t indexNode = 0 ; indexNode < IDTMFile::NeighborNodesTable::MAX_QTY; indexNode++)
    {
    if (m_apNeighborNodes[indexNode] != 0)
    {
    m_apNeighborNodes[indexNode]->Destroy();
    }
    }
    */

    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {
        for (size_t neighborInd = 0; neighborInd < m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT>> newNeighborNode;

            m_apNeighborNodes[neighborPosInd][neighborInd]->AdviseNeighborNodeChange(this, newNeighborNode);
            }
        }


    m_destroyed = true;

    this->clear();
    SetDirty(false);
    m_loaded = false;


    HINVARIANTS;
    if (GetBlockID().IsValid())
        m_store->DestroyBlock(GetBlockID());

    HINVARIANTS;

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndexNode<POINT, EXTENT>::GetIn(const EXTENT& pi_rExtent,
                                              list<POINT>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // Check if coordinate falls inside node extent
    if (ExtentOp<EXTENT>::Overlap(GetNodeExtent(), pi_rExtent))
        {
        // The point is located inside the node ...
        // Obtain objects from subnodes (if any)
        if (HasRealChildren())
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                m_pSubNodeNoSplit->GetIn(pi_rExtent, pio_rListOfObjects);
                }
            else
                {
                for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                    {
                    m_apSubNodes[i]->GetIn(pi_rExtent, pio_rListOfObjects);
                    }
                }

            }

        // Search in present list of objects for current node
        CONTAINER::const_iterator itr = this->begin();
        for (; itr != this->end(); ++itr)
            {
            // Check if point is in extent of object
            if (ExtentOp<EXTENT>::Overlap(pi_rExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(*itr)))
                {
                // The point falls inside extent of object .. we add a reference to the list
                pio_rListOfObjects.push_back(*itr);
                }
            }
        }

    HINVARIANTS;


    // Return number of newly found objects
    return(pio_rListOfObjects.size() - InitialNumberOfObjects);
    }


template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::FindNode(EXTENT ext, size_t level) const
    {
    if (ExtentOp<EXTENT>::OutterOverlap(ext, m_nodeHeader.m_nodeExtent))
        {
        if (abs((ExtentOp<EXTENT>::GetXMin(ext) - ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent))) < 0.0001
            && abs((ExtentOp<EXTENT>::GetXMax(ext) - ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent))) < 0.0001 && abs((ExtentOp<EXTENT>::GetYMin(ext) - ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent))) < 0.0001
            && abs((ExtentOp<EXTENT>::GetYMax(ext) - ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent))) < 0.0001 && abs((ExtentOp<EXTENT>::GetZMin(ext) - ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent)))< 0.0001
            && abs((ExtentOp<EXTENT>::GetZMax(ext) - ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent))) < 0.0001 && level == m_nodeHeader.m_level)
            {
            return const_cast<SMPointIndexNode<POINT,EXTENT>*>(this);
            }
        if (m_apSubNodes.size() > 0 && m_apSubNodes[0] != NULL && m_nodeHeader.m_level <= level)
            {
            for (size_t i = 0; i < m_nodeHeader.m_numberOfSubNodesOnSplit; i++)
                if (m_apSubNodes[i] != nullptr && ExtentOp<EXTENT>::OutterOverlap(ext, m_apSubNodes[i]->m_nodeHeader.m_nodeExtent))
                    {
                    auto node = m_apSubNodes[i]->FindNode(ext, level);
                    if (node != nullptr) return node;
                    }
            }
        }
    return nullptr;
    }


template<class POINT, class EXTENT>
HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::AddChild(EXTENT newExtent)
    {
    auto childNodeP = this->CloneChild(newExtent);
    if (m_apSubNodes[0] == nullptr) m_apSubNodes.clear();
    m_pSubNodeNoSplit = nullptr;
    m_apSubNodes.push_back(childNodeP);
    m_nodeHeader.m_numberOfSubNodesOnSplit = m_apSubNodes.size();
    m_nodeHeader.m_IsLeaf = false;
    m_nodeHeader.m_IsBranched = true;
    m_nodeHeader.m_apSubNodeID.resize(m_nodeHeader.m_numberOfSubNodesOnSplit);
    SetDirty(true);
    return childNodeP;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SortSubNodes()
    {
    if (GetNumberOfSubNodesOnSplit() != 4 && GetNumberOfSubNodesOnSplit() != 8) return;

    vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> newSubNodes(GetNumberOfSubNodesOnSplit());
    for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); i++)
        {
        if (ExtentOp<EXTENT>::GetXMin(m_apSubNodes[i]->m_nodeHeader.m_nodeExtent) == ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent))
            {
            if (ExtentOp<EXTENT>::GetYMin(m_apSubNodes[i]->m_nodeHeader.m_nodeExtent) == ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent))
                {
                if (GetNumberOfSubNodesOnSplit() == 8 && ExtentOp<EXTENT>::GetZMax(m_apSubNodes[i]->m_nodeHeader.m_nodeExtent) == ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent))
                    newSubNodes[6] = m_apSubNodes[i];
                else newSubNodes[2] = m_apSubNodes[i];
                }
            else
                {
                if (GetNumberOfSubNodesOnSplit() == 8 && ExtentOp<EXTENT>::GetZMax(m_apSubNodes[i]->m_nodeHeader.m_nodeExtent) == ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent))
                    newSubNodes[4] = m_apSubNodes[i];
                else newSubNodes[0] = m_apSubNodes[i];
                }
            }
        else
            {
            if (ExtentOp<EXTENT>::GetYMin(m_apSubNodes[i]->m_nodeHeader.m_nodeExtent) == ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent))
                {
                if (GetNumberOfSubNodesOnSplit() == 8 && ExtentOp<EXTENT>::GetZMax(m_apSubNodes[i]->m_nodeHeader.m_nodeExtent) == ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent))
                    newSubNodes[7] = m_apSubNodes[i];
                else newSubNodes[3] = m_apSubNodes[i];
                }
            else
                {
                if (GetNumberOfSubNodesOnSplit() == 8 && ExtentOp<EXTENT>::GetZMax(m_apSubNodes[i]->m_nodeHeader.m_nodeExtent) == ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent))
                    newSubNodes[5] = m_apSubNodes[i];
                else newSubNodes[1] = m_apSubNodes[i];
                }
            }
        }
    m_apSubNodes = newSubNodes;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SplitNode(POINT splitPosition, bool propagateSplit)
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
        HASSERT(GetParentNode() != NULL);
        //        HASSERT (IsBalanced());

        // Advise parent that a delayed split is in order
        GetParentNode()->AdviseDelayedSplitRequested();

        // We end the process there ... split will occur eventually at the parent's discretion
        return;
        }


    // The split process is organised differently if there are sub-levels non-split under the parent node
    if (m_pSubNodeNoSplit != NULL)
        {
        if (IsParentOfARealUnsplitNode())
            {
            // In this case we must regather all the points back to current level then split...
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pCurrentNode = m_pSubNodeNoSplit;
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
            }
        m_pSubNodeNoSplit = NULL;
        m_nodeHeader.m_IsLeaf = true;
        }

    // The node must be a leaf
    HPRECONDITION(IsLeaf());

    // The position of this is very important ... the delayed split must be desactivated the woonest possible.
    m_DelayedSplitRequested = false;

   /* double Width = ExtentOp<EXTENT>::GetWidth(GetNodeExtent());
    double Height = ExtentOp<EXTENT>::GetHeight(GetNodeExtent());
    double Thickness = ExtentOp<EXTENT>::GetThickness(GetNodeExtent());*/

    // Make sure that the nodes can be split. Sometimes, the width or height is very close to zero when max and min are subtracted as a result
    // of the limit of the double of representing 15 digits maximum. If this representation limit is attained
    // then we will not split and mark the node as "unspliteable"
    // We only check the size of one extent
    if ((HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
        PointOp<POINT>::GetX(splitPosition),
        HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent))) ||
        (HNumeric<double>::EQUAL(PointOp<POINT>::GetY(splitPosition),
        ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
        HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent))) ||
       ( m_nodeHeader.m_numberOfSubNodesOnSplit == 8 && (HNumeric<double>::EQUAL(PointOp<POINT>::GetZ(splitPosition),
        ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent),
        HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent)))))
        {
        // Values would be virtually equal ... we will not split
        HDEBUGCODE(m_unspliteable = true;)
            return;
        }

    if (m_nodeHeader.m_numberOfSubNodesOnSplit == 4)
        {

        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        }
    else
        {
        HPRECONDITION(ExtentOp<EXTENT>::GetThickness(GetNodeExtent()) > 0.0);

        if (HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            HNumeric<double>::EPSILON_MULTIPLICATOR() * ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent)))
            {
            // Values would be virtually equal ... we will not split
            HDEBUGCODE(m_unspliteable = true;)
                return;
            }


        m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition)));

        m_apSubNodes[4] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[5] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[6] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            PointOp<POINT>::GetX(splitPosition),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

        m_apSubNodes[7] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
            ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetZ(splitPosition),
            ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
            PointOp<POINT>::GetY(splitPosition),
            ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));
        }
    // Indicate node is not a leaf anymore
    m_nodeHeader.m_IsLeaf = false;
    m_nodeHeader.m_IsBranched = true;

    SetupNeighborNodesAfterSplit();

    if (this->size() > 0)
        {
        PropagateDataDownImmediately(false);
        }


    SetDirty(true);

    if (propagateSplit)
        {
        if (GetParentNode() != NULL)
            {
            size_t targetLevel = max(GetLevel() + 1, deepestLevelNumber);
            if (targetLevel > GetLevel() + 1)
                {
                // In this case sub-nodes must be pushed down
                for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                    {
                    if (!m_isGenerating)
                        {
                        if (IsBalanced())  m_apSubNodes[indexNodes]->PushNodeDown(targetLevel);
                        else m_apSubNodes[indexNodes]->PushNodeDownVirtual(targetLevel);
                        }
                    }
                }
            GetParentNode()->PropagateSplitNode(this, targetLevel);
            }
        }


    HINVARIANTS;

    }

    //=======================================================================================
    // @bsimethod                                                  Elenie.Godzaridis 07/15
    //=======================================================================================
    template<class POINT, class EXTENT>
    void SMPointIndexNode<POINT, EXTENT>::ReSplitNode(POINT splitPosition, size_t newNumberOfChildNodesOnSplit)
        {
        SMLeafPointIndexQuery<POINT, EXTENT> * query = new SMLeafPointIndexQuery<POINT, EXTENT>(m_nodeHeader.m_nodeExtent);
        vector<QueriedNode> meshNodes;
        Query(query, meshNodes);
        vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(m_nodeHeader.m_numberOfSubNodesOnSplit);
        for (size_t i = 0; i < m_nodeHeader.m_numberOfSubNodesOnSplit; i++)subNodes[i] = m_apSubNodes[i];
        m_apSubNodes.resize(newNumberOfChildNodesOnSplit);
        m_nodeHeader.m_apSubNodeID.resize(newNumberOfChildNodesOnSplit);
        for (size_t i = 0; i < newNumberOfChildNodesOnSplit; i++)m_nodeHeader.m_apSubNodeID[i] = HPMBlockID();
        m_nodeHeader.m_numberOfSubNodesOnSplit = newNumberOfChildNodesOnSplit;
        if (newNumberOfChildNodesOnSplit == 8)
            {
            m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetX(splitPosition),
                ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetZ(splitPosition)));

            m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetZ(splitPosition)));

            m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetX(splitPosition),
                PointOp<POINT>::GetY(splitPosition),
                PointOp<POINT>::GetZ(splitPosition)));

            m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
                ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetY(splitPosition),
                PointOp<POINT>::GetZ(splitPosition)));

            m_apSubNodes[4] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetY(splitPosition),
                PointOp<POINT>::GetZ(splitPosition),
                PointOp<POINT>::GetX(splitPosition),
                ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

            m_apSubNodes[5] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
                PointOp<POINT>::GetY(splitPosition),
                PointOp<POINT>::GetZ(splitPosition),
                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

            m_apSubNodes[6] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetZ(splitPosition),
                PointOp<POINT>::GetX(splitPosition),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

            m_apSubNodes[7] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
                ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetZ(splitPosition),
                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));
            m_apSubNodes[0]->SetDirty(true);
            m_apSubNodes[1]->SetDirty(true);
            m_apSubNodes[2]->SetDirty(true);
            m_apSubNodes[3]->SetDirty(true);
            m_apSubNodes[4]->SetDirty(true);
            m_apSubNodes[5]->SetDirty(true);
            m_apSubNodes[6]->SetDirty(true);
            m_apSubNodes[7]->SetDirty(true);
            }
        else if (newNumberOfChildNodesOnSplit == 4)
            {
            m_apSubNodes[0] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetX(splitPosition),
                ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

            m_apSubNodes[1] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

            m_apSubNodes[2] = this->CloneChild(ExtentOp<EXTENT>::Create(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetX(splitPosition),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));

            m_apSubNodes[3] = this->CloneChild(ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(splitPosition),
                ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                PointOp<POINT>::GetY(splitPosition),
                ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent)));
            m_apSubNodes[0]->SetDirty(true);
            m_apSubNodes[1]->SetDirty(true);
            m_apSubNodes[2]->SetDirty(true);
            m_apSubNodes[3]->SetDirty(true);
            }
        SetupNeighborNodesAfterSplit();
        for (auto& node : meshNodes)
            {
            if (node.m_indexNode->size() == 0) continue;
            size_t lastPtIndex = 0;
            while (lastPtIndex < node.m_indexNode->size())
                {
                for (size_t indexNode = 0; indexNode < newNumberOfChildNodesOnSplit; indexNode++)
                    {
                    lastPtIndex = m_apSubNodes[indexNode]->AddArrayConditional(&(*node.m_indexNode)[0], lastPtIndex, node.m_indexNode->size(), true, m_nodeHeader.m_arePoints3d);
                    }
                }
            }
        for (size_t i = 0; i < subNodes.size(); i++)subNodes[i]->Destroy();
        }

    //=====================================================================================
    // @bsimethod                                                   Mathieu.St-Pierre 08/14
    //=======================================================================================
    template< class POINT, class EXTENT>
    void SMPointIndexNode<POINT, EXTENT>::SetupNeighborNodesAfterPushDown()
        {
        assert(m_pSubNodeNoSplit != 0);

        //Neighbor node 0
        vector<size_t> neighborSubNodeIndexes;

        neighborSubNodeIndexes.push_back(3);
        neighborSubNodeIndexes.push_back(7);

        SetNeighborRelationAfterPushDown(0, neighborSubNodeIndexes, 7);

        //Neighbor node 1
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(2);
        neighborSubNodeIndexes.push_back(3);
        neighborSubNodeIndexes.push_back(6);
        neighborSubNodeIndexes.push_back(7);

        SetNeighborRelationAfterPushDown(1, neighborSubNodeIndexes, 6);

        //Neighbor node 2
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(2);
        neighborSubNodeIndexes.push_back(6);

        SetNeighborRelationAfterPushDown(2, neighborSubNodeIndexes, 5);

        //Neighbor node 3
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(1);
        neighborSubNodeIndexes.push_back(3);
        neighborSubNodeIndexes.push_back(5);
        neighborSubNodeIndexes.push_back(7);

        SetNeighborRelationAfterPushDown(3, neighborSubNodeIndexes, 4);

        //Neighbor node 4
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(0);
        neighborSubNodeIndexes.push_back(2);
        neighborSubNodeIndexes.push_back(4);
        neighborSubNodeIndexes.push_back(6);

        SetNeighborRelationAfterPushDown(4, neighborSubNodeIndexes, 3);

        //Neighbor node 5
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(1);
        neighborSubNodeIndexes.push_back(5);

        SetNeighborRelationAfterPushDown(5, neighborSubNodeIndexes, 2);

        //Neighbor node 6
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(0);
        neighborSubNodeIndexes.push_back(1);
        neighborSubNodeIndexes.push_back(4);
        neighborSubNodeIndexes.push_back(5);

        SetNeighborRelationAfterPushDown(6, neighborSubNodeIndexes, 1);

        //Neighbor node 7
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(0);
        neighborSubNodeIndexes.push_back(4);

        SetNeighborRelationAfterPushDown(7, neighborSubNodeIndexes, 0);

        //Neighbor node 8
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(7);

        SetNeighborRelationAfterPushDown(8, neighborSubNodeIndexes, 25);

        //Neighbor node 9
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(6);
        neighborSubNodeIndexes.push_back(7);

        SetNeighborRelationAfterPushDown(9, neighborSubNodeIndexes, 24);

        //Neighbor node 10
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(6);

        SetNeighborRelationAfterPushDown(10, neighborSubNodeIndexes, 23);

        //Neighbor node 11
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(5);
        neighborSubNodeIndexes.push_back(7);

        SetNeighborRelationAfterPushDown(11, neighborSubNodeIndexes, 22);

        //Neighbor node 12
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(4);
        neighborSubNodeIndexes.push_back(5);
        neighborSubNodeIndexes.push_back(6);
        neighborSubNodeIndexes.push_back(7);

        SetNeighborRelationAfterPushDown(12, neighborSubNodeIndexes, 21);

        //Neighbor node 13
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(4);
        neighborSubNodeIndexes.push_back(6);

        SetNeighborRelationAfterPushDown(13, neighborSubNodeIndexes, 20);


        //Neighbor node 14
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(5);

        SetNeighborRelationAfterPushDown(14, neighborSubNodeIndexes, 19);


        //Neighbor node 15
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(4);
        neighborSubNodeIndexes.push_back(5);

        SetNeighborRelationAfterPushDown(15, neighborSubNodeIndexes, 18);


        //Neighbor node 16
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(4);

        SetNeighborRelationAfterPushDown(16, neighborSubNodeIndexes, 17);


        //Neighbor node 17
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(3);

        SetNeighborRelationAfterPushDown(17, neighborSubNodeIndexes, 16);


        //Neighbor node 18
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(2);
        neighborSubNodeIndexes.push_back(3);

        SetNeighborRelationAfterPushDown(18, neighborSubNodeIndexes, 15);


        //Neighbor node 19
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(2);

        SetNeighborRelationAfterPushDown(19, neighborSubNodeIndexes, 14);


        //Neighbor node 20
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(1);
        neighborSubNodeIndexes.push_back(3);

        SetNeighborRelationAfterPushDown(20, neighborSubNodeIndexes, 13);


        //Neighbor node 21
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(0);
        neighborSubNodeIndexes.push_back(1);
        neighborSubNodeIndexes.push_back(2);
        neighborSubNodeIndexes.push_back(3);

        SetNeighborRelationAfterPushDown(21, neighborSubNodeIndexes, 12);


        //Neighbor node 22
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(0);
        neighborSubNodeIndexes.push_back(2);

        SetNeighborRelationAfterPushDown(22, neighborSubNodeIndexes, 11);


        //Neighbor node 23
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(1);

        SetNeighborRelationAfterPushDown(23, neighborSubNodeIndexes, 10);


        //Neighbor node 24
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(0);
        neighborSubNodeIndexes.push_back(1);

        SetNeighborRelationAfterPushDown(24, neighborSubNodeIndexes, 9);


        //Neighbor node 25
        neighborSubNodeIndexes.clear();

        neighborSubNodeIndexes.push_back(0);

        SetNeighborRelationAfterPushDown(25, neighborSubNodeIndexes, 8);

        if (s_inEditing && IsParentOfARealUnsplitNode())
            {
            m_pSubNodeNoSplit->InvalidateStitching();
            }

        ValidateNeighborsOfChildren();
        /*
        for (size_t nodeInd = 0; nodeInd < m_apNeighborNodes[0].size(); nodeInd++)
        {
        if (m_apNeighborNodes[0][nodeInd]->m_apSubNodes[3] != 0)
        {
        assert(m_apNeighborNodes[0][nodeInd]->m_apSubNodes[3].m_nodeHeader.m_level == m_pSubNodeNoSplit->m_nodeHeader.m_level);
        m_pSubNodeNoSplit->m_apNeighborNodes[0].push_back(m_apNeighborNodes[0][nodeInd]->m_apSubNodes[3]);

        assert(m_apNeighborNodes[0][nodeInd]->m_apSubNodes[3]->m_apNeighborNodes[7].size() == 0);
        m_apNeighborNodes[0][nodeInd]->m_apSubNodes[3]->m_apNeighborNodes[7].push_back(m_pSubNodeNoSplit);
        }
        else
        if (m_apNeighborNodes[0][nodeInd]->m_apSubNodes[7] != 0)
        {
        assert(m_apNeighborNodes[0][nodeInd]->m_apSubNodes[7].m_nodeHeader.m_level == m_pSubNodeNoSplit->m_nodeHeader.m_level);
        m_pSubNodeNoSplit->m_apNeighborNodes[0].push_back(m_apNeighborNodes[0][nodeInd]->m_apSubNodes[7]);

        assert(m_apNeighborNodes[0][nodeInd]->m_apSubNodes[7]->m_apNeighborNodes[7].size() == 0);
        m_apNeighborNodes[0][nodeInd]->m_apSubNodes[7]->m_apNeighborNodes[7].push_back(m_pSubNodeNoSplit);
        }
        }
        */
        }

        //=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================

/**----------------------------------------------------------------------------
     Add neighbor nodes of newly creating sub-nodes after a split.
    -----------------------------------------------------------------------------*/

template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SetupNeighborNodesAfterSplit()
    {

/* Neighbor nodes are ordered as follow :  

Middle layer                Bottom layer                Top layer
---------------------       ---------------------       ---------------------
|   0  |   1  |  2  |       |  8   |  9   |  10 |       |  17  |  18  |  19 |
|      |      |     |       |      |      |     |       |      |      |     |
---------------------       ---------------------       ---------------------
---------------------       ---------------------       ---------------------
|   3  | this |  4  |       |  11  |  12  |  13 |       |  20  |  21  |  22 |
|      | node |     |       |      |      |     |       |      |      |     |
---------------------       ---------------------       ---------------------
---------------------       ---------------------       ---------------------
|   5  |   6  |  7  |       |  14  |  15  |  16 |       |  23  |  24  |  25 |
|      |      |     |       |      |      |     |       |      |      |     |
---------------------       ---------------------       ---------------------

*/
    //Assigned neighbor nodes for sub-nodes
    m_apSubNodes[0]->m_apNeighborNodes[4].push_back(m_apSubNodes[1]);    
    m_apSubNodes[0]->m_apNeighborNodes[6].push_back(m_apSubNodes[2]);    
    m_apSubNodes[0]->m_apNeighborNodes[7].push_back(m_apSubNodes[3]);
    if (m_apSubNodes.size() == 8)
        {
    m_apSubNodes[0]->m_apNeighborNodes[21].push_back(m_apSubNodes[4]);
        m_apSubNodes[0]->m_apNeighborNodes[22].push_back(m_apSubNodes[5]);
        m_apSubNodes[0]->m_apNeighborNodes[24].push_back(m_apSubNodes[6]);
        m_apSubNodes[0]->m_apNeighborNodes[25].push_back(m_apSubNodes[7]);
        }
      

    SetNeighborRelationAfterSplit(0, 0);
    SetNeighborRelationAfterSplit(0, 1);
    SetNeighborRelationAfterSplit(0, 3);
    if (m_apSubNodes.size() == 8)
        {
        SetNeighborRelationAfterSplit(0, 8);
        SetNeighborRelationAfterSplit(0, 9);
        SetNeighborRelationAfterSplit(0, 11);
        SetNeighborRelationAfterSplit(0, 12);
        }


#ifndef NDEBUG
    //ValidateNeighborsAfterSplit(m_apSubNodes[0]);
#endif



    m_apSubNodes[1]->m_apNeighborNodes[3].push_back(m_apSubNodes[0]);
    m_apSubNodes[1]->m_apNeighborNodes[5].push_back(m_apSubNodes[2]);
    m_apSubNodes[1]->m_apNeighborNodes[6].push_back(m_apSubNodes[3]);
    if (m_apSubNodes.size() == 8)
        {
        m_apSubNodes[1]->m_apNeighborNodes[20].push_back(m_apSubNodes[4]);
        m_apSubNodes[1]->m_apNeighborNodes[21].push_back(m_apSubNodes[5]);
        m_apSubNodes[1]->m_apNeighborNodes[23].push_back(m_apSubNodes[6]);
        m_apSubNodes[1]->m_apNeighborNodes[24].push_back(m_apSubNodes[7]);
        }

    SetNeighborRelationAfterSplit(1, 1);
    SetNeighborRelationAfterSplit(1, 2);
    SetNeighborRelationAfterSplit(1, 4);

    SetNeighborRelationAfterSplit(1, 9);
    SetNeighborRelationAfterSplit(1, 10);
    SetNeighborRelationAfterSplit(1, 12);
    SetNeighborRelationAfterSplit(1, 13);
    
#ifndef NDEBUG
    //ValidateNeighborsAfterSplit(m_apSubNodes[1]);
#endif

    m_apSubNodes[2]->m_apNeighborNodes[1].push_back(m_apSubNodes[0]);
    m_apSubNodes[2]->m_apNeighborNodes[2].push_back(m_apSubNodes[1]);
    m_apSubNodes[2]->m_apNeighborNodes[4].push_back(m_apSubNodes[3]);
    if (m_apSubNodes.size() == 8)
        {
        m_apSubNodes[2]->m_apNeighborNodes[18].push_back(m_apSubNodes[4]);
        m_apSubNodes[2]->m_apNeighborNodes[19].push_back(m_apSubNodes[5]);
        m_apSubNodes[2]->m_apNeighborNodes[21].push_back(m_apSubNodes[6]);
        m_apSubNodes[2]->m_apNeighborNodes[22].push_back(m_apSubNodes[7]);
        }

    SetNeighborRelationAfterSplit(2, 3);
    SetNeighborRelationAfterSplit(2, 5);
    SetNeighborRelationAfterSplit(2, 6);

    SetNeighborRelationAfterSplit(2, 11);
    SetNeighborRelationAfterSplit(2, 12);
    SetNeighborRelationAfterSplit(2, 14);
    SetNeighborRelationAfterSplit(2, 15);
    

#ifndef NDEBUG
    //ValidateNeighborsAfterSplit(m_apSubNodes[2]);
#endif

    m_apSubNodes[3]->m_apNeighborNodes[0].push_back(m_apSubNodes[0]);
    m_apSubNodes[3]->m_apNeighborNodes[1].push_back(m_apSubNodes[1]);
    m_apSubNodes[3]->m_apNeighborNodes[3].push_back(m_apSubNodes[2]);
    if (m_apSubNodes.size() == 8)
        {
        m_apSubNodes[3]->m_apNeighborNodes[17].push_back(m_apSubNodes[4]);
        m_apSubNodes[3]->m_apNeighborNodes[18].push_back(m_apSubNodes[5]);
        m_apSubNodes[3]->m_apNeighborNodes[20].push_back(m_apSubNodes[6]);
        m_apSubNodes[3]->m_apNeighborNodes[21].push_back(m_apSubNodes[7]);
        }

    SetNeighborRelationAfterSplit(3, 4);
    SetNeighborRelationAfterSplit(3, 6);
    SetNeighborRelationAfterSplit(3, 7);

    SetNeighborRelationAfterSplit(3, 12);
    SetNeighborRelationAfterSplit(3, 13);
    SetNeighborRelationAfterSplit(3, 15);
    SetNeighborRelationAfterSplit(3, 16);

#ifndef NDEBUG
    //ValidateNeighborsAfterSplit(m_apSubNodes[3]);
#endif
        
    if (m_apSubNodes.size() == 8)
        {
        m_apSubNodes[4]->m_apNeighborNodes[12].push_back(m_apSubNodes[0]);
        m_apSubNodes[4]->m_apNeighborNodes[13].push_back(m_apSubNodes[1]);
        m_apSubNodes[4]->m_apNeighborNodes[15].push_back(m_apSubNodes[2]);
        m_apSubNodes[4]->m_apNeighborNodes[16].push_back(m_apSubNodes[3]);
        m_apSubNodes[4]->m_apNeighborNodes[4].push_back(m_apSubNodes[5]);
        m_apSubNodes[4]->m_apNeighborNodes[6].push_back(m_apSubNodes[6]);
        m_apSubNodes[4]->m_apNeighborNodes[7].push_back(m_apSubNodes[7]);


        SetNeighborRelationAfterSplit(4, 0);
        SetNeighborRelationAfterSplit(4, 1);
        SetNeighborRelationAfterSplit(4, 3);

        SetNeighborRelationAfterSplit(4, 17);
        SetNeighborRelationAfterSplit(4, 18);
        SetNeighborRelationAfterSplit(4, 20);
        SetNeighborRelationAfterSplit(4, 21);


#ifndef NDEBUG
        //ValidateNeighborsAfterSplit(m_apSubNodes[4]);
#endif

        m_apSubNodes[5]->m_apNeighborNodes[11].push_back(m_apSubNodes[0]);
        m_apSubNodes[5]->m_apNeighborNodes[12].push_back(m_apSubNodes[1]);
        m_apSubNodes[5]->m_apNeighborNodes[14].push_back(m_apSubNodes[2]);
        m_apSubNodes[5]->m_apNeighborNodes[15].push_back(m_apSubNodes[3]);
        m_apSubNodes[5]->m_apNeighborNodes[3].push_back(m_apSubNodes[4]);
        m_apSubNodes[5]->m_apNeighborNodes[5].push_back(m_apSubNodes[6]);
        m_apSubNodes[5]->m_apNeighborNodes[6].push_back(m_apSubNodes[7]);

        SetNeighborRelationAfterSplit(5, 1);
        SetNeighborRelationAfterSplit(5, 2);
        SetNeighborRelationAfterSplit(5, 4);

        SetNeighborRelationAfterSplit(5, 18);
        SetNeighborRelationAfterSplit(5, 19);
        SetNeighborRelationAfterSplit(5, 21);
        SetNeighborRelationAfterSplit(5, 22);

#ifndef NDEBUG
        //ValidateNeighborsAfterSplit(m_apSubNodes[5]);
#endif

        m_apSubNodes[6]->m_apNeighborNodes[9].push_back(m_apSubNodes[0]);
        m_apSubNodes[6]->m_apNeighborNodes[10].push_back(m_apSubNodes[1]);
        m_apSubNodes[6]->m_apNeighborNodes[12].push_back(m_apSubNodes[2]);
        m_apSubNodes[6]->m_apNeighborNodes[13].push_back(m_apSubNodes[3]);
        m_apSubNodes[6]->m_apNeighborNodes[1].push_back(m_apSubNodes[4]);
        m_apSubNodes[6]->m_apNeighborNodes[2].push_back(m_apSubNodes[5]);
        m_apSubNodes[6]->m_apNeighborNodes[4].push_back(m_apSubNodes[7]);

        SetNeighborRelationAfterSplit(6, 3);
        SetNeighborRelationAfterSplit(6, 5);
        SetNeighborRelationAfterSplit(6, 6);


        SetNeighborRelationAfterSplit(6, 20);
        SetNeighborRelationAfterSplit(6, 21);
        SetNeighborRelationAfterSplit(6, 23);
        SetNeighborRelationAfterSplit(6, 24);


#ifndef NDEBUG
        //ValidateNeighborsAfterSplit(m_apSubNodes[6]);
#endif

        m_apSubNodes[7]->m_apNeighborNodes[8].push_back(m_apSubNodes[0]);
        m_apSubNodes[7]->m_apNeighborNodes[9].push_back(m_apSubNodes[1]);
        m_apSubNodes[7]->m_apNeighborNodes[11].push_back(m_apSubNodes[2]);
        m_apSubNodes[7]->m_apNeighborNodes[12].push_back(m_apSubNodes[3]);
        m_apSubNodes[7]->m_apNeighborNodes[0].push_back(m_apSubNodes[4]);
        m_apSubNodes[7]->m_apNeighborNodes[1].push_back(m_apSubNodes[5]);
        m_apSubNodes[7]->m_apNeighborNodes[3].push_back(m_apSubNodes[6]);


        SetNeighborRelationAfterSplit(7, 4);
        SetNeighborRelationAfterSplit(7, 6);
        SetNeighborRelationAfterSplit(7, 7);

        SetNeighborRelationAfterSplit(7, 21);
        SetNeighborRelationAfterSplit(7, 22);
        SetNeighborRelationAfterSplit(7, 24);
        SetNeighborRelationAfterSplit(7, 25);
        }

#ifndef NDEBUG
    //ValidateNeighborsAfterSplit(m_apSubNodes[7]);
#endif

#ifndef NDEBUG
    for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit() && m_apSubNodes.size() == 8; indexNodes++)
        {
        //ValidateNeighborsAfterSplit(m_apSubNodes[indexNodes]);                
        }     
#endif

    ValidateNeighborsOfChildren();

    if (s_inEditing)
        {
        for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
            {
            m_apSubNodes[indexNodes]->InvalidateStitching();
            }     
        }

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::PushNodeDown(size_t targetLevel)
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
            assert(!m_pSubNodeNoSplit->IsVirtualNode());
            m_pSubNodeNoSplit->PushNodeDown(targetLevel);
            }
        else
            {
            for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                {
                m_apSubNodes[indexNodes]->PushNodeDown(targetLevel);
                }
            }
        }
    else
        {
        // The node is a leaf and its level is insufficiant
        m_pSubNodeNoSplit = this->CloneUnsplitChild(EXTENT(m_nodeHeader.m_nodeExtent));

        // We copy the whole content to this sub-node
        m_pSubNodeNoSplit->reserve(this->size());
        m_pSubNodeNoSplit->m_nodeHeader.m_arePoints3d = m_nodeHeader.m_arePoints3d;
        if (!m_pSubNodeNoSplit->m_nodeHeader.m_arePoints3d) m_pSubNodeNoSplit->SetNumberOfSubNodesOnSplit(4);
        else m_pSubNodeNoSplit->SetNumberOfSubNodesOnSplit(8);
        OnPushNodeDown(); //we push the feature definitions first so that they can take care of their own point data

#if (0)
        for (size_t indexSpatial = 0; indexSpatial < this->size(); indexSpatial++)
            {
            m_pSubNodeNoSplit->push_back(this->operator[](indexSpatial));
            }
#else
        CopyVector<CONTAINER>::Copy(m_pSubNodeNoSplit, this);
#endif

        m_pSubNodeNoSplit->m_nodeHeader.m_contentExtent = m_nodeHeader.m_contentExtent;
        m_pSubNodeNoSplit->m_nodeHeader.m_contentExtentDefined = m_nodeHeader.m_contentExtentDefined;
        m_pSubNodeNoSplit->m_nodeHeader.m_totalCount = m_pSubNodeNoSplit->size();
        SetupNeighborNodesAfterPushDown();

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
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::PushNodeDownVirtual(size_t targetLevel)
    {
    this->ValidateInvariantsSoft();

    if (!IsLoaded())
        Load();
    if (m_nodeHeader.m_level >= targetLevel)
        return;

    // Make sure conditions are acceptable
    if (!m_nodeHeader.m_IsLeaf)
        {
        // The node is not a leaf ... we ask its descendents to push themselves
        if (m_pSubNodeNoSplit != NULL)
            {
            assert(m_pSubNodeNoSplit->IsVirtualNode()); //unsplit nodes in a tree can either be all virtual or all real (because they are two different balancing strategies to achieve similar results).
            m_pSubNodeNoSplit->PushNodeDownVirtual(targetLevel);
            }
        else
            {
            for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                {
                m_apSubNodes[indexNodes]->PushNodeDownVirtual(targetLevel);
                }
            }
        }
    else
        {
        // The node is a leaf and its level is insufficiant
        m_pSubNodeNoSplit = this->CloneUnsplitChildVirtual();

      
        SetupNeighborNodesAfterPushDown();

        m_nodeHeader.m_IsLeaf = false;

        // Check if the new node is deep enough
        if (m_pSubNodeNoSplit->GetLevel() < targetLevel)
            m_pSubNodeNoSplit->PushNodeDownVirtual(targetLevel);
        }


    SetDirty(true);

    this->ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndexNode<POINT, EXTENT>::GetReciprocalNeighborPos(size_t neighborPos)
    {
    switch (neighborPos)
        {
        case 0: return 7;
        case 1: return 6;
        case 2: return 5;
        case 3: return 4;
        case 4: return 3;
        case 5: return 2;
        case 6: return 1;
        case 7: return 0;
        case 8: return  25;
        case 9: return  24;
        case 10: return 23;
        case 11: return 22;
        case 12: return 21;
        case 13: return 20;
        case 14: return 19;
        case 15: return 18;
        case 16: return 17;
        case 17: return 16;
        case 18: return 15;
        case 19: return 14;
        case 20: return 13;
        case 21: return 12;
        case 22: return 11;
        case 23: return 10;
        case 24: return 9;
        case 25: return 8;
        default:
            assert(!"Invalid neighbor position");
            return 26;
        }
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::Balance(size_t depth)
    {
    this->ValidateInvariantsSoft();

    if (!IsLoaded())
        Load();
    m_isGenerating = false;
    //if (!m_nodeHeader.m_balanced)
        {
       // m_nodeHeader.m_balanced = true;

        if (m_nodeHeader.m_IsLeaf)
            {
            if (m_nodeHeader.m_level < depth)
                { 
                    if (m_nodeHeader.m_balanced) PushNodeDown(depth);
                    else PushNodeDownVirtual(depth);
                    }

            if (!m_nodeHeader.m_contentExtentDefined && m_nodeHeader.m_totalCount > 0)
                {
                assert(s_inEditing == true);

                for (size_t pointInd = 0; pointInd < size(); pointInd++)
                    {
                    if (!m_nodeHeader.m_contentExtentDefined)
                        {
                        m_nodeHeader.m_contentExtent = SpatialOp<POINT, POINT, EXTENT>::GetExtent(this->operator[](pointInd));
                        m_nodeHeader.m_contentExtentDefined = true;
                        }
                    else
                        {
                        m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(this->operator[](pointInd)));
                        }
                    }
                }
            }
        else
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                m_pSubNodeNoSplit->Balance(depth);

                if (!m_nodeHeader.m_contentExtentDefined && m_nodeHeader.m_totalCount > 0)
                    {
                    assert(s_inEditing == true);
                    assert(m_pSubNodeNoSplit->m_nodeHeader.m_contentExtentDefined == true);
                    m_nodeHeader.m_contentExtent = m_pSubNodeNoSplit->m_nodeHeader.m_contentExtent;
                    m_nodeHeader.m_contentExtentDefined = true;
                    }
                }
            else
                {
                for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    m_apSubNodes[indexNode]->Balance(depth);
                    }

                if (!m_nodeHeader.m_contentExtentDefined && m_nodeHeader.m_totalCount > 0)
                    {
                    assert(s_inEditing);

                    for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
                        if (m_apSubNodes[indexNode]->m_nodeHeader.m_contentExtentDefined)
                            {
                            if (!m_nodeHeader.m_contentExtentDefined)
                                {
                                m_nodeHeader.m_contentExtent = m_apSubNodes[indexNode]->m_nodeHeader.m_contentExtent;
                                m_nodeHeader.m_contentExtentDefined = true;
                                }
                            else
                                {
                                m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, m_apSubNodes[indexNode]->m_nodeHeader.m_contentExtent);
                                }
                            }
                        }
                    }
                }
            }
        }
   // else
        if (s_inEditing)
            {
            assert(m_nodeHeader.m_contentExtentDefined);
            ReBalance(depth);
            }

    HINVARIANTS;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::PropagateSplitNode(HFCPtr<SMPointIndexNode<POINT, EXTENT>> initiator,
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
            if (!m_isGenerating)
                {
                if (m_nodeHeader.m_balanced) PushNodeDown(quadTreeDepth);
                else PushNodeDownVirtual(quadTreeDepth);
                }
            }
        // Not abolutely sure about this condition due to the various possible propagations that can occur at the same time
        // HASSERT (GetLevel() == quadTreeDepth);
        }


    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            if (m_pSubNodeNoSplit != initiator)
                m_pSubNodeNoSplit->PropagateSplitNode(this, quadTreeDepth);
            }
        else
            {
            for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                {
                if (m_apSubNodes[indexNodes] != initiator)
                    m_apSubNodes[indexNodes]->PropagateSplitNode(this, quadTreeDepth);
                }
            }
        }

    // Propagate to parent
    if (GetParentNode() != NULL)
        {
        if (GetParentNode() != initiator)
            GetParentNode()->PropagateSplitNode(this, quadTreeDepth);
        }


    HINVARIANTS;

    }



/**----------------------------------------------------------------------------
PRIVATE METHOD
This method move this node extent all its sub nodes' extent down for ensuring
2.5D nodes have no bottom neighbor.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::MoveDownNodes(double moveDownDistance)
    {
    assert(m_pSubNodeNoSplit == 0);

    ExtentOp<EXTENT>::SetZMin(m_nodeHeader.m_nodeExtent, ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) - moveDownDistance);
    ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent) - moveDownDistance);

    for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
        {
        if (m_apSubNodes[indexNode] != 0)
            m_apSubNodes[indexNode]->MoveDownNodes(moveDownDistance);
        }
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT> void  SMPointIndexNode<POINT, EXTENT>::SetBalanced(bool balanced)
    {
    if (!IsLoaded())
        Load();
    m_nodeHeader.m_balanced = balanced;
    if (!IsLeaf())
        {
        if (static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit) != NULL)
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->SetBalanced(balanced);
        else
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                if (m_apSubNodes[indexNode] != NULL)
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->SetBalanced(balanced);
                }
            }
        }

    for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        if (m_apSubNodes[indexNode] != NULL)
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->SetBalanced(balanced);
        }
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> double* SMPointIndexNode<POINT, EXTENT>::GetViewDependentMetrics()
    {
    if (!IsLoaded())
        Load();

    return m_nodeHeader.m_ViewDependentMetrics;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::ChangeStore(HFCPtr<SMPointTileStore<POINT, EXTENT> > newStore)
    {
    HINVARIANTS;

    // Call node
    if (!IsLeaf())
        {
        if (static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit) != NULL)
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->ChangeStore(newStore);
        else
            {
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                if (m_apSubNodes[indexNode] != NULL)
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->ChangeStore(newStore);
                }
            }
        }

    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        if (m_apSubNodes[indexNode] != NULL)
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->ChangeStore(newStore);
        }

    // Call ancester
    HPMStoredPooledVector::ChangeStore (newStore);

    SetDirty(true);
    Discard();

    HINVARIANTS;

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Store() const
    {
    HINVARIANTS;

    if (!IsLoaded())
        return true;

    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Store(); 	

            HASSERT (m_nodeHeader.m_SubNodeNoSplitID.IsValid());
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                if (m_apSubNodes[indexNode] != NULL)
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Store();
                HASSERT(m_apSubNodes[indexNode]== NULL || m_apSubNodes[indexNode]->IsVirtualNode() || m_nodeHeader.m_apSubNodeID[indexNode].IsValid());
                }
            }
        }


    if (!Discarded())
        {
        return Discard();
        }
    else
        {
        if (IsDirty())
            {
            Inflate();
            Discard();
            }
        }

    HINVARIANTS;


    return true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::SetDirty (bool dirty) const // Intentionaly const ... only mutable field changed
    {
    // In theory the two next variable invalidations are not required as
    // but just in case the HGFIndexNode
    m_NbObjects = -1;
    HPMCountLimitedPoolItem<POINT>::SetDirty (dirty);
    }

#ifndef NDEBUG
//=====================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::ValidateNeighbors()
    {
    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != 0)
            {
            m_pSubNodeNoSplit->ValidateNeighbors();
            }
        else
            {
            for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                {
                m_apSubNodes[indexNodes]->ValidateNeighbors();
                }
            }
        }

   // ValidateNeighborsAfterPushDown(this);
    }

//=====================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
//NEEDS_WORK_SM : Replace pi_node by this
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::ValidateDualNeighborship(HFCPtr<SMPointIndexNode<POINT, EXTENT>> pi_node)
    {

    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {
        for (size_t neighborInd = 0; neighborInd < pi_node->m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
            {
            size_t otherNeighborPosInd = 0;

            for (; otherNeighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; otherNeighborPosInd++)
                {
                size_t otherNeighborInd = 0;

                for (; otherNeighborInd < pi_node->m_apNeighborNodes[neighborPosInd][neighborInd]->m_apNeighborNodes[otherNeighborPosInd].size(); otherNeighborInd++)
                    {
                    if (pi_node->m_apNeighborNodes[neighborPosInd][neighborInd]->m_apNeighborNodes[otherNeighborPosInd][otherNeighborInd].GetPtr() == pi_node.GetPtr())
                        {
                        break;
                        }
                    }

                if (otherNeighborInd < pi_node->m_apNeighborNodes[neighborPosInd][neighborInd]->m_apNeighborNodes[otherNeighborPosInd].size())
                    break;
                }

            assert(otherNeighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY);
            }
        }
    }


#ifndef NEIGHBOR_VALIDATION_PRECISION
#define NEIGHBOR_VALIDATION_PRECISION 0.0000000001
#endif

template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::ValidateNeighborsAfterSplit(HFCPtr<SMPointIndexNode<POINT, EXTENT>> pi_node)
    {
    double xMin = ExtentOp<EXTENT>::GetXMin(pi_node->m_nodeHeader.m_nodeExtent);
    double xMax = ExtentOp<EXTENT>::GetXMax(pi_node->m_nodeHeader.m_nodeExtent);
    double yMin = ExtentOp<EXTENT>::GetYMin(pi_node->m_nodeHeader.m_nodeExtent);
    double yMax = ExtentOp<EXTENT>::GetYMax(pi_node->m_nodeHeader.m_nodeExtent);
    double zMin = ExtentOp<EXTENT>::GetZMin(pi_node->m_nodeHeader.m_nodeExtent);
    double zMax = ExtentOp<EXTENT>::GetZMax(pi_node->m_nodeHeader.m_nodeExtent);



    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[0].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[1].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[2].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[3].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[4].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[5].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[6].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[7].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[8].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[8][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[8][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[8][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[9].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[10].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[10][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[10][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[10][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[11].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[12].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[13].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[14].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[14][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[14][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[14][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[15].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[16].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[16][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[16][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[16][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }
    //----------------------
    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[17].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[17][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[17][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[17][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[18].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[19].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[19][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[19][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[19][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[20].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[21].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[22].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[23].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[23][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[23][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[23][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[24].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[25].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[25][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[25][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[25][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    ValidateDualNeighborship(pi_node);
    }

template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::ValidateNeighborsAfterPushDown(HFCPtr<SMPointIndexNode<POINT, EXTENT>> pi_node)
    {
    double xMin = ExtentOp<EXTENT>::GetXMin(pi_node->m_nodeHeader.m_nodeExtent);
    double xMax = ExtentOp<EXTENT>::GetXMax(pi_node->m_nodeHeader.m_nodeExtent);
    double yMin = ExtentOp<EXTENT>::GetYMin(pi_node->m_nodeHeader.m_nodeExtent);
    double yMax = ExtentOp<EXTENT>::GetYMax(pi_node->m_nodeHeader.m_nodeExtent);
    double zMin = ExtentOp<EXTENT>::GetZMin(pi_node->m_nodeHeader.m_nodeExtent);
    double zMax = ExtentOp<EXTENT>::GetZMax(pi_node->m_nodeHeader.m_nodeExtent);

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[0].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[0][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[1].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[1][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[2].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[2][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[3].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[3][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[4].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[4][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[5].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[5][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[6].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[6][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[7].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[7][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION)))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[8].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[8][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[8][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[8][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[9].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[9][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[10].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[10][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[10][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[10][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[11].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[11][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[12].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[12][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[13].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[13][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[14].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[14][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[14][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[14][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[15].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[15][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[16].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[16][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[16][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMax(pi_node->m_apNeighborNodes[16][nodeInd]->m_nodeHeader.m_nodeExtent), zMin, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }
    //----------------------
    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[17].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[17][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[17][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[17][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[18].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[18][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[19].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[19][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[19][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[19][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[20].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[20][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[21].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[21][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[22].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMin(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), yMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[22][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[23].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[23][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[23][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[23][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[24].size(); nodeInd++)
        {
        if ((HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::SMALLER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            (HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), xMin, NEIGHBOR_VALIDATION_PRECISION) &&
            HNumeric<double>::GREATER(ExtentOp<EXTENT>::GetXMax(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION)) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[24][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    for (size_t nodeInd = 0; nodeInd < pi_node->m_apNeighborNodes[25].size(); nodeInd++)
        {
        if (!HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetXMin(pi_node->m_apNeighborNodes[25][nodeInd]->m_nodeHeader.m_nodeExtent), xMax, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetYMax(pi_node->m_apNeighborNodes[25][nodeInd]->m_nodeHeader.m_nodeExtent), yMin, NEIGHBOR_VALIDATION_PRECISION) ||
            !HNumeric<double>::EQUAL(ExtentOp<EXTENT>::GetZMin(pi_node->m_apNeighborNodes[25][nodeInd]->m_nodeHeader.m_nodeExtent), zMax, NEIGHBOR_VALIDATION_PRECISION))
            {
            assert(!"Bad neighbor");
            }
        }

    ValidateDualNeighborship(pi_node);
    }

#endif

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SetNeighborRelationAfterSplit(size_t subNodeInd, size_t parentNeighborInd)
    {
    //NEEDS_WORK_SM:Reactivate this
  //  assert((m_apNeighborNodes[parentNeighborInd].size() == 0) ||
  //         ((m_apNeighborNodes[parentNeighborInd].size() == 1) /*&& (m_apNeighborNodes[parentNeighborInd][0]->m_nodeHeader.m_level < m_apSubNodes[subNodeInd]->m_nodeHeader.m_level)*/));

    //        m_apSubNodes[0]->m_apNeighborNodes[0].push_back(m_apNeighborNodes[0]);


    /*
    if (m_apNeighborNodes[parentNeighborInd]->m_apSubNodes[3] != 0)
    {
    assert(m_apNeighborNodes[parentNeighborInd]->m_apSubNodes[3]->m_nodeHeader.m_level == m_apSubNodes[subNodeInd]->m_nodeHeader.m_level);
    m_apSubNodes[subNodeInd]->m_apNeighborNodes[parentNeighborInd] = m_apNeighborNodes[parentNeighborInd]->m_apSubNodes[3];

    assert(m_apNeighborNodes[parentNeighborInd]->m_apSubNodes[3]->m_apNeighborNodes[7] == 0);
    m_apNeighborNodes[parentNeighborInd]->m_apSubNodes[3]->m_apNeighborNodes[7] = m_apSubNodes[subNodeInd];
    }
    */

    if (m_apNeighborNodes[parentNeighborInd].size() == 1)
        {
        size_t parentNeighborSubNodeInd = 0;
        size_t parentNeighborSubNodeNeighorInd = 0;

        switch (parentNeighborInd)
            {
            //Middle layer
            case 0:
                {
                parentNeighborSubNodeNeighorInd = 7;

                switch (subNodeInd)
                    {
                    case 0:
                        parentNeighborSubNodeInd = 3;
                        break;
                    case 4:
                        parentNeighborSubNodeInd = 7;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 1:
                {
                parentNeighborSubNodeNeighorInd = 6;

                switch (subNodeInd)
                    {
                    case 0:
                        parentNeighborSubNodeInd = 2;
                        break;
                    case 1:
                        parentNeighborSubNodeInd = 3;
                        break;
                    case 4:
                        parentNeighborSubNodeInd = 6;
                        break;
                    case 5:
                        parentNeighborSubNodeInd = 7;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 2:
                {
                parentNeighborSubNodeNeighorInd = 5;

                switch (subNodeInd)
                    {
                    case 1:
                        parentNeighborSubNodeInd = 2;
                        break;
                    case 5:
                        parentNeighborSubNodeInd = 6;
                        break;

                    default:
                        assert(!"Invalid case");
                    }

                }
                break;

            case 3:
                {
                parentNeighborSubNodeNeighorInd = 4;

                switch (subNodeInd)
                    {
                    case 0:
                        parentNeighborSubNodeInd = 1;
                        break;
                    case 2:
                        parentNeighborSubNodeInd = 3;
                        break;
                    case 4:
                        parentNeighborSubNodeInd = 5;
                        break;
                    case 6:
                        parentNeighborSubNodeInd = 7;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 4:
                {
                parentNeighborSubNodeNeighorInd = 3;

                switch (subNodeInd)
                    {
                    case 1:
                        parentNeighborSubNodeInd = 0;
                        break;
                    case 3:
                        parentNeighborSubNodeInd = 2;
                        break;
                    case 5:
                        parentNeighborSubNodeInd = 4;
                        break;
                    case 7:
                        parentNeighborSubNodeInd = 6;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 5:
                {
                parentNeighborSubNodeNeighorInd = 2;

                switch (subNodeInd)
                    {
                    case 2:
                        parentNeighborSubNodeInd = 1;
                        break;
                    case 6:
                        parentNeighborSubNodeInd = 5;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 6:
                {
                parentNeighborSubNodeNeighorInd = 1;

                switch (subNodeInd)
                    {
                    case 2:
                        parentNeighborSubNodeInd = 0;
                        break;
                    case 3:
                        parentNeighborSubNodeInd = 1;
                        break;
                    case 6:
                        parentNeighborSubNodeInd = 4;
                        break;
                    case 7:
                        parentNeighborSubNodeInd = 5;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 7:
                {
                parentNeighborSubNodeNeighorInd = 0;

                switch (subNodeInd)
                    {
                    case 3:
                        parentNeighborSubNodeInd = 0;
                        break;
                    case 7:
                        parentNeighborSubNodeInd = 4;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

                //Bottom layer
            case 8:
                {
                parentNeighborSubNodeNeighorInd = 25;

                switch (subNodeInd)
                    {
                    case 0:
                        parentNeighborSubNodeInd = 7;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 9:
                {
                parentNeighborSubNodeNeighorInd = 24;

                switch (subNodeInd)
                    {
                    case 0:
                        parentNeighborSubNodeInd = 6;
                        break;

                    case 1:
                        parentNeighborSubNodeInd = 7;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 10:
                {
                parentNeighborSubNodeNeighorInd = 23;

                switch (subNodeInd)
                    {
                    case 1:
                        parentNeighborSubNodeInd = 6;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 11:
                {
                parentNeighborSubNodeNeighorInd = 22;

                switch (subNodeInd)
                    {
                    case 0:
                        parentNeighborSubNodeInd = 5;
                        break;

                    case 2:
                        parentNeighborSubNodeInd = 7;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 12:
                {
                parentNeighborSubNodeNeighorInd = 21;

                switch (subNodeInd)
                    {
                    case 0:
                        parentNeighborSubNodeInd = 4;
                        break;

                    case 1:
                        parentNeighborSubNodeInd = 5;
                        break;

                    case 2:
                        parentNeighborSubNodeInd = 6;
                        break;

                    case 3:
                        parentNeighborSubNodeInd = 7;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 13:
                {
                parentNeighborSubNodeNeighorInd = 20;

                switch (subNodeInd)
                    {
                    case 1:
                        parentNeighborSubNodeInd = 4;
                        break;

                    case 3:
                        parentNeighborSubNodeInd = 6;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 14:
                {
                parentNeighborSubNodeNeighorInd = 19;

                switch (subNodeInd)
                    {
                    case 2:
                        parentNeighborSubNodeInd = 5;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 15:
                {
                parentNeighborSubNodeNeighorInd = 18;

                switch (subNodeInd)
                    {
                    case 2:
                        parentNeighborSubNodeInd = 4;
                        break;

                    case 3:
                        parentNeighborSubNodeInd = 5;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 16:
                {
                parentNeighborSubNodeNeighorInd = 17;

                switch (subNodeInd)
                    {
                    case 3:
                        parentNeighborSubNodeInd = 4;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;


            case 17:
                {
                parentNeighborSubNodeNeighorInd = 16;

                switch (subNodeInd)
                    {
                    case 4:
                        parentNeighborSubNodeInd = 3;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 18:
                {
                parentNeighborSubNodeNeighorInd = 15;

                switch (subNodeInd)
                    {
                    case 4:
                        parentNeighborSubNodeInd = 2;
                        break;

                    case 5:
                        parentNeighborSubNodeInd = 3;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 19:
                {
                parentNeighborSubNodeNeighorInd = 14;

                switch (subNodeInd)
                    {
                    case 5:
                        parentNeighborSubNodeInd = 2;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 20:
                {
                parentNeighborSubNodeNeighorInd = 13;

                switch (subNodeInd)
                    {
                    case 4:
                        parentNeighborSubNodeInd = 1;
                        break;

                    case 6:
                        parentNeighborSubNodeInd = 3;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 21:
                {
                parentNeighborSubNodeNeighorInd = 12;

                switch (subNodeInd)
                    {
                    case 4:
                        parentNeighborSubNodeInd = 0;
                        break;

                    case 5:
                        parentNeighborSubNodeInd = 1;
                        break;

                    case 6:
                        parentNeighborSubNodeInd = 2;
                        break;

                    case 7:
                        parentNeighborSubNodeInd = 3;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 22:
                {
                parentNeighborSubNodeNeighorInd = 11;

                switch (subNodeInd)
                    {
                    case 5:
                        parentNeighborSubNodeInd = 0;
                        break;

                    case 7:
                        parentNeighborSubNodeInd = 2;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 23:
                {
                parentNeighborSubNodeNeighorInd = 10;

                switch (subNodeInd)
                    {
                    case 6:
                        parentNeighborSubNodeInd = 1;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 24:
                {
                parentNeighborSubNodeNeighorInd = 9;

                switch (subNodeInd)
                    {
                    case 6:
                        parentNeighborSubNodeInd = 0;
                        break;

                    case 7:
                        parentNeighborSubNodeInd = 1;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;

            case 25:
                {
                parentNeighborSubNodeNeighorInd = 8;

                switch (subNodeInd)
                    {
                    case 7:
                        parentNeighborSubNodeInd = 0;
                        break;

                    default:
                        assert(!"Invalid case");
                    }
                }
                break;
            }


            if (m_apNeighborNodes[parentNeighborInd][0]->m_apSubNodes.size() >= parentNeighborSubNodeInd && m_apNeighborNodes[parentNeighborInd][0]->m_apSubNodes[parentNeighborSubNodeInd] != 0)
            {
            //NEEDS_WORK_SM:Reactivate this
           // assert(m_apNeighborNodes[parentNeighborInd][0]->m_apSubNodes[parentNeighborSubNodeInd]->m_nodeHeader.m_level == m_apSubNodes[subNodeInd]->m_nodeHeader.m_level);
           // assert(m_apSubNodes[subNodeInd]->m_apNeighborNodes[parentNeighborInd].size() == 0);

            m_apSubNodes[subNodeInd]->m_apNeighborNodes[parentNeighborInd].push_back(m_apNeighborNodes[parentNeighborInd][0]->m_apSubNodes[parentNeighborSubNodeInd]);
            //NEEDS_WORK_SM:Reactivate this
           // assert(m_apNeighborNodes[parentNeighborInd][0]->m_apSubNodes[parentNeighborSubNodeInd]->m_apNeighborNodes[parentNeighborSubNodeNeighorInd].size() == 0);
            m_apNeighborNodes[parentNeighborInd][0]->m_apSubNodes[parentNeighborSubNodeInd]->m_apNeighborNodes[parentNeighborSubNodeNeighorInd].push_back(m_apSubNodes[subNodeInd]);
            }
        }
    }

//=====================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/14
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SetNeighborRelationAfterPushDown(size_t          neighborNodeInd,
                                                                                                                vector<size_t>& neighborSubNodeIndexes,
                                                                                                                size_t          neighborSubNodeNeighborInd)
    {

#ifndef NDEBUG
    //Corner at different layer (e.g. : middle versus top) must not have more than one neighbor. 
    switch (neighborNodeInd)
        {
        case 8:
        case 10:
        case 14:
        case 16:
        case 17:
        case 19:
        case 23:
        case 25:
            assert(m_apNeighborNodes[neighborNodeInd].size() <= 1);
        }
#endif

   // assert((m_apNeighborNodes[neighborNodeInd].size() <= 1) || (GetParentNode() != 0) && (GetParentNode()->m_pSubNodeNoSplit != 0));

    for (size_t nodeInd = 0; nodeInd < m_apNeighborNodes[neighborNodeInd].size(); nodeInd++)
        {
        if (!m_apNeighborNodes[neighborNodeInd][nodeInd]->m_nodeHeader.m_IsLeaf && m_apNeighborNodes[neighborNodeInd][nodeInd]->m_pSubNodeNoSplit != 0)
            {
            assert(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_pSubNodeNoSplit->m_nodeHeader.m_level == m_pSubNodeNoSplit->m_nodeHeader.m_level);
            m_pSubNodeNoSplit->m_apNeighborNodes[neighborNodeInd].push_back(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_pSubNodeNoSplit);

            //assert(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_pSubNodeNoSplit->m_apNeighborNodes[neighborSubNodeNeighborInd].size() == 0);            
            m_apNeighborNodes[neighborNodeInd][nodeInd]->m_pSubNodeNoSplit->m_apNeighborNodes[neighborSubNodeNeighborInd].push_back(m_pSubNodeNoSplit);

#ifndef NDEBUG
       //     ValidateNeighborsAfterPushDown(m_pSubNodeNoSplit);
       //     ValidateNeighborsAfterPushDown(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_pSubNodeNoSplit);
#endif
            }
        else
            {
            vector<size_t>::const_iterator neighborSubNodeIndexIter(neighborSubNodeIndexes.begin());
            vector<size_t>::const_iterator neighborSubNodeIndexIterEnd(neighborSubNodeIndexes.end());

            while (neighborSubNodeIndexIter != neighborSubNodeIndexIterEnd)
                {
                if (m_apNeighborNodes[neighborNodeInd][nodeInd]->m_nodeHeader.m_IsBranched && m_apNeighborNodes[neighborNodeInd][nodeInd]->m_apSubNodes.size() > *neighborSubNodeIndexIter && m_apNeighborNodes[neighborNodeInd][nodeInd]->m_apSubNodes[*neighborSubNodeIndexIter] != 0)
                    {
                    assert(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_apSubNodes[*neighborSubNodeIndexIter]->m_nodeHeader.m_level == m_pSubNodeNoSplit->m_nodeHeader.m_level);
                    m_pSubNodeNoSplit->m_apNeighborNodes[neighborNodeInd].push_back(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_apSubNodes[*neighborSubNodeIndexIter]);

                    //assert(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_apSubNodes[*neighborSubNodeIndexIter]->m_apNeighborNodes[neighborSubNodeNeighborInd].size() == 0);            
                    m_apNeighborNodes[neighborNodeInd][nodeInd]->m_apSubNodes[*neighborSubNodeIndexIter]->m_apNeighborNodes[neighborSubNodeNeighborInd].push_back(m_pSubNodeNoSplit);

#ifndef NDEBUG
               //     ValidateNeighborsAfterPushDown(m_pSubNodeNoSplit);
             //       ValidateNeighborsAfterPushDown(m_apNeighborNodes[neighborNodeInd][nodeInd]->m_apSubNodes[*neighborSubNodeIndexIter]);
#endif
                    }

                neighborSubNodeIndexIter++;
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::PropagateDataDownImmediately(bool propagateRecursively)
    {

    // We do not call invariants because this method is called during transformation of node.
    if (!IsLoaded())
        Load();

    //Not touch during editing, nothing to propagate down.
    if (s_inEditing && m_nodeHeader.m_filtered)
        {
        return;
        }
    
    // For a leaf nothing need be done
    if (HasRealChildren() && (this->size() > 0))
        {
        OnPropagateDataDown();
        if (m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode())
            {
            size_t numberSpatial = this->size();
            POINT* spatialArray = new POINT[numberSpatial];
            this->get(spatialArray, numberSpatial);
            this->clear();
            m_nodeHeader.m_3dPointsDescBins.clear();
            // We copy the whole content to this sub-node
            m_pSubNodeNoSplit->AddArrayUnconditional (spatialArray, numberSpatial, m_nodeHeader.m_arePoints3d);

            delete spatialArray;
            }
        else
            {

            size_t numberSpatial = this->size();
            POINT* INSpatialArray = new POINT[numberSpatial];
            this->get(INSpatialArray, numberSpatial);

            vector<bool>   areSpatialArray3dPoints(m_nodeHeader.m_numberOfSubNodesOnSplit,false);
            vector<POINT*> spatialArray(m_nodeHeader.m_numberOfSubNodesOnSplit);
            vector<size_t> spatialArrayNumber(m_nodeHeader.m_numberOfSubNodesOnSplit);

            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                {
                spatialArray[indexNodes] = new POINT[numberSpatial];
                spatialArrayNumber[indexNodes] = 0;
                }

            size_t points3dDescBinInd = 0;
            bool is3d;             
            size_t nextFirst2dInd; //2.5D data
            size_t nextFirst3dInd;
                            
            if (m_nodeHeader.m_3dPointsDescBins.size() > 0)
                {
                is3d = m_nodeHeader.m_3dPointsDescBins[points3dDescBinInd].m_startIndex == 0;
                nextFirst2dInd = m_nodeHeader.m_3dPointsDescBins[points3dDescBinInd].m_startIndex + m_nodeHeader.m_3dPointsDescBins[points3dDescBinInd].m_length;
                nextFirst3dInd = m_nodeHeader.m_3dPointsDescBins[points3dDescBinInd].m_startIndex;
                }   
            else
                {
                is3d = false;
                nextFirst2dInd = numberSpatial;
                nextFirst3dInd = numberSpatial;
                }

            for (size_t indexSpatial = 0; indexSpatial < numberSpatial ; indexSpatial++)
                {
                bool addedToNode = false;

                if (is3d)
                    {
                    if (indexSpatial == nextFirst2dInd)
                        {
                        is3d = false;
                        points3dDescBinInd++;
                        nextFirst2dInd = m_nodeHeader.m_3dPointsDescBins[points3dDescBinInd].m_startIndex + m_nodeHeader.m_3dPointsDescBins[points3dDescBinInd].m_length;
                        nextFirst3dInd = m_nodeHeader.m_3dPointsDescBins[points3dDescBinInd].m_startIndex;
                        }                    
                    }
                else
                    {
                    if (indexSpatial == nextFirst3dInd)
                        {
                        is3d = true;                        
                        }
                    }
                                                
                if (m_nodeHeader.m_arePoints3d)
                    {
                    for (size_t indexNodes = 0; !addedToNode && indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                        {
                        if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent3D(INSpatialArray[indexSpatial], m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent))
                            {
                            spatialArray[indexNodes][spatialArrayNumber[indexNodes]] = INSpatialArray[indexSpatial];
                            spatialArrayNumber[indexNodes]++;
                            areSpatialArray3dPoints[indexNodes] = is3d ? true : areSpatialArray3dPoints[indexNodes];
                            addedToNode = true;
                            }
                        } 

                    assert(addedToNode == true);
                    }
                else
                    {/*
                    //In 2.5D always push the data in the lowest sub-node to allow the stitching to work correctly.                                                                
                    assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[4]->m_nodeHeader.m_nodeExtent));
                    assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[5]->m_nodeHeader.m_nodeExtent));
                    assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[6]->m_nodeHeader.m_nodeExtent));
                    assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[7]->m_nodeHeader.m_nodeExtent));*/
                    assert(m_apSubNodes.size() == 4);
                    assert(is3d == false);

                    size_t nbNodes = m_nodeHeader.m_numberOfSubNodesOnSplit;// / 2;
                    assert(nbNodes == 4);

                    for (size_t indexNodes = 0; !addedToNode && indexNodes < nbNodes; indexNodes++)
                        {
                        if (m_apSubNodes[indexNodes]!= NULL && SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(INSpatialArray[indexSpatial], m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent))
                            {
                            spatialArray[indexNodes][spatialArrayNumber[indexNodes]] = INSpatialArray[indexSpatial];
                            spatialArrayNumber[indexNodes]++;                            
                            addedToNode = true;
                            }
                        }                    
                    std::string s;
                    if (addedToNode != true)
                        {
                        s += "(" + std::to_string(PointOp<POINT>::GetX(INSpatialArray[indexSpatial])) + "," + std::to_string(PointOp<POINT>::GetY(INSpatialArray[indexSpatial]))
                            + "," + std::to_string(PointOp<POINT>::GetZ(INSpatialArray[indexSpatial])) + ")";
                        }

                    //assert(addedToNode == true);
                    }                
                }

            this->clear();
            m_nodeHeader.m_3dPointsDescBins.clear();

            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                {
                if (spatialArrayNumber[indexNodes] > 0)
                    {
                    assert((areSpatialArray3dPoints[indexNodes] == true) || (indexNodes < 4));
                    m_apSubNodes[indexNodes]->AddArrayUnconditional(spatialArray[indexNodes], spatialArrayNumber[indexNodes], areSpatialArray3dPoints[indexNodes]);
                    }
                }

            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                delete [] spatialArray[indexNodes];

            delete [] INSpatialArray;
            }
        }

    // As a result of previous operations it is possible that delayed split be invoked for the present node ...
    if (m_DelayedSplitRequested || this->size() > m_nodeHeader.m_SplitTreshold)
        SplitNode(GetDefaultSplitPosition());

    if (HasRealChildren()  && propagateRecursively)
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

    //NEEDS_WORK_SM - Why setting that? propagating should be done after importing as a separate call. 
    m_nodeHeader.m_filtered = false;    
    //m_nodeHeader.m_nbFaceIndexes = 0;

    HINVARIANTS;

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Discard() const // Intentionaly const ... only mutable members are modified
    {
    HINVARIANTS;
    bool returnValue = true;

    if (!m_destroyed)
        {
        if (!IsLoaded())
            Load();

        // Save the current blockID
        HPMBlockID currentBlockId = GetBlockID();
        bool needStoreHeader = IsDirty();

            // Call ancester Discard
            returnValue = HPMStoredPooledVector<POINT>::Discard();

            if (returnValue)
                {
                //NEEDS_WORK_SM : During partial update some synchro problem can occur.
                if (s_inEditing)
                    {
                    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
                        {
                        m_nodeHeader.m_apNeighborNodeID[neighborPosInd].resize(m_apNeighborNodes[neighborPosInd].size());

                        for (size_t neighborInd = 0; neighborInd < m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
                            {
                            m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd] = m_apNeighborNodes[neighborPosInd][neighborInd]->GetBlockID();
                            }
                        }

                    if (GetParentNodePtr() != 0 && GetParentNodePtr()->GetBlockID().m_integerInitialized)
                        {
                        m_nodeHeader.m_parentNodeID = GetParentNodePtr()->GetBlockID();
                        }
                    }
                //NEEDS_WORK_SM : END

                if (needStoreHeader) static_cast<SMPointTileStore<POINT, EXTENT>*>(&*m_store)->StoreHeader(&m_nodeHeader, GetBlockID()); // TODO: Why do we store the header when not dirty??
                
            }

        if (returnValue && (currentBlockId != GetBlockID()))
            {
            HASSERT(GetBlockID().IsValid());

            // Block ID changed (tile reallocated on store ... advise parent)
            if (GetParentNode() != NULL)
                GetParentNode()->AdviseSubNodeIDChanged(const_cast<SMPointIndexNode<POINT, EXTENT>*>(this));
            
            for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
                {       
                for (size_t neighborInd = 0; neighborInd < m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
                    {                    
                    m_apNeighborNodes[neighborPosInd][neighborInd]->AdviseNeighborNodeIDChanged(const_cast<SMPointIndexNode<POINT, EXTENT>*>(this));
                    }                                     
                }

            AdviseParentNodeIDChanged();
            }
        }
    HINVARIANTS;

    return returnValue;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::AdviseSubNodeIDChanged(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& p_subNode)
    {
    if (!IsLoaded())
        Load();

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (p_subNode == m_pSubNodeNoSplit)
            m_nodeHeader.m_SubNodeNoSplitID = p_subNode->GetBlockID();

        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                if (p_subNode == m_apSubNodes[indexNode])
                    {
                    m_nodeHeader.m_apSubNodeID[indexNode] = p_subNode->GetBlockID();
                    break;
                    }
                }
            }
        SetDirty(true);
        }
    //HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::AdviseNeighborNodeIDChanged(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& p_neighborNode)
    {
    //NEEDS_WORK_SM_NEIGHBOR_ID_STORAGE - Not done yet.
    if (!IsLoaded())
        Load();
        
    size_t neighborPosInd = 0;
    
    for (; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {       
        size_t neighborInd = 0;

        for (; neighborInd < m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
            {                    
            if (m_apNeighborNodes[neighborPosInd][neighborInd].GetPtr() == p_neighborNode.GetPtr())
                {              
                if (m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size() <= neighborInd)
                    {
                    m_nodeHeader.m_apNeighborNodeID[neighborPosInd].resize(neighborInd + 1); 
                    }
                
                m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd] = p_neighborNode->GetBlockID();                
                break;
                }            
            }    

        if (neighborInd < m_apNeighborNodes[neighborPosInd].size())
            {
            break;
            }
        }
    
    assert(neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY);

    SetDirty(true);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::AdviseParentNodeIDChanged() const
    {
    if (!IsLoaded())
        Load();

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != 0)
            {
            m_pSubNodeNoSplit->m_nodeHeader.m_parentNodeID = GetBlockID();
            if(!m_pSubNodeNoSplit->IsVirtualNode()) m_pSubNodeNoSplit->SetDirty(true);
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {                                
                m_apSubNodes[indexNode]->m_nodeHeader.m_parentNodeID = GetBlockID();                                
                m_apSubNodes[indexNode]->SetDirty(true);
                }
            }        
        }
    //HINVARIANTS;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::AdviseNeighborNodeChange(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& oldNeighborNode, HFCPtr<SMPointIndexNode<POINT, EXTENT> >& newNeighborNode)
    {
    assert(oldNeighborNode != 0);

    if (!IsLoaded())
        Load();

    size_t neighborPosInd = 0;

    //NEEDS_WORK_SM : Could change those loops in Advise function by direct position mapping
    for (; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {
        vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>::iterator neighborIter(m_apNeighborNodes[neighborPosInd].begin());
        vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>::iterator neighborIterEnd(m_apNeighborNodes[neighborPosInd].end());

        bool isFound = false;

        if (m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size() > 0)
            {
            vector<HPMBlockID>::iterator neighborIdIter(m_nodeHeader.m_apNeighborNodeID[neighborPosInd].begin());
            vector<HPMBlockID>::iterator neighborIdIterEnd(m_nodeHeader.m_apNeighborNodeID[neighborPosInd].end());

            while (neighborIter != neighborIterEnd && neighborIdIter != neighborIdIterEnd)
                {
                assert(neighborIdIter != neighborIdIterEnd);

                if (neighborIter->GetPtr() == oldNeighborNode.GetPtr())
                    {
                    assert(*neighborIdIter == oldNeighborNode->GetBlockID() ||
                           !neighborIdIter->m_integerInitialized ||
                           !oldNeighborNode->GetBlockID().m_integerInitialized);

                    isFound = true;

                    if (newNeighborNode == 0)
                        {
                        m_apNeighborNodes[neighborPosInd].erase(neighborIter);
                        neighborIdIter = m_nodeHeader.m_apNeighborNodeID[neighborPosInd].erase(neighborIdIter);
                        }
                    else
                        {
                        *neighborIter = newNeighborNode;
                        *neighborIdIter = newNeighborNode->GetBlockID();
                        }

                    m_nodeHeader.m_apAreNeighborNodesStitched[neighborPosInd] = false;

                    break;
                    }

                neighborIter++;
                neighborIdIter++;
                }
            }
        else
            {
            while (neighborIter != neighborIterEnd)
                {
                if (neighborIter->GetPtr() == oldNeighborNode.GetPtr())
                    {
                   // assert(!oldNeighborNode->GetBlockID().m_integerInitialized);

                    isFound = true;

                    if (newNeighborNode == 0)
                        {
                        m_apNeighborNodes[neighborPosInd].erase(neighborIter);
                        }
                    else
                        {
                        *neighborIter = newNeighborNode;
                        }

                    break;
                    }

                neighborIter++;
                }
            }

        if (isFound)
            {
            break;
            }
        }

    //assert(neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY);

    SetDirty(true);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::SetParentNode(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& pi_rpParentNode)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    // The node must be orphan(parentless)
    HPRECONDITION(GetParentNodePtr() == 0);

    SetParentNodePtr(pi_rpParentNode);

    SetDirty(true);

    if (GetParentNodePtr() != NULL)
        GetParentNodePtr()->AdviseSubNodeIDChanged(this);

    HINVARIANTS;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre   12/1
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::SetParentNodePtr(const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& pi_rpParentNode)
    {
    if (m_pParentNode == 0 || pi_rpParentNode == 0)
        {
        m_pParentNode = pi_rpParentNode;

        if (m_pParentNode != 0)
            {
            this->m_nodeHeader.m_parentNodeID = m_pParentNode->GetBlockID();
            }
        else
            {
            this->m_nodeHeader.m_parentNodeID = IDTMFile::GetNullNodeID();
            }

        m_isParentNodeSet = true;
        }
#ifndef NDEBUG
    else
        {
        //Cannot change the parent.
        assert(m_pParentNode.GetPtr() == pi_rpParentNode.GetPtr());
        }
#endif
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& SMPointIndexNode<POINT, EXTENT>::GetParentNode() const
    {
    if (!IsLoaded())
        Load();

    return m_pParentNode;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT> const HFCPtr<SMPointIndexNode<POINT, EXTENT> >& SMPointIndexNode<POINT, EXTENT>::GetParentNodePtr() const
    {
    if (m_isParentNodeSet == true)
        {
        return m_pParentNode;
        }
    else
        {
        return GetParentNode();
        }
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 02/15
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::Track3dPoints(size_t countPoints)
    {

    assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
    bool merged = false;

    if (m_nodeHeader.m_3dPointsDescBins.size() > 0)
        {
        if (m_nodeHeader.m_3dPointsDescBins.back().m_startIndex + m_nodeHeader.m_3dPointsDescBins.back().m_length == this->size())
            {
            m_nodeHeader.m_3dPointsDescBins.back().m_length += countPoints;
            merged = true;
            }
        }

    if (!merged)
        {
        m_nodeHeader.m_3dPointsDescBins.push_back(SMIndexNodeHeader<EXTENT>::RLC3dPoints(this->size(), countPoints));
        }
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::SetSubNodes(HFCPtr<SMPointIndexNode<POINT, EXTENT> > pi_apSubNodes[], size_t numSubNodes)
    {
    HINVARIANTS;

    HINVARIANTS;

    if (!IsLoaded())
        Load();

    // Indicate node is not a leaf anymore
    m_nodeHeader.m_IsLeaf = false;
    m_nodeHeader.m_IsBranched = true;

    // Table must be provided and each subnode pointer point to an existing node
    HPRECONDITION(pi_apSubNodes != NULL);

    size_t indexNode;
    for (indexNode = 0; indexNode < numSubNodes; indexNode++)
        {
        HASSERT(pi_apSubNodes[indexNode] != 0);
        m_apSubNodes[indexNode] = pi_apSubNodes[indexNode];
        }
    for (indexNode = 0; indexNode < numSubNodes; indexNode++)
        {
        m_apSubNodes[indexNode]->SetParentNode(REALTHIS());
        }


    SetDirty(true);


    size_t indexNode;
    for (indexNode = 0 ; indexNode < numSubNodes; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = m_apSubNodes[indexNode]->GetBlockID();
        }
//    SetDirty(true);
    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/14
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::SetNeighborNodes()
    {  
    HINVARIANTS;
        

    size_t neighborPosIndex;
    for (neighborPosIndex = 0 ; neighborPosIndex < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosIndex++)
        {
        for (size_t neighborInd = 0; neighborInd < m_apNeighborNodes[neighborPosIndex].size(); neighborInd++)
            {
            //Should be call only when the node is created.             
            if (m_nodeHeader.m_apNeighborNodeID[neighborPosIndex].size() <= neighborInd)
                {
                m_nodeHeader.m_apNeighborNodeID[neighborPosIndex].resize(neighborInd + 1);
                }            

            m_nodeHeader.m_apNeighborNodeID[neighborPosIndex][neighborInd] = m_apNeighborNodes[neighborPosIndex][neighborInd]->GetBlockID();
            }
        }

    SetDirty(true);
    HINVARIANTS;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 02/15
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::AreAllNeighbor2_5d() const
    {
    bool areAllNeighbor2_5d = true;

    for (size_t neighborPosInd = 0; areAllNeighbor2_5d == true && neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {
        auto neighborIter(m_apNeighborNodes[neighborPosInd].begin());
        auto neighborIterEnd(m_apNeighborNodes[neighborPosInd].end());

        while (neighborIter != neighborIterEnd)
            {
            if ((*neighborIter)->m_nodeHeader.m_arePoints3d)
                {
                areAllNeighbor2_5d = false;
                break;
                }

            neighborIter++;
            }
        }

    return areAllNeighbor2_5d;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SetSplitTreshold(size_t pi_SplitTreshold)
    {
    if (!IsLoaded())
        Load();
    HINVARIANTS;

    // Set new value
    m_nodeHeader.m_SplitTreshold = pi_SplitTreshold;

    // Check if there are sub-nodes
    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->SetSplitTreshold(GetSplitTreshold());
        else
            {
            // Set treshold for all sub-nodes

            for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
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
            SplitNode(GetDefaultSplitPosition());
            }
        }
    SetDirty(true);

    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndexNode<POINT, EXTENT>::GetSplitTreshold() const
    {
    // HINVARIANTS; // We do not call invariants for simple accessors as they are extensively called within reorganising methods
    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_SplitTreshold);
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndexNode<POINT, EXTENT>::GetNumberOfSubNodesOnSplit() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_numberOfSubNodesOnSplit);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::IsBalanced() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_balanced);
    }




//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/14
//=======================================================================================
template<class POINT, class EXTENT>
HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::PullSubNodeNoSplitUp(size_t levelToPullTo)
    {
    if (!IsLoaded())
        Load();

    this->ValidateInvariantsSoft();

    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pulledNode;

    if (m_pSubNodeNoSplit == 0)
        {
        bool hasParentFilteringInvalidated = false;

        for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
            {
            if (m_apSubNodes[indexNode] != 0)
                {
                assert(m_apSubNodes[indexNode]->m_nodeHeader.m_level <= levelToPullTo);

                HFCPtr<SMPointIndexNode<POINT, EXTENT> > pulledNode(m_apSubNodes[indexNode]->PullSubNodeNoSplitUp(levelToPullTo));

                if (m_apSubNodes[indexNode]->m_nodeHeader.m_level == levelToPullTo)
                    {
                    assert((m_apSubNodes[indexNode]->m_pSubNodeNoSplit != 0) || (m_apSubNodes[indexNode]->m_nodeHeader.m_IsLeaf));
                    assert(pulledNode != 0);

                    //NEEDS_WORK_SM : Don't duplicate loop - see below
                    //Neighbors of replaced node become neighbors of replacing node.
                    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
                        {
                        pulledNode->m_apNeighborNodes[neighborPosInd].clear();
                        pulledNode->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].clear();

                        for (size_t neighborInd = 0; neighborInd < m_apSubNodes[indexNode]->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
                            {
                            pulledNode->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].push_back(m_apSubNodes[indexNode]->m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd]);
                            }

                        for (size_t neighborInd = 0; neighborInd < m_apSubNodes[indexNode]->m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
                            {
                            pulledNode->m_apNeighborNodes[neighborPosInd].push_back(m_apSubNodes[indexNode]->m_apNeighborNodes[neighborPosInd][neighborInd]);
                            m_apSubNodes[indexNode]->m_apNeighborNodes[neighborPosInd][neighborInd]->AdviseNeighborNodeChange(m_apSubNodes[indexNode], pulledNode);
                            }
                        }

                    pulledNode->InvalidateStitching();

                    m_apSubNodes[indexNode]->m_pSubNodeNoSplit = 0;

                    //Avoid some assert.
                    m_apSubNodes[indexNode]->m_nodeHeader.m_IsLeaf = true;
                    m_apSubNodes[indexNode]->setNbPointsUsedForMeshIndex(0);
                    m_apSubNodes[indexNode]->Destroy();

                    m_apSubNodes[indexNode] = pulledNode;
                    this->m_nodeHeader.m_apSubNodeID[indexNode] = pulledNode->GetBlockID();
                    m_apSubNodes[indexNode]->SetParentNodePtr(this);
                    m_apSubNodes[indexNode]->m_nodeHeader.m_IsUnSplitSubLevel = false;
                    m_apSubNodes[indexNode]->m_nodeHeader.m_level = levelToPullTo;
                    m_apSubNodes[indexNode]->SetDirty(true);
                    SetDirty(true);

                    if (!hasParentFilteringInvalidated)
                        {
                        m_apSubNodes[indexNode]->InvalidateParentFiltering();
                        hasParentFilteringInvalidated = true;
                        }
                    }
                }
            }
        }
    else
        if (m_pSubNodeNoSplit->m_nodeHeader.m_level < levelToPullTo)
            {
            m_pSubNodeNoSplit->PullSubNodeNoSplitUp(levelToPullTo);
            }
        else
            if (m_pSubNodeNoSplit->m_nodeHeader.m_level == levelToPullTo)
                {
                if (m_pSubNodeNoSplit->m_pSubNodeNoSplit != 0)
                    {
                    pulledNode = m_pSubNodeNoSplit->PullSubNodeNoSplitUp(levelToPullTo);
                    assert(pulledNode != 0);

                    //Neighbors of replaced node become neighbors of replacing node.
                    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
                        {
                        pulledNode->m_apNeighborNodes[neighborPosInd].clear();
                        pulledNode->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].clear();

                        for (size_t neighborInd = 0; neighborInd < m_pSubNodeNoSplit->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
                            {
                            pulledNode->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].push_back(m_pSubNodeNoSplit->m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd]);
                            }

                        for (size_t neighborInd = 0; neighborInd < m_pSubNodeNoSplit->m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
                            {
                            pulledNode->m_apNeighborNodes[neighborPosInd].push_back(m_pSubNodeNoSplit->m_apNeighborNodes[neighborPosInd][neighborInd]);
                            m_pSubNodeNoSplit->m_apNeighborNodes[neighborPosInd][neighborInd]->AdviseNeighborNodeChange(m_pSubNodeNoSplit, pulledNode);
                            }
                        }

                    pulledNode->InvalidateStitching();

                    m_pSubNodeNoSplit->m_pSubNodeNoSplit = 0;
                    //Avoid some assert.
                    m_pSubNodeNoSplit->m_nodeHeader.m_IsLeaf = true;
                    m_pSubNodeNoSplit->setNbPointsUsedForMeshIndex(0);

                    m_pSubNodeNoSplit->Destroy();
                    m_pSubNodeNoSplit = pulledNode;
                    this->m_nodeHeader.m_SubNodeNoSplitID = pulledNode->GetBlockID();
                    m_pSubNodeNoSplit->SetParentNodePtr(this);
                    m_pSubNodeNoSplit->m_nodeHeader.m_level = levelToPullTo;
                    m_pSubNodeNoSplit->SetDirty(true);
                    m_pSubNodeNoSplit->InvalidateParentFiltering();
                    SetDirty(true);
                    }
                }
            else
                {
                assert(m_apSubNodes[0] == 0);

                if (m_pSubNodeNoSplit->m_nodeHeader.m_IsLeaf == false)
                    {
                    assert(m_pSubNodeNoSplit->m_pSubNodeNoSplit != 0);

                    pulledNode = m_pSubNodeNoSplit->PullSubNodeNoSplitUp(levelToPullTo);
                    assert(pulledNode != 0);

                    m_pSubNodeNoSplit->m_pSubNodeNoSplit = 0;
                    //Avoid some assert.
                    m_pSubNodeNoSplit->m_nodeHeader.m_IsLeaf = true;
                    m_pSubNodeNoSplit->Destroy();
                    }
                else
                    {
                    pulledNode = m_pSubNodeNoSplit;
                    }
                }

    HINVARIANTS;

    return pulledNode;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 10/14
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::ReBalance(size_t depth)
    {
    this->ValidateInvariantsSoft();

    if (!IsLoaded())
        Load();

    //Only balanced node can need to be rebalance
    //if (m_nodeHeader.m_balanced)
        {
        if (m_nodeHeader.m_IsLeaf)
            {
            if (m_nodeHeader.m_level < depth)
                if (!m_isGenerating)
                    {
                    if (m_nodeHeader.m_balanced) PushNodeDown(depth);
                    else PushNodeDownVirtual(depth);
                    }
            }
        else
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                //assert(m_pSubNodeNoSplit->m_nodeHeader.m_balanced == true);
                m_pSubNodeNoSplit->ReBalance(depth);
                m_pSubNodeNoSplit->InvalidateFilteringMeshing();
                }
            else
                {
                for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    //assert(m_apSubNodes[indexNode]->m_nodeHeader.m_balanced == true);
                    m_apSubNodes[indexNode]->ReBalance(depth);
                    m_apSubNodes[indexNode]->InvalidateFilteringMeshing();
                    }
                }
            }
        }
    HINVARIANTS;
    }


template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::HasRealChildren() const
    {
    return !IsVirtualNode() && !IsLeaf() && (IsParentOfARealUnsplitNode()) || (m_nodeHeader.m_IsBranched
        && m_apSubNodes.size() == m_nodeHeader.m_numberOfSubNodesOnSplit && m_apSubNodes[0] != NULL);
    }

template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::IsParentOfAVirtualNode() const
    {
    return m_pSubNodeNoSplit != NULL && m_pSubNodeNoSplit->IsVirtualNode();
    }

template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::IsParentOfARealUnsplitNode() const
    {
    return m_pSubNodeNoSplit != NULL && !m_pSubNodeNoSplit->IsVirtualNode();
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/14
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::UnsplitEmptyNode()
    {
    if (!IsLoaded())
        Load();

    this->ValidateInvariantsSoft();

    bool unsplitDone = false;

    //Only unbalanced node might need to be unsplit
    if (!m_nodeHeader.m_balanced && !m_nodeHeader.m_IsLeaf && (m_pSubNodeNoSplit == NULL))
        {
        size_t indexNode = 0;

        for (; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
            {
            if (m_apSubNodes[indexNode]->m_nodeHeader.m_totalCount > 0)
                break;
            }

        if (indexNode == m_nodeHeader.m_numberOfSubNodesOnSplit)
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                m_apSubNodes[indexNode]->Destroy();
                m_apSubNodes[indexNode] = 0;
                }

            m_nodeHeader.m_IsLeaf = true;
            m_nodeHeader.m_IsBranched = false;
            SetDirty(true);

            unsplitDone = true;
            }
        else
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                unsplitDone |= m_apSubNodes[indexNode]->UnsplitEmptyNode();
                }
            /*
            if (unsplitOccurred)
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
            {
            if (m_apSubNodes[indexNode]->m_pSubNodeNoSplit != 0)
            {
            m_apSubNodes[indexNode] = m_apSubNodes[indexNode]->PullSubNodeNoSplitUp();
            size_t decreaseLevels = m_apSubNodes[indexNode]->m_nodeHeader.m_level - this->m_nodeHeader.m_level - 1;
            assert(decreaseLevels >= 1);
            assert((m_apSubNodes[indexNode]->m_pSubNodeNoSplit == 0) && (m_apSubNodes[indexNode]->m_apSubNodes[0] == 0));
            m_apSubNodes[indexNode]->DecreaseLevel(decreaseLevels);
            }
            }
            }
            */
            }
        }

    HINVARIANTS;

    return unsplitDone;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 10/14
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::InvalidateFilteringMeshing(bool becauseDataRemoved)
    {

    // In theory the two next variable invalidations are not required as
    // but just in case the HGFIndexNode            
    if (m_nodeHeader.m_filtered)
        {
        if (!m_nodeHeader.m_IsLeaf)
            {
            //Remove the sub-resolution data.
            setNbPointsUsedForMeshIndex(0);
            clear();
            }
        else
            {
            //Remove only the mesh data.
            //NEEDS_WORK_SM : STITCH_ADD             
            //NEEDS_WORK_SM : Should have some kind of invalid stitching here instead of invalidating the whole mesh
            //                of neighbor nodes. 
            size_t ind = size();
            size_t totalSize = ind;//sizeTotal();

            setNbPointsUsedForMeshIndex(0);

            if (ind < totalSize)
                {
                clearFrom(ind);
                }
            }

        m_nodeHeader.m_filtered = false;
        }

    InvalidateStitching();

    if (becauseDataRemoved)
        {
        m_nodeHeader.m_contentExtentDefined = false;
        }

    m_nodeHeader.m_nbFaceIndexes = 0;
    m_nodeHeader.m_balanced = false;
    SetDirty(true);

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 12/14
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::InvalidateStitching()
    {
    assert(s_inEditing == true);

    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {
        size_t reciprocalPos = GetReciprocalNeighborPos(neighborPosInd);
        m_nodeHeader.m_apAreNeighborNodesStitched[neighborPosInd] = false;

        for (size_t neighborInd = 0; neighborInd < m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
            {
            assert(m_apNeighborNodes[neighborPosInd].size() == 1 || m_apNeighborNodes[neighborPosInd][neighborInd]->m_apNeighborNodes[reciprocalPos].size() == 1);

            m_apNeighborNodes[neighborPosInd][neighborInd]->m_nodeHeader.m_apAreNeighborNodesStitched[reciprocalPos] = false;

            for (size_t otherNeighborInd = 0; otherNeighborInd < m_apNeighborNodes[neighborPosInd][neighborInd]->m_apNeighborNodes[reciprocalPos].size(); otherNeighborInd++)
                {
                m_apNeighborNodes[neighborPosInd][neighborInd]->m_apNeighborNodes[reciprocalPos][otherNeighborInd]->m_nodeHeader.m_apAreNeighborNodesStitched[neighborPosInd] = false;
                }
            }
        }

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/14
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::InvalidateParentFiltering()
    {
    // In theory the two next variable invalidations are not required as
    // but just in case the HGFIndexNode            
    if (GetParentNodePtr() != 0 && GetParentNodePtr()->m_nodeHeader.m_filtered)
        {
        assert(GetParentNodePtr()->m_nodeHeader.m_IsLeaf == false);

        //Remove the sub-resolution data.
        GetParentNodePtr()->setNbPointsUsedForMeshIndex(0);
        GetParentNodePtr()->clear();

        GetParentNodePtr()->m_nodeHeader.m_filtered = false;
        GetParentNodePtr()->m_nodeHeader.m_nbFaceIndexes = 0;
        GetParentNodePtr()->SetDirty(true);

        GetParentNodePtr()->InvalidateParentFiltering();
        }

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::Unbalance()
    {


    if (!IsLoaded())
        Load();

    this->ValidateInvariantsSoft();

    m_nodeHeader.m_balanced = false;

    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->Unbalance();
        else
            {
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
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
template<class POINT, class EXTENT> ISMPointIndexFilter<POINT, EXTENT>* SMPointIndexNode<POINT, EXTENT>::GetFilter() const
    {
    if (!IsLoaded())
        Load();
    // Non validation of invariants in intentional ... this gets called during a
    // temporary state during the creation of sub-nodes, by the subnodes
    return(m_filter);
    }




//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::AddConditional (const POINT pi_rpSpatialObject, bool ExtentFixed, bool isPoint3d)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode(GetDefaultSplitPosition());

    // Check is spatial extent is in node ...
    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(m_nodeHeader.m_nodeExtent, pi_rpSpatialObject))
        {
        return Add (pi_rpSpatialObject, isPoint3d);
        }
    // The spatial object is not in extent ... check if we can increase extent (not extent fixed, no parent and no sub-nodes)
    else if (!ExtentFixed && GetParentNode() == NULL && m_nodeHeader.m_IsLeaf)
        {
        // We can increase the extent ... do it
        m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_nodeExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));

        // We maintain the extent square
        if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) && 
            ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
            }
        else
        if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) && 
            ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
            }
        else
        if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) && 
            ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
            }

        return Add (pi_rpSpatialObject, isPoint3d);
        }

    HINVARIANTS;

    return false;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> size_t SMPointIndexNode<POINT, EXTENT>::AddArrayConditional (const POINT* pointsArray, size_t startPointIndex, size_t countPoints, bool ExtentFixed, bool are3dPoints)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();            
    assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
    //NEEDS_WORK_SM : s_inEditing must not be a static member.
    if (s_inEditing)
        {
        InvalidateFilteringMeshing (); 
        }

    if (this->size() == 0) m_nodeHeader.m_arePoints3d = are3dPoints;
    else m_nodeHeader.m_arePoints3d |= are3dPoints;
    if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
    else SetNumberOfSubNodesOnSplit(8);

    if (m_DelayedSplitRequested)
        SplitNode(GetDefaultSplitPosition());    

    // If nothing to be added ... get out right away
    if (startPointIndex >= countPoints)
        return countPoints;

    // If node is not extent limited ...
    if (!ExtentFixed && GetParentNode() == NULL && m_nodeHeader.m_IsLeaf)
        {

        size_t endPointIndex;
        if (countPoints - startPointIndex + this->size() >= m_nodeHeader.m_SplitTreshold)
            {
            // Not all points will be held by node ...
            endPointIndex = m_nodeHeader.m_SplitTreshold - this->size() + startPointIndex;
            }
        else
            {
            endPointIndex = countPoints;
            }

        for (size_t currIndex = startPointIndex ; currIndex < endPointIndex; currIndex++)
            {
            // We can increase the extent ... do it
            m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(GetNodeExtent(), SpatialOp<POINT, POINT, EXTENT>::GetExtent(pointsArray[currIndex]));
            }

        // We maintain the extent square
         if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) && 
             ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent)));
            }
        else
        if (ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) && 
            ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent)));
            }
        else
        if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent) && 
            ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
            ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, (ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetThickness(m_nodeHeader.m_nodeExtent)));
            }
       
        AddArrayUnconditional (&(pointsArray[startPointIndex]), endPointIndex, are3dPoints);


        if (endPointIndex < countPoints)
            return AddArrayConditional(pointsArray, endPointIndex, countPoints, true, are3dPoints);
        else
            return countPoints;

        }
    else
        {
        // Extent is limited ...
        size_t lastPointsIndexInExtent = startPointIndex;

        if (m_nodeHeader.m_arePoints3d)
            {        
            while ((lastPointsIndexInExtent < countPoints) && (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent3D(pointsArray[lastPointsIndexInExtent], m_nodeHeader.m_nodeExtent)))
                {
                lastPointsIndexInExtent++;
                }
            }
        else
            {
            while ((lastPointsIndexInExtent < countPoints) && (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(pointsArray[lastPointsIndexInExtent], m_nodeHeader.m_nodeExtent)))
                {
                lastPointsIndexInExtent++;
                }
            }

        if (lastPointsIndexInExtent > startPointIndex)
            {
            AddArrayUnconditional (&(pointsArray[startPointIndex]), lastPointsIndexInExtent - startPointIndex, are3dPoints);
            }
        HINVARIANTS;

        return lastPointsIndexInExtent;
        }

    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>    POINT SMPointIndexNode<POINT, EXTENT>::GetDefaultSplitPosition()
    {
    POINT splitPosition;
    double Width = ExtentOp<EXTENT>::GetWidth(GetNodeExtent());
    double Height = ExtentOp<EXTENT>::GetHeight(GetNodeExtent());
    double Thickness = ExtentOp<EXTENT>::GetThickness(GetNodeExtent());
    PointOp<POINT>::SetX(splitPosition,ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + (Width / 2));
    PointOp<POINT>::SetY(splitPosition,ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + (Height / 2));
    PointOp<POINT>::SetZ(splitPosition,ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent) + (Thickness / 2));
    return splitPosition;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>    void SMPointIndexNode<POINT, EXTENT>::SetNumberOfSubNodesOnSplit(size_t numberOfSubNodesOnSplit)
    {
    if (numberOfSubNodesOnSplit == m_nodeHeader.m_numberOfSubNodesOnSplit) return;
    if (!IsLoaded())
        Load();
    if (m_nodeHeader.m_IsBranched)
        {
        ReSplitNode(GetDefaultSplitPosition(), numberOfSubNodesOnSplit);
        }
    else
        {
        m_nodeHeader.m_apSubNodeID.resize(numberOfSubNodesOnSplit);
        m_apSubNodes.resize(numberOfSubNodesOnSplit);
        }
    m_nodeHeader.m_numberOfSubNodesOnSplit = numberOfSubNodesOnSplit;

    SetDirty(true);
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>    bool SMPointIndexNode<POINT, EXTENT>::NeighborsFillParentExtent()

    {
    if (GetParentNode() == nullptr) return false;
    EXTENT ext = m_nodeHeader.m_nodeExtent;
    for (auto& neighborPosition : m_apNeighborNodes)
        {
        for (auto& neighbor : neighborPosition)
            {
            ext = ExtentOp<EXTENT>::MergeExtents(ext, neighbor->GetNodeExtent());
            }
        }
    EXTENT parentExtent = GetParentNode()->GetNodeExtent();
    if (ExtentOp<EXTENT>::GetXMin(parentExtent) < ExtentOp<EXTENT>::GetXMin(ext) ||
        ExtentOp<EXTENT>::GetYMin(parentExtent) < ExtentOp<EXTENT>::GetYMin(ext) ||
        ExtentOp<EXTENT>::GetZMin(parentExtent) < ExtentOp<EXTENT>::GetZMin(ext) ||
        ExtentOp<EXTENT>::GetXMax(parentExtent) > ExtentOp<EXTENT>::GetXMax(ext) ||
        ExtentOp<EXTENT>::GetYMax(parentExtent) > ExtentOp<EXTENT>::GetYMax(ext) ||
        ExtentOp<EXTENT>::GetZMax(parentExtent) > ExtentOp<EXTENT>::GetZMax(ext)) return false;
    else return true;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>    bool SMPointIndexNode<POINT, EXTENT>::AllSiblingsAreNeighbors()

    {
    if (GetParentNode() == nullptr) return false;
    auto siblings = GetParentNode()->m_apSubNodes;
    vector<bool> isSiblingANeighbor(siblings.size(), false);
    for (auto& neighborPosition : m_apNeighborNodes)
        {
        for (auto& neighbor : neighborPosition)
            {
            for (auto& sibling : siblings)
                {
                if (sibling.GetPtr() == neighbor.GetPtr() || sibling.GetPtr() == this)  isSiblingANeighbor[&sibling - &siblings[0]] = true;
                }
            }
        }
    for (auto wasFound : isSiblingANeighbor)
        {
        if (!wasFound) return false;
        }
    return true;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>    bool SMPointIndexNode<POINT, EXTENT>::AllNeighborsAreAtSameLevel()

    {
    for (auto& neighborPosition : m_apNeighborNodes)
        {
        for (auto& neighbor : neighborPosition)
            {
            if (neighbor->m_nodeHeader.m_level != m_nodeHeader.m_level) return false;
            }
        }
        return true;
    }


//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/15
//=======================================================================================
template<class POINT, class EXTENT>    void SMPointIndexNode<POINT, EXTENT>::ValidateNeighborsOfChildren()

    {
    if (m_nodeHeader.m_IsLeaf) return;
    if (m_nodeHeader.m_IsBranched && m_apSubNodes.size() > 0 && m_apSubNodes[0] != NULL)
        {
        for (size_t idx = 0; idx < m_apSubNodes.size(); ++idx)
            {
            if (m_apSubNodes[idx] == NULL) continue;
            assert(m_apSubNodes[idx]->NeighborsFillParentExtent());
            assert(m_apSubNodes[idx]->AllSiblingsAreNeighbors());
            if(m_nodeHeader.m_balanced) assert(m_apSubNodes[idx]->AllNeighborsAreAtSameLevel());
            }
        }
    if (!m_nodeHeader.m_IsBranched && m_pSubNodeNoSplit != NULL)
        {
        if (m_nodeHeader.m_balanced) assert(m_pSubNodeNoSplit->AllNeighborsAreAtSameLevel());
        }
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::AddArrayUnconditional(const POINT* pointsArray, size_t countPoints, bool arePoints3d)
    {
    HINVARIANTS;
    HASSERT (countPoints > 0);
    
    if (!IsLoaded())
        Load();
        
    if (s_inEditing)
        {
        InvalidateFilteringMeshing (); 
        }
    assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
    //If the node turns 3d redistribute the points in a 3D fashion (i.e. : not only in the lowest node). 
    /*
    if (!m_nodeHeader.m_arePoints3d && arePoints3d && node->GetParentNode() != 0 && this->size() > 0)
        {        
        assert(node->GetParentNode()->m_pSubNodeNoSplit == 0);         
                        
        for (size_t pointInd = 0; pointInd < this->size(); pointInd++)
            {
            
            }

        for (size_t indexNode = 0 ; indexNode < node->GetParentNode()->GetNumberOfSubNodesOnSplit(); indexNode++)
            {

            node->GetParentNode()->m_apSubNodes[indexNode]->AddArrayConditional(pointsArray, startIndex, countPoints, true, arePoints3d);
            }
        }
        */
    if (this->size() == 0) m_nodeHeader.m_arePoints3d = arePoints3d;
    else m_nodeHeader.m_arePoints3d |= arePoints3d;
    if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
    else SetNumberOfSubNodesOnSplit(8);
    assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
    //Only bottommost node can have points in 2.5D.
    HASSERT (m_nodeHeader.m_arePoints3d || m_apNeighborNodes[12].size() == 0);

    if (m_DelayedSplitRequested)
        SplitNode(GetDefaultSplitPosition());
    assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
    // All points must be fully contained in node extent

    // Check if the threshold amount of objects is attained
    if (!HasRealChildren() && (this->size() + countPoints >= m_nodeHeader.m_SplitTreshold))
        {
        // There are too much objects ... need to split current node
        SplitNode(GetDefaultSplitPosition());
        assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
        }
    else if (m_delayedDataPropagation && (this->size() + countPoints >= m_nodeHeader.m_SplitTreshold))
        {
        PropagateDataDownImmediately(false);
        assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
        }

    // The total count increase by countSpatial whatever the path selected below
    m_nodeHeader.m_totalCount += countPoints;

    for (size_t indexPoints = 0 ; indexPoints < countPoints ; indexPoints++)
        {
        if (!m_nodeHeader.m_contentExtentDefined)
            {
            m_nodeHeader.m_contentExtent = SpatialOp<POINT, POINT, EXTENT>::GetExtent(pointsArray[0]);
            m_nodeHeader.m_contentExtentDefined = true;
            }
        else
            {
            m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(pointsArray[indexPoints]));
            }
        }

    assert(m_nodeHeader.m_3dPointsDescBins.size() == 0 || m_nodeHeader.m_3dPointsDescBins.back().m_startIndex < this->size());
    // Check if node is still a leaf ...
    if (!HasRealChildren() || (m_delayedDataPropagation && (this->size() + countPoints < m_nodeHeader.m_SplitTreshold)) )
        {
        HDEBUGCODE(size_t initialSize = this->size());
        HDEBUGCODE(bool wasDiscarded = this->Discarded(); wasDiscarded;);

        if (this->size() + countPoints >= this->capacity())
            this->reserve (this->size() + countPoints + m_nodeHeader.m_SplitTreshold / 10);

        HDEBUGCODE(HASSERT(initialSize == this->size()));

        if (arePoints3d)
            {
            Track3dPoints(countPoints);
            }        

        // It is a leaf ... we add reference in list
        this->push_back(pointsArray, countPoints);        
        }
    else
        {
        if (IsParentOfARealUnsplitNode())
            m_pSubNodeNoSplit->AddArrayUnconditional (pointsArray, countPoints, arePoints3d);
        else
            {
            size_t nbNodes;

          //  if (m_nodeHeader.m_arePoints3d)
          //      {
                nbNodes = GetNumberOfSubNodesOnSplit();
         //       }
         /*   else
                {
                assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[4]->m_nodeHeader.m_nodeExtent));
                assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[5]->m_nodeHeader.m_nodeExtent));
                assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[6]->m_nodeHeader.m_nodeExtent));
                assert(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[0]->m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetZMax(m_apSubNodes[7]->m_nodeHeader.m_nodeExtent));

                nbNodes = 4//GetNumberOfSubNodesOnSplit() / 2;
                SetNumberOfSubNodesOnSplit(nbNodes);
                assert(nbNodes == 4);
                }*/

            size_t startIndex = 0;
            while (startIndex < countPoints)
                {
                std::string extentS;
               HDEBUGCODE (size_t previousCount = startIndex);
               for (size_t indexNode = 0 ; indexNode < nbNodes; indexNode++)
                    {
                    startIndex = m_apSubNodes[indexNode]->AddArrayConditional(pointsArray, startIndex, countPoints, true, arePoints3d);
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
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Add(const POINT pi_rpSpatialObject, bool isPoint3d)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (this->size() == 0) m_nodeHeader.m_arePoints3d = isPoint3d;
    else m_nodeHeader.m_arePoints3d |= isPoint3d;
    if (!m_nodeHeader.m_arePoints3d) SetNumberOfSubNodesOnSplit(4);
    else SetNumberOfSubNodesOnSplit(8);
    if (m_DelayedSplitRequested)
        SplitNode(GetDefaultSplitPosition());

    // The total count increases by 1
    m_nodeHeader.m_totalCount += 1;

    // The object must be fully contained in node extent
    HPRECONDITION((SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(pi_rpSpatialObject, m_nodeHeader.m_nodeExtent)));

    // Check if the threshold amount of objects is attained
    if ((!HasRealChildren() || m_nodeHeader.m_IsUnSplitSubLevel) && (this->size() + 1 >= m_nodeHeader.m_SplitTreshold))
        {
        // There are too much objects ... need to split current node
        SplitNode(GetDefaultSplitPosition());
        }
    else if (m_delayedDataPropagation && (this->size() + 1 >= m_nodeHeader.m_SplitTreshold))
        {
        PropagateDataDownImmediately(false);
        }


    if (m_nodeHeader.m_contentExtentDefined)
        {
        m_nodeHeader.m_contentExtent = SpatialOp<POINT, POINT, EXTENT>::GetExtent(pi_rpSpatialObject);
        m_nodeHeader.m_contentExtentDefined = true;
        }
    else
        {
        m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_contentExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));
        }

    // Check if node is still a leaf ...
    if (!HasRealChildren() || m_delayedDataPropagation)
        {
        if (this->size() + 1 >= this->capacity())
            this->reserve (this->size() + m_nodeHeader.m_SplitTreshold / 10);

        if (isPoint3d)
            {
            Track3dPoints(1);
            }

        // It is a leaf ... we add reference in list
        this->push_back(pi_rpSpatialObject);
        }
    else
        {
        // Attempt to add in one of the subnodes
        bool Added = false;
        if (m_pSubNodeNoSplit != NULL)
            {
            m_pSubNodeNoSplit->Add (pi_rpSpatialObject, isPoint3d);
            Added = true;
            }
        else
            {
            for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit() && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent3D(pi_rpSpatialObject, m_apSubNodes[i]->GetNodeExtent()))
                    {
                    // The object is contained ... we add to subnode
                    m_apSubNodes[i]->Add(pi_rpSpatialObject, isPoint3d);
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
                this->reserve (this->size() + m_nodeHeader.m_SplitTreshold / 10);


            this->push_back(pi_rpSpatialObject);
            }
        }
    SetDirty(true);

    HINVARIANTS;

    return true;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::clear()
    {
    this->ValidateInvariantsSoft();

    // Do not waste time on already empty tiles.
    if (this->size() == 0)
        return;

    // Invoque the container clear method
    HPMStoredPooledVector<POINT>::clear();

    // We then Destroy the block ID if stored on file
    if (m_storeBlockID.IsValid())
        m_store->DestroyBlock(m_storeBlockID);

    m_storeBlockID = HPMBlockID();

    SetDirty(true);

    // Block ID changed (tile reallocated on store ... advise parent)
    if (GetParentNode() != NULL)
        GetParentNode()->AdviseSubNodeIDChanged(const_cast<SMPointIndexNode<POINT, EXTENT>*>(this));

    this->ValidateInvariantsSoft();

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> size_t SMPointIndexNode<POINT, EXTENT>::Clear(HFCPtr<HVEShape> pi_shapeToClear, double pi_minZ, double pi_maxZ)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    size_t removedCount = 0;

    // Obtain the shape extent
    HGF2DExtent shapeExtent = pi_shapeToClear->GetExtent();
    // Convert shape into an EXTENT
    EXTENT shapeExtent2 = ExtentOp<EXTENT>::Create(shapeExtent.GetXMin(),
                                                   shapeExtent.GetYMin(),
                                                   pi_minZ,
                                                   shapeExtent.GetXMax(),
                                                   shapeExtent.GetYMax(),
                                                   pi_maxZ);


    // Check if node is a leaf ...
    if (!m_nodeHeader.m_IsLeaf)
        {

        if (m_pSubNodeNoSplit != NULL)
            {
            removedCount += m_pSubNodeNoSplit->Clear (pi_shapeToClear, pi_minZ, pi_maxZ);
            }
        else
            {

            for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit() ; ++ i)
                {
                // Check if extent of node overlap shape.
                // Obtain the extent of the shape
                // Check if object is contained in this sub-node
                if (ExtentOp<EXTENT>::Overlap(m_apSubNodes[i]->GetNodeExtent(), shapeExtent2))
                    {
                    // The object is contained ... we add to subnode
                    removedCount += m_apSubNodes[i]->Clear(pi_shapeToClear, pi_minZ, pi_maxZ);
                    }
                }
            }
        }

    for (size_t currentIndex = 0 ; currentIndex < this->size(); currentIndex++)
        {
        // Check if current object is in shape

        if ((pi_shapeToClear->GetShapePtr()->CalculateSpatialPositionOf(HGF2DLocation (PointOp<POINT>::GetX(this->operator[](currentIndex)),
                                                                                      PointOp<POINT>::GetY(this->operator[](currentIndex)),
                                                                                      pi_shapeToClear->GetCoordSys())) == HVE2DShape::S_IN) &&
            (pi_minZ <= PointOp<POINT>::GetZ(this->operator[](currentIndex))) && (pi_maxZ >= PointOp<POINT>::GetZ(this->operator[](currentIndex))))
            {
            // We have found it... erase it
            this->erase(currentIndex);
            removedCount++;
            }
        }



    if (removedCount > 0)
        {
        SetDirty(true);

        // The total count has changed
        m_nodeHeader.m_totalCount -= removedCount;
        }

    HINVARIANTS;


    return(removedCount);
    }


//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/14
//=======================================================================================
template<class POINT, class EXTENT> size_t SMPointIndexNode<POINT, EXTENT>::RemovePoints(const EXTENT& pi_extentToClear)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    assert(m_filter->IsProgressiveFilter() == false);
    assert(s_inEditing == true);

    size_t removedCount = 0;
    
    if (ExtentOp<EXTENT>::Overlap(GetContentExtent(), pi_extentToClear))
        {
        InvalidateFilteringMeshing (true); 
    
        // Check if node is a leaf ...
        if (!m_nodeHeader.m_IsLeaf)
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                removedCount += m_pSubNodeNoSplit->RemovePoints (pi_extentToClear);
                }
            else
                {
                for (size_t i = 0 ; i < GetNumberOfSubNodesOnSplit() ; ++ i)
                    {
                    // Check if extent of node overlap shape.
                    // Obtain the extent of the shape
                    // Check if object is contained in this sub-node
                    if (ExtentOp<EXTENT>::Overlap(m_apSubNodes[i]->GetNodeExtent(), pi_extentToClear))
                        {
                        // The object is contained ... we add to subnode
                        removedCount += m_apSubNodes[i]->RemovePoints (pi_extentToClear);
                        }
                    }
                }
            }
        else
        if (ExtentOp<EXTENT>::Overlap(GetContentExtent(), pi_extentToClear))
            {
            iterator pointIter(this->begin());
                                                                             
            while (pointIter != this->end())
                {                                                           
                if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(pi_extentToClear, *pointIter))      
                    {                                    
                    // We have found it... erase it
                    pointIter = this->erase(pointIter);     
                    removedCount++;
                    }
                else
                    {
                    pointIter++;
                    }
                }
            }
        }

    if (removedCount > 0)
        {
        SetDirty(true);

        // The total count has changed
        m_nodeHeader.m_totalCount -= removedCount;
        }

    HINVARIANTS;


    return(removedCount);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::PropagatesDataDown() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    // No need to load the node as this parameter is not stored
    return(!m_delayedDataPropagation);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SetPropagateDataDown(bool propagate)
    {
    HINVARIANTS;
    // No need to load the node as this parameter is not stored
    m_delayedDataPropagation = !propagate;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndexNode<POINT, EXTENT>::GetLevel() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_level);
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndexNode<POINT, EXTENT>::GetDepth() const
    {
    HINVARIANTS;

    if (!IsLoaded())
        Load();

        if (m_nodeHeader.m_IsLeaf)
            return m_nodeHeader.m_level;
        else if (m_pSubNodeNoSplit != NULL)
            return m_pSubNodeNoSplit->GetDepth();
        else
            {
            size_t maxDepth = m_nodeHeader.m_level;
            for (size_t indexNode = 0; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                if (m_apSubNodes[indexNode] == NULL) break;
                maxDepth = max(m_apSubNodes[indexNode]->GetDepth(), maxDepth);
                }
            return maxDepth;
            }
       // }
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 11/14
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::DecreaseLevel(size_t numberOfLevels)
    {
    assert(numberOfLevels > 0);
    assert(m_nodeHeader.m_level >= numberOfLevels);

    HINVARIANTS;
    if (!IsLoaded())
        Load();

    m_nodeHeader.m_level -= numberOfLevels;

    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->DecreaseLevel(numberOfLevels);
        else
            {
            for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                m_apSubNodes[indexNode]->DecreaseLevel(numberOfLevels);
                }
            }
        }

    SetDirty(true);

    HINVARIANTS;

    }


template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::IncreaseTotalCount(size_t nOfPoints)
    {
    m_nodeHeader.m_totalCount += nOfPoints;
    if (!m_nodeHeader.m_totalCountDefined) m_nodeHeader.m_totalCountDefined = true;
    if (m_pParentNode != NULL)m_pParentNode->IncreaseTotalCount(nOfPoints);
    }
//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::IncreaseLevel()
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
            for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                m_apSubNodes[indexNode]->IncreaseLevel();
                }
            }
        }

    //NEEDS_WORK_SM : Not sure why it works before with STM without that call.
    if(!IsVirtualNode()) SetDirty(true);

    HINVARIANTS;

    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::IsLeaf() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_IsLeaf);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
const EXTENT& SMPointIndexNode<POINT, EXTENT>::GetNodeExtent() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_nodeExtent);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
const EXTENT& SMPointIndexNode<POINT, EXTENT>::GetContentExtent() const
    {
    // We do not call invariants for simple accessors as they are extensively called within reorganising methods

    if (!IsLoaded())
        Load();

    return(m_nodeHeader.m_contentExtent);
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::SetNodeExtent(const EXTENT& extent)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    m_nodeHeader.m_nodeExtent = extent;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
uint64_t SMPointIndexNode<POINT, EXTENT>::GetCount() const
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    //NEEDS_WORK_SM - Should behave differently based on the filter type
    if (false/*m_filter->IsProgressiveFilter() == true*/)
        {
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
                    for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                        {
                        count += m_apSubNodes[i]->GetCount();
                        }
                    }
                }
            m_nodeHeader.m_totalCount = count;
            m_nodeHeader.m_totalCountDefined = true;
            }
        }
    else
        {
        static bool s_bypassTotalCount = false;

        if (!m_nodeHeader.m_totalCountDefined || s_bypassTotalCount)
            {
            uint64_t count = 0;

            if (!IsLeaf())
                {
                if (m_pSubNodeNoSplit != NULL)
                    {
                    count += m_pSubNodeNoSplit->GetCount();
                    }
                else
                    {
                    for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                        {
                        count += m_apSubNodes[i]->GetCount();
                        }
                    }
                }
            else
                {
                count += this->size();
                }

            m_nodeHeader.m_totalCount = count;
            m_nodeHeader.m_totalCountDefined = true;
            }
        }

    HINVARIANTS;

    return m_nodeHeader.m_totalCount;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndexNode<POINT, EXTENT>::GetSplitDepth() const
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
            for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
                {
                deepestSplitLevel = max(deepestSplitLevel, m_apSubNodes[i]->GetSplitDepth());
                }
            }
        }
    return deepestSplitLevel;
    }

//=======================================================================================
// @bsimethod                                                   Richard.Bois 02/13
//=======================================================================================
template<class POINT, class EXTENT>
HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndexNode<POINT, EXTENT>::GetSubNodeNoSplit() const
    {
    return m_pSubNodeNoSplit;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::IsEmpty() const
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_nodeHeader.m_totalCountDefined)
        return (m_nodeHeader.m_totalCount == 0);

    if (this->size() > 0)
        return false;

    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            return m_pSubNodeNoSplit->IsEmpty();
            }
        else
            {
            for (size_t i = 0; i < GetNumberOfSubNodesOnSplit(); ++i)
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
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::PreQuery (ISMPointIndexQuery<POINT, EXTENT>* queryObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->PreQuery (this, pSubNodes, 1);

            if (digDown)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PreQuery (queryObject);
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);
            digDown = queryObject->PreQuery(this, &subNodes[0], GetNumberOfSubNodesOnSplit());

            if (digDown)
                {
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->PreQuery(queryObject);
                    }
                }

            }
        }
    else
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->PreQuery (this, pSubNodes, 0);
        }

    HINVARIANTS;

    return digDown;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, list<POINT>& resultPoints)
    {
    HINVARIANTS;

    if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
        {
        return false;
        }

    if (!IsLoaded())
        Load();

    bool digDown = true;   

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->Query (this, pSubNodes, 1, resultPoints);

            if (digDown)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query (queryObject, resultPoints);
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

            digDown = queryObject->Query(this, &subNodes[0], GetNumberOfSubNodesOnSplit(), resultPoints);

            if (digDown)
                {
                vector<size_t> queryNodeOrder;

                queryObject->GetQueryNodeOrder(queryNodeOrder, this, &subNodes[0], GetNumberOfSubNodesOnSplit());
                                
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[queryNodeOrder[indexNodes]]))->Query(queryObject, resultPoints);
                    }
                }

            }
        }
    else
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->Query (this, pSubNodes, 0, resultPoints);
        }


    HINVARIANTS;

    return digDown;
    }
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, HPMMemoryManagedVector<POINT>& resultPoints)
    {
    HINVARIANTS;

    if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
        {
        return false;
        }

    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->Query (this, pSubNodes, 1, resultPoints);

            if (digDown)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query (queryObject, resultPoints);
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

            digDown = queryObject->Query(this, &subNodes[0], GetNumberOfSubNodesOnSplit(), resultPoints);

            if (digDown)
                {
                vector<size_t> queryNodeOrder;

                queryObject->GetQueryNodeOrder(queryNodeOrder, this, &subNodes[0], GetNumberOfSubNodesOnSplit());
                                
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[queryNodeOrder[indexNodes]]))->Query(queryObject, resultPoints);
                    }                
                }
            }
        }
    else
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->Query (this, pSubNodes, 0, resultPoints);
        }


    HINVARIANTS;

    return digDown;
    }



template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Query(ISMPointIndexQuery<POINT, EXTENT>* queryObject, BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* resultMesh)
    {
    HINVARIANTS;

    if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
        {
        return false;
        }

    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->Query (this, pSubNodes, 1, resultMesh);

            if (digDown)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query (queryObject, resultMesh);
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

            digDown = queryObject->Query(this, &subNodes[0], GetNumberOfSubNodesOnSplit(), resultMesh);

            if (digDown)
                {
                vector<size_t> queryNodeOrder;

                queryObject->GetQueryNodeOrder(queryNodeOrder, this, &subNodes[0], GetNumberOfSubNodesOnSplit());
                                
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[queryNodeOrder[indexNodes]]))->Query(queryObject, resultMesh);
                    }                                
                }
            }
        }
    else
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->Query (this, pSubNodes, 0, resultMesh);
        }


    HINVARIANTS;

    return digDown;
    }

static bool s_progressiveDisplay2 = false;

template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject, vector<QueriedNode>& meshNodes)
    {
    HINVARIANTS;

    if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
        {
        return false;
        }

    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->Query (this, pSubNodes, 1, meshNodes);

            if (digDown)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query (queryObject, meshNodes);
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

            digDown = queryObject->Query(this, &subNodes[0], GetNumberOfSubNodesOnSplit(), meshNodes);

            if (digDown)
                {                
                if (s_progressiveDisplay2)
                    {
                    size_t indexNodes = 0;

                    for (; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                        {                        
                        if (static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->Discarded())
                            break;
                        }                      

                    if (indexNodes < GetNumberOfSubNodesOnSplit())
                        {
                        meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(this));
                        }
                    }

                vector<size_t> queryNodeOrder;

                queryObject->GetQueryNodeOrder(queryNodeOrder, this, &subNodes[0], GetNumberOfSubNodesOnSplit());
                                
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    if (s_progressiveDisplay2)
                        {
                        if (m_apSubNodes[queryNodeOrder[indexNodes]]->IsLoaded() == true)
                            {
                            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[queryNodeOrder[indexNodes]]))->Query(queryObject, meshNodes);
                            }
                        else
                            {
                            meshNodes.push_back(SMPointIndexNode<POINT, EXTENT>::QueriedNode(this));
                            }
                        }
                    else
                        {
                        static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[queryNodeOrder[indexNodes]]))->Query(queryObject, meshNodes);
                        }
                    }                                
                }
            }
        }
    else        
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->Query (this, pSubNodes, 0, meshNodes);
        }


    HINVARIANTS;

    return digDown;
    }

template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::Query(ISMPointIndexQuery<POINT, EXTENT>* queryObject, HFCPtr<SMPointIndexNode<POINT, EXTENT>>& resultNode)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->Query(this, pSubNodes, 1, resultNode);

            if (digDown)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query(queryObject, resultNode);
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
            for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

            digDown = queryObject->Query(this, &subNodes[0], GetNumberOfSubNodesOnSplit(), resultNode);

            if (digDown)
                {
                for (size_t indexNodes = 0; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                    {
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->Query(queryObject, resultNode);
                    }
                }

            }
        }
    else
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->Query(this, pSubNodes, 0, resultNode);
        }


    HINVARIANTS;

    return digDown;
    }



//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::PostQuery (ISMPointIndexQuery<POINT, EXTENT>* queryObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->PostQuery (this, pSubNodes, 1);

            if (digDown)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PostQuery (queryObject);
            }
        else
            {
            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);
            digDown = queryObject->PostQuery(this, &subNodes[0], GetNumberOfSubNodesOnSplit());

            if (digDown)
                {
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->PostQuery(queryObject);
                    }
                }

            }
        }
    else
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->PostQuery (this, pSubNodes, 0);
        }

    HINVARIANTS;

    return digDown;
    }



/**----------------------------------------------------------------------------
 This method returns the number of points corresponding to a particular node.

 @param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> uint32_t SMPointIndexNode<POINT, EXTENT>::GetNbObjects() const
    {
    if (m_NbObjects == -1 || IsDirty())
        {
        uint32_t NbObjects;

        NbObjects = (uint32_t)size();

        //Compute the
        if (((m_filter == NULL) ||(m_filter->IsProgressiveFilter() == true)) && (GetParentNodePtr() != 0))
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNumberObjectsInAncestors(m_nodeHeader.m_nodeExtent, NbObjects);
            }

        m_NbObjects = NbObjects;
        }

    HASSERT(m_NbObjects != -1);

    return (uint32_t)m_NbObjects;
    }

/**----------------------------------------------------------------------------
 This method returns the number of points corresponding to a particular node on
 a particular level.

 @param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> uint64_t SMPointIndexNode<POINT, EXTENT>::GetNbObjectsAtLevel(size_t pi_depthLevel) const
    {
    if (!IsLoaded())
        Load();

    HASSERT(pi_depthLevel >= GetLevel());

    uint64_t nbObjects = 0;

    if (m_filter->IsProgressiveFilter() == true)
        {
        nbObjects = size();

        if (GetLevel() < pi_depthLevel)
            {
            if (m_pSubNodeNoSplit != NULL)
                {
                nbObjects += m_pSubNodeNoSplit->GetNbObjectsAtLevel(pi_depthLevel);
                }
            else
                {
                for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    nbObjects += m_apSubNodes[indexNode]->GetNbObjectsAtLevel(pi_depthLevel);
                    }
                }
            }
        }
    else
        {
        if (pi_depthLevel == GetLevel())
            {
            nbObjects = GetNbObjects();
            }
        else
            {
            nbObjects = 0;

            if (m_pSubNodeNoSplit != NULL)
                {
                nbObjects = m_pSubNodeNoSplit->GetNbObjectsAtLevel(pi_depthLevel);
                }
            else
                {
                for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    nbObjects += m_apSubNodes[indexNode]->GetNbObjectsAtLevel(pi_depthLevel);
                    }
                }
            }
        }

    return (uint32_t)nbObjects;
    }

/**----------------------------------------------------------------------------
 Gets a list of objects for the node. Note that in progressive mode some
 objects for the node are located in the ancestor nodes.

 @param pio_rListOfObjects The list of objects pertaining to the node.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> size_t SMPointIndexNode<POINT, EXTENT>::GetNodeObjects(list<POINT>& pio_rListOfObjects) const
    {
    for (uint32_t PtInd = 0; PtInd < size(); PtInd++)
        {
        pio_rListOfObjects.push_back(this->operator[](PtInd));
        }
    
    if ((m_filter->IsProgressiveFilter() == true) && (GetParentNodePtr() != 0))
        {
        static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNodeObjectsInAncestors(m_nodeHeader.m_nodeExtent, pio_rListOfObjects);
        }

    return pio_rListOfObjects.size();
    }


#ifdef INDEX_DUMPING_ACTIVATED

/**----------------------------------------------------------------------------
 This method returns the number of points corresponding to a particular node.

 @param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::DumpOctTreeNode(FILE* pi_pOutputXmlFileStream,
        bool pi_OnlyLoadedNode) const
    {
    if ((pi_OnlyLoadedNode == true) && (IsLoaded() == false))
        return;

    if (!IsLoaded())
        Load();

    char   TempBuffer[3000];
    int    NbChars;
    size_t NbWrittenChars;        
    uint32_t nodeId;

    if (GetBlockID().IsValid())
        {
        nodeId = GetBlockID().m_integerID;
        }
    else
        {
        nodeId = IDTMFile::GetNullNodeID();
        }   

    NbChars = sprintf(TempBuffer, "<ChildNode NodeId=\"%i\" TotalPoints=\"%i\" SplitDepth=\"%i\" ArePoints3d=\"%i\">", nodeId, GetCount(), GetSplitDepth(), m_nodeHeader.m_arePoints3d ? 1 : 0);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

    //Extent
    NbChars = sprintf(TempBuffer,
                      "<NodeExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></NodeExtent>\n",
                      ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent), 
                      ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent));

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    if (m_nodeHeader.m_contentExtentDefined)
        {
        NbChars = sprintf(TempBuffer,
                          "<ContentExtent><MinX>%.20f</MinX><MaxX>%.20f</MaxX><MinY>%.20f</MinY><MaxY>%.20f</MaxY><MinZ>%.20f</MinZ><MaxZ>%.20f</MaxZ></ContentExtent>\n",
                          ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent), 
                          ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_contentExtent));

        NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

        HASSERT(NbWrittenChars == NbChars);
        }


    //Number Of Points
    NbChars = sprintf(TempBuffer, "<NbOfPoints>%u</NbOfPoints>\n", GetNbObjects());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Cumulative Number Of Points
    NbChars = sprintf(TempBuffer, "<CumulNbOfPoints>%u</CumulNbOfPoints>\n", GetCount());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //Level
    NbChars = sprintf(TempBuffer, "<Level>%i</Level>\n", m_nodeHeader.m_level);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // SplitTreshold
    NbChars = sprintf(TempBuffer, "<SplitTreshold>%i</SplitTreshold>", m_nodeHeader.m_SplitTreshold);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // Balanced
    if (m_nodeHeader.m_balanced)
        NbChars = sprintf(TempBuffer, "<Balanced>true</Balanced>");
    else
        NbChars = sprintf(TempBuffer, "<Balanced>false</Balanced>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);


    //View Dependent Metrics
    /*
    NbChars = sprintf(TempBuffer,
                      "<ViewDependentMetrics>%.3f</ViewDependentMetrics>",
                      m_nodeHeader.m_ViewDependentMetrics[0]);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);
    */


    // Neighbor Node    
    NbChars = sprintf(TempBuffer, "<NeighborNode> ");
        
    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {              
        for (size_t neighborInd = 0; neighborInd < m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
            {
            NbChars = sprintf(TempBuffer, "P %i I %i Id %i ", neighborPosInd, neighborInd, m_nodeHeader.m_apNeighborNodeID[neighborPosInd][neighborInd].m_integerID);
        
            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);                        
            }                                     
        }

    NbChars = sprintf(TempBuffer, "</NeighborNode>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    //GraphID
    NbChars = sprintf(TempBuffer, "<GraphID>%u</GraphID>\n", m_nodeHeader.m_graphID.m_integerID);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    // Neighbor Stitching    
    NbChars = sprintf(TempBuffer, "<NeighborStitching> ");
        
    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
        {      
        if (m_nodeHeader.m_apAreNeighborNodesStitched[neighborPosInd] == true)
            {
            NbChars = sprintf(TempBuffer, "1 ");
        
            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }
        else
            {
            NbChars = sprintf(TempBuffer, "0 ");
        
            NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

            HASSERT(NbWrittenChars == NbChars);
            }                                       
        }

    NbChars = sprintf(TempBuffer, "</NeighborStitching>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);


    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_pSubNodeNoSplit))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->DumpOctTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
                }
            }
        }

    NbChars = sprintf(TempBuffer, "</ChildNode>\n");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

    static bool s_checkFilteringRatio = false;

    if (s_checkFilteringRatio)
        {
        if (!m_nodeHeader.m_IsLeaf)
            {
            size_t nbPointsInChildren = 0; 

            if (m_pSubNodeNoSplit != 0)
                {
                nbPointsInChildren = m_pSubNodeNoSplit->size();
                }  
            else
                {                
                for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    nbPointsInChildren += m_apSubNodes[indexNode]->size();
                    }                
                }
                        
            if (nbPointsInChildren > 50)
                {                                
                double diff = (double)size() / nbPointsInChildren;

                static FILE* pFile = 0;

                if (pFile == 0)
                    {                        
                    pFile = fopen("D:\\MyDoc\\Scalable Mesh Iteration 8\\Display Optimisation\\Log\\filtering.csv", "w+");
                    }

                NbChars = sprintf(TempBuffer, "%.05f\n", diff);

                NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pFile);
    
                HASSERT(NbChars == NbWrittenChars);
                
                fflush(pFile);

                if (diff < 1/8.0 * 0.95 || diff > 1/8.0 * 1.05)
                    {
                    
                    //assert(0);
                    }   
                }
            }
        }
    }

#endif


#ifdef __HMR_DEBUG
/**----------------------------------------------------------------------------
 This method returns the number of points corresponding to a particular node.

 @param
-----------------------------------------------------------------------------*/        
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::ValidateIs3dDataNodeState(const vector<DRange3d>& source2_5dRanges, const vector<DRange3d>& source3dRanges) const
    {     
    //Content extent must intersect with extent of a source containing same point type.
    DRange3d contentRange(DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent),
                                         ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent),
                                         ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_contentExtent),
                                         ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent),
                                         ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent),
                                         ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_contentExtent)));            

    if (!m_nodeHeader.m_arePoints3d)
        {
        auto source2_5dRangeIter(source2_5dRanges.begin());
        auto source2_5dRangeIterEnd(source2_5dRanges.end());

        while (source2_5dRangeIter != source2_5dRangeIterEnd)
            {            
            if (source2_5dRangeIter->IntersectsWith(contentRange))
                break;

            source2_5dRangeIter++;
            }

        assert(source2_5dRangeIter != source2_5dRangeIterEnd);
        }
    else
        {
        auto source3dRangeIter(source3dRanges.begin());
        auto source3dRangeIterEnd(source3dRanges.end());

        while (source3dRangeIter != source3dRangeIterEnd)
            {
            if (source3dRangeIter->IntersectsWith(contentRange))
                break;

            source3dRangeIter++;
            }

        assert(source3dRangeIter != source3dRangeIterEnd);
        }           

    //Only bottommost node can have points in 2.5D.
    HASSERT (m_nodeHeader.m_arePoints3d || size() == 0 || m_apNeighborNodes[12].size() == 0);

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_pSubNodeNoSplit))->ValidateIs3dDataNodeState(source2_5dRanges, source3dRanges);
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->ValidateIs3dDataNodeState(source2_5dRanges, source3dRanges);
                }
            }
        }        
    }    
#endif


/**----------------------------------------------------------------------------
 This method return the number of points corresponding to a particular node.

 @param Current number of points for the node.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::AddNumberObjectsInAncestors(EXTENT& pi_rChildNodeExtent,
        uint32_t& pio_rNbPoints) const
    {
    HPRECONDITION((m_filter == NULL) || (m_filter->IsProgressiveFilter() == true));

    for (uint32_t PtInd = 0; PtInd < size(); PtInd++)
        {
        if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent3D(this->operator[](PtInd), pi_rChildNodeExtent))
            //{

            // Check if point lies inside (Borders are inclusive here)


            /*
                    if ((this->operator[](PtInd).GetX() >= m_XMin) &&
                        (this->operator[](PtInd).GetX() < m_XMax) &&
                        (this->operator[](PtInd).GetY() >= m_YMin) &&
                        (this->operator[](PtInd).GetY() < m_YMax))*/
            {
            pio_rNbPoints++;
            }
        }

    if (GetParentNodePtr() != 0)
        {
        static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNumberObjectsInAncestors(pi_rChildNodeExtent, pio_rNbPoints);
        }
    }

/**----------------------------------------------------------------------------
 This method return the objects that are

 @param Current number of points for the node.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::AddNodeObjectsInAncestors(const EXTENT& pi_rChildNodeExtent,
        list<POINT>&  pio_rListOfObjects) const
    {
    HPRECONDITION(m_filter->IsProgressiveFilter() == true);

    for (uint32_t PtInd = 0; PtInd < size(); PtInd++)
        {
        if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent3D(this->operator[](PtInd), pi_rChildNodeExtent))
            {
            pio_rListOfObjects.push_back(this->operator[](PtInd));
            }
        }

    if (GetParentNodePtr() != 0)
        {
        static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNodeObjectsInAncestors(pi_rChildNodeExtent, pio_rListOfObjects);
        }
    }






//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::PreFilter()
    {
    HINVARIANTS;

    if (!IsLoaded())
        Load();

    // This variable is used to determine if parent node must be pre-filtered
    // after sub-nodes have been filtered. Any sub-nodes indicating diggin up is required
    // will result in parent node being processed.
    bool digUp = true;

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            if (static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsFiltering())
                digUp = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PreFilter();

            if (digUp)
                {
                vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >> pSubNodes(1);
                pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT> *>(&*m_pSubNodeNoSplit);

                digUp = m_filter->PreFilter (this, pSubNodes, 1);
                }
            }
        else
            {
            digUp = false;
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                if (static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsFiltering())
                    digUp = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->PreFilter() || digUp;
                }


            if (digUp)
                {
                vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
                for (size_t indexNodes = 0 ; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                    subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

                digUp = m_filter->PreFilter (this, subNodes, m_nodeHeader.m_numberOfSubNodesOnSplit);
                }
            }

        }
    else
        {
        vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >> pSubNodes(1);
        digUp = m_filter->PreFilter (this, pSubNodes, 0);
        }

    return digUp;

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::Filter(int pi_levelToFilter)
    {
    if (!IsLoaded())
        Load();

    HINVARIANTS;

    if (pi_levelToFilter == -1 || (int)this->m_nodeHeader.m_level <= pi_levelToFilter)
        {
        // If there are sub-nodes and these need filtering then first do the subnodes
        if (HasRealChildren())
            {
            if (m_pSubNodeNoSplit != NULL)
                {
    #ifdef __HMR_DEBUG
                if ((m_pSubNodeNoSplit->m_unspliteable) || (m_pSubNodeNoSplit->m_parentOfAnUnspliteableNode))
                    this->m_parentOfAnUnspliteableNode = true;
    #endif
                if (pi_levelToFilter == -1 || (int)this->m_nodeHeader.m_level < pi_levelToFilter)
                    {
                    if (static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsFiltering())
                        static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Filter(pi_levelToFilter);
                    }

                if (pi_levelToFilter == -1 || (int)this->m_nodeHeader.m_level == pi_levelToFilter)
                    {
                    if (m_pSubNodeNoSplit->size() > 0)
                        {
                        vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >> pSubNodes(1);
                        pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);

                        m_filter->Filter (this, pSubNodes, 1, m_nodeHeader.m_ViewDependentMetrics);
                    
                        if (m_filter->IsProgressiveFilter())
                            {
                            m_pSubNodeNoSplit->SetDirty(true);
                            }
                        }

                    m_nodeHeader.m_filtered = true;
                                
                    SetDirty (true);
                    }                
                }
            else
                {
                if (pi_levelToFilter == -1 || (int)this->m_nodeHeader.m_level < pi_levelToFilter)
                    {
                    for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                        {
        #ifdef __HMR_DEBUG
                        if ((m_apSubNodes[indexNode]->m_unspliteable) || (m_apSubNodes[indexNode]->m_parentOfAnUnspliteableNode))
                            this->m_parentOfAnUnspliteableNode = true;
        #endif
                        if (static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsFiltering())
                            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Filter(pi_levelToFilter);
                        }
                    }

                if ((pi_levelToFilter == -1 || this->m_nodeHeader.m_level == pi_levelToFilter) && (this->m_nodeHeader.m_level > 0 || NeedsFiltering()))
                    {
                    vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
                    for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                        subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

                    m_filter->Filter (this, subNodes, m_nodeHeader.m_numberOfSubNodesOnSplit, m_nodeHeader.m_ViewDependentMetrics);
                    if (m_filter->IsProgressiveFilter())
                        {
                        for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                            {
                            m_apSubNodes[indexNode]->SetDirty(true);
                            }
                        }

                    m_nodeHeader.m_filtered = true;

                    SetDirty (true);
                    }
                }

            //NEEDS_WORK_SM : Not sure it should be done here.
            if ((m_nodeHeader.m_filtered == true) && (m_nodeHeader.m_totalCountDefined == true))
                {
                if (m_pSubNodeNoSplit != 0)
                    {
                    m_nodeHeader.m_totalCount = m_pSubNodeNoSplit->m_nodeHeader.m_totalCount;
                    }
                else
                    {
                    m_nodeHeader.m_totalCount = 0;
            
                    for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                        {
                        if (m_apSubNodes[indexNodes] != NULL)
                            {                        
                            m_nodeHeader.m_totalCount += m_apSubNodes[indexNodes]->m_nodeHeader.m_totalCount;
                            }
                        }                                       
                    }
                }
            }
        else
            {
            if (m_filter->FilterLeaf (this, m_nodeHeader.m_ViewDependentMetrics))
                SetDirty (true);

            m_nodeHeader.m_filtered = true;
            }        
        }

    // Now filtering can be performed using the sub-nodes filtered data. This data
    // accessed using the HPMPooledVector interface the Node is a descendant of.
    // Do not hesitate to increase the HPMPooledVector interface if required.
    // The result of the filtering must be added to the Node itself in the
    // HPMVectorPool<POINT> descendant class using the push_back interface.
    // If refiltering is required then clear() must be called beforehand.
    // The member m_nodeHeader.m_filtered should be set to true after the filtering process
    // All members that must be serialized in the file must be added in the
    // m_nodeHeader fields/struct and these will automatically be serialized in the
    // store. Note that changing this structure automatically
    // renders invalid any previous file.

    ValidateInvariants();
    }




//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::PostFilter()
    {
    HINVARIANTS;

    if (!IsLoaded())
        Load();

    // This variable is used to determine if parent node must be pre-filtered
    // after sub-nodes have been filtered. Any sub-nodes indicating diggin up is required
    // will result in parent node being processed.
    bool digUp = true;

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (HasRealChildren())
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            digUp = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PostFilter();

            vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >> pSubNodes(1);
            pSubNodes[0] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);

            if (digUp)
                digUp = m_filter->PostFilter (this, pSubNodes, 1);
            }
        else
            {
            digUp = false;
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                digUp = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->PostFilter() || digUp;
                }


            if (digUp)
                {
                vector<HFCPtr<SMPointIndexNode<POINT, EXTENT>>> subNodes(GetNumberOfSubNodesOnSplit());
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                    subNodes[indexNodes] = static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);


                digUp = m_filter->PostFilter (this, subNodes, GetNumberOfSubNodesOnSplit());
                }
            }

        }
    else
        {
        vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >> pSubNodes(1);
        digUp = m_filter->PostFilter (this, pSubNodes, 0);
        }

    HINVARIANTS;

    return digUp;

    }


/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::ComputeObjectRelevance()
    {
    if (!IsLoaded())
        Load();

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (!m_nodeHeader.m_IsLeaf)


        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->ComputeObjectRelevance();
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->ComputeObjectRelevance();
                }

            //Relevance parts of the filtering??
            }
        }
    else
        {
        m_filter->ComputeObjectRelevance(this);
        }

    SetDirty (true);

    ValidateInvariants();
    }

/**----------------------------------------------------------------------------
 Adjust the view dependent metric.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::AdjustViewDependentMetric(HVE2DShape& pi_rConvexHull)
    {
    if (!IsLoaded())
        Load();

    HVE2DRectangle TileExtent(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                              ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                              ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                              ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent),
                              pi_rConvexHull.GetCoordSys());

    HVE2DShape::SpatialPosition SpatialPos1 = TileExtent.CalculateSpatialPositionOf(pi_rConvexHull);
    HVE2DShape::SpatialPosition SpatialPos2 = pi_rConvexHull.CalculateSpatialPositionOf(TileExtent);

    if (SpatialPos2 == HVE2DShape::S_IN)
        m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;
    else if ((SpatialPos1 == HVE2DShape::S_OUT) && (SpatialPos2 == HVE2DShape::S_OUT))
        m_nodeHeader.m_ViewDependentMetrics[0] = 0.0;
    else
        //The tile is not completely inside the convex hull or the convex hull is completly inside the tile,
        //recompute the view dependent metric and those of its children.
        {
        if (!m_nodeHeader.m_IsLeaf)
            {
            if (m_pSubNodeNoSplit != NULL)
                static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->AdjustViewDependentMetric(pi_rConvexHull);
            else
                {
                for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->AdjustViewDependentMetric(pi_rConvexHull);
                    }
                }
            }

        double DTMAreaWithinTile = pi_rConvexHull.IntersectShape(TileExtent)->CalculateArea();


        //Compute with the DTM
#if 0 //WIP
        list<POINT> listOfPoints;

        size_t NbPts = GetNodeObjects(listOfPoints);

        double DTMAreaWithinTileVal = DTMAreaWithinTile;

        BC_DTM_OBJ* pDTMObj = 0;
        P3D         pt;

        int status = bcdtmObject_createDtmObject(&pDTMObj);

        list<POINT>::iterator ptIter    = listOfPoints.begin();
        list<POINT>::iterator ptIterEnd = listOfPoints.end();

        while (ptIter != ptIterEnd)
            {
            pt.X = ptIter->GetX();
            pt.Y = ptIter->GetY();
            pt.Z = ptIter->GetZ();

            if (bcdtmObject_storeDtmFeatureInDtmObject(pDTMObj, DTMF_RANDOM_SPOT, pDTMObj->nullUserTag, 1, &pDTMObj->nullFeatureId, &pt, 1))
                {
                //Problem with the insertion of a point.
                status = 1;
                break;
                }

            ptIter++;
            }

        HASSERT(status == 0);

        status =  bcdtmObject_triangulateDtmObject(pDTMObj);


        HASSERT(status == 0);

        P3D* hullPtsP = 0;
        long numHullPts;

        status = bcdtmList_extractHullDtmObject(pDTMObj, &hullPtsP, &numHullPts);

        HASSERT(status == 0);

        //Adjust view dependent metrics considering the convex hull.
        HArrayAutoPtr<double> pHullPts(new double[numHullPts * 2]);

        for (int hullPtInd = 0, bufferInd = 0; hullPtInd < numHullPts; hullPtInd++)
            {
            pHullPts[bufferInd] = hullPtsP[hullPtInd].X;
            bufferInd++;
            pHullPts[bufferInd] = hullPtsP[hullPtInd].Y;
            bufferInd++;
            }

        HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());

        HVE2DPolygonOfSegments ConvexHull(numHullPts * 2,
                                          pHullPts,
                                          pCoordSys);


        /*
                BC_DTM_OBJ*

                listOfObjects

                */

#endif

        double TileArea = TileExtent.CalculateArea();

        m_nodeHeader.m_ViewDependentMetrics[0] = DTMAreaWithinTile / TileArea;


#if 0 //WIP
        double ConvexHullForPointsInTileDTM = ConvexHull.CalculateArea();

        double DTMCoverageInTile = ConvexHullForPointsInTileDTM / TileArea;


        m_nodeHeader.m_ViewDependentMetrics[0] = DTMCoverageInTile;

        double CoverageDiff = DTMCoverageInTile - m_nodeHeader.m_ViewDependentMetrics[0];
        double CoverageDiffInPercent = (DTMCoverageInTile - m_nodeHeader.m_ViewDependentMetrics[0]) / m_nodeHeader.m_ViewDependentMetrics[0] * 100;
#endif


        //m_nodeHeader.m_ViewDependentMetrics[0] *= DTMAreaWithinTile / TileArea;

        //Temporary using m_ViewDependentMetrics[0] as a storing location for the DTM area to tile area ratio.





        //ToDo : We also need to update the metrics for the other camera position.

        }

    SetDirty(true);

    // In theory the following assertion would be valid yet since the convex hull is approximate
    // and may not contain all points, this will occur. For the moment we will allow the
    // metric to remain 0.0 which may lead to a desity decrease in this area ... too bad
    // We may only fix this by refining the convex hull computation.
    //HASSERT ((this->size() == 0) ||  (m_nodeHeader.m_ViewDependentMetrics[0] > 0.00000000000000001));

    ValidateInvariants();
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::IsFiltered() const
    {
    if (!IsLoaded())
        Load();
    return m_nodeHeader.m_filtered;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndexNode<POINT, EXTENT>::NeedsFiltering() const
    {
    if (!IsLoaded())
        Load();

    bool needsFiltering = false;

    if (m_nodeHeader.m_IsLeaf)
        needsFiltering = !m_nodeHeader.m_filtered;
    else
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            needsFiltering = !m_nodeHeader.m_filtered || static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsFiltering();
            }
        else
            {
            needsFiltering = !m_nodeHeader.m_filtered;
            
            for (size_t indexNode = 0 ; !needsFiltering && indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                needsFiltering = (needsFiltering || static_cast<SMPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsFiltering());
                }
            }
        }

    return needsFiltering;

    }

























//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// SMPointIndex Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

/**----------------------------------------------------------------------------------------------
 Constructor for this class. The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split.

    @param pool IN The memory manager pool to be used. This pool is used to limit the
    number of points in memory at any given time.

    @param store IN The store to which points are stored. This store is used in conjunction with
        pool management system.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-------------------------------------------------------------------------------------------------*/
template<class POINT, class EXTENT> SMPointIndex<POINT, EXTENT>::SMPointIndex(HFCPtr<HPMCountLimitedPool<POINT> > pool, HFCPtr<SMPointTileStore<POINT, EXTENT> > store, size_t pi_SplitTreshold, ISMPointIndexFilter<POINT, EXTENT>* filter,bool balanced, bool propagatesDataDown)
    //: HGFSpatialIndex <POINT, POINT, EXTENT, SMPointIndexNode<POINT, EXTENT>, SMPointIndexHeader<EXTENT> >(pi_SplitTreshold, balanced, propagatesDataDown, new SMPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, ExtentOp<EXTENT>::Create(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), pool, store, filter, balanced, propagatesDataDown, &m_createdNodeMap)),
     : m_pool(pool),
      m_store (store),
      m_filter (filter)

    {

    m_propagatesDataDown = propagatesDataDown;
    m_indexHeader.m_numberOfSubNodesOnSplit = 8;

    m_indexHeaderDirty = true;

    HINVARIANTS;
    m_indexHeader.m_SplitTreshold = pi_SplitTreshold;
    m_indexHeader.m_HasMaxExtent = false;
    m_indexHeader.m_balanced = balanced;
    m_indexHeader.m_depth = 0;
    m_isGenerating = true;
    // If a store is provided ...
    if (store != NULL)
        {
        // Try to load master header
        if (0 != store->LoadMasterHeader(&m_indexHeader, sizeof(m_indexHeader)))
            {

            m_isGenerating = false;
            // File allready contains a DTM ... load it
            // First reset pool pointer and store pointer that got wiped during load
            m_pool = pool;
            m_store = store;
            // (Split threshold, max extent and so on are ignored (we retain stored value)
            // In this particular case, the root node is created immediately

            // We check the root node address which should normaly be valid. If there is a master header but there is no root node ...
            // If I had to make a guess, I would think that we may dealing with a corrupt file
            // however since this file structure is not explicitely invalid, it may simply be that the
            // initial creator of the file never filed the index yet and intended to do it eventually.
            // Given this assumption we simply do not create the root node and it will be created upon the initial
            // addition
            //if (m_indexHeader.m_rootNodeBlockID.IsValid())
               // {
                // If the index is currently balanced ... we create the node as stipulated by construction parameter
                // If it is unbalanced however we want the root node to indicate it is unbalanced in order to balance it
                // immediately after construction
               // if (m_indexHeader.m_balanced)
                //    m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);//new SMPointIndexNode<POINT, EXTENT>(m_indexHeader.m_rootNodeBlockID, HFCPtr<SMPointIndexNode<POINT, EXTENT> >(NULL), pool, store, m_filter, balanced, PropagatesDataDown(), &m_createdNodeMap);
               // else
               //     m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);//new SMPointIndexNode<POINT, EXTENT>(m_indexHeader.m_rootNodeBlockID, HFCPtr<SMPointIndexNode<POINT, EXTENT> >(NULL), pool, store, m_filter, m_indexHeader.m_balanced, PropagatesDataDown(),  &m_createdNodeMap);                                                 

                // We insist on using the given configuration (we will reconfigure all nodes accordingly below)
                // if the request is for a balanced index. If an unbalanced index is required then we will not force unbalance it
                // as the current balanced or unbalanced topology is equally satisfactory.
               // m_indexHeader.m_balanced = balanced;

               /* if (balanced)
                    m_pRootNode->Balance(GetDepth());*/
               // }


            // Index header just loaded ... it is clean
            m_indexHeaderDirty = false;


            }
        else
            {
            // No master header ... force writting it (to prime the store)
            if (!store->StoreMasterHeader (&m_indexHeader, sizeof(m_indexHeader)))
                {
                HASSERT(!"Error in store master header!");
                throw; //TDORAY: throw something significant here
                }
            }

        }

    // TDORAY: Validate soft instead...
    //HINVARIANTS;
    }





/**----------------------------------------------------------------------------
 Copy Constructor
 The internal index items (nodes) are copied but the indexed spatial objects
 are not.

 @param pi_rObj IN The spatial index to copy construct from.
-----------------------------------------------------------------------------*/
// template<class POINT, class EXTENT> SMPointIndex<POINT, EXTENT>::SMPointIndex(const SMPointIndex& pi_rObj)
// : SMPointIndex<POINT, EXTENT, SMPointIndexNode> (pi_rObj)
// {
//     HINVARIANTS;
//      if (pi_rObj != this)
//      {
//      m_LastNode = 0;
//      m_indexHeader.m_SplitTreshold = pi_rObj.GetSplitTreshold();
//      m_indexHeader.m_MaxExtent = pi_rObjm_indexHeader..m_MaxExtent;
//      m_indexHeader.m_HasMaxExtent = pi_rObjm_indexHeader..m_HaxMaxExtent;
//         m_indexHeader.m_rootNodeBlockID = 0;
//
//              m_pool = pi_rObj->GetPool();
//         m_store = pi_rObj->GetStore();
//      }
// }



/**----------------------------------------------------------------------------
 Destructor
 If the index has unstored nodes then those will be stored.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> SMPointIndex<POINT, EXTENT>::~SMPointIndex()
    {
    HINVARIANTS;

    Store();            

    m_createdNodeMap.clear();

    if (m_pRootNode != NULL)
        m_pRootNode->Unload();

    m_pRootNode = NULL;

    // Close store
    m_store->Close();

    if (m_filter != NULL)
        delete m_filter;

    
    m_store = NULL;
    }

/**----------------------------------------------------------------------------
 Stores the present node on store (Discard) and stores all sub-nodes prior to this
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::ChangeStore(HFCPtr<SMPointTileStore<POINT, EXTENT> > newStore)
    {
    HINVARIANTS;

    // Set the store
    m_store = newStore;

    if (NULL != m_pRootNode)
        m_pRootNode->ChangeStore (newStore);

    // Store master header
    m_store->StoreMasterHeader (&m_indexHeader, sizeof(m_indexHeader));


    HINVARIANTS;

    return true;
    }

/**----------------------------------------------------------------------------
 Return the filter

 @return Point to filter or NULL if none set
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> ISMPointIndexFilter<POINT, EXTENT>* SMPointIndex<POINT, EXTENT>::GetFilter()
    {
    //HINVARIANTS;

    return(m_filter);
    }


#ifdef INDEX_DUMPING_ACTIVATED
/**----------------------------------------------------------------------------
 This method dumps the content of the QuadTree as an XML file.

 @param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::DumpOctTree(char* pi_pOutputXMLFileName,
        bool pi_OnlyLoadedNode) const
    {
    FILE* pOutputFileStream = fopen(pi_pOutputXMLFileName, "w+");

    char TempBuffer[500];
    int  NbChars;

    NbChars = sprintf(TempBuffer, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");

    size_t NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);

    HASSERT(NbWrittenChars == NbChars);

    NbChars = sprintf(TempBuffer, "<RootNode>\n");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);

    HASSERT(NbWrittenChars == NbChars);

    m_pRootNode->DumpOctTreeNode(pOutputFileStream, pi_OnlyLoadedNode);

    NbChars = sprintf(TempBuffer, "</RootNode>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);

    HASSERT(NbWrittenChars == NbChars);

    fclose(pOutputFileStream);
    }
#endif


#ifdef __HMR_DEBUG
/**----------------------------------------------------------------------------
 ValidateIs3dDataStates 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::ValidateIs3dDataStates(const vector<DRange3d>& source2_5dRanges, const vector<DRange3d>& source3dRanges) const        
    {        
    if (m_pRootNode != 0)
        {
        m_pRootNode->ValidateIs3dDataNodeState(source2_5dRanges, source3dRanges);    
        }
    }
#endif

/**----------------------------------------------------------------------------
 Balance
 Balance the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::BalanceDown(size_t depthBeforePartialUpdate, bool keepUnbalanced)
    {    

    m_pRootNode->PropagateDataDownImmediately (true);
    
    //size_t depth = m_pRootNode->GetDepth();    

    //NEEDS_WORK_SM - Not good when removing points, probably only good when adding points. 
    if (true/*depthBeforePartialUpdate != depth*/)
        {               
        if (s_inEditing)
            {
            bool hasRootChanged = BalanceRoot();
            bool hasUnsplitDone = UnsplitEmptyNode();

            if (hasRootChanged || hasUnsplitDone)
                {            
                PullSubNodeNoSplitUp();
                }
            }

        assert(m_pRootNode->m_nodeHeader.m_balanced == false);
        m_indexHeader.m_balanced = false;
 
        SetBalanced(!keepUnbalanced);
            auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode);
            volatile size_t nFeaturesInit = meshNode->CountAllFeatures();
           nFeaturesInit = nFeaturesInit;
        //NEEDS_WORK_SM - Couldn't we pass the depth to the function given that it 
        //might require loading and passing all the node.
         Balance();
        }
    auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(m_pRootNode);
    volatile size_t nFeaturesEnd = meshNode->CountAllFeatures();
    nFeaturesEnd = nFeaturesEnd;
    assert(m_pRootNode->GetSplitDepth() == m_pRootNode->GetDepth());    
    //assert(depth == m_pRootNode->GetDepth());
    }

template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::SetBalanced(bool balanced)
    {
    m_indexHeader.m_balanced = balanced;
    if (m_pRootNode == 0) return;
    m_pRootNode->SetBalanced(balanced);
    }

/**----------------------------------------------------------------------------
 BalanceRoot
 Change the root node if after points removal all the points are found in 
 only one children. 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::BalanceRoot()
    {
    assert(m_pRootNode->m_pSubNodeNoSplit == 0);
    bool hasRootChanged = false;

    size_t nbNodesWithPoint = 0;
    size_t lastIndexNodeWithPoints = 0;

    for (size_t indexNode = 0 ; indexNode < m_pRootNode->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
        {
        if (m_pRootNode->m_apSubNodes[indexNode]->m_nodeHeader.m_totalCount > 0)
            {
            nbNodesWithPoint++;        
            lastIndexNodeWithPoints = indexNode;
            }
        }

    assert(nbNodesWithPoint > 0);

    if (nbNodesWithPoint == 1)
        {
        HFCPtr<SMPointIndexNode<POINT, EXTENT>> pNewRootNode(m_pRootNode->m_apSubNodes[lastIndexNodeWithPoints]);

        if (!pNewRootNode->IsLoaded())
            pNewRootNode->Load();
        
        pNewRootNode->SetParentNodePtr(0);         
        
        //Avoid some assert by calling Destroy on sub-node separately. 
        for (size_t indexNode = 0 ; indexNode < m_pRootNode->m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
            {
            if (indexNode != lastIndexNodeWithPoints)
                {
                m_pRootNode->m_apSubNodes[indexNode]->Destroy();
                }

            m_pRootNode->m_apSubNodes[indexNode] = 0;
            }

#ifndef NDEBUG        
        for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
            {       
            assert(pNewRootNode->m_apNeighborNodes[neighborPosInd].size() == 0);
            assert(pNewRootNode->m_nodeHeader.m_apNeighborNodeID[neighborPosInd].size() == 0);
            }
#endif

        //Cut the link with the new root so that it is not destroyed
        m_pRootNode->m_nodeHeader.m_IsLeaf = true;
        //Avoid some assert.
        m_pRootNode->setNbPointsUsedForMeshIndex(0);
        m_pRootNode->Destroy();
        m_pRootNode = pNewRootNode;

        //NEEDS_WORK_SM : Could but this function in the node and decrease level only once in the index.
        m_pRootNode->DecreaseLevel(1);
		m_pRootNode->SetDirty(true);
        
        hasRootChanged = true;
        
        BalanceRoot();        
        }

    return hasRootChanged;
    }

/**----------------------------------------------------------------------------
 PullSubNodeNoSplitUp
 Pull empty node after data removal
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::PullSubNodeNoSplitUp()
    {
    size_t depth = m_pRootNode->GetDepth();
    size_t splitDepth = m_pRootNode->GetSplitDepth();

    assert(depth >= splitDepth);

    if (depth > splitDepth)
        {        
        m_pRootNode->PullSubNodeNoSplitUp(splitDepth);
        }    
    }

/**----------------------------------------------------------------------------
 UnsplitEmptyNode
 Unsplit empty node after data removal
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::UnsplitEmptyNode()
    {
    return m_pRootNode->UnsplitEmptyNode();
    }

template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndex<POINT, EXTENT>::CreateNewNode(EXTENT extent)
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), extent, m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown(), &m_createdNodeMap);
    pNewNode->m_isGenerating = m_isGenerating;
    return pNewNode;
    }

template<class POINT, class EXTENT>  HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndex<POINT, EXTENT>::CreateNewNode(HPMBlockID blockID)
    {
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > pNewNode = new SMPointIndexNode<POINT, EXTENT>(blockID, m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown(), &m_createdNodeMap);
    pNewNode->m_isGenerating = m_isGenerating;
    return pNewNode;
    }


/**----------------------------------------------------------------------------
 Filter
 Filter the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::Filter(int pi_levelToFilter)
    {
    HINVARIANTS;

    // Check if root node is present
    if (m_pRootNode != NULL)
        {        
        if (m_pRootNode->GetFilter()->GlobalPreFilter (*this))
            {
            try
                {
                try
                    {
                    if (m_pRootNode->GetFilter()->ImplementsPreFiltering())
                        {
                        assert(pi_levelToFilter == -1);
                        m_pRootNode->PreFilter ();
                        }

                    m_pRootNode->Filter(pi_levelToFilter);
                    }
                catch (...)
                    {
                    if (m_pRootNode->GetFilter()->ImplementsPostFiltering())
                        {
                        assert(pi_levelToFilter == -1);
                        m_pRootNode->PostFilter ();
                        }
                    throw;
                    }

                if (m_pRootNode->GetFilter()->ImplementsPreFiltering())
                    {
                    assert(pi_levelToFilter == -1);
                    m_pRootNode->PostFilter ();
                    }
                }
            catch (...)
                {
                m_pRootNode->GetFilter()->GlobalPostFilter (*this);
                throw;
                }

            // Call global post filter
            m_pRootNode->GetFilter()->GlobalPostFilter (*this);

            }
        }

    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Filter
 Filter the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::ComputeObjectRelevance()
    {
    m_pRootNode->ComputeObjectRelevance();
    }

/**----------------------------------------------------------------------------
 AdjustViewDependentMetric
 Adjust the view dependent metric.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::AdjustViewDependentMetric(HVE2DShape& pi_rConvexHull)
    {
    m_pRootNode->AdjustViewDependentMetric(pi_rConvexHull);
    }




/**----------------------------------------------------------------------------
 This method returns the pool

 @return The pool

-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> HFCPtr<HPMCountLimitedPool<POINT> > SMPointIndex<POINT, EXTENT>::GetPool() const
    {
    HINVARIANTS;
    return m_pool;
    }

/**----------------------------------------------------------------------------
 This method returns the store

 @return The store

-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> HFCPtr<SMPointTileStore<POINT, EXTENT> > SMPointIndex<POINT, EXTENT>::GetStore() const
    {
    HINVARIANTS;
    return m_store;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 12/2010
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::Store()
    {
    HINVARIANTS;

    // Store root node
    if (m_pRootNode != NULL)
        {
        m_pRootNode->Store();        

        //NEEDS_WORK_SM : Ugly hack to ensure the neighbor node ID are saved. Check if the ID couldn't be determine before 
        //storing the node to avoid doing double store.        
        if (!m_pRootNode->IsLoaded())
            {
            m_pRootNode->Load();
            }

        m_pRootNode->Store();
        //NEEDS_WORK_SM - END

#ifdef INDEX_DUMPING_ACTIVATED
        static bool s_dropNodes = false;

        if (s_dropNodes)
            {                           
            DumpOctTree("D:\\MyDoc\\Scalable Mesh Iteration 8\\PartialUpdate\\Neighbor\\Log\\NodeAferCreationR.xml", false);                
          //  ValidateNeighbors();
            }        
#endif

        HPMBlockID newBlockID = m_pRootNode->GetBlockID();        
        if (m_indexHeader.m_rootNodeBlockID != newBlockID)
            {
            m_indexHeader.m_rootNodeBlockID = newBlockID;
            m_indexHeaderDirty = true;
            }
        }

    if (m_indexHeaderDirty)
    {
        m_store->StoreMasterHeader (&m_indexHeader, sizeof(m_indexHeader));
        m_indexHeaderDirty = false;
    }

    return true;
    }

/**----------------------------------------------------------------------------
 This method adds a spatial object reference in the spatial index

 @param pi_rpSpatialObject IN A smart pointer to spatial object to index.
 No check are performed to verify that the spatial object is already indexed or not.
 If the spatial object is already indexed then it will be indexed twice.

 @return true if item is added and false otherwise. It is possible to refuse addition
 of an item if the spatial index is limited in size (HasMaxExtent()) and the
 item extent does not completely lie within this maximum extent (GetMaxExtent()).

-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::Add(const POINT pi_rpSpatialObject, bool isPoint3d)
    {
    HINVARIANTS;
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    //// Update content extent

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_HasMaxExtent)
            m_pRootNode = CreateNewNode(m_indexHeader.m_MaxExtent);//new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), m_indexHeader.m_MaxExtent, m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown());
        else
            m_pRootNode = CreateNewNode(SpatialOp<POINT, POINT, EXTENT>::ExtentExtract(pi_rpSpatialObject));//new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), SpatialOp<POINT, POINT, EXTENT>::ExtentExtract(pi_rpSpatialObject), m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown());
        m_pRootNode->m_nodeHeader.m_arePoints3d |= isPoint3d;
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    if (!m_pRootNode->AddConditional (pi_rpSpatialObject, m_indexHeader.m_HasMaxExtent, isPoint3d))
        {
        // Item could not be added

        // Make sure that the node extent contains the object extent
        while (!((m_pRootNode->GetNodeExtent().IsPointIn(SpatialOp<POINT, POINT, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin())) &&
                 (m_pRootNode->GetNodeExtent().IsPointIn(SpatialOp<POINT, POINT, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_indexHeader.m_HasMaxExtent)
                return false;

            // The extent is not contained... we must create a new node
            PushRootDown(SpatialOp<POINT, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));
            }


        // The root node contains the spatial object ... add it
        m_pRootNode->Add(pi_rpSpatialObject);
        }


    HINVARIANTS;

    return true;
    }



/**----------------------------------------------------------------------------
 This method adds a list of coordinates to the index.

 @param pi_rpSpatialObject IN A smart pointer to spatial object to index.
 No check are performed to verify that the spatial object is already indexed or not.
 If the spatial object is already indexed then it will be indexed twice.

 @return true if item is added and false otherwise. It is possible to refuse addition
 of an item if the spatial index is limited in size (HasMaxExtent()) and the
 item extent does not completely lie within this maximum extent (GetMaxExtent()).

-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::AddArray(const POINT* pointsArray, size_t countOfPoints, bool are3dPoints)
    {
    HINVARIANTS;
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    if(0 == countOfPoints)
        return true;

#ifdef SCALABLE_MESH_ATP
    m_nbInputPoints += countOfPoints;
#endif

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = CreateNewNode(m_indexHeader.m_MaxExtent); // new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), m_indexHeader.m_MaxExtent, m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
        else
            m_pRootNode = CreateNewNode(SpatialOp<POINT, POINT, EXTENT>::GetExtent(pointsArray[0])); //new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), SpatialOp<POINT, POINT, EXTENT>::GetExtent(pointsArray[0]), m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown(), m_mesher2_5d, m_mesher3d, &m_createdNodeMap);
        m_pRootNode->m_nodeHeader.m_arePoints3d |= are3dPoints;
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    size_t numberOfPointsAdded = 0;

    while (numberOfPointsAdded < countOfPoints)
        {
        numberOfPointsAdded = m_pRootNode->AddArrayConditional (pointsArray, numberOfPointsAdded, countOfPoints, m_indexHeader.m_HasMaxExtent, are3dPoints);

        if (numberOfPointsAdded < countOfPoints)
            {
            // Not all Item could not be added

            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_indexHeader.m_HasMaxExtent)
                return false;

            // The extent is not contained... we must create a new node
            PushRootDown(SpatialOp<POINT, POINT, EXTENT>::GetExtent(pointsArray[numberOfPointsAdded]));
            }
        }    

    HINVARIANTS;
    return true;
    }



/**----------------------------------------------------------------------------
 This method removes a spatial object reference from the spatial index.

 Notice that if the object is indexed many times in the spatial index then it
 will only be removed once.

 @param pi_rpSpatialObject IN Pointer to spatial index to remove.

 @return true if the object was removed and false otherwise
-----------------------------------------------------------------------------*/
//template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::Remove(const POINT pi_rpSpatialObject)
//{
//    HINVARIANTS;
//
//    bool Removed = false;
//
//    // Check if initial node allocated
//    if (m_pRootNode != 0)
//    {
//        // The root node present ... remove
//        Removed = m_pRootNode->Remove(pi_rpSpatialObject);
//    }
//
//    return(Removed);
//}




template<class POINT, class EXTENT> HFCPtr<SMPointIndexNode<POINT, EXTENT> > SMPointIndex<POINT, EXTENT>::FindNode(EXTENT ext, size_t level) const
    {
    if (m_pRootNode == nullptr) return nullptr;
    return m_pRootNode->FindNode(ext, level);
    }

/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the neighborhood of given coordinate
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::GetAt(const POINT& pi_rCoord,
                                                                               list<POINT>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    m_pRootNode->GetAt(pi_rCoord, pio_rListOfObjects, 0, &m_pLastNode);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 This method returns the number of objects at a particular level.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> uint64_t SMPointIndex<POINT, EXTENT>::GetNbObjectsAtLevel(size_t pi_depthLevel)
    {
    uint64_t nbObjects;

    if (pi_depthLevel > GetDepth())
        {
        nbObjects = 0;
        }
    else
        {
        nbObjects = m_pRootNode->GetNbObjectsAtLevel(pi_depthLevel);
        }

    return nbObjects;
    }

/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the given extent
-----------------------------------------------------------------------------*/
//template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::GetIn(const EXTENT& pi_rExtent,
//                                                               list<POINT>& pio_rListOfObjects) const
//{
//    HINVARIANTS;
//
//    // Save the number of objects currently in list
//    size_t InitialNumberOfObjects = pio_rListOfObjects.size();
//
//    m_pRootNode->GetIn(pi_rExtent, pio_rListOfObjects);
//
//    // Return number of newly found objects
//    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
//}




#if (0)
/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the given extent and at a resolution dependent on the current viewing point.
-----------------------------------------------------------------------------*/

template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::GetInUsingViewParameters(const EXTENT&       pi_rExtent,
        double              pi_RootToViewMatrix[][4],
        double              pi_ViewportRotMatrix[][3],
        list<POINT>&        pio_rListOfObjects,
        list<HVE2DSegment>* pio_pListOfTileBreaklines) const
    {
    int nearestPredefinedCameraOri = 0;
#ifdef BCLIB_PRESENT
    bcdtmMultiResolution_getSampleCameraOri(pi_ViewportRotMatrix, &nearestPredefinedCameraOri);
#endif
    if (nearestPredefinedCameraOri == 0)
        {
        int level = m_pRootNode->GetAppropriateConstantLevel(pi_rExtent, pi_RootToViewMatrix);

        // Sometimes there are no points in extent ... so no data to base decision upon.
        if (level < 0)
            level = 0;

        return m_pRootNode->GetInUsingViewParametersConstantLevel(pi_rExtent,
                                                                  &level,
                                                                  pi_RootToViewMatrix,
                                                                  pio_rListOfObjects,
                                                                  pio_pListOfTileBreaklines);
        }
    else
        {

#ifdef __HMR_DEBUG
        m_filter->OpenTracingXMLFile();
#endif

        size_t RetVal;
        RetVal = m_pRootNode->GetInUsingViewParameters(pi_rExtent,
                                                       nearestPredefinedCameraOri,
                                                       pi_RootToViewMatrix,
                                                       pio_rListOfObjects,
                                                       pio_pListOfTileBreaklines);

#ifdef __HMR_DEBUG
        m_filter->CloseTracingXMLFile();
#endif

        return RetVal;
        }
    }

#endif



/**----------------------------------------------------------------------------
 This method returns a list of all pointer to all spatial objects that are located in
 the index. These objects are returned somewhat ordered spatially
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                                                                                list<POINT>& resultPoints)
    {
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->OpenTracingXMLFile();
#endif

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = resultPoints.size();

    if (m_pRootNode != NULL)
        {
        // Call global pre query
        if (queryObject->GlobalPreQuery (*this, resultPoints))
            {
            try
                {
                try
                    {
                    m_pRootNode->PreQuery (queryObject);

                    m_pRootNode->Query (queryObject, resultPoints);
                    }
                catch (...)
                    {
                    m_pRootNode->PostQuery (queryObject);
                    throw;
                    }

                m_pRootNode->PostQuery (queryObject);
                }
            catch (...)
                {
                queryObject->GlobalPostQuery (*this, resultPoints);
                throw;
                }

            // Call global post query
            queryObject->GlobalPostQuery (*this, resultPoints);
            }
        }

#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->CloseTracingXMLFile();
#endif

    // Return number of newly found objects
    return(resultPoints.size()- InitialNumberOfObjects);
    }


template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                                                                                HPMMemoryManagedVector<POINT>& resultPoints)
    {
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->OpenTracingXMLFile();
#endif

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = resultPoints.size();

    if (m_pRootNode != NULL)
        {
        // Call global pre query
        if (queryObject->GlobalPreQuery (*this, resultPoints))
            {
            try
                {
                try
                    {
                    m_pRootNode->PreQuery (queryObject);

                    m_pRootNode->Query (queryObject, resultPoints);
                    }
                catch (...)
                    {
                    m_pRootNode->PostQuery (queryObject);
                    throw;
                    }

                m_pRootNode->PostQuery (queryObject);
                }
            catch (...)
                {
                queryObject->GlobalPostQuery (*this, resultPoints);
                throw;
                }

            // Call global post query
            queryObject->GlobalPostQuery (*this, resultPoints);
            }
        }

#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->CloseTracingXMLFile();
#endif

    // Return number of newly found objects
    return(resultPoints.size()- InitialNumberOfObjects);
    }


template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                                                                                BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshMesh* resultMesh)
    {
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    assert(resultMesh!= 0);

#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->OpenTracingXMLFile();
#endif

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = resultMesh->GetNbPoints();

    if (m_pRootNode != NULL)
        {
        // Call global pre query        
        if (queryObject->GlobalPreQuery (*this, resultMesh))
            {
            try
                {
                try
                    {
                    //m_pRootNode->PreQuery (queryObject);                        
                    m_pRootNode->Query (queryObject, resultMesh);
                    }
                catch (...)
                    {
                  //  m_pRootNode->PostQuery (queryObject);
                    throw;
                    }

                //m_pRootNode->PostQuery (queryObject);
                }
            catch (...)
                {
                //queryObject->GlobalPostQuery (*this, resultPoints);
                throw;
                }

            // Call global post query
           // queryObject->GlobalPostQuery (*this, resultPoints);
            }
        }

#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->CloseTracingXMLFile();
#endif

    // Return number of newly found objects
    return(resultMesh->GetNbPoints()- InitialNumberOfObjects);
    }

/*NEEDS_WORK_SM : WIP
template<class POINT, class EXTENT> class PointIndexParallelQuerier
    {
    private: 

        std::vector<SMPointIndex<POINT, EXTENT>> m_vector

    public:
        PointIndexParallelQuerier() {};
        virtual ~PointIndexParallelQuerier() {};
    
    

        RunQuery()
            {

            }


        

    }
*/


template<class POINT, class EXTENT> void SMPointIndexNode<POINT, EXTENT>::SetZRange(const double zMin, const double zMax)
    {
    double oldMin = ExtentOp<EXTENT>::GetZMin(m_nodeHeader.m_nodeExtent);
    double oldMax = ExtentOp<EXTENT>::GetZMax(m_nodeHeader.m_nodeExtent);
    ExtentOp<EXTENT>::SetZMin(m_nodeHeader.m_nodeExtent, zMin);
    ExtentOp<EXTENT>::SetZMax(m_nodeHeader.m_nodeExtent, zMax);
    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL) m_pSubNodeNoSplit->SetZRange(zMin, zMax);
        for (size_t node = 0; node < m_nodeHeader.m_numberOfSubNodesOnSplit; ++node)
            {
            if (m_apSubNodes[node] != NULL && oldMin == ExtentOp<EXTENT>::GetZMin(m_apSubNodes[node]->m_nodeHeader.m_nodeExtent)) m_apSubNodes[node]->SetZRange(zMin, ExtentOp<EXTENT>::GetZMax(m_apSubNodes[node]->m_nodeHeader.m_nodeExtent));
            if (m_apSubNodes[node] != NULL && oldMax == ExtentOp<EXTENT>::GetZMax(m_apSubNodes[node]->m_nodeHeader.m_nodeExtent)) m_apSubNodes[node]->SetZRange(ExtentOp<EXTENT>::GetZMin(m_apSubNodes[node]->m_nodeHeader.m_nodeExtent),zMax);
            }
        }
    }
/**----------------------------------------------------------------------------
PRIVATE METHOD
This method inserts a new root that contains the old root in order
to attempt inclusion of given extent. The root is pushed down by one level.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void SMPointIndex<POINT, EXTENT>::PushRootDown(const EXTENT& pi_rObjectExtent)
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
    HFCPtr<SMPointIndexNode<POINT, EXTENT>> pNewRootNode = CreateNewNode(NewRootExtent);//new SMPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), NewRootExtent, GetPool(), dynamic_cast<SMPointTileStore<POINT, EXTENT>* >(&*(GetStore())), GetFilter(), IsBalanced(), PropagatesDataDown(), &m_createdNodeMap);


    pNewRootNode->m_nodeHeader.m_arePoints3d = m_pRootNode->m_nodeHeader.m_arePoints3d;
    if (!pNewRootNode->m_nodeHeader.m_arePoints3d)  pNewRootNode->SetNumberOfSubNodesOnSplit(4);
    else  pNewRootNode->SetNumberOfSubNodesOnSplit(8);
    POINT splitPosition;
    double Width = ExtentOp<EXTENT>::GetWidth(pNewRootNode->GetNodeExtent());
    double Height = ExtentOp<EXTENT>::GetHeight(pNewRootNode->GetNodeExtent());
    double Thickness = ExtentOp<EXTENT>::GetThickness(pNewRootNode->GetNodeExtent());
    PointOp<POINT>::SetX(splitPosition,ExtentOp<EXTENT>::GetXMin(pNewRootNode->m_nodeHeader.m_nodeExtent) + (Width / 2));
    PointOp<POINT>::SetY(splitPosition,ExtentOp<EXTENT>::GetYMin(pNewRootNode->m_nodeHeader.m_nodeExtent) + (Height / 2));
    PointOp<POINT>::SetZ(splitPosition,ExtentOp<EXTENT>::GetZMin(pNewRootNode->m_nodeHeader.m_nodeExtent) + (Thickness / 2));
    // Split new rootnode
    pNewRootNode->SplitNode(splitPosition,false);
    // Replace the appropriate node by current root node
    for (size_t indexNode = 0; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
        {
        if ((m_pRootNode->m_nodeHeader.m_arePoints3d && ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(pNewRootNode->m_apSubNodes[indexNode]->GetNodeExtent(), RootExtentCenter)) ||
            (!m_pRootNode->m_nodeHeader.m_arePoints3d && ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(pNewRootNode->m_apSubNodes[indexNode]->GetNodeExtent(), RootExtentCenter)))
            {
            assert(m_pRootNode->m_nodeHeader.m_arePoints3d || indexNode < 4);

            if (!m_pRootNode->m_nodeHeader.m_arePoints3d &&
                (ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent) > ExtentOp<EXTENT>::GetZMin(pNewRootNode->m_apSubNodes[indexNode]->m_nodeHeader.m_nodeExtent)))
                {
               // m_pRootNode->MoveDownNodes(ExtentOp<EXTENT>::GetZMin(m_pRootNode->m_nodeHeader.m_nodeExtent) - ExtentOp<EXTENT>::GetZMin(pNewRootNode->m_apSubNodes[indexNode]->m_nodeHeader.m_nodeExtent));
                }

            //Propagate neighbor to old root
            for (size_t nodeInd = 0; nodeInd < IDTMFile::NeighborNodesTable::MAX_QTY; nodeInd++)
                {
                assert(m_pRootNode->m_apNeighborNodes[nodeInd].size() == 0);

                m_pRootNode->m_apNeighborNodes[nodeInd].insert(m_pRootNode->m_apNeighborNodes[nodeInd].begin(),
                                                               pNewRootNode->m_apSubNodes[indexNode]->m_apNeighborNodes[nodeInd].begin(),
                                                               pNewRootNode->m_apSubNodes[indexNode]->m_apNeighborNodes[nodeInd].end());
                }

            //Connect old root to its new neighbors
            for (size_t neighborPosInd = 0; neighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY; neighborPosInd++)
                {
                for (size_t neighborInd = 0; neighborInd < pNewRootNode->m_apSubNodes[indexNode]->m_apNeighborNodes[neighborPosInd].size(); neighborInd++)
                    {
                    HFCPtr<SMPointIndexNode<POINT, EXTENT>> otherNeighbor(pNewRootNode->m_apSubNodes[indexNode]->m_apNeighborNodes[neighborPosInd][neighborInd]);
                    bool isFound = false;

                    for (size_t otherNeighborPosInd = 0; otherNeighborPosInd < IDTMFile::NeighborNodesTable::MAX_QTY && !isFound; otherNeighborPosInd++)
                        {
                        for (size_t otherNeighborInd = 0; otherNeighborInd < otherNeighbor->m_apNeighborNodes[otherNeighborPosInd].size(); otherNeighborInd++)
                            {
                            if (otherNeighbor->m_apNeighborNodes[otherNeighborPosInd][otherNeighborInd].GetPtr() == pNewRootNode->m_apSubNodes[indexNode].GetPtr())
                                {
                                otherNeighbor->m_apNeighborNodes[otherNeighborPosInd][otherNeighborInd] = m_pRootNode;
                                if (otherNeighbor->m_nodeHeader.m_apNeighborNodeID[otherNeighborPosInd].size() <= otherNeighborInd)
                                    {
                                    otherNeighbor->m_nodeHeader.m_apNeighborNodeID[otherNeighborPosInd].resize(otherNeighborInd + 1);
                                    }

                                otherNeighbor->m_nodeHeader.m_apNeighborNodeID[otherNeighborPosInd][otherNeighborInd] = m_pRootNode->GetBlockID();
                                isFound = true;
                                break;
                                }
                            }
                        }

                    assert(isFound == true);
                    }

                pNewRootNode->m_apSubNodes[indexNode]->m_apNeighborNodes[neighborPosInd].clear();
                }
            if (!pNewRootNode->m_nodeHeader.m_arePoints3d) m_pRootNode->SetZRange(ExtentOp<EXTENT>::GetZMin(pNewRootNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(pNewRootNode->m_nodeHeader.m_nodeExtent));
            m_pRootNode->SetDirty(true);
            pNewRootNode->m_apSubNodes[indexNode]->Destroy();
            pNewRootNode->m_apSubNodes[indexNode] = m_pRootNode;
            pNewRootNode->m_apSubNodes[indexNode]->IncreaseLevel();
            pNewRootNode->m_apSubNodes[indexNode]->SetParentNode(&*pNewRootNode);
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
        m_pRootNode->PropagateSplitNode(NULL, fullDepth + 1);
        }

    HINVARIANTS;


    m_pRootNode->ValidateInvariants();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndex<POINT, EXTENT>::GetSplitTreshold() const
    {
    HINVARIANTS;

    return(m_indexHeader.m_SplitTreshold);
    }

/**----------------------------------------------------------------------------
PROTECTED
Gets the limiting outter extent (maximum extent) if the spatial index
was created with a limiting maximum extent. Prior to calling this method
It must be verified that the spatial effectively has a maximum extent using
the HasMaxExtent() method.

@return Returns the maximum extent of spatial index.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT>
EXTENT SMPointIndex<POINT, EXTENT>::GetMaxExtent() const
    {
    HPRECONDITION(HasMaxExtent());

    return m_indexHeader.m_MaxExtent;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndex<POINT, EXTENT>::GetNumberOfSubNodesOnSplit() const
    {
    HINVARIANTS;

    return m_indexHeader.m_numberOfSubNodesOnSplit;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndex<POINT, EXTENT>::Remove(const POINT& pi_rpSpatialObject)
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return false;

    return m_pRootNode->Remove(pi_rpSpatialObject);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndex<POINT, EXTENT>::GetIn(const EXTENT& pi_rExtent,
                                                                         list<POINT>& pio_rListOfObjects) const
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return 0;

    return m_pRootNode->GetIn(pi_rExtent, pio_rListOfObjects);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndex<POINT, EXTENT>::PropagateDataDownImmediately()
    {
    if (m_pRootNode != NULL)
        {
        m_pRootNode->PropagateDataDownImmediately(true);
        }
    }

    //=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndex<POINT, EXTENT>::IsBalanced() const
    {
    HINVARIANTS;

    return(m_indexHeader.m_balanced);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
bool SMPointIndex<POINT, EXTENT>::PropagatesDataDown() const
    {
    HINVARIANTS;

    return(m_propagatesDataDown);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> uint64_t SMPointIndex<POINT, EXTENT>::GetCount() const
    {
    HINVARIANTS;
    if (m_pRootNode == NULL)
        return 0;

    return m_pRootNode->GetCount();
    }

/**----------------------------------------------------------------------------
PROTECTED
Indicates if the index has a limiting outter extent (maximum extent).
In order to have a limited extent the spatial index must have been created with
a limiting maximum extent.

@return true if the spatial index has a maximum extent.
-----------------------------------------------------------------------------*/
template< class POINT, class EXTENT>
bool SMPointIndex<POINT, EXTENT>::HasMaxExtent() const
    {
    return m_indexHeader.m_HasMaxExtent;
    }
//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
size_t SMPointIndex<POINT, EXTENT>::GetDepth() const
    {

    if (m_pRootNode == NULL)
        return 0;
    if (m_indexHeader.m_depth == 0)
        {
        const_cast<SMPointIndex<POINT, EXTENT>*>(this)->m_indexHeader.m_depth = m_pRootNode->GetDepth();
        const_cast<SMPointIndex<POINT, EXTENT>*>(this)->m_indexHeaderDirty = true;
        const_cast<SMPointIndex<POINT, EXTENT>*>(this)->m_store->StoreMasterHeader(&const_cast<SMPointIndex<POINT, EXTENT>*>(this)->m_indexHeader, sizeof(m_indexHeader));
        }
    return m_indexHeader.m_depth;
    }
/**----------------------------------------------------------------------------
PROTECTED
Returns the root node of the spatial index.

@return Root node.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT>
HFCPtr<SMPointIndexNode<POINT, EXTENT> >   SMPointIndex<POINT, EXTENT>::GetRootNode() const
    {
    return m_pRootNode;
    }

/**----------------------------------------------------------------------------
PROTECTED
Returns the root node of the spatial index.

@return Root node.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT>
HFCPtr<SMPointIndexNode<POINT, EXTENT> >    SMPointIndex<POINT, EXTENT>::CreateRootNode() 
    {
    assert(m_pRootNode == 0);

    // Check if initial node allocated
    
    // There is no root node at the moment
    // Allocate root node the size of the object extent
    if (m_indexHeader.m_HasMaxExtent)
        m_pRootNode = CreateNewNode(m_indexHeader.m_MaxExtent); 
    else
        {        
        m_pRootNode = CreateNewNode(ExtentOp<EXTENT>::Create(0, 0, 0, 0, 0, 0)); 
        }    
    
    return m_pRootNode;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndexNode<POINT, EXTENT>::AdviseDelayedSplitRequested() const
    {
    HASSERT(m_pSubNodeNoSplit != NULL);

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
template<class POINT, class EXTENT>
bool SMPointIndexNode<POINT, EXTENT>::Remove(const POINT& pi_rpSpatialObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    bool Removed = false;

    // Check if node is still a leaf ...
    if (HasRealChildren())
        {

        if (m_pSubNodeNoSplit != NULL)
            {
            Removed = m_pSubNodeNoSplit->Remove(pi_rpSpatialObject);
            }
        else
            {
            for (size_t i = 0; i < GetNumberOfSubNodesOnSplit() && !Removed; ++i)
                {
                // Check if object is contained in this sub-node
                if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn3D(m_apSubNodes[i]->GetNodeExtent(), pi_rpSpatialObject))
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

        for (; !Removed && itr != this->end(); itr++)
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
        m_nodeHeader.m_totalCount--;
        SetDirty(true);
        }

    HINVARIANTS;


    return(Removed);
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::IsEmpty() const
    {
    HINVARIANTS;
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        const_cast<SMPointIndex<POINT, EXTENT>*>(this)->m_pRootNode = const_cast<SMPointIndex<POINT, EXTENT>*>(this)->CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    if (m_pRootNode == NULL)
        return true;


    return m_pRootNode->IsEmpty();
    }


/**----------------------------------------------------------------------------
PROTECTED
Gets the effective limiting outter extent.

@return Returns the extent of spatial index.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT>
EXTENT SMPointIndex<POINT, EXTENT>::GetIndexExtent() const
    {
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        const_cast<SMPointIndex<POINT, EXTENT>*>(this)->m_pRootNode = const_cast<SMPointIndex<POINT, EXTENT>*>(this)->CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
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
template<class POINT, class EXTENT>
EXTENT SMPointIndex<POINT, EXTENT>::GetContentExtent() const
    {
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        const_cast<SMPointIndex<POINT, EXTENT>*>(this)->m_pRootNode = const_cast<SMPointIndex<POINT, EXTENT>*>(this)->CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    if (m_pRootNode == NULL)
        return EXTENT();

    return m_pRootNode->GetContentExtent();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT>
void SMPointIndex<POINT, EXTENT>::Balance()
    {
    HINVARIANTS;

    //if (!m_indexHeader.m_balanced)
       // {
        //m_indexHeader.m_balanced = true;

        m_indexHeaderDirty = true;
        m_isGenerating = false;

        if (m_pRootNode != NULL)
            {

            size_t depth = m_pRootNode->GetDepth();
            m_pRootNode->Balance(depth);
            }
       // }

    HINVARIANTS;
    }


template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::Query (ISMPointIndexQuery<POINT, EXTENT>* queryObject,
                                                                                vector<typename SMPointIndexNode<POINT, EXTENT>::QueriedNode>& meshNodes)
    {    
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->OpenTracingXMLFile();
#endif

    // Save the number of objects currently in list
    //NEEDS_WORK_SM
    //size_t InitialNumberOfObjects = resultMesh->GetNbPoints();

    if (m_pRootNode != NULL)
        {
        // Call global pre query        
        if (queryObject->GlobalPreQuery (*this, meshNodes))
            {
            try
                {
                try
                    {
                    //m_pRootNode->PreQuery (queryObject);                        
                    m_pRootNode->Query (queryObject, meshNodes);
                    }
                catch (...)
                    {
                  //  m_pRootNode->PostQuery (queryObject);
                    throw;
                    }

                //m_pRootNode->PostQuery (queryObject);
                }
            catch (...)
                {
                //queryObject->GlobalPostQuery (*this, resultPoints);
                throw;
                }

            // Call global post query
           // queryObject->GlobalPostQuery (*this, resultPoints);
            }
        }

#ifdef ACTIVATE_NODE_QUERY_TRACING
    queryObject->CloseTracingXMLFile();
#endif

    // Return number of newly found objects
                //NEEDS_WORK_SM
    return meshNodes.size(); //(resultMesh->GetNbPoints()- InitialNumberOfObjects);
    }

template<class POINT, class EXTENT> size_t SMPointIndex<POINT, EXTENT>::Query(ISMPointIndexQuery<POINT, EXTENT>* queryObject, HFCPtr<SMPointIndexNode<POINT, EXTENT>>& resultNode)
    {
    //we wait until the first query, add, etc, to initialize root node. We do this because CreateNewNode is virtual and we need to call the correct implementation which is not possible while constructing the index.
    if (m_indexHeader.m_rootNodeBlockID.IsValid() && m_pRootNode == nullptr)
        {
        m_pRootNode = CreateNewNode(m_indexHeader.m_rootNodeBlockID);
        }
    if (m_pRootNode != NULL)
        {
        m_pRootNode->Query(queryObject, resultNode);

        if (resultNode != NULL)
            {
            return resultNode->size();
            }
        }

    return 0;
    }

/**----------------------------------------------------------------------------
 This method remove a list of points.

 @param pi_extentToClear IN The extent to clear.

 @return true if something was clear and false otherwise. 
 
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool SMPointIndex<POINT, EXTENT>::RemovePoints(const EXTENT& pi_extentToClear)
    {
    HINVARIANTS;
          
    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        return false; 
        }

    size_t nbPoints = m_pRootNode->RemovePoints(pi_extentToClear);
        
    HINVARIANTS;
    return (nbPoints != 0);
    }








