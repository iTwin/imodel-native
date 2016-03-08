//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/HPUCompositeArray.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMCompositeArray
//-----------------------------------------------------------------------------

#pragma once
#ifndef VANCOUVER_API
#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HIterators.h>

#include "HPUArray.h"

namespace HPU {

template <typename FacadeT, typename HeaderT, typename ArrayT> class CompositeArrayBase;

template <typename FacadeT, typename HeaderT, typename ArrayT> class CompositeFacadeBaseWExtCopy;


/*---------------------------------------------------------------------------------**//**
* @description  This is a helper base class whose goal is to help implementing a facade
*               class to be used with a composite array class. Specializations of this
*               class are closely linked with composite array base's iterator class. This
*               class automatically generate a valid Swap method for its specializations.
*               Instances of specializations of this class are not permitted to be copied
*               outside of the composite array. Will work for all algorithm that do not
*               need to take pivot values (e.g. std::random_shuffle).
*
*               FacadeT : Specialized class (the actual facade type) that act as
*                         a virtual class for multiple data layers.
*               HeaderT : The header type that is used to index other layers of the
*                         composite array.
*               ArrayT : The composite array class that makes you think it stores
*                        facade types while in reality it stores multiples layers
*                        of data.
*
*               Requisites for specializations of this class are:
*               - Default constructor must have private visibility but be visible
*                 to composite_array_type
*               - Copy constructor and assignment operator should have private
*                 visibility and may not be implemented
*               - OnArrayDataChanged must be overridden for Swap method to work
*                 correctly. This could be a good place to specify that array
*                 sequence has changed and data need to be reordered.
*
*               If user has other states that need to be updated through iteration,
*               he will have to override ShallowInit and ShallowAssign. His own
*               overrides will then have to call this class's version of ShallowInit
*               and ShallowAssign.
*
* @see IDTMCompositeFacadeBaseWExtCopy for a more flexible base
* @see IDTMCompositeArrayBase
* @see IDTMCompositeArrayBase::iterator
* @see IDTMCompositeArrayBase::const_iterator
*
* @bsiclass                                                  Raymond.Gauthier   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FacadeT,
         typename HeaderT,
         typename ArrayT>
class CompositeFacadeBase
    {
public:
    typedef HeaderT  header_type;
    typedef FacadeT  facade_type;
    typedef ArrayT   array_type;

    typedef CompositeFacadeBase<facade_type, header_type, array_type>
    super_class;
    typedef CompositeArrayBase<facade_type, header_type, array_type>
    composite_array_type;


    explicit                                CompositeFacadeBase            ();


    // Derived need to implement this method if he need code to be ran when internal facade is
    // assigned from an external copy.
    void                                    OnArrayDataChanged                 () {
        //HSTATICASSERT(false) /*Need to be overridden*/;
        }


    const header_type*                      GetHeaderIter                      () const {
        return m_HeaderIter;
        }
    header_type*                            EditHeaderIter                     () const {
        return m_HeaderIter;
        }

    const header_type&                      GetHeader                          () const {
        return *(m_HeaderIter);
        }
    header_type&                            EditHeader                         ()       {
        return *(m_HeaderIter);
        }

    const array_type*                       GetArrayP                          () const {
        return m_pArray;
        }
    array_type*                             EditArrayP                         ()       {
        return m_pArray;
        }

    const Array<header_type>&               GetHeaders                         () const;
    Array<header_type>&                     EditHeaders                        ();
public:
    void                                    Swap                               (facade_type&                    pi_rRight);

    friend                                  class CompositeFacadeBaseWExtCopy<facade_type, header_type, array_type>;

    CompositeFacadeBase                (const CompositeFacadeBase&  pi_rRight);
    CompositeFacadeBase&                    operator=                          (const CompositeFacadeBase&  pi_rRight);

    void                                    TriggerArrayDataChangedEvent       ();

    // Used only through composite array base's iterator
    void                                    ShallowInit                        (header_type*                    pi_HeaderIter,
                                                                                array_type*                     pi_pArray);
    void                                    ShallowAssign                      (const CompositeFacadeBase&  pi_rRight);


    array_type*                             m_pArray;
    header_type*                            m_HeaderIter;
    };



