/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Foundations/Iterator.h $
|    $RCSfile: Iterator.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/16 15:36:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


/*__PUBLISH_SECTION_START__*/


#include <ScalableMesh/Foundations/Definitions.h>
#include <iterator>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_ITERATOR_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<class ContainerT,
         class T, 
         class DiffT = ptrdiff_t, 
         class PointerT = T *, 
         class ReferenceT = T&, 
         class CPointerT = const T *, 
         class CReferenceT = const T&>
struct Policy
        {
        typedef ContainerT          container_type;
    typedef T                   value_type;
    typedef DiffT               difference_type;    
    typedef PointerT            pointer;
    typedef ReferenceT          reference;
    typedef CPointerT           const_pointer;
    typedef CReferenceT         const_reference;
    };

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BaseT, typename PolicyT = BaseT>
struct ConstForward
    {
private:
    typedef ConstForward<BaseT, PolicyT>   
                                    Type;
    typedef PolicyT                 Policy;

    friend typename                 Policy::container_type;
    
protected:
    BaseT                           m_base;

    explicit                        Type                       (const BaseT&                base) : m_base(base) 
                                                                                                        {} 
    const BaseT&                    GetBase                    () const                                 { return m_base; }

public:
    typedef std::forward_iterator_tag 
                                    iterator_category;
    typedef const typename Policy::value_type 
                                    value_type;
    typedef typename Policy::difference_type 
                                    difference_type;
    typedef typename Policy::const_pointer   
                                    pointer;
    typedef typename Policy::const_reference 
                                    reference;


    reference                       operator*                  () const                                 { return m_base.Dereference(); }
    pointer                         operator->                 () const                                 { return &m_base.Dereference(); }

    bool                            operator==                 (const Type&                 rhs) const  { return m_base.EqualTo(rhs.m_base); }
    bool                            operator!=                 (const Type&                 rhs) const  { return !m_base.EqualTo(rhs.m_base); }

    Type&                           operator++                 ()                                       { m_base.Increment(); return *this; }
    Type                            operator++                 (int)                                    { Type tmp = *this; m_base.Increment(); return tmp; }
    };

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BaseT, typename PolicyT = BaseT>
struct Forward : public ConstForward<BaseT, PolicyT>
    {
private:
    typedef ConstForward<BaseT, PolicyT>
                                    MyBase;

    typedef Forward<BaseT, PolicyT>   
                                    Type;
    typedef PolicyT                 Policy;

    friend typename                 Policy::container_type;

protected:
    explicit                        Type                       (const BaseT&                base) : MyBase(base) 
                                                                                                        {} 
public:
    typedef typename Policy::value_type      
                                    value_type;
    typedef typename Policy::pointer         
                                    pointer;
    typedef typename Policy::reference       
                                    reference;

    using                           MyBase::operator*;
    using                           MyBase::operator->;

    reference                       operator*                  ()                                       { return m_base.Dereference(); }
    pointer                         operator->                 ()                                       { return &m_base.Dereference(); }

    Type&                           operator++                 ()                                       { m_base.Increment(); return *this; }
    Type                            operator++                 (int)                                    { Type tmp = *this; m_base.Increment(); return tmp; }
    };





/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BaseT, typename PolicyT = BaseT>
struct ConstBidirectional
    {
private:
    typedef ConstBidirectional<BaseT, PolicyT>   
                                    Type;
    typedef PolicyT                 Policy;

    friend typename                 Policy::container_type;
    
protected:
    BaseT                           m_base;

    explicit                        Type                       (const BaseT&                base) : m_base(base) 
                                                                                                        {} 
    const BaseT&                    GetBase                    () const                                 { return m_base; }

public:
    typedef std::bidirectional_iterator_tag 
                                    iterator_category;
    typedef const typename Policy::value_type 
                                    value_type;
    typedef typename Policy::difference_type 
                                    difference_type;
    typedef typename Policy::const_pointer   
                                    pointer;
    typedef typename Policy::const_reference 
                                    reference;


    reference                       operator*                  () const                                 { return m_base.Dereference(); }
    pointer                         operator->                 () const                                 { return &m_base.Dereference(); }

    bool                            operator==                 (const Type&                 rhs) const  { return m_base.EqualTo(rhs.m_base); }
    bool                            operator!=                 (const Type&                 rhs) const  { return !m_base.EqualTo(rhs.m_base); }

    Type&                           operator++                 ()                                       { m_base.Increment(); return *this; }
    Type                            operator++                 (int)                                    { Type tmp = *this; m_base.Increment(); return tmp; }

    Type&                           operator--                 ()                                       { m_base.Decrement(); return *this; }
    Type                            operator--                 (int)                                    { Type tmp = *this; m_base.Decrement(); return tmp; }

    };


template <typename BaseT, typename PolicyT = BaseT>
struct Bidirectional : public ConstBidirectional<BaseT, PolicyT>
    {
private:
    typedef ConstBidirectional<BaseT, PolicyT>
                                    MyBase;

    typedef Bidirectional<BaseT, PolicyT>   
                                    Type;
    typedef PolicyT                 Policy;

    friend typename                 Policy::container_type;

protected:
    explicit                        Type                       (const BaseT&                base) : MyBase(base) 
                                                                                                        {} 
public:
    typedef typename Policy::value_type      
                                    value_type;
    typedef typename Policy::pointer         
                                    pointer;
    typedef typename Policy::reference       
                                    reference;

    using                           MyBase::operator*;
    using                           MyBase::operator->;

    reference                       operator*                  ()                                       { return m_base.Dereference(); }
    pointer                         operator->                 ()                                       { return &m_base.Dereference(); }

    Type&                           operator++                 ()                                       { m_base.Increment(); return *this; }
    Type                            operator++                 (int)                                    { Type tmp = *this; m_base.Increment(); return tmp; }

    Type&                           operator--                 ()                                       { m_base.Decrement(); return *this; }
    Type                            operator--                 (int)                                    { Type tmp = *this; m_base.Decrement(); return tmp; }
    };


END_BENTLEY_SCALABLEMESH_FOUNDATIONS_ITERATOR_NAMESPACE
