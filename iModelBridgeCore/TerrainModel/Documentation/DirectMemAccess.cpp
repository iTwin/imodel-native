Int32 CacheDataBlock::s_pageSize;

CacheDataBlock::CacheDataBlock (DgnModelR dgnModel, int size) : m_dgnModel(dgnModel)
    {
    m_numBytes = size;
    m_nextAvailable = 0;
    m_dirty     = true;
    AllocateMemory ();
    s_pageSize = BeVirtualMemory::GetPageSize();
    }

void CacheDataBlock::AllocateMemory ()
    {
    m_data = (byte*) BeVirtualMemory::Alloc (NULL, m_numBytes, BeVirtualMemory::CommitTopDown, BeVirtualMemory::ReadWrite);
    BeAssert (m_data);
    }

void CacheDataBlock::CloseBlockForWrites ()
    {
    uint32_t closedSize = m_nextAvailable;
    makeMultipleOf (closedSize, s_pageSize);

    if (closedSize >= m_numBytes)
        return;

    int diff = m_numBytes - closedSize;
    m_numBytes = closedSize;

    BentleyStatus stat = BeVirtualMemory::Free (m_data + closedSize, diff, BeVirtualMemory::Decommit);
    if (SUCCESS != stat)
        {
        BeAssert (0);
        }
    }

StatusInt CacheDataBlock::AllocElemMemory (DgnElemDataRef& elem, uint32_t sizeBytes)
    {
    makeMultipleOf (sizeBytes, 8); // ensure multiple of 8 bytes

    // see if it will fit
    if ((m_nextAvailable + sizeBytes) > m_numBytes)
        return  ERROR;

    elem.m_ptr       = m_data + m_nextAvailable;
    elem.m_myBlock   = this;
    elem.m_allocSize = sizeBytes;

    m_nextAvailable += sizeBytes;
    return  SUCCESS;
    }

StatusInt       DgnModel::GetElemMemory (DgnElemDataRef& elm, int sizeBytes)
    {
    if (NULL == m_currDataBlock)
        m_currDataBlock = NewDataBlock (sizeBytes, m_dataBlockSize);

    if (SUCCESS == m_currDataBlock->AllocElemMemory (elm, sizeBytes))
        return SUCCESS;

    m_currDataBlock->CloseBlockForWrites();
    m_currDataBlock = NewDataBlock (sizeBytes, m_dataBlockSize);
    return m_currDataBlock->AllocElemMemory (elm, sizeBytes);
    }

CacheDataBlock* DgnModel::NewDataBlock (int elemSize, int blockSize)
    {
    // make sure block will be big enough to hold this element
    if (elemSize > blockSize)
        blockSize = elemSize;

    // make size a multiple of page size
    blockSize = CacheDataBlock::blockMemSize (blockSize, CacheDataBlock::GetPageSize());

    CacheDataBlockP block = new CacheDataBlock (*this, blockSize);
    m_dataBlocks.push_back (block);
    return  block;
    }

StatusInt XAttributeSet::Delete (XAttrNode& node, DgnModel& dgnModel)
    {
    if (node.m_xattr < &m_entries.front() || node.m_xattr > &m_entries.back())
        return  DGNMODEL_STATUS_InvalidXattribute;

    m_entries.erase (* (XAttributeRefVec::iterator*) &node.m_xattr);
    node.Clear();
    return  SUCCESS;
    }

StatusInt XAttributeRef::Modify (void const* data, uint32_t start, uint32_t length)
    {
    if ((start + length) > GetSize())
        return  ERROR;

    memcpy ((byte*) GetPtrForWrite() + start, data, length);
    return  SUCCESS;
    }

StatusInt XAttributeRef::Replace (DgnModelP cache, void const* data, uint32_t newSize)
    {
    if (0 == newSize)
        newSize = GetSize ();

    if (newSize > m_data.GetAllocSize())
        {
        if (SUCCESS != cache->GetElemMemory (m_data, newSize))
            return  ERROR;
        }

    SetSize (newSize);
    //SetArchiveFlag();   // This is for DTM so ignore
    return Modify (data, 0, newSize);
    }

struct Txn : ITxn
    {
    void a(ChangeTrackAction action, ElementRefP elRef, XAttributeHandlerId handlerId, uint32_t xAttrId, size_t dataSize, DgnElemDataRef addr)
        {
        _SaveXAttrInUndo (action, elRef, handlerId, xAttrId, dataSize, addr);
        }
    };

void testMemory (XAttributeHandle* Xhandle, DgnModelR model)
    {
    Xhandle->m_node.GetXAttr()->Replace (&model, 0, 0);

    // Delete XAttribute.
    Xhandle->m_set->Delete (Xhandle->m_node, model);
    DgnElemDataRef   addr;
    uint32_t datasize;
    XAttrIO::GetInfo (*Xhandle, addr, datasize);     // get the address of the data for this xAttr

    Txn* k = (Txn*)&ITxnManager::GetCurrentTxn();
    k->a (ChangeTrackAction::XAttributeDelete, 0, Xhandle->GetHandlerId(), 0, 0, addr);
    }
