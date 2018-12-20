//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXAListRelativeIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXAListRelativeIndex
//-----------------------------------------------------------------------------
// Relative ordering index using an array list
//-----------------------------------------------------------------------------

#pragma once


#include "HIDXIndex.h"
#include "HIDXAList.h"
#include "HIDXSearchCriteria.h"
#include "HIDXAListRelativeAttribute.h"

BEGIN_IMAGEPP_NAMESPACE

template <class O, class SI = DefaultSubIndexType<O> > class HIDXAListRelativeIndex
    {
public:

    class Parameters
        {
    public:
        Parameters() {};
        ~Parameters() {};
        };

    HIDXAListRelativeIndex(const Parameters& pi_rParameters = Parameters(),
                           const SI* pi_pSubIndex = 0);
    ~HIDXAListRelativeIndex();

    // Define a type for a list of indexables.
    typedef list < O, allocator< O > >  ObjectList;
    typedef typename HIDXIndexable<O>::List      IndexableList;

    // Management

    void            Add(const O pi_Object);
    void            Add(const ObjectList& pi_rObjects);
    void            Remove(const O pi_Object);

    void            AddIndexable(const HFCPtr< HIDXIndexable<O> >& pi_rpObject);
    void            AddIndexables(const HAutoPtr< typename HIDXIndexable<O>::List >& pi_rpObjects);
    void            RemoveIndexable(const HFCPtr< HIDXIndexable<O> >& pi_rpObject);

    void            Promote(const O pi_Object);
    void            Demote(const O pi_Object);
    void            Front(const O pi_Object);
    void            Back(const O pi_Object);

    // Information retrieval

    ObjectList*
    Query(const HIDXSearchCriteria& pi_rCriteria) const;

    typename HIDXIndexable<O>::List*
    QueryIndexables(const HIDXSearchCriteria&           pi_rCriteria,
                    HAutoPtr< typename HIDXIndexable<O>::List >& pi_rSubset,
                    bool                               pi_Sort = true) const;

    typename HIDXIndexable<O>::List*
    QueryIndexables(const HIDXSearchCriteria& pi_rCriteria,
                    bool                     pi_Sort = true) const;

    bool           SupportsInteractingRetrieval() const;

    typename HIDXIndexable<O>::List*
    GetInteractingObjects(typename HIDXIndexable<O>::List const& pi_rpObjects) const;

    const HFCPtr< HIDXIndexable<O> >
    GetFilledIndexableFor(const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const;

    HIDXSortingRequirement
    GetSortingRequirement() const;

    enum
        {
        // This MUST be a multiple of 32 for the sorting algorithm
        // in QueryIndexables to work properly. Otherwise, we'd need
        // to check if we have the good block at each item instead of
        // checking for each 32 values (a long).
        BLOCK_CAPACITY = 64
        };

    typedef HIDXAListRelativeAttribute<O, BLOCK_CAPACITY> Attribute;


    // Object used to sort STL lists...
    class Predicate : public binary_function<HFCPtr< HIDXIndexable<O> >, HFCPtr< HIDXIndexable<O> >, bool>
        {
    public:

        Predicate(void const* pi_Index) {
            m_Index = pi_Index;
            };

        bool operator()(HFCPtr< HIDXIndexable<O> > pi_rpFirst, HFCPtr< HIDXIndexable<O> > pi_rpSecond)
            {
            // Compare their absolute positions
            return ((Attribute*)pi_rpFirst->GetAttribute(m_Index))->GetPosition() <  ((Attribute*)pi_rpSecond->GetAttribute(m_Index))->GetPosition();
            };

    private:

        // The address of the index, used when querying attributes
        void const*     m_Index;
        };


    Predicate       GetPredicate() const;

private:

    // Copy ctor and assignment are disabled
    HIDXAListRelativeIndex(const HIDXAListRelativeIndex<O, SI>& pi_rObj);
    HIDXAListRelativeIndex<O, SI>& operator=(const HIDXAListRelativeIndex<O, SI>& pi_rObj);

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const;
#endif

    void            TryToMakeSpaceAtTheEnd();
    void            TryToMakeSpaceAtTheBeginning();

    // The list of indexables
    HIDXAList<O, BLOCK_CAPACITY>
    m_List;

    // Pointer to the underlying index
    const SI*       m_pSubIndex;
    };

END_IMAGEPP_NAMESPACE

#include "HIDXAListRelativeIndex.hpp"


