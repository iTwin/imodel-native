//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGFPointIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/all/h/HPMPooledVector.h>


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// HGFPointIndexNode Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> HGFPointIndexNode<POINT, EXTENT>::HGFPointIndexNode(size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        HFCPtr<HPMCountLimitedPool<POINT> > pool,
        HFCPtr<HGFPointTileStore<POINT, EXTENT> > store,
        IHGFPointIndexFilter<POINT, EXTENT>* filter,
        bool balanced,
        bool propagateDataDown)
    : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, balanced, propagateDataDown, false, false)
    {

    SetStore (store);
    SetPool (pool);
    m_NbObjects = -1;

    m_filter = filter;
    m_nodeHeader.m_filtered = false;
    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;

    for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = HPMBlockID();
        }


    this->ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> HGFPointIndexNode<POINT, EXTENT>::HGFPointIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const HFCPtr<HGFPointIndexNode<POINT, EXTENT> >& pi_rpParentNode)
    : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, &*pi_rpParentNode)

    {
    SetStore (static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*pi_rpParentNode->GetStore()));
    SetPool (pi_rpParentNode->GetPool());

    m_filter = pi_rpParentNode->GetFilter();
    m_nodeHeader.m_filtered = false;
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
template<class POINT, class EXTENT> HGFPointIndexNode<POINT, EXTENT>::HGFPointIndexNode (size_t pi_SplitTreshold,
        const EXTENT& pi_rExtent,
        const HFCPtr<HGFPointIndexNode<POINT, EXTENT> >& pi_rpParentNode,
        bool IsUnsplitSubLevel)
    : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, &*pi_rpParentNode, IsUnsplitSubLevel)
    {

    m_NbObjects = -1;
    SetStore (static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*pi_rpParentNode->GetStore()));
    SetPool (pi_rpParentNode->GetPool());

    m_filter = pi_rpParentNode->GetFilter();
    m_nodeHeader.m_filtered = false;

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
template<class POINT, class EXTENT> HGFPointIndexNode<POINT, EXTENT>::HGFPointIndexNode (const HGFPointIndexNode<POINT, EXTENT>& pi_rNode)
    : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >(&*pi_rNode)
    {
    // A copied node may not have a parent
    HPRECONDITION(pi_rNode.m_pParentNode == 0);

    SetStore (static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*pi_rNode->GetStore()));
    SetPool (pi_rNode->GetPool());
    m_NbObjects = -1;

    m_filter = pi_rNode->GetFilter();
    m_nodeHeader.m_filtered = false;

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
template<class POINT, class EXTENT> HGFPointIndexNode<POINT, EXTENT>::HGFPointIndexNode(const HGFPointIndexNode& pi_rNode,
        const HFCPtr<HGFPointIndexNode>& pi_rpParentNode)
    : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >(&*pi_rNode, &*pi_rpParentNode)
    {
    SetStore (static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*pi_rNode->GetStore()));
    SetPool (pi_rNode->GetPool());

    m_filter = pi_rpParentNode->GetFilter();
    m_nodeHeader.m_filtered = false;
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
template<class POINT, class EXTENT> HGFPointIndexNode<POINT, EXTENT>::HGFPointIndexNode(HPMBlockID blockID,
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > parent,
        HFCPtr<HPMCountLimitedPool<POINT> > pool,
        HFCPtr<HGFPointTileStore<POINT, EXTENT> > store,
        IHGFPointIndexFilter<POINT, EXTENT>* filter,
        bool balanced,
        bool propagateDataDown)
    : HGFIndexNode<POINT, POINT, EXTENT, HPMStoredPooledVector<POINT>, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >, HGFPointNodeHeader<EXTENT> >(propagateDataDown)
    {
    HPRECONDITION (blockID.IsValid());

    SetStore (store);
    SetPool (pool);

    m_nodeHeader.m_ViewDependentMetrics[0] = 1.0;

    m_loaded = false;
    m_NbObjects = -1;
    m_filter = filter;

    m_storeBlockID = blockID;
    m_discarded = true;
    m_pParentNode = parent;
    m_nodeHeader.m_balanced = balanced;
    HPMCountLimitedPoolItem::SetDirty (false);

    this->ValidateInvariantsSoft();
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> HGFPointIndexNode<POINT, EXTENT>::~HGFPointIndexNode()
    {          
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
template<class POINT, class EXTENT> HFCPtr<HGFPointIndexNode<POINT, EXTENT> > HGFPointIndexNode<POINT, EXTENT>::Clone () const
    {
    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pNewNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), GetNodeExtent(), GetPool(), dynamic_cast<HGFPointTileStore<POINT, EXTENT>* >(&*(GetStore())), GetFilter(), IsBalanced(), PropagatesDataDown());
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<HGFPointIndexNode<POINT, EXTENT> > HGFPointIndexNode<POINT, EXTENT>::Clone (const EXTENT& newNodeExtent) const
    {
    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pNewNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, GetPool(), dynamic_cast<HGFPointTileStore<POINT, EXTENT>* >(&*(GetStore())), GetFilter(), IsBalanced(), PropagatesDataDown());
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<HGFPointIndexNode<POINT, EXTENT> > HGFPointIndexNode<POINT, EXTENT>::CloneChild (const EXTENT& newNodeExtent) const
    {
    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pNewNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<HGFPointIndexNode<POINT, EXTENT>*>(this));
    return pNewNode;
    }
