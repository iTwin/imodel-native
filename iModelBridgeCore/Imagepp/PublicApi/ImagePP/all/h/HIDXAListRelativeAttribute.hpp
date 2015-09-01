//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXAListRelativeAttribute.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HIDXAListRelativeAttribute
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class T, int C> inline HIDXAListRelativeAttribute<T, C>::HIDXAListRelativeAttribute(
    HIDXAListBlock<T, C>* pi_pBlock,
    uint32_t              pi_AbsolutePosition)
    {
    HASSERT(pi_pBlock != 0);

    m_pBlock           = pi_pBlock;
    m_AbsolutePosition = pi_AbsolutePosition;
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class T, int C> HIDXAListRelativeAttribute<T, C>::~HIDXAListRelativeAttribute()
    {
    }


//-----------------------------------------------------------------------------
// Get the block pointer
//-----------------------------------------------------------------------------
template<class T, int C> inline HIDXAListBlock<T, C>* HIDXAListRelativeAttribute<T, C>::GetBlock() const
    {
    return m_pBlock;
    }


//-----------------------------------------------------------------------------
// Get the position
//-----------------------------------------------------------------------------
template<class T, int C> inline uint32_t HIDXAListRelativeAttribute<T, C>::GetPosition() const
    {
    return m_AbsolutePosition;
    }


//-----------------------------------------------------------------------------
// Get the relative position (use the currently set block)
//-----------------------------------------------------------------------------
template<class T, int C> inline uint32_t HIDXAListRelativeAttribute<T, C>::GetRelativePosition() const
    {
    return HIDXALIST_RELATIVE_POS(m_pBlock, m_AbsolutePosition);
    }


//-----------------------------------------------------------------------------
// Set the block pointer
//-----------------------------------------------------------------------------
template<class T, int C> inline void HIDXAListRelativeAttribute<T, C>::SetBlock(HIDXAListBlock<T, C>* pi_pBlock)
    {
    HASSERT(pi_pBlock != 0);

    m_pBlock = pi_pBlock;
    }


//-----------------------------------------------------------------------------
// Set the position
//-----------------------------------------------------------------------------
template<class T, int C> inline void HIDXAListRelativeAttribute<T, C>::SetPosition(uint32_t pi_AbsolutePosition)
    {
    m_AbsolutePosition = pi_AbsolutePosition;
    }


//-----------------------------------------------------------------------------
// Set the relative position
//-----------------------------------------------------------------------------
template<class T, int C> inline void HIDXAListRelativeAttribute<T, C>::SetRelativePosition(uint32_t pi_RelativePosition)
    {
    // The block must have been set before!
    m_AbsolutePosition = HIDXALIST_ABSOLUTE_POS(m_pBlock, pi_RelativePosition);
    }


//-----------------------------------------------------------------------------
// Set the two information fields at once
//-----------------------------------------------------------------------------
template<class T, int C> inline void HIDXAListRelativeAttribute<T, C>::SetInformation(
    HIDXAListBlock<T, C>* pi_pBlock,
    uint32_t pi_AbsolutePosition)
    {
    HASSERT(pi_pBlock != 0);

    m_pBlock = pi_pBlock;
    m_AbsolutePosition = pi_AbsolutePosition;
    }


//-----------------------------------------------------------------------------
// Set the two information fields at once (position is specified relative)
//-----------------------------------------------------------------------------
template<class T, int C> inline void HIDXAListRelativeAttribute<T, C>::SetRelativeInformation(
    HIDXAListBlock<T, C>* pi_pBlock,
    uint32_t pi_RelativePosition)
    {
    HASSERT(pi_pBlock != 0);

    m_pBlock = pi_pBlock;
    m_AbsolutePosition = HIDXALIST_ABSOLUTE_POS(m_pBlock, pi_RelativePosition);
    }
END_IMAGEPP_NAMESPACE