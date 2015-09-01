//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXAList.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HIDXAList
//-----------------------------------------------------------------------------


////////////////////////
// HIDXAList
////////////////////////

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class T, int C> HIDXAList<T, C>::HIDXAList()
    {
    HASSERT(C > 0);

    m_FirstElementPos = 0;
    m_LastElementPos  = C - 1;
    m_FirstFreePos    = 0;

    m_pHead      = 0;
    m_pTail      = 0;
    m_BlockCount = 0;
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class T, int C> HIDXAList<T, C>::~HIDXAList()
    {
    // Delete all the blocks in the list
    HIDXAListBlock<T, C>* pCurrentBlock = m_pHead;
    HIDXAListBlock<T, C>* pNextBlock;

    while (pCurrentBlock != 0)
        {
        pNextBlock = pCurrentBlock->m_pNextBlock;

        delete pCurrentBlock;

        pCurrentBlock = pNextBlock;
        }
    }


//-----------------------------------------------------------------------------
// Get the first block
//-----------------------------------------------------------------------------
template<class T, int C> inline HIDXAListBlock<T, C>* HIDXAList<T, C>::GetHead() const
    {
    return m_pHead;
    }


//-----------------------------------------------------------------------------
// Get the last block
//-----------------------------------------------------------------------------
template<class T, int C> inline HIDXAListBlock<T, C>* HIDXAList<T, C>::GetTail() const
    {
    return m_pTail;
    }


//-----------------------------------------------------------------------------
// Test underflow condition
//-----------------------------------------------------------------------------
template<class T, int C> inline bool HIDXAList<T, C>::InsertWouldCauseUnderflow() const
    {
    // Two returns are for optimization...
    if (m_pHead == 0)
        {
        return false;
        }
    else
        {
        // Underflow happens if we try to go below zero...
        return (C > m_pHead->GetBlockOffset());
        }
    }


//-----------------------------------------------------------------------------
// Test overflow condition
//-----------------------------------------------------------------------------
template<class T, int C> inline bool HIDXAList<T, C>::InsertWouldCauseOverflow() const
    {
    // Two returns are for optimization...
    if (m_pTail == 0)
        {
        return false;
        }
    else
        {
        // Overflow happens if we try to go over ULONG_MAX
        return (C > ULONG_MAX - m_pTail->GetBlockOffset());
        }
    }


////////////////////////
// HIDXAListBlock
////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class T, int C> HIDXAListBlock<T, C>::HIDXAListBlock(
    uint32_t              pi_BlockOffset,
    HIDXAListBlock<T, C>* pi_pPreviousBlock,
    HIDXAListBlock<T, C>* pi_pNextBlock)
    : m_Elements(C)
    {
    HASSERT(C > 0);
    HASSERT(pi_BlockOffset > 0);

    m_BlockOffset    = pi_BlockOffset;
    m_pPreviousBlock = pi_pPreviousBlock;
    m_pNextBlock     = pi_pNextBlock;

    m_ElementCount  = 0;
    }


//-----------------------------------------------------------------------------
// Retrieve block's absolute offset
//-----------------------------------------------------------------------------
template<class T, int C> inline uint32_t HIDXAListBlock<T, C>::GetBlockOffset() const
    {
    return m_BlockOffset;
    }


//-----------------------------------------------------------------------------
// Set the block's absolute offset
//-----------------------------------------------------------------------------
template<class T, int C> inline void HIDXAListBlock<T, C>::SetBlockOffset(uint32_t pi_NewBlockOffset)
    {
    m_BlockOffset = pi_NewBlockOffset;
    }


//-----------------------------------------------------------------------------
// Get prev. block in deque
//-----------------------------------------------------------------------------
template<class T, int C> inline HIDXAListBlock<T, C>* HIDXAListBlock<T, C>::GetPreviousBlock() const
    {
    return m_pPreviousBlock;
    }


//-----------------------------------------------------------------------------
// Get next block in deque
//-----------------------------------------------------------------------------
template<class T, int C> inline HIDXAListBlock<T, C>* HIDXAListBlock<T, C>::GetNextBlock() const
    {
    return m_pNextBlock;
    }


//-----------------------------------------------------------------------------
// Retrieve an element
//-----------------------------------------------------------------------------
template<class T, int C> inline HFCPtr< HIDXIndexable<T> >& HIDXAListBlock<T, C>::operator[](size_t pi_RelativePosition) const
    {
    return m_Elements[pi_RelativePosition];
    }


//-----------------------------------------------------------------------------
// Retrieve an element
//-----------------------------------------------------------------------------
template<class T, int C> inline HFCPtr< HIDXIndexable<T> >& HIDXAListBlock<T, C>::ElementAt(uint32_t pi_AbsolutePosition) const
    {
    HASSERT(pi_AbsolutePosition >= m_BlockOffset);
    HASSERT(pi_AbsolutePosition - m_BlockOffset < C);
    return m_Elements[pi_AbsolutePosition - m_BlockOffset];
    }