template<class POINT, class EXTENT> HFCPtr<HGFPointIndexNode<POINT, EXTENT> > HGFPointIndexNode<POINT, EXTENT>::CloneUnsplitChild (const EXTENT& newNodeExtent) const
    {
    HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pNewNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<HGFPointIndexNode<POINT, EXTENT>*>(this), true);
    return pNewNode;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::Load() const
    {
    HPRECONDITION (!IsLoaded());

    if (0 == static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*m_store)->LoadHeader (&m_nodeHeader, m_storeBlockID))
        {
        // Something went wrong
        throw HFCUnknownException();
        }

    HGFPointIndexNode<POINT, EXTENT>* UNCONSTTHIS =  const_cast<HGFPointIndexNode<POINT, EXTENT>* >(this);

    UNCONSTTHIS->m_count = UNCONSTTHIS->m_store->GetBlockDataCount (m_storeBlockID);
    HDEBUGCODE(UNCONSTTHIS->m_countLoaded = true;);

    // If there are sub-nodes we must create them
    if (!UNCONSTTHIS->m_nodeHeader.m_IsLeaf)
        {
        if (!UNCONSTTHIS->m_nodeHeader.m_IsBranched)
            {
            UNCONSTTHIS->m_pSubNodeNoSplit = new HGFPointIndexNode<POINT, EXTENT> (UNCONSTTHIS->m_nodeHeader.m_SubNodeNoSplitID, UNCONSTTHIS, UNCONSTTHIS->m_pool, static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)), UNCONSTTHIS->m_filter, UNCONSTTHIS->m_nodeHeader.m_balanced, !(UNCONSTTHIS->m_delayedDataPropagation));
            UNCONSTTHIS->m_pSubNodeNoSplit->m_nodeHeader.m_IsUnSplitSubLevel = true;
            }
        else
            {

            // ATTENTION! DO NOT CALL GetNumberOfSubNodesOnSplit() FUNCTION AS IT WILL CALL Load() RESULTING
            // INTO AN INFINITE LOOP
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                UNCONSTTHIS->m_apSubNodes[indexNode] = new HGFPointIndexNode<POINT, EXTENT>(UNCONSTTHIS->m_nodeHeader.m_apSubNodeID[indexNode], UNCONSTTHIS, UNCONSTTHIS->m_pool, static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*(UNCONSTTHIS->m_store)), UNCONSTTHIS->m_filter, UNCONSTTHIS->m_nodeHeader.m_balanced, !(UNCONSTTHIS->m_delayedDataPropagation));
                UNCONSTTHIS->m_apSubNodes[indexNode]->m_nodeHeader.m_IsUnSplitSubLevel = false;
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
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::Unload() const
    {
    if (IsLoaded())
        {
        HGFPointIndexNode<POINT, EXTENT>* UNCONSTTHIS =  const_cast<HGFPointIndexNode<POINT, EXTENT>* >(this);

        // If there are sub-nodes we must create them
        if (!UNCONSTTHIS->m_nodeHeader.m_IsLeaf)
            {
            if (UNCONSTTHIS->m_pSubNodeNoSplit != NULL)
                {
                UNCONSTTHIS->m_nodeHeader.m_IsBranched = false;

                UNCONSTTHIS->m_pSubNodeNoSplit->Unload();
                }
            else
                {
                UNCONSTTHIS->m_nodeHeader.m_IsBranched = true;

                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    UNCONSTTHIS->m_apSubNodes[indexNode]->Unload();
                    }
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
                UNCONSTTHIS->m_pSubNodeNoSplit->m_pParentNode = NULL;

                UNCONSTTHIS->m_pSubNodeNoSplit = NULL;
                }
            else
                {
                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    UNCONSTTHIS->m_apSubNodes[indexNode]->m_pParentNode = NULL;
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
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::Destroy()
    {
    this->ValidateInvariantsSoft();

    HGFIndexNode::Destroy();
    if (GetBlockID().IsValid())
        m_store->DestroyBlock(GetBlockID());

    HINVARIANTS;

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> double* HGFPointIndexNode<POINT, EXTENT>::GetViewDependentMetrics()
    {
    if (!IsLoaded())
        Load();

    return m_nodeHeader.m_ViewDependentMetrics;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::ChangeStore(HFCPtr<HGFPointTileStore<POINT, EXTENT> > newStore)
    {
    HINVARIANTS;

    // Call node
    if (!IsLeaf())
        {
        if (static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit) != NULL)
            static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->ChangeStore(newStore);
        else
            {
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                if (m_apSubNodes[indexNode] != NULL)
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->ChangeStore(newStore);
                }
            }
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
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::Store() const
    {
    HINVARIANTS;

    if (!IsLoaded())
        return true;

    if (!IsLeaf())
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Store();
            HASSERT (m_nodeHeader.m_SubNodeNoSplitID.IsValid());
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                if (m_apSubNodes[indexNode] != NULL)
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Store();
                HASSERT (m_nodeHeader.m_apSubNodeID[indexNode].IsValid());
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
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::SetDirty (bool dirty) const // Intentionaly const ... only mutable field changed
    {
    // In theory the two next variable invalidations are not required as
    // but just in case the HGFIndexNode
    m_NbObjects = -1;
    HPMCountLimitedPoolItem<POINT>::SetDirty (dirty);
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::PropagateDataDownImmediately(bool propagateRecursively)
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
            POINT* spatialArray = new POINT[numberSpatial];
            this->get(spatialArray, numberSpatial);
            this->clear();

            // We copy the whole content to this sub-node
            m_pSubNodeNoSplit->AddArrayUnconditional (spatialArray, numberSpatial);

            delete spatialArray;
            }
        else
            {

            size_t numberSpatial = this->size();
            POINT* INSpatialArray = new POINT[numberSpatial];
            this->get(INSpatialArray, numberSpatial);

            POINT* spatialArray[8];
            size_t spatialArrayNumber[8];
            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                {
                spatialArray[indexNodes] = new POINT[numberSpatial];
                spatialArrayNumber[indexNodes] = 0;
                }

            for (size_t indexSpatial = 0; indexSpatial < numberSpatial ; indexSpatial++)
                {
                bool addedToNode = false;
                for (size_t indexNodes = 0; !addedToNode && indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                    {
                    if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D (INSpatialArray[indexSpatial], m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent))
                        {
                        spatialArray[indexNodes][spatialArrayNumber[indexNodes]] = INSpatialArray[indexSpatial];
                        spatialArrayNumber[indexNodes]++;
                        addedToNode = true;
                        }
                    }

                }
            this->clear();

            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                {
                if (spatialArrayNumber[indexNodes] > 0)
                    m_apSubNodes[indexNodes]->AddArrayUnconditional(spatialArray[indexNodes], spatialArrayNumber[indexNodes]);
                }



            for (size_t indexNodes = 0; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                delete [] spatialArray[indexNodes];

            delete [] INSpatialArray;

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

    m_nodeHeader.m_filtered = false;

    HINVARIANTS;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::Discard() const // Intentionaly const ... only mutable members are modified
    {
    HINVARIANTS;
    bool returnValue = true;

    if (!m_destroyed)
        {
        if (!IsLoaded())
            Load();

        // Save the current blockID
        HPMBlockID currentBlockId = GetBlockID();

        // Call ancester Discard
        returnValue = HPMStoredPooledVector<POINT>::Discard();

        if (returnValue)
            {
            static_cast<HGFPointTileStore<POINT, EXTENT>*>(&*m_store)->StoreHeader(&m_nodeHeader, GetBlockID()); // TODO: Why do we store the header when not dirty??
            }

        if (returnValue && (currentBlockId != GetBlockID()))
            {
            HASSERT(GetBlockID().IsValid());

            // Block ID changed (tile reallocated on store ... advise parent)
            if (GetParentNode() != NULL)
                GetParentNode()->AdviseSubNodeIDChanged(const_cast<HGFPointIndexNode<POINT, EXTENT>*>(this));
            }
        }
    HINVARIANTS;

    return returnValue;

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::AdviseSubNodeIDChanged(const HFCPtr<HGFPointIndexNode<POINT, EXTENT> >& p_subNode)
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
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::SetSubNodes(HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pi_apSubNodes[], size_t numSubNodes)
    {
    HINVARIANTS;

    HGFIndexNode::SetSubNodes(pi_apSubNodes, numSubNodes);


    size_t indexNode;
    for (indexNode = 0 ; indexNode < numSubNodes; indexNode++)
        {
        m_nodeHeader.m_apSubNodeID[indexNode] = m_apSubNodes[indexNode]->GetBlockID();
        }
//    SetDirty(true);
    HINVARIANTS;

    }




//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> IHGFPointIndexFilter<POINT, EXTENT>* HGFPointIndexNode<POINT, EXTENT>::GetFilter() const
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
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::AddConditional (const POINT pi_rpSpatialObject, bool ExtentFixed)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();

    // Check is spatial extent is in node ...
    if (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(m_nodeHeader.m_nodeExtent, pi_rpSpatialObject))
        {
        return Add (pi_rpSpatialObject);
        }
    // The spatial object is not in extent ... check if we can increase extent (not extent fixed, no parent and no sub-nodes)
    else if (!ExtentFixed && GetParentNode() == NULL && m_nodeHeader.m_IsLeaf)
        {
        // We can increase the extent ... do it
        m_nodeHeader.m_nodeExtent = ExtentOp<EXTENT>::MergeExtents(m_nodeHeader.m_nodeExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(pi_rpSpatialObject));

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
template<class POINT, class EXTENT> size_t HGFPointIndexNode<POINT, EXTENT>::AddArrayConditional (const POINT* pointsArray, size_t startPointIndex, size_t countPoints, bool ExtentFixed)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();

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
        if (ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent) < ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent))
            {
            ExtentOp<EXTENT>::SetXMax(m_nodeHeader.m_nodeExtent, ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetHeight(m_nodeHeader.m_nodeExtent));
            }
        else
            {
            ExtentOp<EXTENT>::SetYMax(m_nodeHeader.m_nodeExtent, ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent) + ExtentOp<EXTENT>::GetWidth(m_nodeHeader.m_nodeExtent));
            }

        AddArrayUnconditional (&(pointsArray[startPointIndex]), endPointIndex);


        if (endPointIndex < countPoints)
            return AddArrayConditional(pointsArray, endPointIndex, countPoints, true);
        else
            return countPoints;

        }
    else
        {
        // Extent is limited ...
        size_t lastPointsIndexInExtent = startPointIndex;
        while ((lastPointsIndexInExtent < countPoints) && (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(pointsArray[lastPointsIndexInExtent], m_nodeHeader.m_nodeExtent)))
            {
            lastPointsIndexInExtent++;
            }

        if (lastPointsIndexInExtent > startPointIndex)
            {
            AddArrayUnconditional (&(pointsArray[startPointIndex]), lastPointsIndexInExtent - startPointIndex);
            }
        HINVARIANTS;

        return lastPointsIndexInExtent;
        }

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::AddArrayUnconditional(const POINT* pointsArray, size_t countPoints)
    {
    HINVARIANTS;
    HASSERT (countPoints > 0);

    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();



    // All points must be fully contained in node extent

    // Check if the threshold amount of objects is attained
    if ((m_nodeHeader.m_IsLeaf || m_nodeHeader.m_IsUnSplitSubLevel) && (this->size() + countPoints >= m_nodeHeader.m_SplitTreshold))
        {
        // There are too much objects ... need to split current node
        SplitNode();
        }
    else if (m_delayedDataPropagation && (this->size() + countPoints >= m_nodeHeader.m_SplitTreshold))
        {
        PropagateDataDownImmediately(false);
        }

    // The total count increases by countSpatial whatever the path selected below
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


    // Check if node is still a leaf ...
    if (m_nodeHeader.m_IsLeaf || (m_delayedDataPropagation && (this->size() + countPoints < m_nodeHeader.m_SplitTreshold)))
        {
        HDEBUGCODE(size_t initialSize = this->size());
        HDEBUGCODE(bool wasDiscarded = this->Discarded(); wasDiscarded;);

        if (this->size() + countPoints >= this->capacity())
            this->reserve (this->size() + countPoints + m_nodeHeader.m_SplitTreshold / 10);

        HDEBUGCODE(HASSERT(initialSize == this->size()));

        // It is a leaf ... we add reference in list
        this->push_back(pointsArray, countPoints);
        }
    else
        {
        if (m_pSubNodeNoSplit != NULL)
            m_pSubNodeNoSplit->AddArrayUnconditional (pointsArray, countPoints);
        else
            {
            size_t startIndex = 0;
            while (startIndex < countPoints)
                {
                HDEBUGCODE (size_t previousCount = startIndex);
                for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    startIndex = m_apSubNodes[indexNode]->AddArrayConditional(pointsArray, startIndex, countPoints, true);
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
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::Add(const POINT pi_rpSpatialObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    if (m_DelayedSplitRequested)
        SplitNode();

    // The total count increases by 1
    m_nodeHeader.m_totalCount += 1;

    // The object must be fully contained in node extent
    HPRECONDITION((SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(pi_rpSpatialObject, m_nodeHeader.m_nodeExtent)));

    // Check if the threshold amount of objects is attained
    if ((m_nodeHeader.m_IsLeaf || m_nodeHeader.m_IsUnSplitSubLevel) && (this->size() + 1 >= m_nodeHeader.m_SplitTreshold))
        {
        // There are too much objects ... need to split current node
        SplitNode();
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
    if (m_nodeHeader.m_IsLeaf || m_delayedDataPropagation)
        {
        if (this->size() + 1 >= this->capacity())
            this->reserve (this->size() + m_nodeHeader.m_SplitTreshold / 10);


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
                if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(pi_rpSpatialObject, m_apSubNodes[i]->GetNodeExtent()))
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
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::clear()
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
        GetParentNode()->AdviseSubNodeIDChanged(const_cast<HGFPointIndexNode<POINT, EXTENT>*>(this));

    this->ValidateInvariantsSoft();

    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> size_t HGFPointIndexNode<POINT, EXTENT>::Clear(HFCPtr<HVEShape> pi_shapeToClear)
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
                                                   0.0,
                                                   shapeExtent.GetXMax(),
                                                   shapeExtent.GetYMax(),
                                                   0.0);


    // Check if node is a leaf ...
    if (!m_nodeHeader.m_IsLeaf)
        {

        if (m_pSubNodeNoSplit != NULL)
            {
            removedCount += m_pSubNodeNoSplit->Clear (pi_shapeToClear);
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
                    removedCount += m_apSubNodes[i]->Clear(pi_shapeToClear);
                    }
                }
            }
        }

    for (size_t currentIndex = 0 ; currentIndex < this->size(); currentIndex++)
        {
        // Check if current object is in shape

        if (pi_shapeToClear->GetShapePtr()->CalculateSpatialPositionOf(HGF2DLocation (PointOp<POINT>::GetX(this->operator[](currentIndex)),
                                                                                      PointOp<POINT>::GetY(this->operator[](currentIndex)),
                                                                                      pi_shapeToClear->GetCoordSys())) == HVE2DShape::S_IN)
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
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::PreQuery (IHGFPointIndexQuery<POINT, EXTENT>* queryObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->PreQuery (this, pSubNodes, 1);

            if (digDown)
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PreQuery (queryObject);
            }
        else
            {
            HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[8];
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);
            digDown = queryObject->PreQuery (this, subNodes, GetNumberOfSubNodesOnSplit());

            if (digDown)
                {
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->PreQuery(queryObject);
                    }
                }

            }
        }
    else
        {
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->PreQuery (this, pSubNodes, 0);
        }

    HINVARIANTS;

    return digDown;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject, list<POINT>& resultPoints)
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
            HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->Query (this, pSubNodes, 1, resultPoints);

            if (digDown)
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query (queryObject, resultPoints);
            }
        else
            {
            HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[8];
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

            digDown = queryObject->Query (this, subNodes, GetNumberOfSubNodesOnSplit(), resultPoints);

            if (digDown)
                {
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->Query(queryObject, resultPoints);
                    }
                }

            }
        }
    else
        {
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->Query (this, pSubNodes, 0, resultPoints);
        }


    HINVARIANTS;

    return digDown;
    }
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject, HPMMemoryManagedVector<POINT>& resultPoints)
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
            HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->Query (this, pSubNodes, 1, resultPoints);

            if (digDown)
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Query (queryObject, resultPoints);
            }
        else
            {
            HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[8];
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

            digDown = queryObject->Query (this, subNodes, GetNumberOfSubNodesOnSplit(), resultPoints);

            if (digDown)
                {
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->Query(queryObject, resultPoints);
                    }
                }

            }
        }
    else
        {
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->Query (this, pSubNodes, 0, resultPoints);
        }


    HINVARIANTS;

    return digDown;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::PostQuery (IHGFPointIndexQuery<POINT, EXTENT>* queryObject)
    {
    HINVARIANTS;
    if (!IsLoaded())
        Load();

    bool digDown = true;

    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
            HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);
            digDown = queryObject->PostQuery (this, pSubNodes, 1);

            if (digDown)
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PostQuery (queryObject);
            }
        else
            {
            HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[8];
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);
            digDown = queryObject->PostQuery (this, subNodes, GetNumberOfSubNodesOnSplit());

            if (digDown)
                {
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit() ; indexNodes++)
                    {
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNodes]))->PostQuery(queryObject);
                    }
                }

            }
        }
    else
        {
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        queryObject->PostQuery (this, pSubNodes, 0);
        }

    HINVARIANTS;

    return digDown;
    }



