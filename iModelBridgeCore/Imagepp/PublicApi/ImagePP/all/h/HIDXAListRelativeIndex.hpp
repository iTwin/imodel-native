//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXAListRelativeIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXAListRelativeIndex
//-----------------------------------------------------------------------------
// General class for relative ordering indexes.
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class O, class SI> inline HIDXAListRelativeIndex<O, SI>::HIDXAListRelativeIndex(
    const Parameters& pi_rParameters,
    const SI*         pi_pSubIndex)
    : m_pSubIndex(pi_pSubIndex)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class O, class SI> inline HIDXAListRelativeIndex<O, SI>::~HIDXAListRelativeIndex()
    {
    }


//-----------------------------------------------------------------------------
// Add an object
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HIDXAListRelativeIndex<O, SI>::Add(const O pi_Object)
    {
    // We can add the object using an indexable
    AddIndexable(new HIDXIndexable<O>(pi_Object));
    }


//-----------------------------------------------------------------------------
// Add an object
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HIDXAListRelativeIndex<O, SI>::Add(const ObjectList& pi_rObjects)
    {
    typename ObjectList::const_iterator Itr(pi_rObjects.begin());
    while (Itr != pi_rObjects.end())
        {
        // We can add the object using an indexable
        AddIndexable(new HIDXIndexable<O>(*Itr));

        ++Itr;
        }
    }


//-----------------------------------------------------------------------------
// Add elements
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HIDXAListRelativeIndex<O, SI>::AddIndexables(
    const HAutoPtr< typename HIDXIndexable<O>::List >& pi_rpObjects)
    {
    typename HIDXIndexable<O>::List::const_iterator Itr(pi_rpObjects->begin());
    while (Itr != pi_rpObjects->end())
        {
        // We can add the object using an indexable
        AddIndexable(*Itr);

        ++Itr;
        }
    }

//-----------------------------------------------------------------------------
// Remove an object
//-----------------------------------------------------------------------------
template<class O, class SI> inline void HIDXAListRelativeIndex<O, SI>::Remove(const O pi_Object)
    {
    // We can remove the object using an indexable
    RemoveIndexable(new HIDXIndexable<O>(pi_Object));
    }