//-----------------------------------------------------------------------------
// Insertion at front
//-----------------------------------------------------------------------------
template<class T, int C> void HIDXAList<T, C>::InsertBlockAtBeginning()
    {
    if (m_pHead != 0)
        {
        // Create a new block. This new block has the old head
        // as its next item, and the old head has the new block
        // before it.
        m_pHead->m_pPreviousBlock = new HIDXAListBlock<T, C>(m_pHead->GetBlockOffset() - C,
                                                             0,
                                                             m_pHead);

        // The head is now the new block
        m_pHead = m_pHead->m_pPreviousBlock;
        }
    else
        {
        // Create a new head
        m_pHead = new HIDXAListBlock<T, C>(START_OFFSET);

        // With the same tail...
        m_pTail = m_pHead;
        }

    ++m_BlockCount;
    }


//-----------------------------------------------------------------------------
// Insertion at back
//-----------------------------------------------------------------------------
template<class T, int C> void HIDXAList<T, C>::InsertBlockAtEnd()
    {
    if (m_pTail != 0)
        {
        // Create a new block. This new block has the old tail
        // as its previous item, and the old tail has the new block
        // after it.
        m_pTail->m_pNextBlock = new HIDXAListBlock<T, C>(m_pTail->GetBlockOffset() + C,
                                                         m_pTail);

        // The head is now the new block
        m_pTail = m_pTail->m_pNextBlock;
        }
    else
        {
        // Create a new head
        m_pHead = new HIDXAListBlock<T, C>(START_OFFSET);

        // With the same tail...
        m_pTail = m_pHead;
        }

    ++m_BlockCount;
    }


//-----------------------------------------------------------------------------
// Deletion of a block
//-----------------------------------------------------------------------------
template<class T, int C> void HIDXAList<T, C>::DeleteBlock(HIDXAListBlock<T, C>* pi_pBlock)
    {
    HASSERT(pi_pBlock != 0);

    // Adjust forward pointer of previous block
    if (pi_pBlock->m_pPreviousBlock != 0)
        {
        // Block's got a previous
        pi_pBlock->m_pPreviousBlock->m_pNextBlock = pi_pBlock->m_pNextBlock;
        }
    else
        {
        // Block is the first of the list
        m_pHead = pi_pBlock->m_pNextBlock;
        }

    // Adjust backward pointer of next block
    if (pi_pBlock->m_pNextBlock != 0)
        {
        // Block has a next
        pi_pBlock->m_pNextBlock->m_pPreviousBlock = pi_pBlock->m_pPreviousBlock;
        }
    else
        {
        // Block is the last
        m_pTail = pi_pBlock->m_pPreviousBlock;
        }

    // Now we can delete it.
    delete pi_pBlock;

    --m_BlockCount;
    }


//-----------------------------------------------------------------------------
// Check if 2 blocks can be merged
//-----------------------------------------------------------------------------
template<class T, int C> bool HIDXAList<T, C>::CanBlockBeMerged(
    HIDXAListBlock<T, C>*  pi_pBlock,
    unsigned short  pi_MaxCombinedSize,
    HIDXAListBlock<T, C>** po_ppOtherBlock) const
    {
    HASSERT(pi_pBlock != 0);
    HASSERT(po_ppOtherBlock != 0);

    bool CanBeMerged = false;
    uint32_t PreviousBlockCount = pi_MaxCombinedSize + 1;

    if (pi_pBlock->m_pPreviousBlock != 0)
        {
        // Check if previous block could be combined
        PreviousBlockCount = pi_pBlock->m_pPreviousBlock->m_ElementCount + pi_pBlock->m_ElementCount;
        if (PreviousBlockCount <= pi_MaxCombinedSize)
            {
            *po_ppOtherBlock = pi_pBlock->m_pPreviousBlock;
            CanBeMerged = true;
            }
        }

    if (pi_pBlock->m_pNextBlock != 0)
        {
        // Check if next block would be a better choice
        if (pi_pBlock->m_pNextBlock->m_ElementCount + pi_pBlock->m_ElementCount < PreviousBlockCount)
            {
            *po_ppOtherBlock = pi_pBlock->m_pNextBlock;
            CanBeMerged = true;
            }
        }

    return CanBeMerged;
    }


//-----------------------------------------------------------------------------
// Get the number of blocks in the list
//-----------------------------------------------------------------------------
template<class T, int C> inline uint32_t HIDXAList<T, C>::GetBlockCount() const
    {
    return m_BlockCount;
    }
END_IMAGEPP_NAMESPACE