/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstanceIterable.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */
#include <ECObjects/ECObjects.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//@addtogroup ECObjectsGroup
//@beginGroup
/*---------------------------------------------------------------------------------**//**
This is the iterator that is exposed using VirtualCollectionIterator. These virtual member
functions delegate the iteration to the appropriate implementations of it based on the container
collection.
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename value_type>
struct   IInstanceCollectionIteratorAdapter :public RefCountedBase, public std::iterator<std::forward_iterator_tag, value_type>
    {
    public:
    typedef value_type&         reference;
    virtual void                MoveToNext  () = 0; //!< Moves to next item in the collection
    virtual bool                IsDifferent (IInstanceCollectionIteratorAdapter const &) const = 0; //!< Compares the item at the current location of the passed in iterator with this iterator's current item
    virtual reference           GetCurrent () = 0; //!< Returns the item at the current iterator location
    };

/*---------------------------------------------------------------------------------**//**
A container collection which allows you to expose different kinds of collection as a single type
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename value_type>
struct IInstanceCollectionAdapterEx : public RefCountedBase
    {
private:

public:
    typedef VirtualCollectionIterator<IInstanceCollectionIteratorAdapter<value_type> > const_iterator;
    virtual const_iterator begin() const = 0; //!< returns the beginning of the collection
    virtual const_iterator end() const = 0; //!< returns the end of the collection
    };

typedef ECN::IInstanceCollectionAdapterEx<IECInstanceP const>                   IECInstanceCollectionAdapter;
typedef RefCountedPtr<IECInstanceCollectionAdapter>                             IECInstanceCollectionAdapterPtr;
typedef ECN::IInstanceCollectionIteratorAdapter<IECInstanceP const>             IECInstanceCollectionIteratorAdapter;
typedef ECN::IInstanceCollectionAdapterEx<IECRelationshipInstanceP const>         IECRelationshipCollectionAdapter;
typedef RefCountedPtr<IECRelationshipCollectionAdapter>                         IECRelationshipCollectionAdapterPtr;
typedef ECN::IInstanceCollectionIteratorAdapter<IECRelationshipInstanceP const> IECRelationshipCollectionIteratorAdapter;

/*__PUBLISH_SECTION_END__*/

