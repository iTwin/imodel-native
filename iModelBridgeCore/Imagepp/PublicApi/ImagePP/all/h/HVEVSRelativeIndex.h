//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEVSRelativeIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVEVSRelativeIndex
//-----------------------------------------------------------------------------
// Relative ordering index that calculates visible surfaces.
//-----------------------------------------------------------------------------

#pragma once


#include "HIDXIndex.h"
#include "HIDXSearchCriteria.h"
#include "HIDXAListRelativeIndex.h"
#include "HVEVSRelativeAttribute.h"


BEGIN_IMAGEPP_NAMESPACE
template <class O, class SI = DefaultSubIndexType<O> > class HVEVSRelativeIndex
    {
public:

    class Parameters
        {
    public:
        Parameters() {};
        ~Parameters() {};
        };

    HVEVSRelativeIndex(const Parameters& pi_rParameters = Parameters(),
                       const SI* pi_pSubIndex = 0);
    ~HVEVSRelativeIndex();

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


    typedef typename HIDXAListRelativeIndex<O, SI>::Predicate  Predicate;

    Predicate       GetPredicate() const;


    // Visible surface operations

    void            CalculateVisibleSurfaces(const HAutoPtr<typename HIDXIndexable<O>::List>& pi_rObjects) const;

    const HFCPtr<HVEShape>&
    GetVisibleSurfaceOf(const HFCPtr< HIDXIndexable<O> >& pi_rObject) const;

    const HFCPtr<HVEShape>&
    GetVisibleSurfaceOf(const O pi_rObject) const;

    void            Invalidate(const HFCPtr< HIDXIndexable<O> >& pi_rpObject,
                               bool                             pi_UseInteractingObjects = true);

    void            Invalidate(const HAutoPtr<typename HIDXIndexable<O>::List>& pi_rObjects,
                               bool                                   pi_UseInteractingObjects = true);

private:

    // Copy ctor and assignment are disabled
    HVEVSRelativeIndex(const HVEVSRelativeIndex<O, SI>& pi_rObj);
    HVEVSRelativeIndex<O, SI>& operator=(const HVEVSRelativeIndex<O, SI>& pi_rObj);

    typedef HVEVSRelativeAttribute Attribute;

    const SI*       m_pSubIndex;

    //////////////////////////////////
    //////////////////////////////////
    //
    // CAUTION: It is very important that there is at least one member declared
    //          before m_Index. The reason is that we use the address of an
    //          index when dealing with attributes in an Indexable. Since this
    //          class does not have a vtable, the address of its first member is
    //          the same as the address of the index. Therefore, there is a problem
    //          when this index and its internal index each try to add an attribute
    //          using their address.
    //          The ANSI C++ standard specifies that the order of declaration of
    //          members defines their address in memory: if member B is declared
    //          after member A, member B's address will be bigger than member A's.
    //
    //////////////////////////////////
    //////////////////////////////////

    // The index we use internally to simulate inheritance from it.
    HIDXAListRelativeIndex<O, SI> m_Index;
    };
END_IMAGEPP_NAMESPACE


#include "HVEVSRelativeIndex.hpp"


