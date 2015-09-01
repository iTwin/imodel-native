//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXComplexIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXComplexIndex
//-----------------------------------------------------------------------------
// Relative ordering index using an array list
//-----------------------------------------------------------------------------

#pragma once


#include "HIDXIndex.h"
#include "HIDXSearchCriteria.h"

BEGIN_IMAGEPP_NAMESPACE
///////////////////////////
// HIDXComplexParameters
///////////////////////////

template <class P1, class P2> class HIDXComplexParameters
    {
public:

    HIDXComplexParameters(const P1& pi_rParameters1,
                          const P2& pi_rParameters2)
        : m_Parameters1(pi_rParameters1),
          m_Parameters2(pi_rParameters2)
        {
        };
    HIDXComplexParameters() {};

    ~HIDXComplexParameters() {};

    P1  m_Parameters1;
    P2  m_Parameters2;
    };


///////////////////////////
// HIDXComplexIndex
///////////////////////////

template <class O, class I1, class I2, class SI = DefaultSubIndexType<O> > class HIDXComplexIndex
    {
public:

    typedef HIDXComplexParameters<typename I1::Parameters, typename I2::Parameters> Parameters;

    HIDXComplexIndex(const Parameters& pi_rParameters = Parameters(),
                     SI* pi_pSubIndex = 0);
    ~HIDXComplexIndex();

    I1*             GetIndex1() const;
    I2*             GetIndex2() const;

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


    // Information retrieval

    ObjectList*
    Query(const HIDXSearchCriteria& pi_rCriteria) const;

    typename HIDXIndexable<O>::List*
    QueryIndexables(const HIDXSearchCriteria&           pi_rCriteria,
                    HAutoPtr< typename HIDXIndexable<O>::List >& pi_rSubset,
                    bool                               pi_Sort) const;
    
    typename HIDXIndexable<O>::List*
    QueryIndexables(const HIDXSearchCriteria& pi_rCriteria,
                    bool                     pi_Sort) const;

    bool           SupportsInteractingRetrieval() const;

    typename HIDXIndexable<O>::List*
    GetInteractingObjects(typename HIDXIndexable<O>::List const&pi_rpObjects) const;

    const HFCPtr< HIDXIndexable<O> >
    GetFilledIndexableFor(const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const;

    HIDXSortingRequirement
    GetSortingRequirement() const;

    // Object used to sort STL lists...
    class Predicate : public binary_function<HFCPtr< HIDXIndexable<O> >, HFCPtr< HIDXIndexable<O> >, bool>
        {
    public:

        Predicate(typename I1::Predicate& pi_Pred1, typename I2::Predicate& pi_Pred2)
            : m_Pred1(pi_Pred1),
              m_Pred2(pi_Pred2)
            {
            };

        bool operator()(HFCPtr< HIDXIndexable<O> > pi_rpFirst, HFCPtr< HIDXIndexable<O> > pi_rpSecond)
            {
            // Try innermost index first. If equal [pred(a,b) && pred(b,a)],
            // use outermost index.
            register bool Pred2Result = m_Pred2(pi_rpFirst, pi_rpSecond);
            if (Pred2Result && m_Pred2(pi_rpSecond, pi_rpFirst))
                return m_Pred1(pi_rpFirst, pi_rpSecond);
            else
                return Pred2Result;
            };

    private:

        typename I1::Predicate m_Pred1;
        typename I2::Predicate m_Pred2;
        };

    Predicate       GetPredicate() const;

private:

    // Copy ctor and assignment are disabled
    HIDXComplexIndex(const HIDXComplexIndex<O, I1, I2, SI>& pi_rObj);
    HIDXComplexIndex<O, I1, I2, SI>& operator=(const HIDXComplexIndex<O, I1, I2, SI>& pi_rObj);

    // Pointer to the two indexes
    I1*             m_pIndex1;
    I2*             m_pIndex2;
    };

END_IMAGEPP_NAMESPACE

#include "HIDXComplexIndex.hpp"


