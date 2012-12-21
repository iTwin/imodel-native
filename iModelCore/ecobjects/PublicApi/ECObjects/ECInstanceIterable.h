/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECInstanceIterable.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/// @cond BENTLEY_SDK_Desktop

#include <ECObjects/ECObjects.h>
/*__PUBLISH_SECTION_END__*/

#pragma  warning(push)
#pragma  warning(disable:4265)
#include <boost/iterator/transform_iterator.hpp>
#include <boost/shared_ptr.hpp>
#pragma  warning(pop)


/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_END__*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CollectionType, class UnaryFunction>
struct CollectionTransformIteratble
    {
    private:
    CollectionType const* m_collection;

    public:
    CollectionTransformIteratble (CollectionType const& collection)
        :m_collection (&collection)
        {}

    typedef boost::transform_iterator<UnaryFunction, typename CollectionType::const_iterator> const_iterator;
    const_iterator begin () const {return const_iterator (m_collection->begin(), UnaryFunction());}
    const_iterator end () const {return const_iterator (m_collection->end(), UnaryFunction());}
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

/*__PUBLISH_SECTION_START__*/

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
    virtual void                MoveToNext  () = 0;
    virtual bool                IsDifferent (IInstanceCollectionIteratorAdapter const &) const = 0;
    virtual reference           GetCurrent () = 0;
    };

/*---------------------------------------------------------------------------------**//**
A container collection which allows you to expose different kinds of collection as a single type
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename value_type>
struct IInstanceCollectionAdapterEx : public Bentley::RefCountedBase
    {
private:

public:
    typedef VirtualCollectionIterator<IInstanceCollectionIteratorAdapter<value_type> > const_iterator;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    };

typedef ECN::IInstanceCollectionAdapterEx<IECInstanceP const>              IECInstanceCollectionAdapter;
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

        virtual typename IInstanceCollectionIteratorAdapter<value_type>::reference GetCurrent () override
            {
            m_ptr = boost::shared_ptr<value_type>(new value_type(*m_adapteriterator));
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
    boost::shared_ptr<CollectionType>   m_adaptedcollection;
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

template <typename T_Instance>
struct ECInstancePVector : public ECN::CollectionTransformIteratble< bvector<RefCountedPtr<T_Instance> >, ECN::RefCountedPtrToValueTransform<T_Instance> >
    {
    bvector<RefCountedPtr<T_Instance> > m_vector;
    public:
    ECInstancePVector (bvector<RefCountedPtr<T_Instance> >const& collection)
        :m_vector(collection), CollectionTransformIteratble< bvector<RefCountedPtr<T_Instance> >, ECN::RefCountedPtrToValueTransform<T_Instance> > (m_vector)
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
        ECInstanceIterable ()
            {
            }
        ECOBJECTS_EXPORT ECInstanceIterable (IECInstanceCollectionAdapter* collection);

        typedef IECInstanceCollectionAdapter::const_iterator  const_iterator;
        ECOBJECTS_EXPORT const_iterator begin () const;
        ECOBJECTS_EXPORT const_iterator end   () const;
        ECOBJECTS_EXPORT bool empty() const;
        ECOBJECTS_EXPORT bool IsNull () const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/// @endcond BENTLEY_SDK_Desktop

/*__PUBLISH_SECTION_END__*/
