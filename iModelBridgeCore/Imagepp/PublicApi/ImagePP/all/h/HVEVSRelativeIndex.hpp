//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEVSRelativeIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVEVSRelativeIndex
//-----------------------------------------------------------------------------
// Relative ordering index that calculates visible surfaces.
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class O, class SI> inline HVEVSRelativeIndex<O, SI>::HVEVSRelativeIndex(
    const Parameters& pi_rParameters,
    const SI*         pi_pSubIndex)
    : m_Index(typename HIDXAListRelativeIndex<O,SI>::Parameters(), pi_pSubIndex)
    {
    m_pSubIndex = pi_pSubIndex;
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class O, class SI> inline HVEVSRelativeIndex<O, SI>::~HVEVSRelativeIndex()
    {
    }


//-----------------------------------------------------------------------------
// Add an element
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HVEVSRelativeIndex<O, SI>::AddIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    m_Index.AddIndexable(pi_rpObject);

    // Add our attribute
    pi_rpObject->AddAttribute(this, new HVEVSRelativeAttribute());

    Invalidate(pi_rpObject);
    }


//-----------------------------------------------------------------------------
// Add an object
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HVEVSRelativeIndex<O, SI>::Add(const O pi_Object)
    {
    AddIndexable(new HIDXIndexable<O>(pi_Object));
    }


//-----------------------------------------------------------------------------
// Remove an element
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HVEVSRelativeIndex<O, SI>::RemoveIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    // Make sure we've got the real indexable
    HFCPtr< HIDXIndexable<O> > pObject(pi_rpObject);
    if (pObject->GetAttribute(this) == 0)
        pObject = GetFilledIndexableFor(pObject);

    Invalidate(pObject);

    m_Index.RemoveIndexable(pObject);

    // Remove our attribute
    pObject->RemoveAttribute(this);
    }


//-----------------------------------------------------------------------------
// Remove an object
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HVEVSRelativeIndex<O, SI>::Remove(const O pi_Object)
    {
    RemoveIndexable(new HIDXIndexable<O>(pi_Object));
    }


//-----------------------------------------------------------------------------
// Add elements
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HVEVSRelativeIndex<O, SI>::AddIndexables(
    const HAutoPtr< typename HIDXIndexable<O>::List >& pi_rpObjects)
    {
    typename HIDXIndexable<O>::List::const_iterator Itr(pi_rpObjects->begin());
    while (Itr != pi_rpObjects->end())
        {
        m_Index.AddIndexable(*Itr);

        // Add our attribute
        (*Itr)->AddAttribute(this, new HVEVSRelativeAttribute());

        ++Itr;
        }

    Invalidate(pi_rpObjects);
    }


//-----------------------------------------------------------------------------
// Add objects in batch
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HVEVSRelativeIndex<O, SI>::Add(const ObjectList& pi_rObjects)
    {
    HAutoPtr<typename HIDXIndexable<O>::List> pIndexables(new typename HIDXIndexable<O>::List);

    typename ObjectList::const_iterator Itr(pi_rObjects.begin());
    while (Itr != pi_rObjects.end())
        {
        pIndexables.push_back(new HIDXIndexable<O>(*Itr));

        ++Itr;
        }

    AddIndexables(pIndexables);
    }



//-----------------------------------------------------------------------------
// Promote element
//-----------------------------------------------------------------------------
template<class O, class SI> void HVEVSRelativeIndex<O, SI>::Promote(const O pi_rpObject)
    {
    m_Index.Promote(pi_rpObject);

    // Invalidate the visible shapes of the promoted object
    // and the one that was over it before...
    Invalidate(new HIDXIndexable<O>(pi_rpObject));
    }


//-----------------------------------------------------------------------------
// Demote element
//-----------------------------------------------------------------------------
template<class O, class SI> void HVEVSRelativeIndex<O, SI>::Demote(const O pi_rpObject)
    {
    m_Index.Demote(pi_rpObject);

    // Invalidate the visible shapes of the demoted object
    // and the one that was under it before...
    Invalidate(new HIDXIndexable<O>(pi_rpObject));
    }


//-----------------------------------------------------------------------------
// Put an element at the front
//-----------------------------------------------------------------------------
template<class O, class SI> void HVEVSRelativeIndex<O, SI>::Front(const O pi_rpObject)
    {
    m_Index.Front(pi_rpObject);

    Invalidate(new HIDXIndexable<O>(pi_rpObject));
    }


