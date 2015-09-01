//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXComplexIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXComplexIndex
//-----------------------------------------------------------------------------
// Complex indexes.
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
HIDXComplexIndex<O, I1, I2, SI>::HIDXComplexIndex(const Parameters& pi_rParameters,
                                                  SI*               pi_pSubIndex)
    {
    m_pIndex2 = new I2(pi_rParameters.m_Parameters2, pi_pSubIndex);
    m_pIndex1 = new I1(pi_rParameters.m_Parameters1, m_pIndex2);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
HIDXComplexIndex<O, I1, I2, SI>::~HIDXComplexIndex()
    {
    delete m_pIndex1;
    delete m_pIndex2;
    }


//-----------------------------------------------------------------------------
// Retrieve the first index
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
I1* HIDXComplexIndex<O, I1, I2, SI>::GetIndex1() const
    {
    return m_pIndex1;
    }


//-----------------------------------------------------------------------------
// Retrieve the second index
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
I2* HIDXComplexIndex<O, I1, I2, SI>::GetIndex2() const
    {
    return m_pIndex2;
    }


//-----------------------------------------------------------------------------
// Add an object
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
void HIDXComplexIndex<O, I1, I2, SI>::Add(const O pi_Object)
    {
    // Add in our two indexes
    HFCPtr< HIDXIndexable<O> > pObject(new HIDXIndexable<O>(pi_Object));

    m_pIndex2->AddIndexable(pObject);
    m_pIndex1->AddIndexable(pObject);
    }


//-----------------------------------------------------------------------------
// Add an object
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
void HIDXComplexIndex<O, I1, I2, SI>::Add(const ObjectList& pi_rObjects)
    {
    // Add in our two indexes
    HAutoPtr<typename HIDXIndexable<O>::List> pObjects(new typename HIDXIndexable<O>::List);

    typename ObjectList::const_iterator Itr(pi_rObjects.begin());
    while (Itr != pi_rObjects.end())
        {
        pObjects->push_back(new HIDXIndexable<O>(*Itr));

        ++Itr;
        }

    m_pIndex2->AddIndexables(pObjects);
    m_pIndex1->AddIndexables(pObjects);
    }


//-----------------------------------------------------------------------------
// Remove an object
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
void HIDXComplexIndex<O, I1, I2, SI>::Remove(const O pi_Object)
    {
    HFCPtr< HIDXIndexable<O> > pObject(new HIDXIndexable<O>(pi_Object));

    // Remove in our two indexables
    m_pIndex1->RemoveIndexable(pObject);
    m_pIndex2->RemoveIndexable(pObject);
    }

//-----------------------------------------------------------------------------
// Add an element
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI>
void HIDXComplexIndex<O, I1, I2, SI>::AddIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    // Make sure we've got the real indexable
    HFCPtr< HIDXIndexable<O> > pObject(pi_rpObject);
    if (pObject->GetAttribute((uint32_t)this) == 0)
        pObject = GetFilledIndexableFor(pObject);

    // Add in our two indexes
    m_pIndex2->AddIndexable(pObject);
    m_pIndex1->AddIndexable(pObject);
    }


//-----------------------------------------------------------------------------
// Add elements
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI>
void HIDXComplexIndex<O, I1, I2, SI>::AddIndexables(
    const HAutoPtr< typename HIDXIndexable<O>::List >& pi_rpObjects)
    {
    // Add in our two indexes
    m_pIndex2->AddIndexables(pi_rpObjects);
    m_pIndex1->AddIndexables(pi_rpObjects);
    }


//-----------------------------------------------------------------------------
// Remove an element
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI>
void HIDXComplexIndex<O, I1, I2, SI>::RemoveIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    // Make sure we've got the real indexable
    HFCPtr< HIDXIndexable<O> > pObject(pi_rpObject);
    if (pObject->GetAttribute((uint32_t)this) == 0)
        pObject = GetFilledIndexableFor(pObject);

    // Remove in our two indexables
    m_pIndex1->RemoveIndexable(pObject);
    m_pIndex2->RemoveIndexable(pObject);
    }



//-----------------------------------------------------------------------------
// Query
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI>
typename HIDXComplexIndex<O, I1, I2, SI>::ObjectList* HIDXComplexIndex<O, I1, I2, SI>::Query(
    const HIDXSearchCriteria& pi_rCriteria) const
    {
    // Since we don't support criterias, we return all the objects, sorted...
    HAutoPtr<ObjectList> pList(new ObjectList);

    // Do a combined search
    HAutoPtr<typename HIDXIndexable<O>::List> pResult(
        m_pIndex1->QueryIndexables(pi_rCriteria, m_pIndex2->QueryIndexables(pi_rCriteria)));

    // Copy elements to the object list
    typename HIDXIndexable<O>::List::const_iterator Itr(pResult->begin());
    while (Itr != pResult->end())
        {
        pList->push_back((*Itr)->GetObject());

        ++Itr;
        }

    return pList.release();
    }


//-----------------------------------------------------------------------------
// Internal query
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
typename HIDXIndexable<O>::List* HIDXComplexIndex<O, I1, I2, SI>::QueryIndexables(
    const HIDXSearchCriteria& pi_rCriteria,
    bool                     pi_Sort) const
    {
    // Can we let the internal indexes sort by themselves?
    bool AutoSort = pi_Sort && (GetSortingRequirement() != SORTING_COMPLEX);

    // Do a combined search
    HAutoPtr<typename HIDXIndexable<O>::List> pResultPre(m_pIndex2->QueryIndexables(pi_rCriteria, AutoSort));
    HAutoPtr<typename HIDXIndexable<O>::List> pResult(m_pIndex1->QueryIndexables(pi_rCriteria, pResultPre, AutoSort));

    if (pi_Sort && !AutoSort)
        {
        // We have to sort the queried elements

        // HChk MR
#if 0
        // Damn. The sort method on the list type doesn't
        // handle a predicate on version MS C++ 5.0
        (*pResult).sort(GetPredicate());
#else
        typedef vector< HFCPtr< HIDXIndexable<O> > > SortVector;
        SortVector TempVector;
        TempVector.reserve(pResult->size());
        typename IndexableList::const_iterator Itr(pResult->begin());
        while (Itr != pResult->end())
            {
            TempVector.push_back(*Itr);
            ++Itr;
            }
        sort(TempVector.begin(), TempVector.end(), GetPredicate());
        pResult->clear();
        typename SortVector::const_iterator SortItr(TempVector.begin());
        while (SortItr != TempVector.end())
            {
            pResult->push_back(*SortItr);
            ++SortItr;
            }
#endif
        }

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Internal query
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
typename HIDXIndexable<O>::List* HIDXComplexIndex<O, I1, I2, SI>::QueryIndexables(
    const HIDXSearchCriteria&         pi_rCriteria,
    HAutoPtr<typename HIDXIndexable<O>::List>& pi_rSubset,
    bool                             pi_Sort) const
    {
    // Can we let the internal indexes sort by themselves?
    bool AutoSort = pi_Sort && (GetSortingRequirement() != SORTING_COMPLEX);

    // Do a combined search
    HAutoPtr<typename HIDXIndexable<O>::List> pResult = m_pIndex1->QueryIndexables(pi_rCriteria, m_pIndex2->QueryIndexables(pi_rCriteria, pi_rSubset, AutoSort), AutoSort);

    if (pi_Sort && !AutoSort)
        {
        // We have to sort the queried elements
#if 0
        // Damn. The sort method on the list type doesn't
        // handle a predicate on version MS C++ 5.0
//        (*pResult).sort(GetPredicate());
#else
        typedef vector< HFCPtr< HIDXIndexable<O> > > SortVector;
        SortVector TempVector;
        TempVector.reserve(pResult->size());
        typename IndexableList::const_iterator Itr(pResult->begin());
        while (Itr != pResult->end())
            {
            TempVector.push_back(*Itr);
            ++Itr;
            }
        sort(TempVector.begin(), TempVector.end(), GetPredicate());
        pResult->clear();
        typename SortVector::const_iterator SortItr(TempVector.begin());
        while (SortItr != TempVector.end())
            {
            pResult->push_back(*SortItr);
            ++SortItr;
            }
#endif
        }

    return pResult.release();
    }


//-----------------------------------------------------------------------------
// Retrieve the internal indexable for the specified object
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
const HFCPtr< HIDXIndexable<O> > HIDXComplexIndex<O, I1, I2, SI>::GetFilledIndexableFor(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const
    {
    // Ask our topmost index. They are all linked together, so index2 will
    // automatically be queried if necessary
    return m_pIndex1->GetFilledIndexableFor(pi_rpObject);
    }


//-----------------------------------------------------------------------------
// This index can't retrieve objects interacting with a specific one
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
bool HIDXComplexIndex<O, I1, I2, SI>::SupportsInteractingRetrieval() const
    {
    return m_pIndex1->SupportsInteractingRetrieval() || m_pIndex2->SupportsInteractingRetrieval();
    }


//-----------------------------------------------------------------------------
// Retrieve objects interacting with the specified one.
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
typename HIDXIndexable<O>::List* HIDXComplexIndex<O, I1, I2, SI>::GetInteractingObjects(
    typename HIDXIndexable<O>::List const& pi_rpObjects) const
    {
    // 2 returns for performance
    if (m_pIndex1->SupportsInteractingRetrieval())
        return m_pIndex1->GetInteractingObjects(pi_rpObjects);
    else
        return m_pIndex2->GetInteractingObjects(pi_rpObjects);
    }


//-----------------------------------------------------------------------------
// Ask the index if it needs to perform some sorting
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI>
HIDXSortingRequirement HIDXComplexIndex<O, I1, I2, SI>::GetSortingRequirement() const
    {
    if (m_pIndex1->GetSortingRequirement() == SORTING_COMPLEX)
        {
        return SORTING_COMPLEX;
        }
    else
        {
        if (m_pIndex1->GetSortingRequirement() == SORTING_SIMPLE)
            {
            if (m_pIndex2->GetSortingRequirement() != SORTING_NONE)
                return SORTING_COMPLEX;
            else
                return SORTING_SIMPLE;
            }
        else
            {
            // Index1 has SORTING_NONE, so only Index2 is useful

            return m_pIndex2->GetSortingRequirement();
            }
        }
    }


//-----------------------------------------------------------------------------
// Get the index's sorting function
//-----------------------------------------------------------------------------
template<class O, class I1, class I2, class SI> inline
typename HIDXComplexIndex<O, I1, I2, SI>::Predicate HIDXComplexIndex<O, I1, I2, SI>::GetPredicate() const
    {
    typename I1::Predicate predicate1 = m_pIndex1->GetPredicate();
    typename I2::Predicate predicate2 = m_pIndex2->GetPredicate();
    return Predicate(predicate1, predicate2);
    }
END_IMAGEPP_NAMESPACE