/*---------------------------------------------------------------------------------**//**
//Utility class to wrap ones own collection iterator as exposes a DgnECInstanceP
* @bsimethod                                    Abeesh.Basheer                  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CollectionType, typename value_type>
struct InstanceCollectionAdapterIteratorImpl :public IInstanceCollectionIteratorAdapter<value_type>
    {
    private:
        typename CollectionType::const_iterator m_adapteriterator;
        CollectionType const*                   m_adapterParentcollection;
        std::shared_ptr<value_type>           m_ptr;
        InstanceCollectionAdapterIteratorImpl (CollectionType const& collection, bool begin)
            :m_adapterParentcollection(&collection), m_adapteriterator(begin ? collection.begin(): collection.end())
            {
            if (!begin)
                return;

            if (m_adapteriterator == m_adapterParentcollection->end())
                return;

            if (NULL == *m_adapteriterator)
                MoveToNext();
            }

    public:

        /*---------------------------------------------------------------------------------**//**
        // IInstanceCollectionIteratorAdapter implementation
        * @bsimethod                                    Abeesh.Basheer                  03/2011
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void    MoveToNext  () override
            {
            do
                {
                ++m_adapteriterator;
                if (m_adapteriterator == m_adapterParentcollection->end())
                    break;
                }
            while (NULL == *m_adapteriterator);
            }

        virtual typename IInstanceCollectionIteratorAdapter<value_type>::reference GetCurrent () override
            {
            m_ptr = std::shared_ptr<value_type>(new value_type(*m_adapteriterator));
            return *m_ptr;
            }

        static IInstanceCollectionIteratorAdapter<value_type>* Create (CollectionType const& collection, bool begin) {
            return new InstanceCollectionAdapterIteratorImpl(collection, begin);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Abeesh.Basheer                  03/2011
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool    IsDifferent (IInstanceCollectionIteratorAdapter<value_type> const & rhs) const override
            {
            InstanceCollectionAdapterIteratorImpl const* rhsImpl = static_cast<InstanceCollectionAdapterIteratorImpl const*> (&rhs);
            if (NULL == rhsImpl)//TODO evaluate performance of this cast. Since only public facing collection do this it should be okay
                return true;

            return m_adapteriterator != rhsImpl->m_adapteriterator;
            }
    };

/*---------------------------------------------------------------------------------**//**
//Utility class to wrap ones own collection type to be used as a collection adapter.
* @bsimethod                                    Abeesh.Basheer                  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CollectionType, typename value_type>
struct InstanceCollectionAdapterImpl : public IInstanceCollectionAdapterEx<value_type>
    {
private:
    std::shared_ptr<CollectionType>   m_adaptedcollection;
protected:
     InstanceCollectionAdapterImpl (CollectionType& collection)
        :m_adaptedcollection(&collection)
        {
        }
public:

    static InstanceCollectionAdapterImpl* Create (CollectionType& collection)
        {
        return new InstanceCollectionAdapterImpl(collection);
        }

    virtual typename IInstanceCollectionAdapterEx<value_type>::const_iterator begin() const override
        {
        return typename IInstanceCollectionAdapterEx<value_type>::const_iterator (*InstanceCollectionAdapterIteratorImpl <CollectionType, value_type>::Create(*m_adaptedcollection, true));
        }

    virtual typename IInstanceCollectionAdapterEx<value_type>::const_iterator end() const override
        {
        return typename IInstanceCollectionAdapterEx<value_type>::const_iterator (*InstanceCollectionAdapterIteratorImpl <CollectionType, value_type>::Create(*m_adaptedcollection, false));
        }

    virtual ~InstanceCollectionAdapterImpl ()
        {
        }
    };

template <typename CollectionType>
struct IECInstanceCollectionAdapterImpl : public ECN::InstanceCollectionAdapterImpl<CollectionType, IECInstanceP const>
    {
    };

template <typename T_Instance, typename T_ReturnType = T_Instance>
struct ECInstancePVector : public IInstanceCollectionAdapterEx<T_ReturnType* const>
    {
    bvector<RefCountedPtr<T_Instance> > m_vector;
    
    ECInstancePVector (bvector<RefCountedPtr<T_Instance> >const& collection)
        :m_vector(collection)
        {}
    
    public:
        struct ECInstancePVectorIterator :public IInstanceCollectionIteratorAdapter<T_ReturnType* const>
        {
        typename bvector<RefCountedPtr<T_Instance> >::const_iterator    m_iter;
        T_ReturnType*                                                   m_value;
        virtual void                MoveToNext() override
            {
            ++m_iter;
            }
        virtual bool                IsDifferent(IInstanceCollectionIteratorAdapter<T_ReturnType* const> const & iter) const override
            {
            ECInstancePVectorIterator const* rhsImpl = static_cast<ECInstancePVectorIterator const*> (&iter);
            if (NULL == rhsImpl)//TODO evaluate performance of this cast. Since only public facing collection do this it should be okay
                return true;

            return rhsImpl->m_iter != m_iter;
            }
        virtual typename IInstanceCollectionIteratorAdapter<T_ReturnType* const>::reference GetCurrent() override
            {
            m_value = m_iter->get();
            return m_value;
            }

        ECInstancePVectorIterator(typename bvector<RefCountedPtr<T_Instance> >::const_iterator const& iter)
            :m_iter(iter), m_value(NULL)
            {}
        };

    virtual typename IInstanceCollectionAdapterEx<T_ReturnType* const>::const_iterator begin() const
        {
        return new ECInstancePVectorIterator(m_vector.begin());
        }
    virtual typename IInstanceCollectionAdapterEx<T_ReturnType* const>::const_iterator end() const
        {
        return new ECInstancePVectorIterator(m_vector.end());
        }

    static ECInstancePVector* Create(bvector<RefCountedPtr<T_Instance> >const& collection)
        {
        return new ECInstancePVector(collection);
        }
    ~ECInstancePVector()
        {}
    };

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
typical usage
for (ECInstanceIterable::const_iterator iter = collection.begin(); iter != collection.end(); ++iter)
    {
    IECInstanceP instance = *iter;
    }
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceIterable
    {
    private:
        IECInstanceCollectionAdapterPtr m_collectionPtr;

    public:
        //! Default constructor
        ECInstanceIterable ()
            {
            }

        //! Constructor that takes another collection
        //! @param[in] collection   The collection to make an ECInstanceIterable out of
        ECOBJECTS_EXPORT ECInstanceIterable (IECInstanceCollectionAdapter* collection);

        typedef IECInstanceCollectionAdapter::const_iterator  const_iterator;
        ECOBJECTS_EXPORT const_iterator begin () const; //!< returns the beginning of this collection
        ECOBJECTS_EXPORT const_iterator end   () const; //!< returns the end of the collection
        ECOBJECTS_EXPORT bool empty() const; //!< returns whether the collection is empty or not
        ECOBJECTS_EXPORT bool IsNull () const; //!< returns whether the collection is Null
    };

/*---------------------------------------------------------------------------------**//**
typical usage 
for (ECRelationshipIterable::const_iterator iter = collection.begin(); iter != collection.end(); ++iter)
    {
    IECRelationshipInstanceP instance = *iter;
    }
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECRelationshipIterable
    {
    private:
        IECRelationshipCollectionAdapterPtr m_collectionPtr;

    public:
        //! Default constructor
        ECRelationshipIterable ()
            {
            }

        //! Constructor that takes another collection
        //! @param[in] collection   The collection to make an ECInstanceIterable out of
        ECOBJECTS_EXPORT ECRelationshipIterable (IECRelationshipCollectionAdapter* collection);
    
        typedef IECRelationshipCollectionAdapter::const_iterator  const_iterator;
        ECOBJECTS_EXPORT const_iterator begin () const; //!< returns the beginning of this collection
        ECOBJECTS_EXPORT const_iterator end   () const; //!< returns the end of the collection
        ECOBJECTS_EXPORT bool empty() const; //!< returns whether the collection is empty or not
        ECOBJECTS_EXPORT bool IsNull () const; //!< returns whether the collection is Null
    };
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */

/*__PUBLISH_SECTION_END__*/
