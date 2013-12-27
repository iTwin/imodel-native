/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstanceIterable.h $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <Bentley/VirtualCollectionIterator.h>
#include <ECObjects/ECObjects.h>
/*__PUBLISH_SECTION_END__*/

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This is the iterator that is exposed using VirtualCollectionIterator. These virtual member
functions delegate the iteration to the appropriate implementations of it based on the container
collection.
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename value_type>
struct   IInstanceCollectionIteratorAdapter :public Bentley::RefCountedBase, std::iterator<std::forward_iterator_tag, value_type>
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
struct IInstanceCollectionAdapter : public Bentley::RefCountedBase
    {
private:

public:
    typedef VirtualCollectionIterator<IInstanceCollectionIteratorAdapter<value_type> > const_iterator;
    virtual const_iterator begin() const = 0; //!< returns the beginning of the collection
    virtual const_iterator end() const = 0; //!< returns the end of the collection
    };

typedef ECN::IInstanceCollectionAdapter<IECInstanceP const>              IECInstanceCollectionAdapter;
typedef RefCountedPtr<IECInstanceCollectionAdapter>                     IECInstanceCollectionAdapterPtr;
typedef ECN::IInstanceCollectionIteratorAdapter<IECInstanceP const>      IECInstanceCollectionIteratorAdapter;

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
struct InstanceCollectionAdapterImpl : public IInstanceCollectionAdapter<value_type>
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
    
    virtual typename IInstanceCollectionAdapter<value_type>::const_iterator begin() const override
        {
        return typename IInstanceCollectionAdapter<value_type>::const_iterator (*InstanceCollectionAdapterIteratorImpl <CollectionType, value_type>::Create(*m_adaptedcollection, true));
        }

    virtual typename IInstanceCollectionAdapter<value_type>::const_iterator end() const override
        {
        return typename IInstanceCollectionAdapter<value_type>::const_iterator (*InstanceCollectionAdapterIteratorImpl <CollectionType, value_type>::Create(*m_adaptedcollection, false));
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
struct ECInstancePVector : public IInstanceCollectionAdapter<T_ReturnType* const>
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
        virtual reference           GetCurrent() override
            {
            m_value = m_iter->get();
            return m_value;
            }

        ECInstancePVectorIterator(typename bvector<RefCountedPtr<T_Instance> >::const_iterator const& iter)
            :m_iter(iter), m_value(NULL)
            {}
        };

    virtual const_iterator begin() const
        {
        return new ECInstancePVectorIterator(m_vector.begin());
        }
    virtual const_iterator end() const
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
@ingroup ECObjectsGroup
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

END_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_END__*/
#pragma make_public (Bentley::ECN::ECInstanceIterable)