//-----------------------------------------------------------------------------
// Put an element at the back
//-----------------------------------------------------------------------------
template<class O, class SI> void HVEVSRelativeIndex<O, SI>::Back(const O pi_rpObject)
    {
    Invalidate(new HIDXIndexable<O>(pi_rpObject));

    m_Index.Back(pi_rpObject);
    }


//-----------------------------------------------------------------------------
// Query
//-----------------------------------------------------------------------------
template<class O, class SI> inline typename HVEVSRelativeIndex<O, SI>::ObjectList* HVEVSRelativeIndex<O, SI>::Query(
    const HIDXSearchCriteria& pi_rCriteria) const
    {
    return m_Index.Query(pi_rCriteria);
    }


//-----------------------------------------------------------------------------
// Internal query
//-----------------------------------------------------------------------------
template<class O, class SI> inline typename HIDXIndexable<O>::List* HVEVSRelativeIndex<O, SI>::QueryIndexables(
    const HIDXSearchCriteria& pi_rCriteria,
    bool                     pi_Sort) const
    {
    return m_Index.QueryIndexables(pi_rCriteria, pi_Sort);
    }


//-----------------------------------------------------------------------------
// Internal query
//-----------------------------------------------------------------------------
template<class O, class SI> inline typename HIDXIndexable<O>::List* HVEVSRelativeIndex<O, SI>::QueryIndexables(
    const HIDXSearchCriteria&         pi_rCriteria,
    HAutoPtr<typename HIDXIndexable<O>::List>& pi_rSubset,
    bool                             pi_Sort) const
    {
    return m_Index.QueryIndexables(pi_rCriteria, pi_rSubset, pi_Sort);
    }