//-----------------------------------------------------------------------------
// Add an element
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::AddIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    HINVARIANTS;

    // Make sure we don't already contain the object
    HASSERT(pi_rpObject->GetAttribute(this) == 0);
    HASSERT(GetFilledIndexableFor(pi_rpObject)->GetAttribute(this) == 0);

    // The confidence represents the possibility
    // that the insert will succeed.
    // 0 : We really think it will
    // 1 : We already tried to reorganize. Try to insert a last time.
    int ConfidenceLevel = 0;

    // This loop will be controled by continue and break statements.
    // It is coded this way to save tests in the normal flow
    // of operation.
    while (true)
        {
        if (m_List.m_LastElementPos == BLOCK_CAPACITY - 1)
            {
            // We need to insert a new block
            if (m_List.InsertWouldCauseOverflow())
                {
                // Busted!
                if (ConfidenceLevel == 0)
                    {
                    TryToMakeSpaceAtTheEnd();

                    ConfidenceLevel = 1;

                    // Use the continue statement here to re-try
                    // the insertion after the reorganization.
                    continue;
                    }
                else
                    {
                    // This was the last insertion try, and
                    // it failed. Stop trying, the insertion failed :(
                    break;
                    }
                }
            else
                {
                m_List.InsertBlockAtEnd();

                // Insert at first position in new block
                pi_rpObject->AddAttribute(this, new Attribute(m_List.GetTail(), HIDXALIST_ABSOLUTE_POS(m_List.GetTail(), 0)));
                (*m_List.GetTail())[0] = pi_rpObject;

                m_List.GetTail()->m_ElementCount = 1;

                m_List.m_LastElementPos = 0;

                if (m_List.GetTail() == m_List.GetHead())
                    {
                    // We've just added the first block. Since it is the
                    // head, we must update m_FirstXxx variables
                    m_List.m_FirstElementPos = 0;
                    m_List.m_FirstFreePos    = 1;
                    }
                }
            }
        else
            {
            // Adding in an already existing and non-full block.

            ++m_List.m_LastElementPos;
            pi_rpObject->AddAttribute(this, new Attribute(m_List.GetTail(), HIDXALIST_ABSOLUTE_POS(m_List.GetTail(), m_List.m_LastElementPos)));
            (*m_List.GetTail())[m_List.m_LastElementPos] = pi_rpObject;

            ++(m_List.GetTail()->m_ElementCount);

            // If we're working on the first block and we used the first free space,
            // adjust the first free space marker
            if (m_List.GetTail() == m_List.GetHead() &&
                m_List.m_FirstFreePos == m_List.m_LastElementPos)
                m_List.m_FirstFreePos++;
            }

        // If we've got here, the insertion succeeded. Stop the loop!
        break;
        }

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Remove an element
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::RemoveIndexable(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject)
    {
    HINVARIANTS;

    // To keep the object alive long enough to do our work
    HFCPtr< HIDXIndexable<O> > pObject = pi_rpObject;

    Attribute* pAttribute = (Attribute*) pObject->GetAttribute(this);

    if (pAttribute == 0)
        {
        pObject = GetFilledIndexableFor(pi_rpObject);
        pAttribute = (Attribute*) pObject->GetAttribute(this);
        }

    if (pAttribute != 0)
        {
        // Clear the entry
        pAttribute->GetBlock()->ElementAt(pAttribute->GetPosition()) = 0;

        // Check what blocks have been touched
        bool HeadChanged  = false;
        bool TailChanged  = false;
        bool BlockDeleted = false;
        if (pAttribute->GetBlock() == m_List.GetHead())
            HeadChanged = true;
        if (pAttribute->GetBlock() == m_List.GetTail())
            TailChanged = true;

        // One less element. Delete block if empty
        if (--(pAttribute->GetBlock()->m_ElementCount) == 0)
            {
            m_List.DeleteBlock(pAttribute->GetBlock());
            BlockDeleted = true;
            }

        // Check if we worked on the first block.
        if (HeadChanged)
            {
            if (m_List.GetHead())
                {
                if (BlockDeleted)
                    {
                    // Old head was deleted.

                    // Find new values for the free space and
                    // first element markers
                    int i = 0;
                    m_List.m_FirstElementPos = BLOCK_CAPACITY;
                    m_List.m_FirstFreePos = BLOCK_CAPACITY;

                    while ((m_List.m_FirstElementPos == BLOCK_CAPACITY ||
                            m_List.m_FirstFreePos    == BLOCK_CAPACITY) &&
                           i < BLOCK_CAPACITY)
                        {
                        if ((*m_List.GetHead())[i] == 0)
                            {
                            if (m_List.m_FirstFreePos == BLOCK_CAPACITY)
                                m_List.m_FirstFreePos = i;
                            }
                        else
                            {
                            if (m_List.m_FirstElementPos == BLOCK_CAPACITY)
                                m_List.m_FirstElementPos = i;
                            }
                        ++i;
                        }

                    // Should never happen, since the first block is not empty
                    HASSERT(m_List.m_FirstElementPos < BLOCK_CAPACITY);
                    }
                else
                    {
                    // Adjust the free space marker
                    if (pAttribute->GetRelativePosition() < m_List.m_FirstFreePos)
                        m_List.m_FirstFreePos = pAttribute->GetRelativePosition();

                    // Adjust the first used element marker
                    if (pAttribute->GetRelativePosition() == m_List.m_FirstElementPos)
                        {
                        ++m_List.m_FirstElementPos;
                        while (m_List.m_FirstElementPos < BLOCK_CAPACITY &&
                               (*m_List.GetHead())[m_List.m_FirstElementPos] == 0)
                            ++m_List.m_FirstElementPos;

                        // Should never happen, since the first block is not empty
                        HASSERT(m_List.m_FirstElementPos < BLOCK_CAPACITY);
                        }
                    }
                }
            else
                {
                // List is empty. Default values...

                m_List.m_FirstElementPos = 0;
                m_List.m_LastElementPos  = BLOCK_CAPACITY - 1;
                m_List.m_FirstFreePos    = 0;
                }
            }

        if (TailChanged)
            {
            if (m_List.GetTail())
                {
                // Adjust the last used element marker

                if (BlockDeleted)
                    {
                    // Old tail was deleted. Find the last used entry
                    // starting from the end.

                    m_List.m_LastElementPos = BLOCK_CAPACITY - 1;
                    while (m_List.m_LastElementPos >= 0 &&
                           (*m_List.GetTail())[m_List.m_LastElementPos] == 0)
                        {
                        --m_List.m_LastElementPos;
                        }

                    // Verify underflow condition
                    HASSERT(m_List.m_LastElementPos < BLOCK_CAPACITY);
                    }

                else
                    {
                    if (pAttribute->GetRelativePosition() == m_List.m_LastElementPos)
                        {
                        // Last element was removed. Find a new one...

                        --m_List.m_LastElementPos;
                        while (m_List.m_LastElementPos >= 0 &&
                               (*m_List.GetTail())[m_List.m_LastElementPos] == 0)
                            {
                            --m_List.m_LastElementPos;
                            }
                        }
                    }
                }
            }

        // Remove our attribute from the object.
        pObject->RemoveAttribute(this);
        }

    HINVARIANTS;
    }



