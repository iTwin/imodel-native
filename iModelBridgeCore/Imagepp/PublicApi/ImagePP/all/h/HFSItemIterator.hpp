//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSItemIterator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> Methods for class HFSItemIterator
//:>---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Destructor.
-----------------------------------------------------------------------------*/
inline HFSItemIterator::~HFSItemIterator()
    {
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HFSItemIterator& HFSItemIterator::operator=(const HFSItemIterator& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        HFCNodeIterator::operator=(pi_rObj);
        }

    return *this;
    }

/**----------------------------------------------------------------------------
 Return the first child item.

 @return const HFCPtr<HFSItem>& The first child item, 0 if no item found.

 @see GetNextItem
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFSItem>& HFSItemIterator::GetFirstItem() const
    {

#ifdef HVERIFYCONTRACT
    // this code will be execute only in debug mode
    m_pResult = GetFirstNode();
    HPRECONDITION(m_pResult == 0 || m_pResult->IsCompatibleWith(HFSItem::CLASS_ID));
    return (const HFCPtr<HFSItem>&)m_pResult;
#else
    // this code will be execute only in release mode
    return (const HFCPtr<HFSItem>&)GetFirstNode();
#endif
    }

/**----------------------------------------------------------------------------
 Return the next child item.

 @note A call to GetFirstItem() must be call to start iteration.

 @return const HFCPtr<HFSItem>& The next child item, 0 if the no item found.
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFSItem>& HFSItemIterator::GetNextItem() const
    {
#ifdef HVERIFYCONTRACT
    // this code will be execute only in debug mode
    m_pResult = GetNextNode();
    HPRECONDITION(m_pResult == 0 || m_pResult->IsCompatibleWith(HFSItem::CLASS_ID));
    return (const HFCPtr<HFSItem>&)m_pResult;
#else
    // this code will be execute only in release mode
    return (const HFCPtr<HFSItem>&)GetNextNode();
#endif
    }

/**----------------------------------------------------------------------------
 Return the current child item.

 @note A call to GetFirstItem() must be call to start iteration.

 @return const HFCPtr<HFSItem>& The next child item, 0 if the no item found.

 @see GetFirstItem
 @see GetNextItem
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFSItem>& HFSItemIterator::GetItem() const
    {
#ifdef HVERIFYCONTRACT
    // this code will be execute only in debug mode
    m_pResult = GetNode();
    HPRECONDITION(m_pResult == 0 || m_pResult->IsCompatibleWith(HFSItem::CLASS_ID));
    return (const HFCPtr<HFSItem>&)m_pResult;
#else
    // this code will be execute only in release mode
    return (const HFCPtr<HFSItem>&)GetNode();
#endif

    }
END_IMAGEPP_NAMESPACE