//-----------------------------------------------------------------------------
// Retrieve the internal indexable for the specified object
//-----------------------------------------------------------------------------
template<class O, class SI> inline const HFCPtr< HIDXIndexable<O> > HVEVSRelativeIndex<O, SI>::GetFilledIndexableFor(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const
    {
    return m_Index.GetFilledIndexableFor(pi_rpObject);
    }


//-----------------------------------------------------------------------------
// This index can't retrieve objects interacting with a specific one
//-----------------------------------------------------------------------------
template<class O, class SI> inline bool HVEVSRelativeIndex<O, SI>::SupportsInteractingRetrieval() const
    {
    return m_Index.SupportsInteractingRetrieval();
    }


//-----------------------------------------------------------------------------
// This index can't retrieve interacting objects
//-----------------------------------------------------------------------------
template<class O, class SI> inline typename HIDXIndexable<O>::List* HVEVSRelativeIndex<O, SI>::GetInteractingObjects(
    typename HIDXIndexable<O>::List const& pi_rpObjects) const
    {
    return m_Index.GetInteractingObjects(pi_rpObjects);
    }


//-----------------------------------------------------------------------------
// Ask the index if it needs to perform some sorting
//-----------------------------------------------------------------------------
template<class O, class SI> inline HIDXSortingRequirement HVEVSRelativeIndex<O, SI>::GetSortingRequirement() const
    {
    return m_Index.GetSortingRequirement();
    }


//-----------------------------------------------------------------------------
// Get the index's sorting function
//-----------------------------------------------------------------------------
template<class O, class SI> inline typename HVEVSRelativeIndex<O, SI>::Predicate HVEVSRelativeIndex<O, SI>::GetPredicate() const
    {
    return (HVEVSRelativeIndex<O, SI>::Predicate) m_Index.GetPredicate();
    }


//-----------------------------------------------------------------------------
// Pre-calculate the visible surface of objects in the list
//-----------------------------------------------------------------------------
template<class O, class SI> void HVEVSRelativeIndex<O, SI>::CalculateVisibleSurfaces(const HAutoPtr<typename HIDXIndexable<O>::List>& pi_rpObjects) const
    {
    // Take a copy of the list of objects to prepare (sorted)
    // IMPORTANT: We remove the "const" from the subset because we know that
    // we never modify it. Our QueryIndexables returns a new list that contains
    // the subset elements, sorted.
    HAutoPtr<typename HIDXIndexable<O>::List> pObjectsToPrepare(QueryIndexables(HIDXSearchCriteria(), const_cast<HAutoPtr<typename HIDXIndexable<O>::List>&>(pi_rpObjects)));

    // Remove objects that are already calculated.
    typename HIDXIndexable<O>::List::iterator Itr(pObjectsToPrepare->begin());
    typename HIDXIndexable<O>::List::iterator TempItr;
    Attribute* pCurrentAttribute;
    while (Itr != pObjectsToPrepare->end())
        {
        pCurrentAttribute = (Attribute*) (*Itr)->GetAttribute(this);
        if (pCurrentAttribute->GetShape() != 0)
            {
            TempItr = Itr;
            ++Itr;
            pObjectsToPrepare->erase(TempItr);
            }
        else
            {
            ++Itr;
            }
        }

    HAutoPtr<typename HIDXIndexable<O>::List> pInteractingObjects;

    if (m_pSubIndex && m_pSubIndex->SupportsInteractingRetrieval())
        {
        // Retrieve the interacting objects, sorted...
        pInteractingObjects =
            QueryIndexables(HIDXSearchCriteria(), m_pSubIndex->GetInteractingObjects(*pObjectsToPrepare));
        }
    else
        {
        // Can't rely on the sub-index. All our objects
        // potentially interact...
        pInteractingObjects = QueryIndexables(HIDXSearchCriteria());
        }

    // Pass through all interacting objects starting at the end (top of stack).
    // Calculate ToRemove while going. When we find the first element of the
    // ObjectsToPrepare (starting at the end), clip its GetRegion() and save
    // the calculated shape.

    // Calculate the hiding shape
    typename HIDXIndexable<O>::List::reverse_iterator InteractingItr(pInteractingObjects->rbegin());
    typename HIDXIndexable<O>::List::reverse_iterator ToCalculateItr(pObjectsToPrepare->rbegin());
    HFCPtr<HVEShape> pToRemove;

    while (ToCalculateItr != pObjectsToPrepare->rend())
        {
        if ((*InteractingItr)->IndexesSameObjectAs(*(*ToCalculateItr)))
            {
            Attribute* pAttribute = (Attribute*) (*ToCalculateItr)->GetAttribute(this);

            // Set the visible shape of the object
            pAttribute->SetShape(new HVEShape(*(*ToCalculateItr)->GetObject()->GetEffectiveShape()));
            if (pToRemove != 0)
                {
                // There were objects over it. Clip the visible shape
                pAttribute->GetShape()->Differentiate(*pToRemove);
                }

            ++ToCalculateItr;
            }

        if ((*InteractingItr)->GetObject()->IsOpaque())
            {
            if (pToRemove == 0)
                pToRemove = new HVEShape(*(*InteractingItr)->GetObject()->GetEffectiveShape());
            else
                pToRemove->Unify(*(*InteractingItr)->GetObject()->GetEffectiveShape());
            }

        // Always advance in the interacting objects.
        ++InteractingItr;
        }
    }


//-----------------------------------------------------------------------------
// Retrieve an object's visible surface
//-----------------------------------------------------------------------------
template<class O, class SI> const HFCPtr<HVEShape>& HVEVSRelativeIndex<O, SI>::GetVisibleSurfaceOf(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const
    {
    Attribute* pAttribute = (Attribute*) pi_rpObject->GetAttribute(this);

    if (pAttribute->GetShape() == 0)
        {
        // We need to calculate the shape

        HAutoPtr<typename HIDXIndexable<O>::List> pInteractingObjects;

        if (m_pSubIndex && m_pSubIndex->SupportsInteractingRetrieval())
            {
            // Retrieve objects that possibly hide the calculated one.
            // We want them sorted!
            HAutoPtr<typename HIDXIndexable<O>::List> pObjects(new typename HIDXIndexable<O>::List);
            pObjects->push_back(pi_rpObject);
            HAutoPtr<typename HIDXIndexable<O>::List> pTempInteractingObjects(m_pSubIndex->GetInteractingObjects(*pObjects));
            pInteractingObjects = QueryIndexables(HIDXSearchCriteria(),
                                                  pTempInteractingObjects,
                                                  true);
            }
        else
            {
            // All our objects potentially hide the specified object
            pInteractingObjects = QueryIndexables(HIDXSearchCriteria());
            }

        // Calculate the hiding shape
        typename HIDXIndexable<O>::List::reverse_iterator Itr(pInteractingObjects->rbegin());
        HFCPtr<HVEShape> pVisibleShape(new HVEShape(*pi_rpObject->GetObject()->GetEffectiveShape()));

        while ((Itr != pInteractingObjects->rend()) &&
               ((*Itr)->IndexesSameObjectAs(*pi_rpObject) == false) &&
               (pVisibleShape->IsEmpty() == false))
            {
            if ((*Itr)->GetObject()->IsOpaque())
                {
                pVisibleShape->Differentiate(*((*Itr)->GetObject()->GetEffectiveShape()));
                }

            ++Itr;
            }

        // By default, the visible shape is the object's fence
        pAttribute->SetShape(pVisibleShape);
        }

    return pAttribute->GetShape();
    }


//-----------------------------------------------------------------------------
// Retrieve an object's visible surface
//-----------------------------------------------------------------------------
template<class O, class SI> const HFCPtr<HVEShape>& HVEVSRelativeIndex<O, SI>::GetVisibleSurfaceOf(const O pi_rObject) const
    {
    return GetVisibleSurfaceOf(GetFilledIndexableFor(new HIDXIndexable<O>(pi_rObject)));
    }


//-----------------------------------------------------------------------------
// Invalidate calculated visible surfaces...
//-----------------------------------------------------------------------------
template<class O, class SI> void HVEVSRelativeIndex<O, SI>::Invalidate(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject,
    bool                             pi_UseInteractingObjects)
    {
    HAutoPtr<typename HIDXIndexable<O>::List> pInteractingObjects;

    if (pi_UseInteractingObjects && m_pSubIndex && m_pSubIndex->SupportsInteractingRetrieval())
        {
        // Retrieve objects that are touched by the added one
        // We want them sorted!
        HAutoPtr<typename HIDXIndexable<O>::List> pObjects(new typename HIDXIndexable<O>::List);
        pObjects->push_back(pi_rpObject);
        HAutoPtr<typename HIDXIndexable<O>::List> pSubSet(m_pSubIndex->GetInteractingObjects(*pObjects));
        pInteractingObjects = QueryIndexables(HIDXSearchCriteria(), pSubSet, true);
        }
    else
        {
        // Retrieve all objects
        pInteractingObjects = QueryIndexables(HIDXSearchCriteria());

        // Remove all objects over the one to invalidate
        while (pInteractingObjects->size() > 0 && !pInteractingObjects->back()->IndexesSameObjectAs(*pi_rpObject))
            pInteractingObjects->pop_back();
        }

    // Invalidate their calculated region
    typename HIDXIndexable<O>::List::const_iterator Itr(pInteractingObjects->begin());
    while (Itr != pInteractingObjects->end())
        {
        ((Attribute*)(*Itr)->GetAttribute(this))->ClearShape();

        ++Itr;
        }
    }


//-----------------------------------------------------------------------------
// Invalidate calculated visible surfaces...
//-----------------------------------------------------------------------------
template<class O, class SI> void HVEVSRelativeIndex<O, SI>::Invalidate(
    const HAutoPtr<typename HIDXIndexable<O>::List>& pi_rObjects,
    bool                                   pi_UseInteractingObjects)
    {
    HAutoPtr<typename HIDXIndexable<O>::List> pInteractingObjects;

    if (pi_UseInteractingObjects && m_pSubIndex && m_pSubIndex->SupportsInteractingRetrieval())
        {
        // Retrieve objects that are touched by the added one We want them sorted!
        HAutoPtr<typename HIDXIndexable<O>::List> pSubSet(m_pSubIndex->GetInteractingObjects(*pi_rObjects));
        pInteractingObjects = QueryIndexables(HIDXSearchCriteria(), pSubSet, true);
        }
    else
        {
        // Retrieve all objects (slow)
        pInteractingObjects = QueryIndexables(HIDXSearchCriteria());
        }

    // Invalidate their calculated region
    typename HIDXIndexable<O>::List::const_iterator Itr(pInteractingObjects->begin());
    while (Itr != pInteractingObjects->end())
        {
        ((Attribute*)(*Itr)->GetAttribute(this))->ClearShape();

        ++Itr;
        }
    }
END_IMAGEPP_NAMESPACE