/**----------------------------------------------------------------------------
 This method returns the number of points corresponding to a particular node.

 @param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> uint32_t HGFPointIndexNode<POINT, EXTENT>::GetNbObjects() const
    {
    if (m_NbObjects == -1 || IsDirty())
        {
        uint32_t NbObjects;

        NbObjects = (uint32_t)size();

        //Compute the
        if (((m_filter == NULL) ||(m_filter->IsProgressiveFilter() == true)) && (m_pParentNode != 0))
            {
            static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNumberObjectsInAncestors(m_nodeHeader.m_nodeExtent, NbObjects);
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
template<class POINT, class EXTENT> uint64_t HGFPointIndexNode<POINT, EXTENT>::GetNbObjectsAtLevel(size_t pi_depthLevel) const
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
template<class POINT, class EXTENT> size_t HGFPointIndexNode<POINT, EXTENT>::GetNodeObjects(list<POINT>& pio_rListOfObjects) const
    {
    for (uint32_t PtInd = 0; PtInd < size(); PtInd++)
        {
        pio_rListOfObjects.push_back(this->operator[](PtInd));
        }

    if ((m_filter->IsProgressiveFilter() == true) && (m_pParentNode != 0))
        {
        static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNodeObjectsInAncestors(m_nodeHeader.m_nodeExtent, pio_rListOfObjects);
        }

    return pio_rListOfObjects.size();
    }
#ifdef __HMR_DEBUG

/**----------------------------------------------------------------------------
 This method returns the number of points corresponding to a particular node.

 @param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::DumpQuadTreeNode(FILE* pi_pOutputXmlFileStream,
        bool pi_OnlyLoadedNode) const
    {
    if ((pi_OnlyLoadedNode == true) && (IsLoaded() == false))
        return;

    if (!IsLoaded())
        Load();

    char   TempBuffer[3000];
    int    NbChars;
    size_t NbWrittenChars;

    NbChars = sprintf(TempBuffer, "<ChildNode TotalPoints=\"%i\" SplitDepth=\"%i\">",  GetCount(), GetSplitDepth());

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);

    //Extent
    NbChars = sprintf(TempBuffer,
                      "<NodeExtent><MinX>%.3f</MinX><MaxX>%.3f</MaxX><MinY>%.3f</MinY><MaxY>%.3f</MaxY></NodeExtent>\n",
                      ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent),
                      ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    if (m_nodeHeader.m_contentExtentDefined)
        {
        NbChars = sprintf(TempBuffer,
                          "<ContentExtent><MinX>%.3f</MinX><MaxX>%.3f</MaxX><MinY>%.3f</MinY><MaxY>%.3f</MaxY></ContentExtent>\n",
                          ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent),
                          ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent));

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
    NbChars = sprintf(TempBuffer,
                      "<ViewDependentMetrics>%.3f</ViewDependentMetrics>",
                      m_nodeHeader.m_ViewDependentMetrics[0]);

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbWrittenChars == NbChars);

    if (!m_nodeHeader.m_IsLeaf)
        {

        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_pSubNodeNoSplit))->DumpQuadTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->DumpQuadTreeNode(pi_pOutputXmlFileStream, pi_OnlyLoadedNode);
                }
            }

        }
    NbChars = sprintf(TempBuffer, "</ChildNode>\n");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pi_pOutputXmlFileStream);

    HASSERT(NbChars == NbWrittenChars);


    }

#endif

/**----------------------------------------------------------------------------
 This method return the number of points corresponding to a particular node.

 @param Current number of points for the node.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::AddNumberObjectsInAncestors(EXTENT& pi_rChildNodeExtent,
        uint32_t& pio_rNbPoints) const
    {
    HPRECONDITION((m_filter == NULL) || (m_filter->IsProgressiveFilter() == true));

    for (uint32_t PtInd = 0; PtInd < size(); PtInd++)
        {
        if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(this->operator[](PtInd), pi_rChildNodeExtent))
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

    if (m_pParentNode != 0)
        {
        static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNumberObjectsInAncestors(pi_rChildNodeExtent, pio_rNbPoints);
        }
    }

/**----------------------------------------------------------------------------
 This method return the objects that are

 @param Current number of points for the node.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::AddNodeObjectsInAncestors(const EXTENT& pi_rChildNodeExtent,
        list<POINT>&  pio_rListOfObjects) const
    {
    HPRECONDITION(m_filter->IsProgressiveFilter() == true);

    for (uint32_t PtInd = 0; PtInd < size(); PtInd++)
        {
        if (SpatialOp<POINT, POINT, EXTENT>::IsSpatialInExtent2D(this->operator[](PtInd), pi_rChildNodeExtent))
            {
            pio_rListOfObjects.push_back(this->operator[](PtInd));
            }
        }

    if (m_pParentNode != 0)
        {
        static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*GetParentNode())->AddNodeObjectsInAncestors(pi_rChildNodeExtent, pio_rListOfObjects);
        }
    }






//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::PreFilter()
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
            if (static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsFiltering())
                digUp = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PreFilter();

            if (digUp)
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
                pSubNodes[0] = static_cast<HGFPointIndexNode<POINT, EXTENT> *>(&*m_pSubNodeNoSplit);

                digUp = m_filter->PreFilter (this, pSubNodes, 1);
                }
            }
        else
            {
            digUp = false;
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
                if (static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsFiltering())
                    digUp = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->PreFilter() || digUp;
                }


            if (digUp)
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[8];
                for (size_t indexNodes = 0 ; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNodes++)
                    subNodes[indexNodes] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);

                digUp = m_filter->PreFilter (this, subNodes, m_nodeHeader.m_numberOfSubNodesOnSplit);
                }
            }

        }
    else
        {
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        digUp = m_filter->PreFilter (this, pSubNodes, 0);
        }

    return digUp;

    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::Filter()
    {
    if (!IsLoaded())
        Load();

    HINVARIANTS;

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (!m_nodeHeader.m_IsLeaf)
        {
        if (m_pSubNodeNoSplit != NULL)
            {
#ifdef __HMR_DEBUG
            if ((m_pSubNodeNoSplit->m_unspliteable) || (m_pSubNodeNoSplit->m_parentOfAnUnspliteableNode))
                this->m_parentOfAnUnspliteableNode = true;
#endif

            if (static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsFiltering())
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->Filter();

            if (m_pSubNodeNoSplit->size() > 0)
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
                pSubNodes[0] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);

                m_filter->Filter (this, pSubNodes, 1, m_nodeHeader.m_ViewDependentMetrics);
                SetDirty (true);

                if (m_filter->IsProgressiveFilter())
                    {
                    m_pSubNodeNoSplit->SetDirty(true);
                    }
                }

            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                {
#ifdef __HMR_DEBUG
                if ((m_apSubNodes[indexNode]->m_unspliteable) || (m_apSubNodes[indexNode]->m_parentOfAnUnspliteableNode))
                    this->m_parentOfAnUnspliteableNode = true;
#endif
                if (static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsFiltering())
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->Filter();
                }


            HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[8];
            for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                subNodes[indexNodes] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);


            m_filter->Filter (this, subNodes, m_nodeHeader.m_numberOfSubNodesOnSplit, m_nodeHeader.m_ViewDependentMetrics);
            if (m_filter->IsProgressiveFilter())
                {
                for (size_t indexNode = 0 ; indexNode < m_nodeHeader.m_numberOfSubNodesOnSplit; indexNode++)
                    {
                    m_apSubNodes[indexNode]->SetDirty(true);
                    }
                }

            SetDirty (true);
            }

        }
    else
        {
        if (m_filter->FilterLeaf (this, m_nodeHeader.m_ViewDependentMetrics))
            SetDirty (true);
        }


    m_nodeHeader.m_filtered = true;

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
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::PostFilter()
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
            digUp = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->PostFilter();

            HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
            pSubNodes[0] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit);

            if (digUp)
                digUp = m_filter->PostFilter (this, pSubNodes, 1);
            }
        else
            {
            digUp = false;
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                digUp = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->PostFilter() || digUp;
                }


            if (digUp)
                {
                HFCPtr<HGFPointIndexNode<POINT, EXTENT>> subNodes[8];
                for (size_t indexNodes = 0 ; indexNodes < GetNumberOfSubNodesOnSplit(); indexNodes++)
                    subNodes[indexNodes] = static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_apSubNodes[indexNodes]);


                digUp = m_filter->PostFilter (this, subNodes, GetNumberOfSubNodesOnSplit());
                }
            }

        }
    else
        {
        HFCPtr<HGFPointIndexNode<POINT, EXTENT> > pSubNodes[1];
        digUp = m_filter->PostFilter (this, pSubNodes, 0);
        }

    HINVARIANTS;

    return digUp;

    }


/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::ComputeObjectRelevance()
    {
    if (!IsLoaded())
        Load();

    // If there are sub-nodes and these need filtering then first do the subnodes
    if (!m_nodeHeader.m_IsLeaf)


        {
        if (m_pSubNodeNoSplit != NULL)
            {
            static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->ComputeObjectRelevance();
            }
        else
            {
            for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->ComputeObjectRelevance();
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
template<class POINT, class EXTENT> void HGFPointIndexNode<POINT, EXTENT>::AdjustViewDependentMetric(HVE2DShape& pi_rConvexHull)
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
                static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->AdjustViewDependentMetric(pi_rConvexHull);
            else
                {
                for (size_t indexNode = 0 ; indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                    {
                    static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->AdjustViewDependentMetric(pi_rConvexHull);
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
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::IsFiltered() const
    {
    if (!IsLoaded())
        Load();
    return m_nodeHeader.m_filtered;
    }

//=======================================================================================
// @bsimethod                                                   Alain.Robert 10/10
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndexNode<POINT, EXTENT>::NeedsFiltering() const
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
            needsFiltering = !m_nodeHeader.m_filtered || static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*m_pSubNodeNoSplit)->NeedsFiltering();
            }
        else
            {
            needsFiltering = !m_nodeHeader.m_filtered;

            for (size_t indexNode = 0 ; !needsFiltering && indexNode < GetNumberOfSubNodesOnSplit(); indexNode++)
                {
                needsFiltering = (needsFiltering || static_cast<HGFPointIndexNode<POINT, EXTENT>*>(&*(m_apSubNodes[indexNode]))->NeedsFiltering());
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

// HGFPointIndex Class

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
template<class POINT, class EXTENT> HGFPointIndex<POINT, EXTENT>::HGFPointIndex(HFCPtr<HPMCountLimitedPool<POINT> > pool, HFCPtr<HGFPointTileStore<POINT, EXTENT> > store, size_t pi_SplitTreshold, IHGFPointIndexFilter<POINT, EXTENT>* filter,bool balanced, bool propagatesDataDown)
    : HGFSpatialIndex <POINT, POINT, EXTENT, HGFPointIndexNode<POINT, EXTENT>, HGFPointIndexHeader<EXTENT> >(pi_SplitTreshold, balanced, propagatesDataDown, new HGFPointIndexNode<POINT, EXTENT>(pi_SplitTreshold, ExtentOp<EXTENT>::Create(0.0, 0.0, 0.0, 1.0, 1.0, 1.0), pool, store, filter, balanced, propagatesDataDown)),
      m_pool(pool),
      m_store (store),
      m_filter (filter)
    {
    m_indexHeader.m_SplitTreshold = pi_SplitTreshold;
    m_indexHeader.m_HasMaxExtent = false;
    m_indexHeader.m_balanced = balanced;

    // If a store is provided ...
    if (store != NULL)
        {
        // Try to load master header
        if (0 != store->LoadMasterHeader(&m_indexHeader, sizeof(m_indexHeader)))
            {


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
            if (m_indexHeader.m_rootNodeBlockID.IsValid())
                {
                // If the index is currently balanced ... we create the node as stipulated by construction parameter
                // If it is unbalanced however we want the root node to indicate it is unbalanced in order to balance it
                // immediately after construction
                if (m_indexHeader.m_balanced)
                    m_pRootNode = new HGFPointIndexNode<POINT, EXTENT>(m_indexHeader.m_rootNodeBlockID, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >(NULL), pool, store, m_filter, balanced, PropagatesDataDown());
                else
                    m_pRootNode = new HGFPointIndexNode<POINT, EXTENT>(m_indexHeader.m_rootNodeBlockID, HFCPtr<HGFPointIndexNode<POINT, EXTENT> >(NULL), pool, store, m_filter, m_indexHeader.m_balanced, PropagatesDataDown());

                // We insist on using the given configuration (we will reconfigure all nodes accordingly below)
                // if the request is for a balanced index. If an unbalanced index is required then we will not force unbalance it
                // as the current balanced or unbalanced topology is equally satisfactory.
                m_indexHeader.m_balanced = balanced;

                if (balanced)
                    m_pRootNode->Balance(GetDepth());
                }


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
// template<class POINT, class EXTENT> HGFPointIndex<POINT, EXTENT>::HGFPointIndex(const HGFPointIndex& pi_rObj)
// : HGFPointIndex<POINT, EXTENT, HGFPointIndexNode> (pi_rObj)
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
template<class POINT, class EXTENT> HGFPointIndex<POINT, EXTENT>::~HGFPointIndex()
    {
    HINVARIANTS;

    Store();
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
template<class POINT, class EXTENT> bool HGFPointIndex<POINT, EXTENT>::ChangeStore(HFCPtr<HGFPointTileStore<POINT, EXTENT> > newStore)
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
template<class POINT, class EXTENT> IHGFPointIndexFilter<POINT, EXTENT>* HGFPointIndex<POINT, EXTENT>::GetFilter()
    {
    //HINVARIANTS;

    return(m_filter);
    }

#ifdef __HMR_DEBUG
/**----------------------------------------------------------------------------
 This method dumps the content of the QuadTree as an XML file.

 @param
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void HGFPointIndex<POINT, EXTENT>::DumpQuadTree(char* pi_pOutputXMLFileName,
        bool pi_OnlyLoadedNode) const
    {
    FILE* pOutputFileStream = fopen(pi_pOutputXMLFileName, "w+");

    char TempBuffer[500];
    int  NbChars;

    NbChars = sprintf(TempBuffer, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");

    size_t NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);

    HASSERT(NbWrittenChars == NbChars);

    NbChars = sprintf(TempBuffer, "<RootNode>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);

    HASSERT(NbWrittenChars == NbChars);

    m_pRootNode->DumpQuadTreeNode(pOutputFileStream, pi_OnlyLoadedNode);

    NbChars = sprintf(TempBuffer, "</RootNode>");

    NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);

    HASSERT(NbWrittenChars == NbChars);

    fclose(pOutputFileStream);
    }
#endif

/**----------------------------------------------------------------------------
 Filter
 Filter the data.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void HGFPointIndex<POINT, EXTENT>::Filter()
    {
    HINVARIANTS;

    // Check if root node is present
    if (m_pRootNode != NULL)
        {
        m_pRootNode->PropagateDataDownImmediately (true);
        if (m_pRootNode->GetFilter()->IsProgressiveFilter())
            Balance();

        if (m_pRootNode->GetFilter()->GlobalPreFilter (*this))
            {
            try
                {
                try
                    {
                    if (m_pRootNode->GetFilter()->ImplementsPreFiltering())
                        m_pRootNode->PreFilter ();

                    m_pRootNode->Filter();
                    }
                catch (...)
                    {
                    if (m_pRootNode->GetFilter()->ImplementsPostFiltering())
                        m_pRootNode->PostFilter ();
                    throw;
                    }

                if (m_pRootNode->GetFilter()->ImplementsPreFiltering())
                    m_pRootNode->PostFilter ();
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
template<class POINT, class EXTENT> void HGFPointIndex<POINT, EXTENT>::ComputeObjectRelevance()
    {
    m_pRootNode->ComputeObjectRelevance();
    }

/**----------------------------------------------------------------------------
 AdjustViewDependentMetric
 Adjust the view dependent metric.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> void HGFPointIndex<POINT, EXTENT>::AdjustViewDependentMetric(HVE2DShape& pi_rConvexHull)
    {
    m_pRootNode->AdjustViewDependentMetric(pi_rConvexHull);
    }




/**----------------------------------------------------------------------------
 This method returns the pool

 @return The pool

-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> HFCPtr<HPMCountLimitedPool<POINT> > HGFPointIndex<POINT, EXTENT>::GetPool() const
    {
    HINVARIANTS;
    return m_pool;
    }

/**----------------------------------------------------------------------------
 This method returns the store

 @return The store

-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> HFCPtr<HGFPointTileStore<POINT, EXTENT> > HGFPointIndex<POINT, EXTENT>::GetStore() const
    {
    HINVARIANTS;
    return m_store;
    }


//=======================================================================================
// @bsimethod                                                   Alain.Robert 12/2010
//=======================================================================================
template<class POINT, class EXTENT> bool HGFPointIndex<POINT, EXTENT>::Store()
    {
    HINVARIANTS;

    // Store root node
    if (m_pRootNode != NULL)
        {
        m_pRootNode->Store();
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
template<class POINT, class EXTENT> bool HGFPointIndex<POINT, EXTENT>::Add(const POINT pi_rpSpatialObject)
    {
    HINVARIANTS;

    //// Update content extent

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_HasMaxExtent)
            m_pRootNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), m_indexHeader.m_MaxExtent, m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown());
        else
            m_pRootNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), SpatialOp<POINT, POINT, EXTENT>::ExtentExtract(pi_rpSpatialObject), m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown());
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    if (!m_pRootNode->AddConditional (pi_rpSpatialObject, m_indexHeader.m_HasMaxExtent))
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
template<class POINT, class EXTENT> bool HGFPointIndex<POINT, EXTENT>::AddArray(const POINT* pointsArray, size_t countOfPoints)
    {
    HINVARIANTS;

    if(0 == countOfPoints)
        return true;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_indexHeader.m_HasMaxExtent)
            m_pRootNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), m_indexHeader.m_MaxExtent, m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown());
        else
            m_pRootNode = new HGFPointIndexNode<POINT, EXTENT>(GetSplitTreshold(), SpatialOp<POINT, POINT, EXTENT>::GetExtent(pointsArray[0]), m_pool, m_store, m_filter, IsBalanced(), PropagatesDataDown());
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    size_t numberOfPointsAdded = 0;

    while (numberOfPointsAdded < countOfPoints)
        {
        numberOfPointsAdded = m_pRootNode->AddArrayConditional (pointsArray, numberOfPointsAdded, countOfPoints, m_indexHeader.m_HasMaxExtent);

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
//template<class POINT, class EXTENT> bool HGFPointIndex<POINT, EXTENT>::Remove(const POINT pi_rpSpatialObject)
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




/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the neighborhood of given coordinate
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> size_t HGFPointIndex<POINT, EXTENT>::GetAt(const POINT& pi_rCoord,
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
template<class POINT, class EXTENT> uint64_t HGFPointIndex<POINT, EXTENT>::GetNbObjectsAtLevel(size_t pi_depthLevel)
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
//template<class POINT, class EXTENT> size_t HGFPointIndex<POINT, EXTENT>::GetIn(const EXTENT& pi_rExtent,
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

template<class POINT, class EXTENT> size_t HGFPointIndex<POINT, EXTENT>::GetInUsingViewParameters(const EXTENT&       pi_rExtent,
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
template<class POINT, class EXTENT> size_t HGFPointIndex<POINT, EXTENT>::Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject,
                                                                                list<POINT>& resultPoints)
    {

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

template<class POINT, class EXTENT> size_t HGFPointIndex<POINT, EXTENT>::Query (IHGFPointIndexQuery<POINT, EXTENT>* queryObject,
                                                                                HPMMemoryManagedVector<POINT>& resultPoints)
    {

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

