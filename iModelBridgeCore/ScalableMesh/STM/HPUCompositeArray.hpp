//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/HPUCompositeArray.hpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


template <typename FacadeT, typename HeaderT, typename ArrayT>
CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::CompositeFacadeBase ()
    :   m_HeaderIter(0),
        m_pArray(0)
    {

    }


template <typename FacadeT, typename HeaderT, typename ArrayT>
CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::CompositeFacadeBase (const CompositeFacadeBase& pi_rRight)
    :   m_HeaderIter(pi_rRight.m_HeaderIter),
        m_pArray(pi_rRight.m_pArray)
    {

    }


template <typename FacadeT, typename HeaderT, typename ArrayT>
CompositeFacadeBase<FacadeT, HeaderT, ArrayT>&
CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::operator= (const CompositeFacadeBase&  pi_rRight)
    {
    m_HeaderIter = pi_rRight.m_HeaderIter;
    m_pArray = pi_rRight.m_pArray;
    return *this;
    }


template <typename FacadeT, typename HeaderT, typename ArrayT>
void CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::ShallowInit(header_type*    pi_HeaderIter,
                                                                array_type*     pi_pArray)
    {
    m_HeaderIter = pi_HeaderIter;
    m_pArray = pi_pArray;
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
void CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::ShallowAssign (const CompositeFacadeBase& pi_rRight)
    {
    m_HeaderIter = pi_rRight.m_HeaderIter;
    m_pArray = pi_rRight.m_pArray;
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
void CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::Swap (facade_type& pi_rRight)
    {
    if (m_pArray != pi_rRight.m_pArray)
        {
        HASSERT(!"Could not swap facades that are not of the same array!");
        return;
        }

    const header_type TempHeader(*m_HeaderIter);
    *m_HeaderIter = *pi_rRight.m_HeaderIter;
    *pi_rRight.m_HeaderIter = TempHeader;

    TriggerArrayDataChangedEvent();
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
void CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::TriggerArrayDataChangedEvent ()
    {
    static_cast<facade_type&>(*this).OnArrayDataChanged();
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
const Array<typename CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::header_type>&
CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::GetHeaders () const
    {
    HPRECONDITION(0 != m_pArray);
    return m_pArray->m_HeaderArray;
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
Array<typename CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::header_type>&
CompositeFacadeBase<FacadeT, HeaderT, ArrayT>::EditHeaders ()
    {
    HPRECONDITION(0 != m_pArray);
    return m_pArray->m_HeaderArray;
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
CompositeFacadeBaseWExtCopy<FacadeT, HeaderT, ArrayT>&
CompositeFacadeBaseWExtCopy<FacadeT, HeaderT, ArrayT>::operator= (const CompositeFacadeBaseWExtCopy& pi_rRight)
    {
    if (0 == this->m_pArray)
        {
        HASSERT(!"Should not be invalid when here");
        return *this;
        }
    if (0 == pi_rRight.m_pArray)
        {
        HASSERT(!"Should not try to assign from invalid facade");
        return *this;
        }
    if (this->m_pArray != pi_rRight.m_pArray)
        {
        HASSERT(!"Trying to assign a facade from another array to an iterator");
        return *this;
        }

    this->m_pArray = pi_rRight.m_pArray;
    *this->m_HeaderIter = *pi_rRight.m_HeaderIter;

    if (!m_OutOfArray)
        this->TriggerArrayDataChangedEvent();

    return *this;
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
bool CompositeFacadeBaseWExtCopy<FacadeT, HeaderT, ArrayT>::IsOutOfArray () const
    {
    return m_OutOfArray;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Default constructor. An iterator is required to be default constructible.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FacadeT, typename HeaderT, typename ArrayT> template <bool IsConst>
CompositeArrayBase<FacadeT, HeaderT, ArrayT>::Iterator<IsConst>::Iterator ()
    {
    m_Current.ShallowInit(0, 0);
    }


template <typename FacadeT, typename HeaderT, typename ArrayT>
CompositeArrayBase<FacadeT, HeaderT, ArrayT>::CompositeArrayBase (size_t pi_Capacity)
    :   m_HeaderArray(pi_Capacity)
    {

    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
size_t CompositeArrayBase<FacadeT, HeaderT, ArrayT>::GetSize () const
    {
    return m_HeaderArray.GetSize();
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
size_t CompositeArrayBase<FacadeT, HeaderT, ArrayT>::GetCapacity () const
    {
    return m_HeaderArray.GetCapacity();
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
void CompositeArrayBase<FacadeT, HeaderT, ArrayT>::Reserve (size_t pi_Capacity)
    {
    m_HeaderArray.Reserve(pi_Capacity);
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
const typename CompositeArrayBase<FacadeT, HeaderT, ArrayT>::array_type* CompositeArrayBase<FacadeT, HeaderT, ArrayT>::ReinterpretAsDerived () const
    {
    return static_cast<const array_type*>(this);
    }


template <typename FacadeT, typename HeaderT, typename ArrayT>
typename CompositeArrayBase<FacadeT, HeaderT, ArrayT>::array_type* CompositeArrayBase<FacadeT, HeaderT, ArrayT>::ReinterpretAsDerived ()
    {
    return static_cast<array_type*>(this);
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
bool CompositeArrayBase<FacadeT, HeaderT, ArrayT>::IsValidIterator (const const_iterator& pi_Iterator)
    {
    return m_HeaderArray.IsValidIterator(pi_Iterator->GetHeaderIter());
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
bool CompositeArrayBase<FacadeT, HeaderT, ArrayT>::IsValidIterator (const iterator& pi_Iterator)
    {
    return m_HeaderArray.IsValidIterator(pi_Iterator->GetHeaderIter());
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
typename CompositeArrayBase<FacadeT, HeaderT, ArrayT>::iterator
CompositeArrayBase<FacadeT, HeaderT, ArrayT>::CreateIterator    (header_type* pi_HeaderIter)
    {
    return iterator(pi_HeaderIter, ReinterpretAsDerived());
    }

template <typename FacadeT, typename HeaderT, typename ArrayT>
typename CompositeArrayBase<FacadeT, HeaderT, ArrayT>::const_iterator
CompositeArrayBase<FacadeT, HeaderT, ArrayT>::CreateConstIterator   (header_type* pi_HeaderIter) const
    {
    return const_iterator(pi_HeaderIter, ReinterpretAsDerived());
    }


