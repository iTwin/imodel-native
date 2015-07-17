/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/PluginRegistryHelper.h $
|    $RCSfile: PluginRegistryHelper.h,v $
|   $Revision: 1.4 $
|       $Date: 2012/02/16 00:37:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PluginCreatorT>
class PluginRegistry
    {
public:
    typedef PluginCreatorT          CreatorPlugin;
    typedef typename PluginCreatorT::ID      
                                    CreatorPluginID;

    
    typedef std::vector<CreatorPluginID>      
                                    CreatorIDList;

    typedef std::vector<const PluginCreatorT>   
                                    CreatorList;
    typedef std::pair<typename CreatorList::const_iterator, typename CreatorList::const_iterator> 
                                    CreatorListRange;


    typedef PluginRegistry<PluginCreatorT>
                                    super_class;

    
    const CreatorList&              GetCreators                    () const; 



    CreatorPluginID                 Register                       (const CreatorPlugin&            creator);
    void                            Unregister                     (CreatorPluginID                 id);

    const CreatorList&              GetUnregistered                () const
        { 
        return m_unregisteredCreators; 
        }

    template <typename T, typename StrictWeakOrdering>
    CreatorListRange                FindCreatorsFor                (const T&                        value,
                                                                    StrictWeakOrdering              ordering) const
        {
        if (HasPostponedUnregister())
            RunPostponedUnregister();

        if (!m_creatorsSorted)
            {
            std::sort(m_creators.begin(), m_creators.end(), ordering);
            m_creatorsSorted = true;
            }

        return equal_range(m_creators.begin(), m_creators.end(), value, ordering);
        }

    template <typename T, typename StrictWeakOrdering>
    const CreatorPlugin*            FindFirstCreatorFor            (const T&                        value,
                                                                    StrictWeakOrdering              ordering) const
        {
        CreatorListRange foundRange = FindCreatorsFor(value, ordering);

        if (foundRange.first == foundRange.second)
            return 0;

        assert (1 == distance(foundRange.first, foundRange.second));
        return &*foundRange.first;
        }

    template <typename Predicate>
    const CreatorPlugin*            BruteForceFindCreator          (Predicate                       predicate) const
        {
        if (HasPostponedUnregister())
            RunPostponedUnregister();

        CreatorList::const_iterator foundIt = find_if(m_creators.begin(), m_creators.end(), predicate);

        return (foundIt != m_creators.end() ? &*foundIt : 0);
        }



    template <typename CreatorIDList, typename CreatorList, typename CreatorLessThan>
    static void                     UnregisterCreators             (CreatorIDList&                  creatorsToRemove, 
                                                                    CreatorList&                    creators, 
                                                                    CreatorLessThan                 lessThan)
        {
        assert(!creatorsToRemove.empty());

        std::sort (creatorsToRemove.begin(), creatorsToRemove.end());
        assert(!IncludesDuplicates(creatorsToRemove.begin(), creatorsToRemove.end(), std::less<CreatorPluginID>()));

        typedef typename CreatorList::value_type CreatorType;

        creators.erase(std::remove_if(creators.begin(), creators.end(), 
                                          IsIncludedInSortedSelection<CreatorType>(creatorsToRemove.begin(), creatorsToRemove.end(), 
                                                                                   lessThan)),
                           creators.end());

        }

    template <typename SortedSetItT, typename LessThanPredT>
    static bool                     IncludesDuplicates             (SortedSetItT            setBegin,
                                                                    SortedSetItT            setEnd,
                                                                    LessThanPredT           lessThanPred)
        {
        // Validate that there are no duplicates (as the range is sorted, this is equivalent 
        // to validating that no adjacent value "not less than"/"equal" can be found)
        return setEnd != std::adjacent_find(setBegin, setEnd, not2(lessThanPred));
        }



protected:
    explicit                        PluginRegistry                 ()
        :   m_creatorsSorted(true)
        {

        }


private:
    struct CompareLessCreatorsWithID : std::binary_function<CreatorPlugin, CreatorPlugin, bool>
        {
        
        bool operator () (const CreatorPlugin& lhs, const CreatorPlugin& rhs) const { return lhs.GetID() < rhs.GetID(); }
        bool operator () (const CreatorPluginID& lhs, const CreatorPlugin& rhs) const { return lhs < rhs.GetID(); }
        bool operator () (const CreatorPlugin& lhs, const CreatorPluginID& rhs) const { return lhs.GetID() < rhs; }
        };

    template <typename EntryType, typename SortedSelectionItT, typename LessThanPredT>
    struct IsIncludedInSortedSelection_t : std::unary_function<EntryType, bool>
        {
        SortedSelectionItT          m_selectionBegin;
        SortedSelectionItT          m_selectionEnd;
        LessThanPredT               m_lessThanPred;

        explicit                    IsIncludedInSortedSelection_t  (SortedSelectionItT      selectionBegin,
                                                                    SortedSelectionItT      selectionEnd,
                                                                    LessThanPredT           lessThanPred) 
            : m_selectionBegin(selectionBegin), m_selectionEnd(selectionEnd), m_lessThanPred(lessThanPred) {}

        bool                        operator()                     (const EntryType&        rhs) const
            {
            return std::binary_search(m_selectionBegin, m_selectionEnd, rhs, m_lessThanPred);
            }
        };

    template <typename EntryType, typename SortedSelectionItT, typename LessThanPredT>
    static IsIncludedInSortedSelection_t<EntryType, SortedSelectionItT, LessThanPredT>
                                    IsIncludedInSortedSelection    (SortedSelectionItT      selectionBegin,
                                                                    SortedSelectionItT      selectionEnd,
                                                                    LessThanPredT           lessThanPred)
        {
        return IsIncludedInSortedSelection_t<EntryType, SortedSelectionItT, LessThanPredT>(selectionBegin, selectionEnd, lessThanPred);
        }

    bool                            HasPostponedUnregister         () const { return !m_unregisteredCreators.empty(); }
    void                            RunPostponedUnregister         () const;


    CreatorList                     mutable m_creators;
    bool                            mutable m_creatorsSorted;
    
    CreatorIDList                   mutable m_unregisteredCreators;
    };
    
#include "PluginRegistryHelper.hpp"

END_BENTLEY_MRDTM_IMPORT_NAMESPACE