/*---------------------------------------------------------------------------------**//**
* @description  This is also a helper base class whose goal is to help implementing a
*               facade class to be used with a composite array class. This class adds the
*               same functionalities as its own base but also permit facades to be copied
*               outside of the composite array. This is at the expense of a supplementary
*               boolean field. Use this class as a base if you want to enable algorithms
*               that takes pivots values to work with the composite array (e.g.: std::sort).
*               Otherwise, use IDTMCompositeFacadeBase which may run faster in most cases.
*
*               Supplementary requisites for specializations of this class are:
*               - Copy constructor and assignment operator must be carefully implemented
*                 and have public visibility
*
* @see IDTMCompositeFacadeBase
*
* @bsiclass                                                  Raymond.Gauthier   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FacadeT,
         typename HeaderT,
         typename ArrayT>
class CompositeFacadeBaseWExtCopy : public CompositeFacadeBase<FacadeT, HeaderT, ArrayT>
    {

public:

    explicit CompositeFacadeBaseWExtCopy        ()
        :   CompositeFacadeBase<FacadeT, HeaderT, ArrayT>(),
            m_OutOfArray(false) {}

    ~CompositeFacadeBaseWExtCopy       ()
        {
        if (m_OutOfArray)
            {
            //HASSERT(0 != CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::m_pArray && 0 != CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::m_HeaderIter);
            delete CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::m_HeaderIter;
            }
        }

    CompositeFacadeBaseWExtCopy        (const CompositeFacadeBaseWExtCopy&  pi_rRight)
        :   CompositeFacadeBase<FacadeT, HeaderT, ArrayT>(pi_rRight),
            m_OutOfArray(false)
        {
        CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::m_HeaderIter = new HeaderT();
        m_OutOfArray = true;

        *CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::m_HeaderIter = *pi_rRight.m_HeaderIter;
        }
    CompositeFacadeBaseWExtCopy<FacadeT, HeaderT, ArrayT>& operator= (const CompositeFacadeBaseWExtCopy&  pi_rRight);

    bool IsOutOfArray () const;

    // TDORAY: Consider overriding Swap here so that TriggerArrayDataChangedEvent is only called when facade is part of the array. It would
    // probably be an inefficient addition though...

private:
    // When true, specifies that the feature was copied out of the array
    bool                                    m_OutOfArray;
    };



/*---------------------------------------------------------------------------------**//**
* @description  This is a helper base class whose goal is to help implementing a
*               composite array that makes you think you're handling separate entities
*               (through a facade class interface) but stores in reality multiples layers
*               of data.
*
*               Composite was chosen as a name because specialization of this
*               class can be viewed as a software engineering's version of a composite
*               material (http://en.wikipedia.org/wiki/Composite_material). Indeed, this
*               class's (specialization of this class) properties when seen from the outside
*               at the macro level (e.g.: when handling an array of facades (sheet of material)),
*               seem quite different from its constituents. Its constituent when seen at the
*               micro level are only two or more "sheets" of data that when glued together
*               by specializations of this class, show different properties then what
*               they do when considered individually.
*
*               A facade class must be implemented so that it inherits from
*               either IDTMCompositeFacadeBase or IDTMCompositeFacadeBaseWExtCopy. This
*               facade is built on an header which is part of an contiguous array of
*               headers. Current header is updated when a composite array's iterator
*               is moved. The facade class can be seen as the glue that links the sheets
*               of data together.
*
*               FacadeT : Class (the actual facade type) whose instances act as
*                         virtual objects for multiple data layers.
*               HeaderT : The header type that is used to index other layers of the
*                         composite array.
*               ArrayT : Specialization of this class.
*
*               Requisites for specializations of this class are:
*                   TDORAY
* @see IDTMCompositeFacadeBase
* @see IDTMCompositeFacadeBaseWExtCopy
*
* @bsiclass                                                  Raymond.Gauthier   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FacadeT, typename HeaderT, typename ArrayT>
class CompositeArrayBase
    {
public:
//protected:
    typedef ArrayT                          array_type;
    typedef FacadeT                         facade_type;
    typedef HeaderT                         header_type;

    typedef CompositeArrayBase<facade_type, header_type, array_type> super_class;
//private:
    typedef super_class                     composite_array_type;


    /*---------------------------------------------------------------------------------**//**
    * @description  An iterator to an item of the array. Referred items are not real objects but
    *               facades(FeatureFacade) to underlying managed data. This iterator is completely
    *               compliant with STL random access iterators interface.
    * @see          IDTMCompositeFacadeBase
    * @see          IDTMCompositeFacadeBaseWExtCopy
    * @see          Bentley::ImagePP::RandomAccessIteratorWithAutoReverseConst
    *
    * @bsiclass                                                  Raymond.Gauthier   5/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <bool IsConst = false>
    class Iterator : public BENTLEY_NAMESPACE_NAME::ImagePP::RandomAccessIteratorWithAutoReverseConst<Iterator<IsConst>,
        Iterator<!IsConst>,
        typename BENTLEY_NAMESPACE_NAME::ImagePP::ConstTriggerTrait<FacadeT, IsConst>::type>
        {
    public:
        typedef BENTLEY_NAMESPACE_NAME::ImagePP::RandomAccessIteratorWithAutoReverseConst<Iterator<IsConst>,
                                                                Iterator<!IsConst>,
                                                                typename BENTLEY_NAMESPACE_NAME::ImagePP::ConstTriggerTrait<FacadeT, IsConst>::type>
                        RandomAccessIteratorWithAutoReverseConst_Type;
        explicit                            Iterator                           ();

        Iterator                           (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight)
            { m_Current.ShallowAssign(pi_rRight.m_Current); }

        typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t& operator= (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight)
            {
            m_Current.ShallowAssign(pi_rRight.m_Current);
            return *this;
            }

        typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t            ConstCast ()
            { return iterator_t(m_Current.m_HeaderIter, m_Current.m_pArray); }
        const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t      ConstCast () const
            { return iterator_t(m_Current.m_HeaderIter, m_Current.m_pArray); }

        // NOTE: Public interface is STL random access iterators interface.
        //       See Bentley::ImagePP::RandomAccessIterator.

        explicit Iterator (header_type* pi_HeaderIter,
                           array_type*  pi_pList)
            { m_Current.ShallowInit(pi_HeaderIter, pi_pList); }

        typename RandomAccessIteratorWithAutoReverseConst_Type::const_iterator_t                    ConvertToConst                     () const
        { return const_iterator_t(m_Current.m_HeaderIter, m_Current.m_pArray); }

        typename RandomAccessIteratorWithAutoReverseConst_Type::const_reference                     Dereference                        () const
            {
            HPRECONDITION(0 != m_Current.m_pArray);
            return m_Current;
            }
        typename RandomAccessIteratorWithAutoReverseConst_Type::reference                           Dereference                        ()
            {
            HPRECONDITION(0 != m_Current.m_pArray);
            return m_Current;
            }

        void Increment ()
            {
            HPRECONDITION(0 != m_Current.m_pArray);
            ++m_Current.m_HeaderIter;
            }
        void Decrement ()
            {
            HPRECONDITION(0 != m_Current.m_pArray);
            --m_Current.m_HeaderIter;
            }

        void AdvanceOf (typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type             pi_Offset)
            {
            HPRECONDITION(0 != m_Current.m_pArray);
            m_Current.m_HeaderIter += pi_Offset;
            }
        typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type                     DistanceFrom                       (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
            {
            HPRECONDITION(0 != m_Current.m_pArray && 0 != pi_rRight.m_Current.m_pArray);;
            return m_Current.m_HeaderIter - pi_rRight.m_Current.m_HeaderIter;
            }
        typename RandomAccessIteratorWithAutoReverseConst_Type::difference_type                     DistanceFrom                       (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
            {
            HPRECONDITION(0 != m_Current.m_pArray && 0 != pi_rRight.m_Current.m_pArray);;
            return m_Current.m_HeaderIter - pi_rRight.m_Current.m_HeaderIter;
            }

        bool                                EqualTo                            (const typename RandomAccessIteratorWithAutoReverseConst_Type:: iterator_t&           pi_rRight) const
            {
            HPRECONDITION(0 != m_Current.m_pArray && 0 != pi_rRight.m_Current.m_pArray);
            return m_Current.m_HeaderIter == pi_rRight.m_Current.m_HeaderIter;
            }
        bool                                LessThan                           (const typename RandomAccessIteratorWithAutoReverseConst_Type::iterator_t&           pi_rRight) const
            {
            HPRECONDITION(0 != m_Current.m_pArray && 0 != pi_rRight.m_Current.m_pArray);
            return m_Current.m_HeaderIter < pi_rRight.m_Current.m_HeaderIter;
            }
        bool                                EqualTo                            (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
            {
            HPRECONDITION(0 != m_Current.m_pArray && 0 != pi_rRight.m_Current.m_pArray);
            return m_Current.m_HeaderIter == pi_rRight.m_Current.m_HeaderIter;
            }
        bool                                LessThan                           (const typename RandomAccessIteratorWithAutoReverseConst_Type::rconst_iterator_t&    pi_rRight) const
            {
            HPRECONDITION(0 != m_Current.m_pArray && 0 != pi_rRight.m_Current.m_pArray);
            return m_Current.m_HeaderIter < pi_rRight.m_Current.m_HeaderIter;
            }


        FacadeT                             m_Current;
        };

public:
    typedef FacadeT                         value_type;
    typedef const value_type&               const_reference;
    typedef value_type&                     reference;
    typedef Iterator<false>                 iterator;
    typedef Iterator<true>                  const_iterator;


    explicit                                CompositeArrayBase                 (size_t                      pi_Capacity = 0);

    size_t                                  GetSize                            () const;
    size_t                                  GetCapacity                        () const;
    void                                    Reserve                            (size_t                      pi_Capacity);

    typedef Array<HeaderT>              HeaderArray;

    iterator BeginEdit ()
        { return iterator(m_HeaderArray.BeginEdit(), ReinterpretAsDerived()); }
    iterator EndEdit ()
        { return iterator(m_HeaderArray.EndEdit(), ReinterpretAsDerived()); }


    const_iterator Begin () const
        {
        return const_iterator(const_cast<HeaderArray::iterator>(m_HeaderArray.Begin()), const_cast<array_type*>(ReinterpretAsDerived())); //TDORay
        }
    const_iterator End () const
        {   
        return const_iterator(const_cast<HeaderArray::iterator>(m_HeaderArray.End()), const_cast<array_type*>(ReinterpretAsDerived())); //TDORay
        }

    bool                                    IsValidIterator                    (const const_iterator&       pi_Iterator);
    bool                                    IsValidIterator                    (const iterator&             pi_Iterator);

//protected:
    const HeaderArray&                      GetHeaders                         () const     {
        return m_HeaderArray;
        }
    HeaderArray&                            EditHeaders                        ()           {
        return m_HeaderArray;
        }

    iterator                                CreateIterator                     (header_type*                pi_HeaderIter);
    const_iterator                          CreateConstIterator                (header_type*                pi_HeaderIter) const;

    const array_type*                       ReinterpretAsDerived               () const;
    array_type*                             ReinterpretAsDerived               ();

    HeaderArray                             m_HeaderArray;
    };

#include "HPUCompositeArray.hpp"
#endif
} // End namespace HPU