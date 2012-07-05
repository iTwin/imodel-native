/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstanceIterable.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECObjects.h>
/*__PUBLISH_SECTION_END__*/

#pragma  warning(push)
#pragma  warning(disable:4265)
#include <boost/iterator/transform_iterator.hpp>
#pragma  warning(pop)


/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_EC_NAMESPACE

/*__PUBLISH_SECTION_END__*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CollectionType, class UnaryFunction>
struct CollectionTransformIteratble
    {
    CollectionType const& m_collection;
    CollectionTransformIteratble (CollectionType const& collection)
        :m_collection (collection)
        {}

    typedef boost::transform_iterator<UnaryFunction, typename CollectionType::const_iterator> const_iterator;
    const_iterator begin () const {return const_iterator (m_collection.begin(), UnaryFunction());}
    const_iterator end () const {return const_iterator (m_collection.end(), UnaryFunction());}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_Instance>
struct RefCountedPtrToValueTransform: public std::unary_function<RefCountedPtr<T_Instance>, T_Instance* const &>
    {
    T_Instance* const& operator () (RefCountedPtr<T_Instance> const& ptr) const
        {
        return ptr.GetCR();
        }
    };

/*---------------------------------------------------------------------------------**//**
//! TODO: test whether we need this adapter or whether the compiler will allow pass through
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename DerivedType, typename BaseType>
struct DerviedToBaseTransform :public std::unary_function<DerivedType, BaseType>
    {
    BaseType* operator () (DerivedType* ptr) const
        {
        return static_cast<BaseType*> (ptr);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CollectionType, typename BaseType>
struct BaseToDerivedCollectionTransformIteratble : public CollectionTransformIteratble<CollectionType, DerviedToBaseTransform <typename CollectionType::const_iterator::value_type, BaseType> >
    {
    BaseToDerivedCollectionTransformIteratble (CollectionType const& collection)
        :CollectionTransformIteratble (collection)
        {}
    };


/*__PUBLISH_SECTION_START_*/
/*---------------------------------------------------------------------------------**//**
//This is the iterator that is exposed using VirtualCollectionIterator. These virtual member
functions delegate the iteration to the appropriate implementations of it based on the container
collection.
* @bsimethod                                    Abeesh.Basheer                  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename value_type>
struct   IInstanceCollectionIteratorAdapter :public Bentley::RefCountedBase, std::iterator<std::forward_iterator_tag, value_type>
    {
    public:
    typedef typename value_type&  reference;
    virtual void                MoveToNext  () = 0;
    virtual bool                IsDifferent (IInstanceCollectionIteratorAdapter const &) const = 0;
    virtual reference           GetCurrent () = 0;
    };

/*---------------------------------------------------------------------------------**//**
//A container collection which allows you to expose different kinds of collection as a single type
* @bsimethod                                    Abeesh.Basheer                  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename value_type>
struct IInstanceCollectionAdapter : public Bentley::RefCountedBase
    {
private:

public:
    typedef VirtualCollectionIterator<IInstanceCollectionIteratorAdapter<value_type> > const_iterator;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    };

typedef EC::IInstanceCollectionAdapter<IECInstanceP const>              IECInstanceCollectionAdapter;
typedef RefCountedPtr<IECInstanceCollectionAdapter>                     IECInstanceCollectionAdapterPtr;
typedef EC::IInstanceCollectionIteratorAdapter<IECInstanceP const>      IECInstanceCollectionIteratorAdapter;

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
        boost::shared_ptr<value_type>           m_ptr;
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

        virtual reference GetCurrent () override
            {
            m_ptr = boost::shared_ptr<value_type>(new value_type(*m_adapteriterator));
            return *m_ptr;
            }

        static InstanceCollectionAdapterIteratorImpl* Create (CollectionType const& collection, bool begin) {
            return new InstanceCollectionAdapterIteratorImpl(collection, begin);
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Abeesh.Basheer                  03/2011
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool    IsDifferent (IInstanceCollectionIteratorAdapter const & rhs) const override
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
    boost::shared_ptr<CollectionType>   m_adaptedcollection;
protected:
     InstanceCollectionAdapterImpl (CollectionType& collection)
        :m_adaptedcollection(&collection)
        {
        }
public:
    
    static InstanceCollectionAdapterImpl* InstanceCollectionAdapterImpl::Create (CollectionType& collection)
        {
        return new InstanceCollectionAdapterImpl(collection);
        }
    
    virtual const_iterator begin() const override
        {
        return const_iterator (*InstanceCollectionAdapterIteratorImpl <CollectionType, value_type>::Create(*m_adaptedcollection, true));
        }

    virtual const_iterator end() const override
        {
        return const_iterator (*InstanceCollectionAdapterIteratorImpl <CollectionType, value_type>::Create(*m_adaptedcollection, false));
        }

    virtual ~InstanceCollectionAdapterImpl ()
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceIterable
    {
    private:
        IECInstanceCollectionAdapterPtr m_collectionPtr;

    public:
        ECInstanceIterable ()
            {
            }
        ECInstanceIterable (IECInstanceCollectionAdapter* collection);
    
        typedef IECInstanceCollectionAdapter::const_iterator  const_iterator;
        ECOBJECTS_EXPORT const_iterator begin () const;
        ECOBJECTS_EXPORT const_iterator end   () const;
        ECOBJECTS_EXPORT bool empty() const;
        ECOBJECTS_EXPORT bool IsNull () const;
    };//ECInstanceIterable

END_BENTLEY_EC_NAMESPACE

/*__PUBLISH_SECTION_END__*/