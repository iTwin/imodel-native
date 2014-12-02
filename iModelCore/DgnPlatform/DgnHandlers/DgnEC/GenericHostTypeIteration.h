/*--------------------------------------------------------------------------------------+ 
|
|     $Source: DgnHandlers/DgnEC/GenericHostTypeIteration.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include    <DgnPlatform/DgnHandlers/FlatteningCollection.h>
#pragma once
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//An iteration of find instances involves the following nesting
//1. An iteration of providers
//2. An iteration of finders
//3. An iteration of scope objects for each finder
//
// For simplicity the first two iterations are not lazy and we precompute the finder list
// Since usually the number of scope objects > number of finders we iterate scope objects.
// For each scope object apply the finder.

//A scope object +  a finder transformation -> A collection of dgnecinstances for the specific combination

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename HostType>
struct BaseHostInstanceFinder : public IDgnECInstanceFinder
    {
    private:
        WhereCriterionPtr m_whereCriteria;
    public:
    virtual StatusInt    _FindInstances (DgnECInstanceVector& results, HostType hostInstance) const = 0;
    typedef RefCountedPtr<BaseHostInstanceFinder>   T_Ptr;
    typedef bvector <T_Ptr>                         T_PtrVector;
    BaseHostInstanceFinder (IECPropertyValueFilterPtr filter, WhereCriterionPtr whereCriteria)
        :IDgnECInstanceFinder (filter), m_whereCriteria(whereCriteria)
        {}

    bool AcceptInstance (DgnECInstanceR instance) const
        {
        if (m_whereCriteria.IsNull())
            return true;

        return m_whereCriteria->Accept (instance);
        }
    };

struct RefcountedInstanceVector;
typedef RefCountedPtr<RefcountedInstanceVector> RefcountedInstanceVectorPtr;

struct RefcountedInstanceVector : public RefCountedBase
    {
    DgnECInstanceVector                         m_instanceList;
    
    template <typename HostType>
    static RefcountedInstanceVectorPtr Create (BaseHostInstanceFinder<HostType> const &finder, HostType const& host)
        {
        RefcountedInstanceVector* vector = new RefcountedInstanceVector();
        StatusInt status = finder._FindInstances (vector->m_instanceList, host);
        BeAssert (SUCCESS == status);
        if (vector->m_instanceList.empty())
            {
            delete vector; vector = NULL;
            }
        return vector;
        }
    
    DgnECInstanceVector::const_iterator begin () const {return m_instanceList.begin();}
    DgnECInstanceVector::const_iterator end () const {return m_instanceList.end();}
    ~RefcountedInstanceVector ()
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename HostType>
struct HostType_Finder_Adapter
    {
    
    private:
    RefcountedInstanceVectorPtr                         m_instanceList;
    typename BaseHostInstanceFinder<HostType>::T_Ptr    m_finder;
    HostType const*                                     m_hostPointer;
#if defined (DEBUG)
    mutable int                                         m_useCount;
#endif
    
    struct RefCountedPointerIterator : public boost::iterator_facade <RefCountedPointerIterator,DgnECInstanceP const, boost::forward_traversal_tag>
        {
        friend class boost::iterator_core_access;
        DgnECInstanceP                      m_currentDgnECInstance;
        HostType_Finder_Adapter const*      m_hostType_Finder_AdapterParent;
        DgnECInstanceVector::const_iterator m_instanceVectorIter;

        RefCountedPointerIterator (HostType_Finder_Adapter const& hostType_Finder_Adapter, bool begin)
            :m_hostType_Finder_AdapterParent(&hostType_Finder_Adapter), m_currentDgnECInstance(NULL)
            {
#if defined (DEBUG)
            ++m_hostType_Finder_AdapterParent->m_useCount;
#endif
            m_instanceVectorIter = (begin)? hostType_Finder_Adapter.m_instanceList->begin() : hostType_Finder_Adapter.m_instanceList->end();
            if (m_instanceVectorIter != hostType_Finder_Adapter.m_instanceList->end())
                m_currentDgnECInstance = m_instanceVectorIter->get();
            }
        RefCountedPointerIterator (RefCountedPointerIterator const& copy)
            :m_currentDgnECInstance (copy.m_currentDgnECInstance), m_hostType_Finder_AdapterParent(copy.m_hostType_Finder_AdapterParent), m_instanceVectorIter(copy.m_instanceVectorIter)
            {
#if defined (DEBUG)
            ++m_hostType_Finder_AdapterParent->m_useCount;
#endif
            }
        
        RefCountedPointerIterator operator = (RefCountedPointerIterator const& copy)
            {
#if defined (DEBUG)
            --m_hostType_Finder_AdapterParent->m_useCount;
#endif
            m_currentDgnECInstance = copy.m_currentDgnECInstance;
            m_instanceVectorIter = copy.m_instanceVectorIter;
            m_hostType_Finder_AdapterParent = copy.m_hostType_Finder_AdapterParent;

#if defined (DEBUG)
            ++m_hostType_Finder_AdapterParent->m_useCount;
#endif
            }

        ~RefCountedPointerIterator ()
            {
#if defined (DEBUG)
            --m_hostType_Finder_AdapterParent->m_useCount;
#endif
            }
       
        void                increment  ()
            {
            BeAssert (NULL != m_hostType_Finder_AdapterParent);
            ++m_instanceVectorIter;
            if (m_instanceVectorIter != m_hostType_Finder_AdapterParent->m_instanceList->end())
                m_currentDgnECInstance = m_instanceVectorIter->get();
            }
        bool    equal (RefCountedPointerIterator const & rhs) const
            {
            return m_instanceVectorIter == rhs.m_instanceVectorIter;
            }

        DgnECInstanceP const&   dereference () const
            {
            return m_currentDgnECInstance;
            }
        };

        HostType_Finder_Adapter (typename BaseHostInstanceFinder<HostType>::T_Ptr finder, HostType const& host)
            :m_finder(finder), m_hostPointer(&host), m_instanceList(NULL)
#if defined (DEBUG)
            , m_useCount(0)
#endif
            {
            }

    public:


        ~HostType_Finder_Adapter ()
            {
#if defined (DEBUG)
            BeAssert (0 == m_useCount);
#endif
            }

    typedef RefCountedPointerIterator const_iterator;
    const_iterator  begin () const
        {
        return const_iterator(*this, true);
        }
    const_iterator  end () const
        {
        return const_iterator(*this, false);
        }

    bool                                        Init ()
        {
        m_instanceList = RefcountedInstanceVector::Create (*m_finder, *m_hostPointer);
        return m_instanceList.IsValid();
        }

    static HostType_Finder_Adapter* Create (typename BaseHostInstanceFinder<HostType>::T_Ptr finder, HostType const& host)
        {
        HostType_Finder_Adapter* adapter = new HostType_Finder_Adapter(finder, host);
        if (!adapter->Init())
            {
            delete adapter; adapter = NULL;
            }
        return adapter;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename HostType>
struct HostType_FinderList_Adapter
    {
    typedef typename BaseHostInstanceFinder<HostType>::T_PtrVector  T_FinderList;
    typedef HostType_Finder_Adapter<HostType>                       T_Collection;
    
    private:
        T_FinderList const* m_findersPointer;
        HostType            m_hostInstance;
       

        struct FinderIterator : public boost::iterator_facade <FinderIterator,T_Collection const, boost::forward_traversal_tag>
            {
            friend class boost::iterator_core_access;
            typename RefCountedCollection<T_Collection>::T_Ptr  m_hostType_Finder_AdapterStatePtr;
            HostType_FinderList_Adapter const*                  m_hostType_FinderList_AdapterParent;
            typename T_FinderList::const_iterator               m_finderListIter;

            FinderIterator (HostType_FinderList_Adapter const& hostType_FinderList_AdapterParent, bool begin)
                :m_hostType_FinderList_AdapterParent(&hostType_FinderList_AdapterParent), m_finderListIter ((begin)? hostType_FinderList_AdapterParent.m_findersPointer->begin() : hostType_FinderList_AdapterParent.m_findersPointer->end()),
                m_hostType_Finder_AdapterStatePtr (NULL)
                {
                if (!begin || m_finderListIter == m_hostType_FinderList_AdapterParent->m_findersPointer->end())
                    return;
            
                T_Collection* collection = T_Collection::Create(*m_finderListIter, m_hostType_FinderList_AdapterParent->m_hostInstance);
                if (NULL != collection)
                    m_hostType_Finder_AdapterStatePtr = RefCountedCollection<T_Collection>::Create (*collection);
                else
                    increment();
                }

            FinderIterator (FinderIterator const& copy)
                :m_finderListIter(copy.m_finderListIter), m_hostType_FinderList_AdapterParent(copy.m_hostType_FinderList_AdapterParent), m_hostType_Finder_AdapterStatePtr(copy.m_hostType_Finder_AdapterStatePtr)
                {
                }

            FinderIterator& operator = (FinderIterator const& copy)
                {
                m_finderListIter = copy.m_finderListIter;
                m_hostType_FinderList_AdapterParent = copy.m_hostType_FinderList_AdapterParent;
                m_hostType_Finder_AdapterStatePtr = copy.m_hostType_Finder_AdapterStatePtr;
                return *this;
                }

            void                increment  ()
                {
                BeAssert (NULL != m_hostType_FinderList_AdapterParent);
                ++m_finderListIter;
                m_hostType_Finder_AdapterStatePtr = NULL;
                if (m_finderListIter != m_hostType_FinderList_AdapterParent->m_findersPointer->end())
                    {
                    T_Collection* collection = T_Collection::Create(*m_finderListIter, m_hostType_FinderList_AdapterParent->m_hostInstance);
                    if (NULL == collection)
                        increment();
                    else
                        m_hostType_Finder_AdapterStatePtr = RefCountedCollection<T_Collection>::Create (*collection);
                    }
                }

            bool    equal (FinderIterator const & rhs) const
                {
                return m_finderListIter == rhs.m_finderListIter;
                }

            T_Collection const&   dereference () const
                {
                return m_hostType_Finder_AdapterStatePtr->GetCollection();
                }

            ~FinderIterator ()
                {
                m_hostType_FinderList_AdapterParent = NULL;
                m_hostType_Finder_AdapterStatePtr = NULL;
                }
            };

    public:

    HostType_FinderList_Adapter (HostType hostInstance, T_FinderList const& finders)
        :m_hostInstance(hostInstance), m_findersPointer(&finders)
        {
        }
    
    typedef FinderIterator const_iterator;
    const_iterator  begin () const
        {
        return const_iterator(*this, true);
        }
    const_iterator  end () const
        {
        return const_iterator(*this, false);
        }

    typedef FlatteningCollection<HostType_FinderList_Adapter<HostType> > FlatCollection;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename HostType, typename HostScope>
struct HostScope_FinderList_Adapter
    {
    typedef HostType_FinderList_Adapter<HostType>                   T_Collection;
    typedef typename T_Collection::FlatCollection                   T_FlatCollection;
    typedef typename BaseHostInstanceFinder<HostType>::T_PtrVector  T_FinderList;
    private:
        HostScope const*    m_scope;
        T_FinderList        m_finders;

        struct ScopeIterator : public boost::iterator_facade <ScopeIterator,T_FlatCollection const, boost::forward_traversal_tag>
            {
            friend class boost::iterator_core_access;
            typename RefCountedCollection<T_FlatCollection>::T_Ptr  m_currentFlatCollectionPtr;
            HostScope_FinderList_Adapter const*                     m_hostScope_FinderList_AdapterParent;
            typename HostScope::const_iterator                      m_scopeIter;

            ScopeIterator (HostScope_FinderList_Adapter const& hostScope_FinderList_AdapterParent, bool begin)
                :m_hostScope_FinderList_AdapterParent(&hostScope_FinderList_AdapterParent), m_scopeIter((begin)? hostScope_FinderList_AdapterParent.m_scope->begin() : hostScope_FinderList_AdapterParent.m_scope->end()),
                m_currentFlatCollectionPtr(NULL)
                {
                if (!begin || m_scopeIter == m_hostScope_FinderList_AdapterParent->m_scope->end())
                    return;

#ifdef DGNV10FORMAT_CHANGES_WIP
                m_currentFlatCollectionPtr = RefCountedCollection<T_FlatCollection>::Create (*new T_FlatCollection (*new T_Collection(*m_scopeIter, m_hostScope_FinderList_AdapterParent->m_finders)));
#endif
                }

            void                increment  ()
                {
                BeAssert (NULL != m_hostScope_FinderList_AdapterParent);
                ++m_scopeIter;
#ifdef DGNV10FORMAT_CHANGES_WIP
                if (m_scopeIter != m_hostScope_FinderList_AdapterParent->m_scope->end())
                    m_currentFlatCollectionPtr = RefCountedCollection<T_FlatCollection>::Create (*new T_FlatCollection (*new T_Collection(*m_scopeIter, m_hostScope_FinderList_AdapterParent->m_finders)));
#endif
                }
            bool    equal (ScopeIterator const & rhs) const
                {
                return m_scopeIter == rhs.m_scopeIter;
                }

            T_FlatCollection const&   dereference () const
                {
                return (m_currentFlatCollectionPtr->GetCollection());
                }
            };
    public:

        HostScope_FinderList_Adapter (HostScope const& scope)
            :m_scope(&scope)
            {}

        typedef ScopeIterator const_iterator;
        const_iterator  begin () const 
            {
            return const_iterator(*this, true);
            }
        const_iterator  end () const 
            {
            return const_iterator(*this, false);
            }
        
        T_FinderList& GetFinderList () {return m_finders;}
    typedef FlatteningCollection<HostScope_FinderList_Adapter<HostType, HostScope> > FlatCollection; 
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE