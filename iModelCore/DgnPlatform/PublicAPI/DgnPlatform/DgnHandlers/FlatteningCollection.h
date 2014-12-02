/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/FlatteningCollection.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

// #include <boost/iterator/iterator_adaptor.hpp>
BEGIN_BENTLEY_API_NAMESPACE
#if defined (REMOVED_FOR_BOOST)

/*---------------------------------------------------------------------------------**//**
A utility class to hold a collection as a refcounted pointer.
* @bsiclass                                    Abeesh.Basheer                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChildCollection>
struct RefCountedCollection : public RefCountedBase
    {
    private:
        ChildCollection*    m_ownedCollection;

        RefCountedCollection (ChildCollection& collection)
            :m_ownedCollection (&collection)
            {}

    public:
         ~RefCountedCollection ()
            {
            delete m_ownedCollection;
            }

        ChildCollection& GetCollection ()
            {
            return *m_ownedCollection;
            }

        typedef RefCountedPtr<RefCountedCollection>   T_Ptr;
        static T_Ptr Create(ChildCollection& collection)
            {
            return new RefCountedCollection (collection);
            }
    };



/*---------------------------------------------------------------------------------**//**
A utility class which exposes a collection of collections as a collection of single objects
* @bsimethod                                    Abeesh.Basheer                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChildCollection>
struct FlatteningCollection
    {
    public:
        typedef typename ChildCollection::const_iterator            outer_iterator;
        typedef typename outer_iterator::value_type::const_iterator inner_iterator;
    private:
        typename RefCountedCollection<ChildCollection>::T_Ptr   m_ownedCollectionPtr;

        typename ChildCollection::const_iterator GetCollectionBegin() const
            {
            BeAssert (m_ownedCollectionPtr.IsValid());
            return m_ownedCollectionPtr->GetCollection().begin();
            }

        typename ChildCollection::const_iterator GetCollectionEnd() const
            {
            BeAssert (m_ownedCollectionPtr.IsValid());
            return m_ownedCollectionPtr->GetCollection().end();
            }

#if defined (DEBUG)
        int                                                     m_useCount;
#endif
    public:
    struct FlatteningCollectionIterator :public boost::iterator_facade <FlatteningCollectionIterator, typename inner_iterator::value_type const, boost::forward_traversal_tag>
        {
        private:
            FlatteningCollection const*             m_parentFlatCollection;
            outer_iterator                          m_outerIterator;
            inner_iterator*                         m_innerIterator;

            //Iterator state
            typedef typename inner_iterator::value_type T_Value;
            mutable T_Value* m_value;

            void                MoveInnerIteratorToNextInOuter ();
            friend class boost::iterator_core_access;
            void                increment  ();
            bool                equal (FlatteningCollectionIterator const &) const;
            typename inner_iterator::value_type const&    dereference () const;

        public:

            FlatteningCollectionIterator (FlatteningCollection const& collection, bool begin);

            FlatteningCollectionIterator (FlatteningCollectionIterator const& copy)
                :m_parentFlatCollection(copy.m_parentFlatCollection),
                m_outerIterator(copy.m_outerIterator),
                m_innerIterator(NULL != copy.m_innerIterator ? new inner_iterator (*copy.m_innerIterator): NULL),
                m_value (NULL)
                {
#if defined (DEBUG)
                FlatteningCollection* collection = const_cast<FlatteningCollection*> (m_parentFlatCollection);
                ++collection->m_useCount;
#endif
                }
            FlatteningCollectionIterator& operator = (FlatteningCollectionIterator const& copy)
                {
#if defined (DEBUG)
                FlatteningCollection* collection = const_cast<FlatteningCollection*> (m_parentFlatCollection);
                --collection->m_useCount;
#endif

                m_parentFlatCollection = copy.m_parentFlatCollection;

#if defined (DEBUG)
                ++collection->m_useCount;
#endif
                m_outerIterator = copy.m_outerIterator;

                delete m_innerIterator;
                m_innerIterator = (NULL != copy.m_innerIterator) ? new inner_iterator (*copy.m_innerIterator) : NULL;

                delete m_value; m_value = NULL;
                return *this;
                }

            ~FlatteningCollectionIterator ()
                {
#if defined (DEBUG)
                FlatteningCollection* collection = const_cast<FlatteningCollection*> (m_parentFlatCollection);
                --collection->m_useCount;
#endif
                m_parentFlatCollection = NULL;
                delete m_innerIterator; m_innerIterator = NULL;
                delete m_value; m_value = NULL;
                }
        };
    typedef FlatteningCollectionIterator const_iterator;


    FlatteningCollection (ChildCollection& ownedCollection) // A constructor such that we own the child collection
        :m_ownedCollectionPtr(RefCountedCollection<ChildCollection>::Create (ownedCollection))
#if defined (DEBUG)
        , m_useCount(0)
#endif
        {

        }

    FlatteningCollectionIterator begin() const
        {
        return FlatteningCollectionIterator (*this, true);
        }

    FlatteningCollectionIterator end() const
        {
        return FlatteningCollectionIterator (*this, false);
        }

    ~FlatteningCollection ()
        {
#if defined (DEBUG)
        BeAssert(0 == m_useCount);
#endif
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChildCollection>
FlatteningCollection<ChildCollection>::FlatteningCollectionIterator::FlatteningCollectionIterator (FlatteningCollection const& collection, bool begin)
    :m_parentFlatCollection(&collection), m_value(NULL), m_outerIterator(begin ? collection.GetCollectionBegin() : collection.GetCollectionEnd()), m_innerIterator (NULL)
    {
#if defined (DEBUG)
    FlatteningCollection* collectionP = const_cast<FlatteningCollection*> (m_parentFlatCollection);
    ++collectionP->m_useCount;
#endif
    if (!begin)
        return;

    if (m_outerIterator == m_parentFlatCollection->GetCollectionEnd())
        return;

    m_innerIterator = new inner_iterator(m_outerIterator->begin());
    if (*m_innerIterator == m_outerIterator->end())
        MoveInnerIteratorToNextInOuter();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChildCollection>
void            FlatteningCollection<ChildCollection>::FlatteningCollectionIterator::MoveInnerIteratorToNextInOuter ()
    {
    while (*m_innerIterator == m_outerIterator->end())
        {
        delete m_innerIterator; m_innerIterator = NULL;
        ++m_outerIterator;
        if (m_outerIterator == m_parentFlatCollection->GetCollectionEnd())
            break;

        m_innerIterator = new inner_iterator(m_outerIterator->begin());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChildCollection>
void            FlatteningCollection<ChildCollection>::FlatteningCollectionIterator::increment ()
    {
    if (m_outerIterator == m_parentFlatCollection->GetCollectionEnd())
        return;

    delete m_value; m_value = NULL;
    ++(*m_innerIterator);

    if (*m_innerIterator == m_outerIterator->end())
        MoveInnerIteratorToNextInOuter ();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChildCollection>
bool            FlatteningCollection<ChildCollection>::FlatteningCollectionIterator::equal (FlatteningCollectionIterator const & other) const
    {
    if (m_outerIterator != other.m_outerIterator)
        return false;

    if (NULL == m_innerIterator && NULL == other.m_innerIterator)
        return true;

    if (NULL == m_innerIterator || NULL == other.m_innerIterator)
        return false;

    return *m_innerIterator == *other.m_innerIterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChildCollection>
typename FlatteningCollection<ChildCollection>::inner_iterator::value_type const& FlatteningCollection<ChildCollection>::FlatteningCollectionIterator::dereference () const
    {
    if (NULL == m_innerIterator)
        {
        BeAssert(false);
        return *m_value;
        }

    if (NULL == m_value)
        m_value = new T_Value(**m_innerIterator);

    return *m_value;
    }

#endif

END_BENTLEY_API_NAMESPACE
