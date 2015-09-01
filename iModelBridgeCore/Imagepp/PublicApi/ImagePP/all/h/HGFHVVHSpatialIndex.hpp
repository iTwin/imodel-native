//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFHVVHSpatialIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXHVVHSpatialIndex
//-----------------------------------------------------------------------------
// General class for relative ordering indexes.
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
///////////////////////////
// HGFHVVHSpatialIndex
///////////////////////////

/** -----------------------------------------------------------------------------
    A Parameters object must be passed to the constructor. It contains the
    coordinate system that will be used internally by the index.

    @param pi_rParameters IN Parameters for the index.

    @param pi_pSubIndex IN Pointer to the sub-index, if any.

    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline HGFHVVHSpatialIndex<O, SI>::HGFHVVHSpatialIndex(
    const Parameters& pi_rParameters,
    const SI*         pi_pSubIndex)
    : m_pSubIndex(pi_pSubIndex),
      m_pCoordSys(pi_rParameters.m_pCoordSys)
    {
    m_pRoot = new Node(true, 0);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class O, class SI> inline HGFHVVHSpatialIndex<O, SI>::~HGFHVVHSpatialIndex()
    {
    delete m_pRoot;
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Try to merge tree nodes together
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::TryToMergeNodes(Node* pi_pNode)
    {
    Node* pCurrentNode = pi_pNode;

    size_t CurrentCount = pCurrentNode->GetTreeLoad();

    if (CurrentCount <= MAX_NODE_LOAD)
        {
        while (pCurrentNode->pAncestorNode &&
               (CurrentCount += pCurrentNode->pAncestorNode->GetTreeLoadWithout(pCurrentNode)) <= MAX_NODE_LOAD)
            {
            pCurrentNode = pCurrentNode->pAncestorNode;
            }

        if (!pCurrentNode->IsLeaf())
            {
            typename HIDXIndexable<O>::List* pCombined = new typename HIDXIndexable<O>::List;

            pCurrentNode->GatherAllObjects(pCombined);

            pCurrentNode->DeleteSubNodes(pCombined);

            // Adjust attributes to point to the merged node
            typename HIDXIndexable<O>::List::iterator Itr(pCombined->begin());
            while (Itr != pCombined->end())
                {
                ((Attribute*)(*Itr)->GetAttribute(this))->SetNode(pCurrentNode);

                ++Itr;
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Try to merge cut nodes together
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::TryToMergeCutNodes(CutNode* pi_pNode)
    {
    CutNode* pCurrentNode = pi_pNode;

    size_t CurrentCount = pCurrentNode->GetTreeLoad();

    if (CurrentCount <= MAX_CUTNODE_LOAD)
        {
        while (pCurrentNode->pAncestorNode &&
               (CurrentCount += pCurrentNode->pAncestorNode->GetTreeLoadWithout(pCurrentNode)) <= MAX_CUTNODE_LOAD)
            {
            pCurrentNode = pCurrentNode->pAncestorNode;
            }

        if (!pCurrentNode->IsLeaf())
            {
            pCurrentNode->pLeftNode->GatherAllObjects(&pCurrentNode->Objects);
            pCurrentNode->pRightNode->GatherAllObjects(&pCurrentNode->Objects);

            pCurrentNode->DeleteSubNodes();
            }
        }
    }


/** -----------------------------------------------------------------------------
    Add the specified HIDXIndexable object to the index. This method is used
    internally by the indexing mechanism.

    The same HIDXIndexable object must not be added more than once. Also,
    there must not be two HIDXIndexable objects that wrap the same object.

    @param pi_rpObject IN The HIDXIndexable object to add.

    @see Add()
    @see RemoveIndexable()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> void HGFHVVHSpatialIndex<O, SI>::AddIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    // Make sure we don't already contain the object
    HASSERT(pi_rpObject->GetAttribute(this) == 0);
    HASSERT(GetFilledIndexableFor(pi_rpObject)->GetAttribute(this) == 0);

    Node* pCurrentNode = m_pRoot;
    bool Inserted = false;

    // Calculate the object's extent once.
    HGF2DExtent ObjectExtent(pi_rpObject->GetObject()->GetExtent());
    ObjectExtent.ChangeCoordSys(m_pCoordSys);

    if (!ObjectExtent.IsDefined())
        {
        // Object can't be indexed...
        m_UnindexedObjects.push_back(pi_rpObject);
        pi_rpObject->AddAttribute(this, new Attribute(0, ObjectExtent));
        Inserted = true;
        }

    while (!Inserted)
        {
        if (pCurrentNode->IsLeaf())
            {
            if (pCurrentNode->GetLoad() < MAX_NODE_LOAD)
                {
                // Insert here
                pCurrentNode->pObjects->push_back(pi_rpObject);

                // Set the object's attribute
                pi_rpObject->AddAttribute(this, new Attribute(pCurrentNode, ObjectExtent));

                Inserted = true;
                }
            else
                {
                // Ask node to split itself
                typename HIDXIndexable<O>::List* pObjectsToInsert = pCurrentNode->CreateSubNodes();

                pCurrentNode->SplitPosition = CalculateSplitPosition(*pObjectsToInsert, pCurrentNode->SplitHorizontally);

                // Re-insert the objects
                typename HIDXIndexable<O>::List::const_iterator Itr(pObjectsToInsert->begin());
                while (Itr != pObjectsToInsert->end())
                    {
                    HGF2DExtent ExtentOfObjectToInsert((*Itr)->GetObject()->GetExtent());
                    ExtentOfObjectToInsert.ChangeCoordSys(m_pCoordSys);

                    switch (CalculateRelativePosition(pCurrentNode->SplitHorizontally, pCurrentNode->SplitPosition, ExtentOfObjectToInsert))
                        {
                        case LEFT:
                            {
                            // Move current element to the left subnode
                            pCurrentNode->pLeftNode->pObjects->push_back((*Itr));
                            SetAttributeOfObject((*Itr), pCurrentNode->pLeftNode);

                            break;
                            }
                        case RIGHT:
                            {
                            // Move current element to the right subnode
                            pCurrentNode->pRightNode->pObjects->push_back((*Itr));
                            SetAttributeOfObject((*Itr), pCurrentNode->pRightNode);

                            break;
                            }
                        case CENTER:
                            {
                            if (pCurrentNode->pCut == 0)
                                pCurrentNode->pCut = new CutNode(0);

                            InsertInCutTree((*Itr), pCurrentNode->SplitHorizontally, pCurrentNode->pCut, ExtentOfObjectToInsert);

                            // Don't need to set the attribute since the object
                            // was already in pCurrentNode before we split it.
                            }
                        }

                    ++Itr;
                    }

                delete pObjectsToInsert;

                // Insert new element. We loop, and come back in the
                // non-leaf case...
                }
            }
        else
            {
            switch (CalculateRelativePosition(pCurrentNode->SplitHorizontally, pCurrentNode->SplitPosition, ObjectExtent))
                {
                case LEFT:
                    {
                    pCurrentNode = pCurrentNode->pLeftNode;

                    break;
                    }
                case RIGHT:
                    {
                    pCurrentNode = pCurrentNode->pRightNode;

                    break;
                    }
                case CENTER:
                    {
                    // Element touches the split line. Put in cut tree
                    if (pCurrentNode->pCut == 0)
                        pCurrentNode->pCut = new CutNode(0);

                    InsertInCutTree(pi_rpObject, pCurrentNode->SplitHorizontally, pCurrentNode->pCut, ObjectExtent);

                    pi_rpObject->AddAttribute(this, new Attribute(pCurrentNode, ObjectExtent));

                    Inserted = true;

                    break;
                    }
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Remove the specified extent from the min and max values of a cut node
//-----------------------------------------------------------------------------
template<class O, class SI> inline
void HGFHVVHSpatialIndex<O, SI>::AdjustCutNodeBoundsAfterRemoval(
    CutNode*           pi_pNode,
    bool              pi_HorizontalCutNode,
    const HGF2DExtent& pi_rObjectExtent,
    bool              pi_ObjectInsertedHere)
    {
    // Don't use EPSILON here since the variables are not the result of a computation

//    if (pi_HorizontalCutNode)
//    {

    // For now, we simply invalidate all the OtherAxis values. We could invalidate
    // only the ones that are modified by checking if the specified extent
    // touches the min or max value before invalidating.

    pi_pNode->MinOnOtherAxis = DBL_MAX;
    pi_pNode->MaxOnOtherAxis = (-DBL_MAX);

    if (pi_ObjectInsertedHere)
        {
        pi_pNode->MinOnSplitAxis = DBL_MAX;
        pi_pNode->MaxOnSplitAxis = (-DBL_MAX);
        }
//    }
#if 0
    else
        {
        pi_pNode->MinOnOtherAxis = DBL_MAX;
        pi_pNode->MaxOnOtherAxis = (-DBL_MAX);

        if (pi_ObjectInsertedHere)
            {
            pi_pNode->MinOnSplitAxis = DBL_MAX;
            pi_pNode->MaxOnSplitAxis = (-DBL_MAX);
            }
        }
#endif
    }


/** -----------------------------------------------------------------------------
    Remove  the specified HIDXIndexable object from the index. This method is
    used internally by the indexing mechanism.

    @param pi_rpObject IN The HIDXIndexable object to remove.

    @see Remove()
    @see AddIndexable()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> void HGFHVVHSpatialIndex<O, SI>::RemoveIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    // To keep the object alive long enough to do our work
    HFCPtr< HIDXIndexable<O> > pObject = pi_rpObject;

    Attribute* pAttribute = (Attribute*) pObject->GetAttribute(this);

    if (pAttribute == 0)
        {
        pObject = GetFilledIndexableFor(pi_rpObject);
        pAttribute = (Attribute*) pObject->GetAttribute(this);
        }

    if (pAttribute != 0)
        {
        Node* pNode = (Node*) pAttribute->GetNode();

        if (pNode == 0)
            {
            // Object was not placed in the index...
            m_UnindexedObjects.remove(pObject);
            }
        else if (pNode->IsLeaf())
            {
            // Directly in the node's list
            pNode->pObjects->remove(pObject);

            // Try to merge some nodes together.
            TryToMergeNodes(pNode);
            }
        else
            {
            // In the cut tree. We must find the cut node containing it.

            CutNode* pCut = pNode->pCut;
            bool Found = false;

            while (pCut && !Found)
                {
                if (pCut->IsLeaf())
                    {
                    pCut->Objects.remove(pObject);

                    AdjustCutNodeBoundsAfterRemoval(pCut, !pNode->SplitHorizontally, pAttribute->GetExtent(), true);

                    Found = true;
                    }
                else
                    {
                    switch (CalculateRelativePosition(!pNode->SplitHorizontally, pCut->SplitPosition, pAttribute->GetExtent()))
                        {
                        case LEFT:
                            {
                            AdjustCutNodeBoundsAfterRemoval(pCut, !pNode->SplitHorizontally, pAttribute->GetExtent(), false);
                            pCut = pCut->pLeftNode;

                            break;
                            }
                        case RIGHT:
                            {
                            AdjustCutNodeBoundsAfterRemoval(pCut, !pNode->SplitHorizontally, pAttribute->GetExtent(), false);
                            pCut = pCut->pRightNode;

                            break;
                            }
                        case CENTER:
                            {
                            pCut->Objects.remove(pObject);

                            AdjustCutNodeBoundsAfterRemoval(pCut, !pNode->SplitHorizontally, pAttribute->GetExtent(), true);

                            Found = true;

                            break;
                            }
                        }
                    }
                }

            TryToMergeCutNodes(pCut);
            }



        // Remove our attribute from the object.
        pObject->RemoveAttribute(this);
        }
    }




//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Test all objects in the specified list
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::TestObjectsInList(
    typename HIDXIndexable<O>::List& pi_rObjects,
    const HGF2DExtent&      pi_rRegion,
    typename HIDXIndexable<O>::List& pi_rResult) const
    {
    // Check objects in this cut node
    typename HIDXIndexable<O>::List::const_iterator Itr(pi_rObjects.begin());

    while (Itr != pi_rObjects.end())
        {
        if (pi_rRegion.DoTheyOverlap(((Attribute*)(*Itr)->GetAttribute(this))->GetExtent()))
            pi_rResult.push_back(*Itr);

        ++Itr;
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Retrieve objects from a cut tree
//-----------------------------------------------------------------------------
template<class O, class SI> void HGFHVVHSpatialIndex<O, SI>::FindObjectsInCutTree(
    CutNode*                pi_pNode,
    bool                   pi_HorizontalCutNode,
    const HGF2DExtent&      pi_rRegion,
    RelativePosition        pi_PositionOfRegionInHVNode,
    typename HIDXIndexable<O>::List& pi_rResult) const
    {
    // The extent provided must be defined
    HPRECONDITION(pi_rRegion.IsDefined());

    if (pi_pNode->IsLeaf())
        {
        TestObjectsInList(pi_pNode->Objects, pi_rRegion, pi_rResult);
        }
    else
        {
        RelativePosition WhereToCheck = CalculateRelativePosition(!pi_HorizontalCutNode, pi_pNode->SplitPosition, pi_rRegion);

        switch(WhereToCheck)
            {
                // The region is at the left of this cut node
            case LEFT:
                {
                // Check our own objects if necessary
                if (pi_HorizontalCutNode)
                    {
                    if (pi_rRegion.GetXMax() > pi_pNode->MinOnSplitAxis)
                        TestObjectsInList(pi_pNode->Objects, pi_rRegion, pi_rResult);
                    }
                else
                    {
                    if (!pi_HorizontalCutNode && pi_rRegion.GetYMax() > pi_pNode->MinOnSplitAxis)
                        TestObjectsInList(pi_pNode->Objects, pi_rRegion, pi_rResult);
                    }

                // Check if search region goes inside the left cut node
                switch(pi_PositionOfRegionInHVNode)
                    {
                    case LEFT:
                        {
                        if (pi_HorizontalCutNode)
                            {
                            if (pi_rRegion.GetYMax() > pi_pNode->pLeftNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }
                        else
                            {
                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMax() > pi_pNode->pLeftNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }

                        break;
                        }
                    case RIGHT:
                        {
                        if (pi_HorizontalCutNode)
                            {
                            if (pi_rRegion.GetYMin() < pi_pNode->pLeftNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }
                        else
                            {
                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMin() < pi_pNode->pLeftNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }

                        break;
                        }
                    case CENTER:
                        {
                        // We have to check the left subnode
                        FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);

                        break;
                        }
                    }

                break;
                }

            // The region is at the right of this cut node
            case RIGHT:
                {
                // Check our own objects if necessary
                if (pi_HorizontalCutNode)
                    {
                    if (pi_rRegion.GetXMin() < pi_pNode->MaxOnSplitAxis)
                        TestObjectsInList(pi_pNode->Objects, pi_rRegion, pi_rResult);
                    }
                else
                    {
                    if (!pi_HorizontalCutNode && pi_rRegion.GetYMin() < pi_pNode->MaxOnSplitAxis)
                        TestObjectsInList(pi_pNode->Objects, pi_rRegion, pi_rResult);
                    }

                // Check if search region goes inside the right cut node
                switch(pi_PositionOfRegionInHVNode)
                    {
                    case LEFT:
                        {
                        if (pi_HorizontalCutNode)
                            {
                            if (pi_rRegion.GetYMax() > pi_pNode->pRightNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }
                        else
                            {
                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMax() > pi_pNode->pRightNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }

                        break;
                        }
                    case RIGHT:
                        {
                        if (pi_HorizontalCutNode)
                            {
                            if (pi_rRegion.GetYMin() < pi_pNode->pRightNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }
                        else
                            {
                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMin() < pi_pNode->pRightNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }

                        break;
                        }
                    case CENTER:
                        {
                        // We have to check the right subnode
                        FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);

                        break;
                        }
                    }

                break;
                }

            // The region touches this cut node's divider
            case CENTER:
                {
                switch(pi_PositionOfRegionInHVNode)
                    {
                    case LEFT:
                        {
                        // All our objects are potentially in
                        TestObjectsInList(pi_pNode->Objects, pi_rRegion, pi_rResult);

                        // Check if we need to visit our children
                        if (pi_HorizontalCutNode)
                            {
                            if (pi_rRegion.GetYMax() > pi_pNode->pLeftNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);

                            if (pi_rRegion.GetYMax() > pi_pNode->pRightNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }
                        else
                            {
                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMax() > pi_pNode->pLeftNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);

                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMax() > pi_pNode->pRightNode->MinOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }

                        break;
                        }
                    case RIGHT:
                        {
                        // All our objects are potentially in
                        TestObjectsInList(pi_pNode->Objects, pi_rRegion, pi_rResult);

                        // Check if we need to visit our children
                        if (pi_HorizontalCutNode)
                            {
                            if (pi_rRegion.GetYMin() < pi_pNode->pLeftNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);

                            if (pi_rRegion.GetYMin() < pi_pNode->pRightNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }
                        else
                            {
                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMin() < pi_pNode->pLeftNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);

                            if (!pi_HorizontalCutNode && pi_rRegion.GetXMin() < pi_pNode->pRightNode->MaxOnOtherAxis)
                                FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                            }

                        break;
                        }
                    case CENTER:
                        {
                        // Special case. The search region intersects the HV Node and the
                        // cut node, so all objects in this node are in the search region.
                        typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->Objects.begin());
                        while (Itr != pi_pNode->Objects.end())
                            {
                            pi_rResult.push_back(*Itr);
                            ++Itr;
                            }

                        // We need to check both subnodes
                        FindObjectsInCutTree(pi_pNode->pLeftNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);
                        FindObjectsInCutTree(pi_pNode->pRightNode, pi_HorizontalCutNode, pi_rRegion, pi_PositionOfRegionInHVNode, pi_rResult);

                        break;
                        }
                    }


                break;
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Retrieve objects in the specified region
//-----------------------------------------------------------------------------
template<class O, class SI> void HGFHVVHSpatialIndex<O, SI>::FindObjectsInTree(
    Node*                   pi_pNode,
    const HGF2DExtent&      pi_rRegion,
    typename HIDXIndexable<O>::List& pi_rResult) const
    {
    // The extent provided must be defined
    HPRECONDITION(pi_rRegion.IsDefined());

    if (pi_pNode->IsLeaf())
        {
        // Check objects in this node
        typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->pObjects->begin());

        while (Itr != pi_pNode->pObjects->end())
            {
            if (pi_rRegion.DoTheyOverlap(((Attribute*)(*Itr)->GetAttribute(this))->GetExtent()))
                pi_rResult.push_back(*Itr);

            ++Itr;
            }
        }
    else
        {
        // Make sure bounds are valid before we use them
        if (pi_pNode->pCut)
            ValidateCutNodeBounds(pi_pNode->pCut, pi_pNode->SplitHorizontally);

        RelativePosition WhereToCheck = CalculateRelativePosition(pi_pNode->SplitHorizontally, pi_pNode->SplitPosition, pi_rRegion);

        switch(WhereToCheck)
            {
            case LEFT:
                {
                // Check if search region goes inside the cut tree
                if (pi_pNode->pCut)
                    {
                    if (pi_pNode->SplitHorizontally)
                        {
                        if (pi_rRegion.GetYMax() > pi_pNode->pCut->MinOnOtherAxis)
                            FindObjectsInCutTree(pi_pNode->pCut, pi_pNode->SplitHorizontally, pi_rRegion, WhereToCheck, pi_rResult);
                        }
                    else
                        {
                        if (pi_rRegion.GetXMax() > pi_pNode->pCut->MinOnOtherAxis)
                            FindObjectsInCutTree(pi_pNode->pCut, pi_pNode->SplitHorizontally, pi_rRegion, WhereToCheck, pi_rResult);
                        }
                    }

                FindObjectsInTree(pi_pNode->pLeftNode, pi_rRegion, pi_rResult);

                break;
                }

            case RIGHT:
                {
                // Check if search region goes inside the cut tree
                if (pi_pNode->pCut)
                    {
                    if (pi_pNode->SplitHorizontally)
                        {
                        if (pi_rRegion.GetYMin() < pi_pNode->pCut->MaxOnOtherAxis)
                            FindObjectsInCutTree(pi_pNode->pCut, pi_pNode->SplitHorizontally, pi_rRegion, WhereToCheck, pi_rResult);
                        }
                    else
                        {
                        if (pi_rRegion.GetXMin() < pi_pNode->pCut->MaxOnOtherAxis)
                            FindObjectsInCutTree(pi_pNode->pCut, pi_pNode->SplitHorizontally, pi_rRegion, WhereToCheck, pi_rResult);
                        }
                    }

                FindObjectsInTree(pi_pNode->pRightNode, pi_rRegion, pi_rResult);

                break;
                }

            case CENTER:
                {
                // We're sure that the cut tree must be checked, since the search
                // region touches the two sides.
                if (pi_pNode->pCut)
                    FindObjectsInCutTree(pi_pNode->pCut, pi_pNode->SplitHorizontally, pi_rRegion, WhereToCheck, pi_rResult);

                FindObjectsInTree(pi_pNode->pLeftNode, pi_rRegion, pi_rResult);
                FindObjectsInTree(pi_pNode->pRightNode, pi_rRegion, pi_rResult);

                break;
                }
            }
        }
    }


/** -----------------------------------------------------------------------------
    Retrieves a list of objects from the index based on the specified search
    criteria. This index supports HGFSpatialCriteria. If there is a
    HGFSpatialCriteria object in the search criteria for this index, only the
    objects that fall in the region will be returned. Otherwise, all objects
    are returned from the query. This index does not sort the objects.

    @param pi_rCriteria IN The search criteria.

    @return A smart pointer to a list of objects of type O. This list contains
            the query results.

    @see HGFSpatialCriteria
    @see QueryIndexable()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> typename HGFHVVHSpatialIndex<O, SI>::ObjectList* HGFHVVHSpatialIndex<O, SI>::Query(
    const HIDXSearchCriteria& pi_rCriteria) const
    {
    HAutoPtr<ObjectList> pList(new ObjectList);

    HIDXCriteria* pCriteria = pi_rCriteria.GetCriteria(this);

    if (pCriteria && pCriteria->IsCompatibleWith(HGFSpatialCriteria::CLASS_ID))
        {
        if (((HGFSpatialCriteria*)pCriteria)->GetRegion().IsDefined())
            {
            // Make sure extent is in our CS
            ((HGFSpatialCriteria*)pCriteria)->GetRegion().ChangeCoordSys(m_pCoordSys);

            HAutoPtr<typename HIDXIndexable<O>::List> pIndexableList(new typename HIDXIndexable<O>::List);

            FindObjectsInTree(m_pRoot, ((HGFSpatialCriteria*)pCriteria)->GetRegion(), *pIndexableList);

            // Copy elements to the object list
            typename HIDXIndexable<O>::List::const_iterator Itr(pIndexableList->begin());
            while (Itr != pIndexableList->end())
                {
                pList->push_back((*Itr)->GetObject());

                ++Itr;
                }
            }
        else
            {
            // Void region. Return the unindexed objects
            typename IndexableList::const_iterator UnindexedItr(m_UnindexedObjects.begin());
            while (UnindexedItr != m_UnindexedObjects.end())
                {
                pList->push_back((*UnindexedItr)->GetObject());

                ++UnindexedItr;
                }
            }
        }
    else
        {
        // We must return all our objects
        m_pRoot->GatherAllObjects(*pList);

        // Add the unindexed objects to the list
        typename IndexableList::const_iterator UnindexedItr(m_UnindexedObjects.begin());
        while (UnindexedItr != m_UnindexedObjects.end())
            {
            pList->push_back((*UnindexedItr)->GetObject());

            ++UnindexedItr;
            }
        }

    return pList.release();
    }


/** -----------------------------------------------------------------------------
    These query methods are used internally be the indexing mechanism to
    retrieve HIDXIndexable objects based on the specified search criteria.

    The method retrieves objects in this index.
    This index supports HGFSpatialCriteria. If there is a HGFSpatialCriteria
    object in the search criteria for this index, only the objects that fall in
    the region will be returned. Otherwise, all objects are returned from the
    query.
    Objects are returned in no particular order.

    @param pi_rCriteria IN The search criteria.

    @param pi_Sort IN Indicates if the index will sort the returned elements.
                      Irrelevant in our case.

    @return A smart pointer to a list of objects of type O. This list contains
            the query results.

    @see HGFSpatialCriteria
    @see Query()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> typename HIDXIndexable<O>::List* HGFHVVHSpatialIndex<O, SI>::QueryIndexables(
    const HIDXSearchCriteria& pi_rCriteria,
    bool                     pi_Sort) const
    {
    HAutoPtr<typename HIDXIndexable<O>::List> pList(new typename HIDXIndexable<O>::List);

    HIDXCriteria* pCriteria = pi_rCriteria.GetCriteria(this);

    if (pCriteria && pCriteria->IsCompatibleWith(HGFSpatialCriteria::CLASS_ID))
        {
        if (((HGFSpatialCriteria*)pCriteria)->GetRegion().IsDefined())
            {
            // Make sure extent is in our CS
            ((HGFSpatialCriteria*)pCriteria)->GetRegion().ChangeCoordSys(m_pCoordSys);

            FindObjectsInTree(m_pRoot, ((HGFSpatialCriteria*)pCriteria)->GetRegion(), *pList);
            }
        else
            {
            // Void region. Return the unindexed objects
            pList->insert(pList->end(), m_UnindexedObjects.begin(), m_UnindexedObjects.end());
            }
        }
    else
        {
        // We must return all our objects
        m_pRoot->GatherAllObjects(pList);

        // Add the unindexed objects to the list
        pList->insert(pList->end(), m_UnindexedObjects.begin(), m_UnindexedObjects.end());
        }

    return pList.release();
    }


/** -----------------------------------------------------------------------------
    These query methods are used internally be the indexing mechanism to
    retrieve HIDXIndexable objects based on the specified search criteria.

    The method second one does a query on the specified subset.
    This index supports HGFSpatialCriteria. If there is a HGFSpatialCriteria
    object in the search criteria for this index, only the objects that fall in
    the region will be returned. Otherwise, all objects are returned from the
    query.
    The order of objects in the subset will remain unchanged.

    @param pi_rCriteria IN The search criteria.

    @param pi_rSubset IN Smart pointer to a list of HIDXIndexable objects
                        to be queried upon.

    @param pi_Sort IN Indicates if the index will sort the returned elements.
                      Irrelevant in our case.

    @return A smart pointer to a list of objects of type O. This list contains
            the query results.

    @see HGFSpatialCriteria
    @see Query()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> typename HIDXIndexable<O>::List* HGFHVVHSpatialIndex<O, SI>::QueryIndexables(
    const HIDXSearchCriteria&         pi_rCriteria,
    HAutoPtr<typename HIDXIndexable<O>::List>& pi_rSubset,
    bool                             pi_Sort) const
    {
    HIDXCriteria* pCriteria = pi_rCriteria.GetCriteria(this);

    if (pCriteria && pCriteria->IsCompatibleWith(HGFSpatialCriteria::CLASS_ID))
        {
        //HChk MR
        // Here is a possible way to implement:
        //
        // 1. Go through the tree using the specified region and gather all nodes
        //    that are useful (Put the node*s in a set<>). Afftected nodes are:
        //      a) Intermediate nodes whose cut tree is touched by the region
        //      b) Leaf nodes that are touched by the region.
        // 2. Pass through the received subset and retain only the elements
        //    that are in the selected nodes (use Node* stored in the attribute).
        // 3. Pass through the remaining objects to test if they are inside
        //    the specified region.
        //
        // I don't know if this would be faster than to do a normal region query,
        // and then retain from the subset only the objects that were found in
        // the search.
        //
        // For now, we pass through the subset and check if they
        // respect the spatial criteria one by one. This could be
        // long if there are many objects...

        if (((HGFSpatialCriteria*)pCriteria)->GetRegion().IsDefined())
            {
            // Make sure extent is in our CS
            ((HGFSpatialCriteria*)pCriteria)->GetRegion().ChangeCoordSys(m_pCoordSys);

            typename IndexableList::iterator Itr(pi_rSubset->begin());
            typename IndexableList::iterator TempItr;
            while (Itr != pi_rSubset->end())
                {
                if (((Attribute*)(*Itr)->GetAttribute(this))->GetExtent().IsDefined() &&
                    ((HGFSpatialCriteria*)pCriteria)->GetRegion().DoTheyOverlap(((Attribute*)(*Itr)->GetAttribute(this))->GetExtent()))
                    {
                    // We're keeping the object
                    ++Itr;
                    }
                else
                    {
                    // Object does not match the criteria, remove
                    TempItr = Itr;
                    ++Itr;
                    pi_rSubset->erase(TempItr);
                    }
                }
            }
        else
            {
            // Void region. Only keep objects with a void region
            typename IndexableList::iterator Itr(pi_rSubset->begin());
            typename IndexableList::iterator TempItr;
            while (Itr != pi_rSubset->end())
                {
                if (!((Attribute*)(*Itr)->GetAttribute(this))->GetExtent().IsDefined())
                    {
                    // We're keeping the object
                    ++Itr;
                    }
                else
                    {
                    // Object does not match the criteria, remove
                    TempItr = Itr;
                    ++Itr;
                    pi_rSubset->erase(TempItr);
                    }
                }
            }
        }
    // else
    //
    // We simply return the subset.

    return pi_rSubset.release();
    }


/** -----------------------------------------------------------------------------
    Tells if the index is capable of returning a list of objects that interact
    with the specified ones. This index supports it.

    @return true

    @see GetInteractingObjects()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline bool HGFHVVHSpatialIndex<O, SI>::SupportsInteractingRetrieval() const
    {
    return true;
    }


/** -----------------------------------------------------------------------------
    Retrieve the list of objects that interact with the specified ones.
    Since we index objects using the spatial position, the interacting objects
    are the ones that "touch" the objects in the specified list. Two objects
    "touch" if their shapes overlap.

    @param pi_rpObjects IN The objects for which we want to find the interactants.

    @return A list of HIDXIndexable objects that interact with the specified ones.

    @see SupportsInteractingRetrieval()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> typename HIDXIndexable<O>::List* HGFHVVHSpatialIndex<O, SI>::GetInteractingObjects(
    typename HIDXIndexable<O>::List const& pi_rpObjects) const
    {
    HAutoPtr< typename HIDXIndexable<O>::List > pResult;


    // Accumulate the extents of the objects
    HGF2DExtent TotalExtent;
    bool AddUnindexedObjects = false;

    typename HIDXIndexable<O>::List::const_iterator Itr(pi_rpObjects.begin());
    while (Itr != pi_rpObjects.end())
        {
        if ((*Itr)->GetObject()->GetExtent().IsDefined())
            {
            if (TotalExtent.IsDefined())
                TotalExtent.Add((*Itr)->GetObject()->GetExtent());
            else
                TotalExtent = (*Itr)->GetObject()->GetExtent();
            }
        else
            {
            // One item has a void region.
            AddUnindexedObjects = true;
            }

        ++Itr;
        }

    if (TotalExtent.IsDefined())
        {
        // Do a query using the total extent as criteria
        pResult = QueryIndexables(HIDXSearchCriteria(this,
                                                     new HGFSpatialCriteria(TotalExtent)));
        }
    else
        {
        pResult = new typename HIDXIndexable<O>::List;
        }

    if (AddUnindexedObjects)
        {
        // Return our list of unindexed objects.
        pResult->insert(pResult->end(), m_UnindexedObjects.begin(), m_UnindexedObjects.end());
        }

    return pResult.release();   // transfert ownership to caller.
    }


/** -----------------------------------------------------------------------------
    Returns the sorting requirement of the index.
    This method is used internally by the indexing mechanism. It is used when
    a complex index needs to know how to sort a Query result.
    This index does not sort.

    @return SORTING_NONE

    @see GetPredicate()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline HIDXSortingRequirement HGFHVVHSpatialIndex<O, SI>::GetSortingRequirement() const
    {
    // Never sort
    return SORTING_NONE;
    }


/** -----------------------------------------------------------------------------
    Returns the sorting predicate for this index. The predicate is a
    binary_function of STL that is used when sorting query results for indexes
    that ask for SORTING_COMPLEX. It contains the logic to sort objects the
    are contained in this index.
    This method is used internally by the indexing mechanism.
    Since this index does not need sorting, its predicate does nothing.

    @return A predicate object that does nothing.

    @see GetSortingRequirement()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline typename HGFHVVHSpatialIndex<O, SI>::Predicate
HGFHVVHSpatialIndex<O, SI>::GetPredicate() const
    {
    return Predicate();
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Calculate a splitting point based on the objects
//-----------------------------------------------------------------------------
template<class O, class SI> double HGFHVVHSpatialIndex<O, SI>::CalculateSplitPosition(
    const typename HIDXIndexable<O>::List& pi_rObjects,
    bool                         pi_NodeSplitsHorizontally) const
    {
    HASSERT(pi_rObjects.size() > 0);

    typename HIDXIndexable<O>::List::const_iterator Itr(pi_rObjects.begin());

    HGF2DExtent ObjectExtent((*Itr)->GetObject()->GetExtent());
    ObjectExtent.ChangeCoordSys(m_pCoordSys);

    HASSERT(ObjectExtent.IsDefined());

    double MinPosition;
    double MaxPosition;

    // Start with the first object
    if (pi_NodeSplitsHorizontally)
        {
        MinPosition = ObjectExtent.GetYMin();
        MaxPosition = ObjectExtent.GetYMax();
        }
    else
        {
        MinPosition = ObjectExtent.GetXMin();
        MaxPosition = ObjectExtent.GetXMax();
        }

    ++Itr;

    // Pass over all others.
    while (Itr != pi_rObjects.end())
        {
        ObjectExtent = (*Itr)->GetObject()->GetExtent();
        ObjectExtent.ChangeCoordSys(m_pCoordSys);

        HASSERT(ObjectExtent.IsDefined());

        if (pi_NodeSplitsHorizontally)
            {
            MinPosition = MIN(MinPosition, ObjectExtent.GetYMin());
            MaxPosition = MAX(MaxPosition, ObjectExtent.GetYMax());
            }
        else
            {
            MinPosition = MIN(MinPosition, ObjectExtent.GetXMin());
            MaxPosition = MAX(MaxPosition, ObjectExtent.GetXMax());
            }

        ++Itr;
        }

    return (MaxPosition - MinPosition) / 2.0 + MinPosition;
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Set the object's attribute
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::SetAttributeOfObject(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject,
    Node*                             pi_pNode)
    {
    // Get the object's attribute
    ((Attribute*) pi_rpObject->GetAttribute(this))->SetNode(pi_pNode);
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Add the specified extent to the min and max values of a cut node
//-----------------------------------------------------------------------------
template<class O, class SI> inline
void HGFHVVHSpatialIndex<O, SI>::AdjustCutNodeBoundsAfterInsertion(
    CutNode*           pi_pNode,
    bool              pi_HorizontalCutNode,
    const HGF2DExtent& pi_rObjectExtent,
    bool              pi_ObjectInsertedHere)
    {
    // The object extent must be defined
    HPRECONDITION(pi_rObjectExtent.IsDefined());

    // Don't use EPSILON here since the variables are not the result of a computation

    if (pi_HorizontalCutNode)
        {
        if (pi_pNode->MinOnOtherAxis != DBL_MAX)
            {
            pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                           pi_rObjectExtent.GetYMin());
            pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                           pi_rObjectExtent.GetYMax());
            }

        if (pi_ObjectInsertedHere && pi_pNode->MinOnSplitAxis != DBL_MAX)
            {
            pi_pNode->MinOnSplitAxis = MIN(pi_pNode->MinOnSplitAxis,
                                           pi_rObjectExtent.GetXMin());
            pi_pNode->MaxOnSplitAxis = MAX(pi_pNode->MaxOnSplitAxis,
                                           pi_rObjectExtent.GetXMax());
            }
        }
    else
        {
        if (pi_pNode->MinOnOtherAxis != DBL_MAX)
            {
            pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                           pi_rObjectExtent.GetXMin());
            pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                           pi_rObjectExtent.GetXMax());
            }

        if (pi_ObjectInsertedHere && pi_pNode->MinOnSplitAxis != DBL_MAX)
            {
            pi_pNode->MinOnSplitAxis = MIN(pi_pNode->MinOnSplitAxis,
                                           pi_rObjectExtent.GetYMin());
            pi_pNode->MaxOnSplitAxis = MAX(pi_pNode->MaxOnSplitAxis,
                                           pi_rObjectExtent.GetYMax());
            }
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Insert an object in a cut tree.
//-----------------------------------------------------------------------------
template<class O, class SI> void HGFHVVHSpatialIndex<O, SI>::InsertInCutTree(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject,
    bool                             pi_HorizontalCutNode,
    CutNode*                          pi_pCutNode,
    const HGF2DExtent&                pi_rObjectExtent)
    {
    CutNode* pCurrentNode = pi_pCutNode;
    bool Inserted = false;

    while (!Inserted)
        {
        if (pCurrentNode->IsLeaf())
            {
            if (pCurrentNode->GetLoad() < MAX_CUTNODE_LOAD)
                {
                // Insert here
                pCurrentNode->Objects.push_back(pi_rpObject);

                AdjustCutNodeBoundsAfterInsertion(pCurrentNode, pi_HorizontalCutNode, pi_rObjectExtent, true);

                Inserted = true;
                }
            else
                {
                // Ask node to split itself
                pCurrentNode->CreateSubNodes();

                pCurrentNode->SplitPosition = CalculateSplitPosition(pCurrentNode->Objects, !pi_HorizontalCutNode);

                // Re-distribute the objects
                typename HIDXIndexable<O>::List::iterator Itr(pCurrentNode->Objects.begin());
                typename HIDXIndexable<O>::List::iterator TempItr;
                while (Itr != pCurrentNode->Objects.end())
                    {
                    HGF2DExtent ExtentOfObjectToInsert((*Itr)->GetObject()->GetExtent());
                    ExtentOfObjectToInsert.ChangeCoordSys(m_pCoordSys);

                    switch (CalculateRelativePosition(!pi_HorizontalCutNode, pCurrentNode->SplitPosition, ExtentOfObjectToInsert))
                        {
                        case LEFT:
                            {
                            TempItr = Itr;
                            ++Itr;

                            // Move current element to the left subnode
                            pCurrentNode->pLeftNode->Objects.push_back((*TempItr));
                            pCurrentNode->Objects.erase(TempItr);

                            break;
                            }
                        case RIGHT:
                            {
                            TempItr = Itr;
                            ++Itr;

                            // Move current element to the right subnode
                            pCurrentNode->pRightNode->Objects.push_back((*TempItr));
                            pCurrentNode->Objects.erase(TempItr);

                            break;
                            }
                        case CENTER:
                            {
                            // Keep the object in this cut node.
                            ++Itr;
                            }
                        }
                    }

                // Insert new element. We loop, just in case all the re-inserted
                // elements went into the same sub-node.
                }
            }
        else
            {
            switch (CalculateRelativePosition(!pi_HorizontalCutNode, pCurrentNode->SplitPosition, pi_rObjectExtent))
                {
                case LEFT:
                    {
                    AdjustCutNodeBoundsAfterInsertion(pCurrentNode, pi_HorizontalCutNode, pi_rObjectExtent, false);

                    pCurrentNode = pCurrentNode->pLeftNode;

                    break;
                    }
                case RIGHT:
                    {
                    AdjustCutNodeBoundsAfterInsertion(pCurrentNode, pi_HorizontalCutNode, pi_rObjectExtent, false);

                    pCurrentNode = pCurrentNode->pRightNode;

                    break;
                    }
                case CENTER:
                    {
                    pCurrentNode->Objects.push_back(pi_rpObject);

                    AdjustCutNodeBoundsAfterInsertion(pCurrentNode, pi_HorizontalCutNode, pi_rObjectExtent, true);

                    Inserted = true;

                    break;
                    }
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Calculate the invalidated bounds for a cut node
//-----------------------------------------------------------------------------
template<class O, class SI> void HGFHVVHSpatialIndex<O, SI>::ValidateCutNodeBounds(CutNode* pi_pNode,
        bool    pi_HorizontalCutNode) const
    {
    // We do not calculate the values of leaf nodes.
    if (!pi_pNode->IsLeaf())
        {
        // Bounds of the sub-nodes must be valid before...
        ValidateCutNodeBounds(pi_pNode->pLeftNode, pi_HorizontalCutNode);
        ValidateCutNodeBounds(pi_pNode->pRightNode, pi_HorizontalCutNode);
        }
    if (pi_pNode->MinOnSplitAxis == DBL_MAX)
        {
        if (pi_pNode->MinOnOtherAxis == DBL_MAX)
            {
            // Calculate the two bounds

            if (pi_HorizontalCutNode)
                {
                typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->Objects.begin());

                // Take our own objects into account
                while (Itr != pi_pNode->Objects.end())
                    {
                    HGF2DExtent ObjectExtent((*Itr)->GetObject()->GetExtent());
                    ObjectExtent.ChangeCoordSys(m_pCoordSys);

                    pi_pNode->MinOnSplitAxis = MIN(pi_pNode->MinOnSplitAxis,
                                                   ObjectExtent.GetXMin());
                    pi_pNode->MaxOnSplitAxis = MAX(pi_pNode->MaxOnSplitAxis,
                                                   ObjectExtent.GetXMax());
                    pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                                   ObjectExtent.GetYMin());
                    pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                                   ObjectExtent.GetYMax());
                    ++Itr;
                    }
                }
            else
                {
                typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->Objects.begin());

                // Take our own objects into account
                while (Itr != pi_pNode->Objects.end())
                    {
                    HGF2DExtent ObjectExtent((*Itr)->GetObject()->GetExtent());
                    ObjectExtent.ChangeCoordSys(m_pCoordSys);

                    pi_pNode->MinOnSplitAxis = MIN(pi_pNode->MinOnSplitAxis,
                                                   ObjectExtent.GetYMin());
                    pi_pNode->MaxOnSplitAxis = MAX(pi_pNode->MaxOnSplitAxis,
                                                   ObjectExtent.GetYMax());
                    pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                                   ObjectExtent.GetXMin());
                    pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                                   ObjectExtent.GetXMax());
                    ++Itr;
                    }
                }

            // and also the sub-nodes
            if (!pi_pNode->IsLeaf())
                {
                pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                               MIN(pi_pNode->pLeftNode->MinOnOtherAxis, pi_pNode->pRightNode->MinOnOtherAxis));
                pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                               MAX(pi_pNode->pLeftNode->MaxOnOtherAxis, pi_pNode->pRightNode->MaxOnOtherAxis));
                }
            }
        else
            {
            // Only the split axis

            if (pi_HorizontalCutNode)
                {
                typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->Objects.begin());

                // Take our own objects into account
                while (Itr != pi_pNode->Objects.end())
                    {
                    HGF2DExtent ObjectExtent((*Itr)->GetObject()->GetExtent());
                    ObjectExtent.ChangeCoordSys(m_pCoordSys);

                    pi_pNode->MinOnSplitAxis = MIN(pi_pNode->MinOnSplitAxis,
                                                   ObjectExtent.GetXMin());
                    pi_pNode->MaxOnSplitAxis = MAX(pi_pNode->MaxOnSplitAxis,
                                                   ObjectExtent.GetXMax());
                    ++Itr;
                    }
                }
            else
                {
                typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->Objects.begin());

                // Take our own objects into account
                while (Itr != pi_pNode->Objects.end())
                    {
                    HGF2DExtent ObjectExtent((*Itr)->GetObject()->GetExtent());
                    ObjectExtent.ChangeCoordSys(m_pCoordSys);

                    pi_pNode->MinOnSplitAxis = MIN(pi_pNode->MinOnSplitAxis,
                                                   ObjectExtent.GetYMin());
                    pi_pNode->MaxOnSplitAxis = MAX(pi_pNode->MaxOnSplitAxis,
                                                   ObjectExtent.GetYMax());
                    ++Itr;
                    }
                }
            }
        }
    else
        {
        if (pi_pNode->MinOnOtherAxis == DBL_MAX)
            {
            // Only the other axis
            if (pi_HorizontalCutNode)
                {
                typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->Objects.begin());

                // Take our own objects into account
                while (Itr != pi_pNode->Objects.end())
                    {
                    HGF2DExtent ObjectExtent((*Itr)->GetObject()->GetExtent());
                    ObjectExtent.ChangeCoordSys(m_pCoordSys);

                    pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                                   ObjectExtent.GetYMin());
                    pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                                   ObjectExtent.GetYMax());
                    ++Itr;
                    }
                }
            else
                {
                typename HIDXIndexable<O>::List::const_iterator Itr(pi_pNode->Objects.begin());

                // Take our own objects into account
                while (Itr != pi_pNode->Objects.end())
                    {
                    HGF2DExtent ObjectExtent((*Itr)->GetObject()->GetExtent());
                    ObjectExtent.ChangeCoordSys(m_pCoordSys);

                    pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                                   ObjectExtent.GetXMin());
                    pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                                   ObjectExtent.GetXMax());
                    ++Itr;
                    }
                }

            // and also the sub-nodes
            if (!pi_pNode->IsLeaf())
                {
                pi_pNode->MinOnOtherAxis = MIN(pi_pNode->MinOnOtherAxis,
                                               MIN(pi_pNode->pLeftNode->MinOnOtherAxis, pi_pNode->pRightNode->MinOnOtherAxis));
                pi_pNode->MaxOnOtherAxis = MAX(pi_pNode->MaxOnOtherAxis,
                                               MAX(pi_pNode->pLeftNode->MaxOnOtherAxis, pi_pNode->pRightNode->MaxOnOtherAxis));
                }
            }
        }
//    }
    }


/** -----------------------------------------------------------------------------
    Add a new object to the index.
    A new HIDXIndexable object that wraps the specified object will be created.
    The same object must not be added more than once.

    @param pi_Object IN The new object to add

    @see Remove()
    @see AddIndexeable()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::Add(const O pi_Object)
    {
    // We can add the object using an indexable
    AddIndexable(new HIDXIndexable<O>(pi_Object));
    }


/** -----------------------------------------------------------------------------
    Add many new objects in a list to the index.
    A new HIDXIndexable object that wraps the specified object will be created.
    The same object must not be added more than once.

    @param pi_Object IN The new object to add

    @see Remove()
    @see AddIndexeables()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::Add(const ObjectList& pi_rObjects)
    {
    typename ObjectList::const_iterator Itr(pi_rObjects.begin());
    while (Itr != pi_rObjects.end())
        {
        // We can add the object using an indexable
        AddIndexable(new HIDXIndexable<O>(*Itr));

        ++Itr;
        }
    }


/** -----------------------------------------------------------------------------
    Add many objects in a list of indexeables to the index.

    @param pi_rpObjects IN The list of indexeables to add to the index.

    @see Remove()
    @see Add()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::AddIndexables(
    const HAutoPtr< typename HIDXIndexable<O>::List >& pi_rpObjects)
    {
    typename HIDXIndexable<O>::List::const_iterator Itr(pi_rpObjects->begin());
    while (Itr != pi_rpObjects->end())
        {
        // We can add the object using an indexable
        AddIndexable(*Itr);

        ++Itr;
        }
    }


/** -----------------------------------------------------------------------------
    Remove the specified object from the index.

    @param pi_Object IN The new object to remove

    @see Remove()
    @see AddIndexeable()
    -----------------------------------------------------------------------------
*/
template<class O, class SI> inline void HGFHVVHSpatialIndex<O, SI>::Remove(const O pi_Object)
    {
    // We can remove the object using an indexable
    RemoveIndexable(new HIDXIndexable<O>(pi_Object));
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Calculate the position of an object relative to the split line
// Multiple return points for performance!
//-----------------------------------------------------------------------------
template<class O, class SI> inline
typename HGFHVVHSpatialIndex<O, SI>::RelativePosition HGFHVVHSpatialIndex<O, SI>::CalculateRelativePosition(
    bool                             pi_NodeSplitsHorizontally,
    double                           pi_SplitPosition,
    const HGF2DExtent&                pi_rObjectExtent) const
    {
    // The extent provided must be defined
    HPRECONDITION(pi_rObjectExtent.IsDefined());

    if (pi_NodeSplitsHorizontally)
        {
        if (pi_rObjectExtent.GetYMin() <= pi_SplitPosition)
            {
            // Object touches left side.

            if (pi_rObjectExtent.GetYMax() <= pi_SplitPosition)
                {
                // Object is fully on the left side
                return LEFT;
                }
            else
                {
                return CENTER;
                }
            }
        else
            {
            return RIGHT;
            }
        }
    else
        {
        if (pi_rObjectExtent.GetXMin() <= pi_SplitPosition)
            {
            // Object touches left side.

            if (pi_rObjectExtent.GetXMax() <= pi_SplitPosition)
                {
                // Object is fully on the left side
                return LEFT;
                }
            else
                {
                return CENTER;
                }
            }
        else
            {
            return RIGHT;
            }
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Retrieve the internal indexable for the specified object
//-----------------------------------------------------------------------------
template<class O, class SI> const HFCPtr< HIDXIndexable<O> > HGFHVVHSpatialIndex<O, SI>::GetFilledIndexableFor(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const
    {
    HFCPtr< HIDXIndexable<O> > pResult( pi_rpObject );

    // Calculate the object's extent once.
    HGF2DExtent ObjectExtent(pi_rpObject->GetObject()->GetExtent());
    ObjectExtent.ChangeCoordSys(m_pCoordSys);

    Node* pCurrentNode = m_pRoot;
    bool StopLooking = false;

    if (!ObjectExtent.IsDefined())
        {
        // Object was not indexed. Check in the unindexed list only.

        typename HIDXIndexable<O>::List::const_iterator Itr(m_UnindexedObjects.begin());
        while (Itr != m_UnindexedObjects.end())
            {
            if ((*Itr)->IndexesSameObjectAs(*pi_rpObject))
                {
                pResult = (*Itr);
                Itr = m_UnindexedObjects.end();
                }
            else
                ++Itr;
            }

        StopLooking = true;
        }

    // Look for it in the tree
    while (!StopLooking)
        {
        if (pCurrentNode->IsLeaf())
            {
            typename HIDXIndexable<O>::List::const_iterator Itr(pCurrentNode->pObjects->begin());
            while (Itr != pCurrentNode->pObjects->end())
                {
                if ((*Itr)->IndexesSameObjectAs(*pi_rpObject))
                    {
                    pResult = (*Itr);
                    Itr = pCurrentNode->pObjects->end();
                    }
                else
                    ++Itr;
                }

            StopLooking = true;
            }
        else
            {
            switch (CalculateRelativePosition(pCurrentNode->SplitHorizontally, pCurrentNode->SplitPosition, ObjectExtent))
                {
                case LEFT:
                    {
                    pCurrentNode = pCurrentNode->pLeftNode;

                    break;
                    }
                case RIGHT:
                    {
                    pCurrentNode = pCurrentNode->pRightNode;

                    break;
                    }
                case CENTER:
                    {
                    // In cut tree. Look there...

                    CutNode* pCurrentCutNode = pCurrentNode->pCut;

                    while (!StopLooking)
                        {
                        if (pCurrentCutNode == 0)
                            {
                            StopLooking = true;
                            }
                        else
                            {
                            if (pCurrentCutNode->IsLeaf())
                                {
                                typename HIDXIndexable<O>::List::const_iterator Itr(pCurrentCutNode->Objects.begin());
                                while (Itr != pCurrentCutNode->Objects.end())
                                    {
                                    if ((*Itr)->IndexesSameObjectAs(*pi_rpObject))
                                        {
                                        pResult = (*Itr);
                                        Itr = pCurrentCutNode->Objects.end();
                                        }
                                    else
                                        ++Itr;
                                    }

                                StopLooking = true;
                                }
                            else
                                {
                                switch (CalculateRelativePosition(!pCurrentNode->SplitHorizontally, pCurrentCutNode->SplitPosition, ObjectExtent))
                                    {
                                    case LEFT:
                                        {
                                        pCurrentCutNode = pCurrentCutNode->pLeftNode;

                                        break;
                                        }
                                    case RIGHT:
                                        {
                                        pCurrentCutNode = pCurrentCutNode->pRightNode;

                                        break;
                                        }
                                    case CENTER:
                                        {
                                        typename HIDXIndexable<O>::List::const_iterator Itr(pCurrentCutNode->Objects.begin());
                                        while (Itr != pCurrentCutNode->Objects.end())
                                            {
                                            if ((*Itr)->IndexesSameObjectAs(*pi_rpObject))
                                                {
                                                pResult = (*Itr);
                                                Itr = pCurrentCutNode->Objects.end();
                                                }
                                            else
                                                ++Itr;
                                            }

                                        StopLooking = true;

                                        break;
                                        }
                                    }
                                }
                            }
                        }

                    break;
                    }
                }
            }
        }

    // Check in sub-index if we haven't found it
    if (pResult == pi_rpObject && m_pSubIndex != 0)
        {
        pResult = m_pSubIndex->GetFilledIndexableFor(pi_rpObject);
        }

    return pResult;
    }
END_IMAGEPP_NAMESPACE