//-----------------------------------------------------------------------------
// Promote element (put it farther in the list)
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::Promote(const O pi_rpObject)
    {
    HINVARIANTS;

    HFCPtr< HIDXIndexable<O> > pObject(new HIDXIndexable<O>(pi_rpObject));

    Attribute* pAttribute = (Attribute*) pObject->GetAttribute(this);

    if (pAttribute == 0)
        {
        pObject = GetFilledIndexableFor(pObject);
        pAttribute = (Attribute*) pObject->GetAttribute(this);
        }

    if (pAttribute != 0)
        {
        // find the object following it.

        HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = pAttribute->GetBlock();
        uint32_t CurrentIndex = pAttribute->GetRelativePosition() + 1;
        uint32_t LastIndex    = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;
        bool Found         = false;

        // Pass through all blocks
        while (!Found && pCurrentBlock)
            {
            // Pass through all elements of the block
            while (!Found && CurrentIndex <= LastIndex)
                {
                if ((*pCurrentBlock)[CurrentIndex] != 0)
                    Found = true;
                else
                    ++CurrentIndex;
                }

            if (!Found)
                {
                CurrentIndex = 0;
                pCurrentBlock = pCurrentBlock->GetNextBlock();
                LastIndex = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;
                }
            }

        if (Found)
            {
            // Replace current element with the one over it
            pAttribute->GetBlock()->ElementAt(pAttribute->GetPosition()) = (*pCurrentBlock)[CurrentIndex];
            ((Attribute*) pAttribute->GetBlock()->ElementAt(pAttribute->GetPosition())->GetAttribute(this))->SetInformation(pAttribute->GetBlock(), pAttribute->GetPosition());

            // And put current element in its place
            (*pCurrentBlock)[CurrentIndex] = pObject;
            ((Attribute*) (*pCurrentBlock)[CurrentIndex]->GetAttribute(this))->SetRelativeInformation(pCurrentBlock, CurrentIndex);
            }
        }

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Demote element (put it higher in the list)
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::Demote(const O pi_rpObject)
    {
    HINVARIANTS;

    HFCPtr< HIDXIndexable<O> > pObject(new HIDXIndexable<O>(pi_rpObject));

    Attribute* pAttribute = (Attribute*) pObject->GetAttribute(this);

    if (pAttribute == 0)
        {
        pObject = GetFilledIndexableFor(pObject);
        pAttribute = (Attribute*) pObject->GetAttribute(this);
        }

    if (pAttribute != 0)
        {
        // find the object preceding it.

        HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = pAttribute->GetBlock();
        bool ChangeBlock = pAttribute->GetRelativePosition() == 0;
        uint32_t CurrentIndex = pAttribute->GetRelativePosition() - 1;
        uint32_t FirstIndex   = pCurrentBlock == m_List.GetHead() ? m_List.m_FirstElementPos : 0;
        bool Found         = false;

        // Pass through all blocks
        while (!Found && pCurrentBlock)
            {
            // Pass through all elements of the block
            while (!Found && !ChangeBlock)
                {
                if ((*pCurrentBlock)[CurrentIndex] != 0)
                    Found = true;
                else
                    {
                    if (CurrentIndex > 0)
                        --CurrentIndex;
                    else
                        ChangeBlock = true;
                    }
                }

            if (!Found)
                {
                CurrentIndex  = BLOCK_CAPACITY - 1;
                pCurrentBlock = pCurrentBlock->GetPreviousBlock();
                FirstIndex    = pCurrentBlock == m_List.GetHead() ? m_List.m_FirstElementPos : 0;
                ChangeBlock   = false;
                }
            }

        if (Found)
            {
            // Replace current element with the one over it
            pAttribute->GetBlock()->ElementAt(pAttribute->GetPosition()) = (*pCurrentBlock)[CurrentIndex];
            ((Attribute*) pAttribute->GetBlock()->ElementAt(pAttribute->GetPosition())->GetAttribute(this))->SetInformation(pAttribute->GetBlock(), pAttribute->GetPosition());

            // And put current element in its place
            (*pCurrentBlock)[CurrentIndex] = pObject;
            ((Attribute*) (*pCurrentBlock)[CurrentIndex]->GetAttribute(this))->SetRelativeInformation(pCurrentBlock, CurrentIndex);
            }
        }

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Put an element at the front (end of list)
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::Front(const O pi_rpObject)
    {
    HINVARIANTS;

    HFCPtr< HIDXIndexable<O> > pObject(new HIDXIndexable<O>(pi_rpObject));
    pObject = GetFilledIndexableFor(pObject);

    RemoveIndexable(pObject);
    AddIndexable(pObject);

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Put an element at the back (beginning of list)
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::Back(const O pi_rpObject)
    {
    HINVARIANTS;

    HFCPtr< HIDXIndexable<O> > pObject(new HIDXIndexable<O>(pi_rpObject));
    pObject = GetFilledIndexableFor(pObject);

    RemoveIndexable(pObject);

    // Have to add at the beginning...

    // Ensure we've got a block
    if (!m_List.GetHead())
        {
        // t'is the only element

        m_List.InsertBlockAtEnd();

        (*m_List.GetHead())[0] = pObject;
        pObject->AddAttribute(this, new Attribute(m_List.GetHead(), HIDXALIST_ABSOLUTE_POS(m_List.GetHead(), 0)));

        m_List.m_FirstElementPos = 0;
        m_List.m_FirstFreePos = 1;
        m_List.m_LastElementPos = 0;

        // We added an element there
        m_List.GetHead()->m_ElementCount++;
        }
    else
        {
        // The confidence represents the possibility
        // that the operation will succeed.
        // 0 : We really think it will
        // 1 : We already tried to reorganize. Try a last time.
        int ConfidenceLevel = 0;

        // This loop will be controled by continue and break statements.
        // It is coded this way to save tests in the normal flow
        // of operation.
        while (true)
            {
            if (m_List.m_FirstElementPos == 0)
                {
                // First position is occupied.

                if (m_List.m_FirstFreePos < BLOCK_CAPACITY / 2)
                    {
                    // We will shift a few elements...
                    for (int i = m_List.m_FirstFreePos ; i > 0 ; i--)
                        {
                        (*m_List.GetHead())[i] = (*m_List.GetHead())[i-1];
                        ((Attribute*)(*m_List.GetHead())[i]->GetAttribute(this))->SetRelativePosition(i);
                        }

                    (*m_List.GetHead())[0] = pObject;
                    pObject->AddAttribute(this, new Attribute(m_List.GetHead(), HIDXALIST_ABSOLUTE_POS(m_List.GetHead(), 0)));

                    // We have added a new element in the block. We must adjust
                    // since the RemoveIndexable call has decremented it...
                    if ((m_List.GetHead() == m_List.GetTail()) && m_List.m_LastElementPos < m_List.m_FirstFreePos)
                        m_List.m_LastElementPos++;

                    // Find the first free position
                    m_List.m_FirstFreePos++;
                    while (m_List.m_FirstFreePos < BLOCK_CAPACITY && ((*m_List.GetHead())[m_List.m_FirstFreePos] != 0))
                        m_List.m_FirstFreePos++;
                    }
                else
                    {
                    // We must add a new block
                    if (m_List.InsertWouldCauseUnderflow())
                        {
                        // Busted!
                        if (ConfidenceLevel == 0)
                            {
                            TryToMakeSpaceAtTheBeginning();

                            ConfidenceLevel = 1;

                            // Use the continue statement here to re-try
                            // the operation after the reorganization.
                            continue;
                            }
                        else
                            {
                            // This was the last try, and it failed.
                            // Stop trying :(
                            break;
                            }
                        }
                    else
                        {
                        m_List.InsertBlockAtBeginning();
                        m_List.m_FirstElementPos = BLOCK_CAPACITY - 1;
                        m_List.m_FirstFreePos = BLOCK_CAPACITY;
                        (*m_List.GetHead())[m_List.m_FirstElementPos] = pObject;
                        pObject->AddAttribute(this, new Attribute(m_List.GetHead(), HIDXALIST_ABSOLUTE_POS(m_List.GetHead(), m_List.m_FirstElementPos)));
                        }
                    }
                }
            else
                {
                // We can safely insert before the current first element
                m_List.m_FirstElementPos--;
                (*m_List.GetHead())[m_List.m_FirstElementPos] = pObject;
                pObject->AddAttribute(this, new Attribute(m_List.GetHead(), HIDXALIST_ABSOLUTE_POS(m_List.GetHead(), m_List.m_FirstElementPos)));

                if (m_List.m_FirstElementPos == m_List.m_FirstFreePos)
                    {
                    // We have used the first free position. Find a new first free position
                    m_List.m_FirstFreePos++;
                    while (m_List.m_FirstFreePos < BLOCK_CAPACITY && ((*m_List.GetHead())[m_List.m_FirstFreePos] != 0))
                        m_List.m_FirstFreePos++;
                    }
                }

            // We added an element there
            m_List.GetHead()->m_ElementCount++;

            // If we got here, the operation succeeded. Stop the loop!
            break;
            }
        }

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Query
//-----------------------------------------------------------------------------
template<class O, class SI> typename HIDXAListRelativeIndex<O, SI>::ObjectList* HIDXAListRelativeIndex<O, SI>::Query(
    const HIDXSearchCriteria& pi_rCriteria) const
    {
    HINVARIANTS;

    // Since we don't support criterias, we return all the objects, sorted...
    HAutoPtr<ObjectList> pList(new ObjectList);

    HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetHead();
    uint32_t CurrentIndex = m_List.m_FirstElementPos;
    uint32_t LastIndex    = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;

    while (pCurrentBlock != 0)
        {
        // Pass through all elements of the block
        while (CurrentIndex <= LastIndex)
            {
            if ((*pCurrentBlock)[CurrentIndex] != 0)
                {
                pList->push_back((*pCurrentBlock)[CurrentIndex]->GetObject());
                }
            ++CurrentIndex;
            }

        CurrentIndex = 0;
        pCurrentBlock = pCurrentBlock->GetNextBlock();
        LastIndex = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;
        }

    return pList.release();
    }


//-----------------------------------------------------------------------------
// Internal query
//-----------------------------------------------------------------------------
template<class O, class SI> typename HIDXIndexable<O>::List* HIDXAListRelativeIndex<O, SI>::QueryIndexables(
    const HIDXSearchCriteria& pi_rCriteria,
    bool                     pi_Sort) const
    {
    HINVARIANTS;

    HAutoPtr<typename HIDXIndexable<O>::List> pList(new typename HIDXIndexable<O>::List);

    // No subset. Return the full list

    HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetHead();
    uint32_t CurrentIndex = m_List.m_FirstElementPos;
    uint32_t LastIndex    = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;

    while (pCurrentBlock != 0)
        {
        // Pass through all elements of the block
        while (CurrentIndex <= LastIndex)
            {
            if ((*pCurrentBlock)[CurrentIndex] != 0)
                {
                pList->push_back((*pCurrentBlock)[CurrentIndex]);
                }
            ++CurrentIndex;
            }

        CurrentIndex = 0;
        pCurrentBlock = pCurrentBlock->GetNextBlock();
        LastIndex = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;
        }

    return pList.release();
    }


//-----------------------------------------------------------------------------
// Internal query
//-----------------------------------------------------------------------------
template<class O, class SI> typename HIDXIndexable<O>::List* HIDXAListRelativeIndex<O, SI>::QueryIndexables(
    const HIDXSearchCriteria&         pi_rCriteria,
    HAutoPtr<typename HIDXIndexable<O>::List>& pi_rSubset,
    bool                             pi_Sort) const
    {
    HINVARIANTS;

    if (pi_Sort)
        {
        // There is a subset. Sort it

        HAutoPtr<typename HIDXIndexable<O>::List> pList(new typename HIDXIndexable<O>::List);

        vector<uint32_t> Elements;

        // Pass elements to sort, and light the corresponding bits.

        typename HIDXIndexable<O>::List::const_iterator Itr(pi_rSubset->begin());
        while (Itr != pi_rSubset->end())
            {
            // Elements we receive
            HASSERT((*Itr)->GetAttribute(this) != 0);

            Elements.push_back(((Attribute*)(*Itr)->GetAttribute(this))->GetPosition());

            ++Itr;
            }

        // Place elements in ascending order
        sort(Elements.begin(), Elements.end());

        // Retrieve elements in that order...
        HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetHead();
        vector<uint32_t>::iterator ElementsItr(Elements.begin());
        while (ElementsItr != Elements.end())
            {
            // Advance to corresponding block
            while ((*ElementsItr) >= (pCurrentBlock->GetBlockOffset() + BLOCK_CAPACITY))
                {
                pCurrentBlock = pCurrentBlock->GetNextBlock();

                HASSERT(pCurrentBlock != 0);
                }

            // Take element
            pList->push_back(pCurrentBlock->ElementAt(*ElementsItr));
            HASSERT(pCurrentBlock->ElementAt(*ElementsItr) != 0);

            ++ElementsItr;
            }

        // Should return the same number of elements as we received
        HASSERT(pList->size() == pi_rSubset->size());

        return pList.release();
        }
    else
        {
        // No sort. We simply return the subset
        return pi_rSubset.release();
        }
    }


//-----------------------------------------------------------------------------
// Retrieve the internal indexable for the specified object
//-----------------------------------------------------------------------------
template<class O, class SI> const HFCPtr< HIDXIndexable<O> > HIDXAListRelativeIndex<O, SI>::GetFilledIndexableFor(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const
    {
    HINVARIANTS;

    HFCPtr<HIDXIndexable<O> > pResult( pi_rpObject );

    // Check in sub-index first
    if (m_pSubIndex != 0)
        {
        pResult = m_pSubIndex->GetFilledIndexableFor(pi_rpObject);
        }

    if (pResult == pi_rpObject)
        {
        // Not found in sub-index. Look for it in our list... (The hard way)
        HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetHead();
        uint32_t CurrentIndex = m_List.m_FirstElementPos;
        uint32_t LastIndex    = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;
        bool Found         = false;

        // Pass through all blocks
        while (!Found && pCurrentBlock)
            {
            // Pass through all elements of the block
            while (!Found && CurrentIndex <= LastIndex)
                {
                if ((*pCurrentBlock)[CurrentIndex] != 0 &&
                    pi_rpObject->IndexesSameObjectAs(*((*pCurrentBlock)[CurrentIndex])) )
                    {
                    Found = true;
                    pResult = (*pCurrentBlock)[CurrentIndex];
                    }
                ++CurrentIndex;
                }

            CurrentIndex = 0;
            pCurrentBlock = pCurrentBlock->GetNextBlock();
            LastIndex = pCurrentBlock == m_List.GetTail() ? m_List.m_LastElementPos : BLOCK_CAPACITY - 1;
            }
        }

    return pResult;
    }


//-----------------------------------------------------------------------------
// This index can't retrieve objects interacting with a specific one
//-----------------------------------------------------------------------------
template<class O, class SI> inline bool HIDXAListRelativeIndex<O, SI>::SupportsInteractingRetrieval() const
    {
    return false;
    }


//-----------------------------------------------------------------------------
// This index can't retrieve interacting objects
//-----------------------------------------------------------------------------
template<class O, class SI> inline typename HIDXIndexable<O>::List* HIDXAListRelativeIndex<O, SI>::GetInteractingObjects(
    typename HIDXIndexable<O>::List const& pi_rpObjects) const
    {
    return new typename HIDXIndexable<O>::List;
    }


//-----------------------------------------------------------------------------
// Ask the index if it needs to perform some sorting
//-----------------------------------------------------------------------------
template<class O, class SI> inline HIDXSortingRequirement HIDXAListRelativeIndex<O, SI>::GetSortingRequirement() const
    {
    // Always sort, because we don't handle criterias...
    return SORTING_SIMPLE;
    }


//-----------------------------------------------------------------------------
// Get the index's sorting function
//-----------------------------------------------------------------------------
template<class O, class SI> inline typename HIDXAListRelativeIndex<O, SI>::Predicate
HIDXAListRelativeIndex<O, SI>::GetPredicate() const
    {
    return Predicate(this);
    }


//-----------------------------------------------------------------------------
// Reorganize so that we can insert at the end
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::TryToMakeSpaceAtTheEnd()
    {
    HINVARIANTS;

    HASSERT(m_List.GetTail() != 0);

    //
    // FIRST
    // Try to find an empty space inside a block. If there
    // is one, shift the elements to make an empty space at the end.
    //

    HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetTail();
    uint32_t CurrentIndex = m_List.m_LastElementPos;
    bool Found         = false;

    // Look for an empty space
    while (!Found && pCurrentBlock)
        {
        while (!Found && CurrentIndex >= 0)
            {
            if ((*pCurrentBlock)[CurrentIndex] == 0)
                {
                Found = true;
                }
            else
                {
                // unsigned. We need to stop or else we'll loop infinitely...
                if (CurrentIndex == 0)
                    break;

                CurrentIndex--;
                }
            }

        if (!Found)
            {
            CurrentIndex  = BLOCK_CAPACITY - 1;
            pCurrentBlock = pCurrentBlock->GetPreviousBlock();
            }
        }

    if (Found)
        {
        // A space was found. Shift the elements

        // The current block gets one more element
        pCurrentBlock->m_ElementCount++;

        if (pCurrentBlock == m_List.GetHead())
            {
            // If left of first element is all blank, the first element
            // will now be the current empty space (after the shift)
            if (CurrentIndex < m_List.m_FirstElementPos)
                m_List.m_FirstElementPos = CurrentIndex;

            if (CurrentIndex == m_List.m_FirstFreePos)
                {
                // Was the first free position. There will now be none.
                m_List.m_FirstFreePos = BLOCK_CAPACITY;
                }
            }

        while (pCurrentBlock)
            {
            for ( ; CurrentIndex < BLOCK_CAPACITY - 1 ; ++CurrentIndex)
                {
                // Shift in the same block
                (*pCurrentBlock)[CurrentIndex] = (*pCurrentBlock)[CurrentIndex+1];
                ((Attribute*)(*pCurrentBlock)[CurrentIndex]->GetAttribute(this))->SetRelativePosition(CurrentIndex);
                }

            if (pCurrentBlock->GetNextBlock())
                {
                // Take first element of next block to place in last element
                // of the current block
                (*pCurrentBlock)[CurrentIndex] = (*pCurrentBlock->GetNextBlock())[0];
                ((Attribute*)(*pCurrentBlock)[CurrentIndex]->GetAttribute(this))->SetRelativeInformation(pCurrentBlock, CurrentIndex);
                }

            pCurrentBlock = pCurrentBlock->GetNextBlock();
            CurrentIndex  = 0;
            }

        // The last block has lost an element
        m_List.GetTail()->m_ElementCount--;

        // The block's last entry is now empty.
        m_List.m_LastElementPos--;
        }
    else
        {
        //
        // SECOND
        // All current blocks are full. If it's possible, we'll
        // shift full blocks to be able to add a new block at the
        // end.

        if (!m_List.InsertWouldCauseUnderflow())
            {
            pCurrentBlock = m_List.GetHead();

            while (pCurrentBlock)
                {
                // Move block
                pCurrentBlock->SetBlockOffset(pCurrentBlock->GetBlockOffset() - BLOCK_CAPACITY);

                for (int i = 0 ; i < BLOCK_CAPACITY ; i++)
                    {
                    // Relative position didn't change, but this is enough to get
                    // the attribute recalculate its absolute position based on
                    // the block's new offset.
                    ((Attribute*)(*pCurrentBlock)[i]->GetAttribute(this))->SetRelativePosition(i);
                    }

                pCurrentBlock = pCurrentBlock->GetNextBlock();
                }
            }
        else
            {
            // All else fails! The list is really full of elements!
            HASSERT(false);
            }
        }
    }


//-----------------------------------------------------------------------------
// Reorganize so that we can insert at the beginning
//-----------------------------------------------------------------------------
template<class O, class SI> void HIDXAListRelativeIndex<O, SI>::TryToMakeSpaceAtTheBeginning()
    {
    HINVARIANTS;

    HASSERT(m_List.GetHead() != 0);

    //
    // FIRST
    // Try to find an empty space inside a block. If there
    // is one, shift the elements to make an empty space at the beginning.
    //

    HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetHead();
    uint32_t CurrentIndex = m_List.m_FirstElementPos;
    bool Found         = false;

    // Look for an empty space
    while (!Found && pCurrentBlock)
        {
        while (!Found && CurrentIndex < BLOCK_CAPACITY)
            {
            if ((*pCurrentBlock)[CurrentIndex] == 0)
                {
                Found = true;
                }
            else
                {
                ++CurrentIndex;
                }
            }

        if (!Found)
            {
            CurrentIndex  = 0;
            pCurrentBlock = pCurrentBlock->GetNextBlock();
            }
        }

    if (Found)
        {
        // A space was found. Shift the elements

        // The current block gets one more element
        pCurrentBlock->m_ElementCount++;

        // Adjust last element if we're moving it
        if (pCurrentBlock == m_List.GetTail() && CurrentIndex > m_List.m_LastElementPos)
            {
            m_List.m_LastElementPos = CurrentIndex;
            }

        while (pCurrentBlock)
            {
            for ( ; CurrentIndex > 0 ; --CurrentIndex)
                {
                // Shift in the same block
                (*pCurrentBlock)[CurrentIndex] = (*pCurrentBlock)[CurrentIndex-1];
                ((Attribute*)(*pCurrentBlock)[CurrentIndex]->GetAttribute(this))->SetRelativePosition(CurrentIndex);
                }

            if (pCurrentBlock->GetPreviousBlock())
                {
                // Take first element of next block to place in last element
                // of the current block
                (*pCurrentBlock)[CurrentIndex] = (*pCurrentBlock->GetPreviousBlock())[BLOCK_CAPACITY-1];
                ((Attribute*)(*pCurrentBlock)[CurrentIndex]->GetAttribute(this))->SetRelativeInformation(pCurrentBlock, CurrentIndex);
                }

            pCurrentBlock = pCurrentBlock->GetPreviousBlock();
            CurrentIndex  = BLOCK_CAPACITY - 1;
            }

        // We're creating an empty space at the beginning...
        m_List.m_FirstElementPos = 1;
        m_List.m_FirstFreePos    = 0;

        // Head has lost one entry
        m_List.GetHead()->m_ElementCount--;
        }
    else
        {
        //
        // SECOND
        // All current blocks are full. If it's possible, we'll
        // shift full blocks to be able to add a new block at the
        // beginning.

        if (!m_List.InsertWouldCauseOverflow())
            {
            pCurrentBlock = m_List.GetTail();

            while (pCurrentBlock)
                {
                // Move block
                pCurrentBlock->SetBlockOffset(pCurrentBlock->GetBlockOffset() + BLOCK_CAPACITY);

                for (int i = 0 ; i < BLOCK_CAPACITY ; i++)
                    {
                    // Relative position didn't change, but this is enough to get
                    // the attribute recalculate its absolute position based on
                    // the block's new offset.
                    ((Attribute*)(*pCurrentBlock)[i]->GetAttribute(this))->SetRelativePosition(i);
                    }

                pCurrentBlock = pCurrentBlock->GetPreviousBlock();
                }
            }
        else
            {
            // All else fails! The list is really full of elements!
            HASSERT(false);
            }
        }
    }



#ifdef HVERIFYCONTRACT
template<class O, class SI> inline void HIDXAListRelativeIndex<O, SI>::ValidateInvariants() const
    {
    // Check blocks (count, etc...)
    int BlockCount = 0;
    HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetHead();
    while (pCurrentBlock != 0)
        {
        ++BlockCount;
        HIDXAListBlock<O, BLOCK_CAPACITY>* pNextBlock = pCurrentBlock->m_pNextBlock;
        if (pNextBlock != 0)
            {
            HASSERT(pNextBlock->m_pPreviousBlock == pCurrentBlock);
            }
        pCurrentBlock = pNextBlock;
        }
    HASSERT(BlockCount == m_List.GetBlockCount());

    if (BlockCount > 0)
        {
        // Check m_FirstElementPos
        pCurrentBlock = m_List.GetHead();
        int Position = 0;
        while (Position < BLOCK_CAPACITY)
            {
            if ((*pCurrentBlock)[Position] != 0)
                break;
            ++Position;
            }
        HASSERT(Position == m_List.m_FirstElementPos);
        HASSERT(m_List.m_FirstElementPos < BLOCK_CAPACITY);

        // Check m_FirstFreePos
        pCurrentBlock = m_List.GetHead();
        Position = 0;
        while (Position < BLOCK_CAPACITY)
            {
            if ((*pCurrentBlock)[Position] == 0)
                break;
            ++Position;
            }
        HASSERT(Position == m_List.m_FirstFreePos);
        HASSERT(m_List.m_FirstFreePos <= BLOCK_CAPACITY);

        // Check m_LastElementPos
        pCurrentBlock = m_List.GetTail();
        Position = BLOCK_CAPACITY - 1;
        while (Position >= 0)
            {
            if ((*pCurrentBlock)[Position] != 0)
                break;
            --Position;
            }
        HASSERT(Position == m_List.m_LastElementPos);
        HASSERT(m_List.m_LastElementPos < BLOCK_CAPACITY);

        // Check elements in each block
        pCurrentBlock = m_List.GetHead();
        while (pCurrentBlock != 0)
            {
            int ElementCount = 0;
            for (int i = 0 ; i < BLOCK_CAPACITY ; ++i)
                {
                if ((*pCurrentBlock)[i] != 0)
                    {
                    ++ElementCount;

                    // Verify information in elements's attribute
                    HASSERT(((Attribute*)(*pCurrentBlock)[i]->GetAttribute(this))->GetBlock() == pCurrentBlock);

                    uint32_t Position = ((Attribute*)(*pCurrentBlock)[i]->GetAttribute(this))->GetPosition();
                    HASSERT(Position >= pCurrentBlock->GetBlockOffset() && Position < pCurrentBlock->GetBlockOffset() + BLOCK_CAPACITY);

                    uint32_t RelativePosition = ((Attribute*)(*pCurrentBlock)[i]->GetAttribute(this))->GetRelativePosition();
                    HASSERT(RelativePosition < m_List.GetBlockCount() * BLOCK_CAPACITY);
                    }
                }
            HASSERT(ElementCount == pCurrentBlock->m_ElementCount);

            pCurrentBlock = pCurrentBlock->m_pNextBlock;
            }
        }
    }
#endif



// This will be good in a later compiler version. We'll be able to
// remove the code inside DefaultSubIndexType...
#if 0
//-----------------------------------------------------------------------------
// Retrieve the internal indexable for the specified object
//-----------------------------------------------------------------------------
template<class O, SI> inline const HFCPtr< HIDXIndexable<O> > HIDXAListRelativeIndex<O, SI>::GetFilledIndexableFor(
    const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const
    {
    HFCPtr< HIDXIndexable<O> > pResult( pi_rpObject );

    // Not found in sub-index. Look for it in our list... (The hard way)
    HIDXAListBlock<O, BLOCK_CAPACITY>* pCurrentBlock = m_List.GetHead();
    uint32_t CurrentIndex = m_List.m_FirstElementPos;
    bool Found         = false;

    // Pass through all blocks
    while (!Found && pCurrentBlock)
        {
        // Pass through all elements of the block
        while (!Found && CurrentIndex < BLOCK_CAPACITY)
            {
            if (pi_rpObject->IndexesSameObjectAs((*pCurrentBlock)[CurrentIndex]))
                {
                Found = true;
                pResult = (*pCurrentBlock)[CurrentIndex];
                }
            ++CurrentIndex;
            }

        CurrentIndex = 0;
        pCurrentBlock = pCurrentBlock->GetNextBlock();
        }

    return pResult;
    }
#endif

END_IMAGEPP_NAMESPACE