//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXAList.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HIDXAList
//-----------------------------------------------------------------------------
// Array list used in relative indexing.
//
// DO NOT add virtual methods to these classes.
//-----------------------------------------------------------------------------

#pragma once


#include "HFCPtr.h"
#include "HIDXIndexable.h"

BEGIN_IMAGEPP_NAMESPACE

// Convert between absolute and relative positions
#define HIDXALIST_ABSOLUTE_POS(pi_pBlock, pi_RelativePosition) (pi_pBlock->GetBlockOffset() + pi_RelativePosition)

#define HIDXALIST_RELATIVE_POS(pi_pBlock, pi_AbsolutePosition) (pi_AbsolutePosition - pi_pBlock->GetBlockOffset())


////////////////////////
// HIDXAListBlock
////////////////////////

template<class T, int C> class HNOVTABLEINIT HIDXAListBlock
    {
public:

    // Constructor
    HIDXAListBlock(uint32_t              pi_BlockOffset,
                   HIDXAListBlock<T, C>* pi_pPreviousBlock = 0,
                   HIDXAListBlock<T, C>* pi_pNextBlock = 0);

    // Accessing

    HFCPtr< HIDXIndexable<T> >&
    operator[](size_t pi_RelativePosition) const;
    HFCPtr< HIDXIndexable<T> >&
    ElementAt(uint32_t pi_AbsolutePosition) const;

    // Information

    uint32_t        GetBlockOffset() const;
    void            SetBlockOffset(uint32_t pi_NewBlockOffset);

    HIDXAListBlock<T, C>*
    GetPreviousBlock() const;
    HIDXAListBlock<T, C>*
    GetNextBlock() const;

    //
    // Public attributes. These are maintained by the user.
    //

    // Number of used entries in the block.
    // We initialize it to 0 at construction (at least :-))
    uint32_t        m_ElementCount;


//    protected:

    // This class maintains the links
//        friend class HIDXAList;

    // Block linking (we're a deque)
    HIDXAListBlock<T, C>*
    m_pPreviousBlock;
    HIDXAListBlock<T, C>*
    m_pNextBlock;

private:

    typedef vector < HFCPtr< HIDXIndexable<T> >, allocator< HFCPtr< HIDXIndexable<T> > > >
    ElementVector;

    // The list of objects in the block
    mutable ElementVector
    m_Elements;

    // Value to add to relative pos. in block to obtain
    // the real position.
    uint32_t        m_BlockOffset;
    };


////////////////////////
// HIDXAList
////////////////////////

template<class T, int C> class HNOVTABLEINIT HIDXAList
    {
public:

    HIDXAList();
    ~HIDXAList();

    // Management

    void            InsertBlockAtBeginning();
    void            InsertBlockAtEnd();
    void            DeleteBlock(HIDXAListBlock<T, C>* pi_pBlock);


    // Information

    HIDXAListBlock<T, C>*
    GetHead() const;
    HIDXAListBlock<T, C>*
    GetTail() const;

    uint32_t        GetBlockCount() const;

    bool           InsertWouldCauseUnderflow() const;
    bool           InsertWouldCauseOverflow() const;

    // Caution: This method uses the m_ElementCount member of
    // the HIDXAListBlock class. The user must be usre that
    // the member has been maintained properly.
    bool           CanBlockBeMerged(HIDXAListBlock<T, C>*  pi_pBlock,
                                     unsigned short        pi_MaxCombinedSize,
                                     HIDXAListBlock<T, C>** po_ppOtherBlock) const;

    //
    // Public attributes. These are maintained by the user.
    // Initialized to:
    //      m_FirstElementPos : 0
    //      m_LastElementPos  : BlockCapacity - 1
    //      m_FirstFreePos    : 0
    //

    uint32_t        m_FirstElementPos;
    uint32_t        m_LastElementPos;
    uint32_t        m_FirstFreePos;

    enum
        {
        // ULONG_MAX / 2, rounded to a multiple of 32
        // Can be put in enum since it's a little bit
        // smaller than INT_MAX
        START_OFFSET = 2147483616
        };

private:

    // Copy ctor and assignment are disabled
    HIDXAList(const HIDXAList<T, C>& pi_rObj);
    HIDXAList<T, C>& operator=(const HIDXAList<T, C>& pi_rObj);

    // The two ends of the block list
    HIDXAListBlock<T, C>*
    m_pHead;
    HIDXAListBlock<T, C>*
    m_pTail;

    // The number of blocks in the list
    uint32_t        m_BlockCount;
    };

END_IMAGEPP_NAMESPACE
#include "HIDXAList.hpp"


