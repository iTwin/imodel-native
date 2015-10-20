/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMXAttributeHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

#ifndef NDEBUG
#define USELOG
#endif

#ifdef USELOG
#define write_to_log if (HasDebugLog()) LogDebugV
#else
#define write_to_log(...)
#endif

struct IDTMTxnMonitor : TxnMonitor, ITxn::ICustomEntry
    {
    };

struct DTMTxnMonitor : RefCounted<IDTMTxnMonitor>, DgnFileAppData
    {
    private:
        Key m_key;
        static RefCountedPtr<DTMTxnMonitor> s_instance;
        bvector<DTMXAttributeHandler*> m_openedDTMS;
        bvector<DTMXAttributeHandler*> m_modifiedDTMS;
        bvector<ElementRefP> m_tmPersistElements;

        DTMTxnMonitor()
            {
            ITxnManager::GetManager ().AddTxnMonitor (*this);
            }

    // ITxnMonitor
    public:
        static DTMTxnMonitor& GetInstance ()
            {
            if (s_instance.IsNull())
                s_instance = new DTMTxnMonitor ();
            return *s_instance.get();
            }

        void ReloadData (ElementRefP elemRef)
            {
            bvector<DTMXAttributeHandler*> openedDTMS = m_openedDTMS;
            bvector<DTMXAttributeHandler*>::iterator iter = openedDTMS.begin ();
            for (; iter < openedDTMS.end (); iter++)
                if ((*iter)->GetElemHandle ().GetElementRef () == elemRef)
                    (*iter)->UndoRedo ();
            }
        virtual void _OnXAttributeUndoRedo (XAttributeHandleCR xAttr,  ChangeTrackAction action, bool isUndo, ChangeTrackSource source, ChangeTrackInfo const* info) override
            {
            if (xAttr.GetHandlerId() == XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER))
                {
                ElementRefP elemRef = xAttr.GetElementRef ();
                if (std::find (m_tmPersistElements.begin (), m_tmPersistElements.end (), elemRef) == m_tmPersistElements.end())
                    m_tmPersistElements.push_back (elemRef);
                }
            }

        virtual void _OnUndoRedoFinished (bool isUndo) override
            {
            for (auto elmRef : m_tmPersistElements)
                ReloadData (elmRef);
            m_tmPersistElements.clear ();
            }

        virtual void _OnTxnValidate () override
            {
            // Need to an EndModify on all opened dtms.
            CloseTxn();
            }

        virtual void _OnTxnClose () override    // Added this because TxnValidate may not be called.
            {
            // Need to an EndModify on all opened dtms.
            CloseTxn();
            }

    // DgnModelAppData
        virtual void _OnCleanup (DgnFileR host) override
            {
            }

        virtual void _OnSave (DgnFileR host, ProcessChangesWhen when, DgnFileChanges changesFlag, DgnSaveReason reason, double timestamp) override
            {
            CloseTxn ();
            host.DropAppData (m_key);
            }

        void AttachToDgnFile (DgnFileP file)
            {
            BeAssert (file);
            if (!file->FindAppData (m_key))
                file->AddAppData (m_key, s_instance.get());
            }

        void AddXAttributeHandler (DTMXAttributeHandler* xAttributeHandler)
            {
            m_openedDTMS.push_back (xAttributeHandler);
            }

        void RemoveXAttributeHandler (DTMXAttributeHandler* xAttributeHandler)
            {
            bvector<DTMXAttributeHandler*>::iterator iter = m_openedDTMS.begin();
            for (; iter < m_openedDTMS.end(); iter++)
                {
                if (*iter == xAttributeHandler)
                    {
                    m_openedDTMS.erase (iter);
                    break;
                    }
                }
            }
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================
        void CloseTxn()
            {
            bvector<DTMXAttributeHandler*> openedDTMS = m_modifiedDTMS;
            bvector<DTMXAttributeHandler*>::iterator iter = openedDTMS.begin();
            for (; iter < openedDTMS.end(); iter++)
                (*iter)->EndModify ();
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================
        void AddToTxn (DTMXAttributeHandler* xAttributeHandler)
            {
            bvector<DTMXAttributeHandler*>::iterator iter = m_modifiedDTMS.begin();
            for (; iter < m_modifiedDTMS.end(); iter++)
                {
                if (*iter == xAttributeHandler)
                    return;
                }
            AttachToDgnFile (xAttributeHandler->GetElemHandle().GetDgnFileP ());
            m_modifiedDTMS.push_back (xAttributeHandler);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================
        void DropToTxn (DTMXAttributeHandler* xAttributeHandler)
            {
            bvector<DTMXAttributeHandler*>::iterator iter = m_modifiedDTMS.begin();
            for (; iter < m_modifiedDTMS.end(); iter++)
                {
                if (*iter == xAttributeHandler)
                    {
                    m_modifiedDTMS.erase (iter);
                    break;
                    }
                }
            }

        static RefCountedPtr<ICustomEntry> DeserializeCustomItemFunc (byte const* data, size_t numBytes)
            {
            return s_instance;
            }

        // Before we persist a TM (for modify new one doesn't matter!) we Save a custom entry in the undo buffer.
        // We store the Header of the
        void StartTMPersist ()
            {
            bool start = false;
            ITxnManager::GetCurrentTxn ().SaveCustomEntryInUndo ((byte*)&start, sizeof (start), DeserializeCustomItemFunc);
            }

        void EndTMPersist ()
            {
            bool end = true;
            ITxnManager::GetCurrentTxn ().SaveCustomEntryInUndo ((byte*)&end, sizeof (end), DeserializeCustomItemFunc);
            }
        };

RefCountedPtr<DTMTxnMonitor> DTMTxnMonitor::s_instance;

struct DTMInPersistUndoEntry : RefCounted<ITxn::ICustomEntry>
    {
    static DTMInPersistUndoEntry& GetUndoEntry()
        {
        static RefCountedPtr<DTMInPersistUndoEntry> s_instance;
        if (s_instance.IsNull())
            s_instance = new DTMInPersistUndoEntry();
        return *s_instance.get();
        }
    };


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
// Writes the data supplied to the header XAttribute
void DTMXAttributeHandler::WriteHeader (const byte* data, int size)
    {
    BC_DTM_OBJ* dtmobj = (BC_DTM_OBJ*)_alloca (size);
    memcpy (dtmobj, data, size);
    dtmobj->dtmObjType = BC_DTM_ELM_TYPE;

    if (m_noSchedule)
        {
        XAttributeHandle xAttr (m_handle.GetElementRef(), XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER), 0);
        if (xAttr.IsValid())
            ITxnManager::GetCurrentTxn().ReplaceXAttributeData (xAttr, dtmobj, (UInt32)size);
        else
            ITxnManager::GetCurrentTxn().AddXAttribute (m_handle.GetElementRef (), XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER), 0, dtmobj, (UInt32)size);
        }
    else
        m_handle.ScheduleWriteXAttribute (XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER), 0, size, dtmobj);
    write_to_log(L"WriteHeader %x", this);
    }

#ifdef DEBUG
UInt32 GetCheckSum (void* mem, size_t size)
    {
    UInt32 checkSum = 0;
    unsigned char* memChar = (unsigned char*)mem;

    for (size_t p = size; p < size; size++, memChar++)
        {
        checkSum += *memChar;
        int shift = size % 4;

        checkSum ^= ((UInt32)*memChar) << (8 * shift);
        }
    return checkSum;
    }
#endif

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
void DTMXAttributeHandler::StoreSnapShot ()
    {
    ElementHandle::XAttributeIter iter (m_handle, XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER), 0);

    BeAssert (iter.IsValid());
    size_t size = iter.GetSize();

    if (size > sizeof (BC_DTM_OBJ)) size = sizeof (BC_DTM_OBJ);
    if (iter.IsValid ())
        memcpy(&m_originalHeader, iter.PeekData(), size);

#ifdef COMPLETE_TEMPORARY_CHECK
    for (DTMPartition type = DTMPartition::None; type < (DTMPartition)NUMPARTITIONTYPES; type = (DTMPartition)(1 + (int)type))
        {
        UInt32 xAttrId = GetXAttrId(type);
        XAttributeHandlerId handlerId (TMElementMajorId, xAttrId);

        ElementHandle::XAttributeIter xAttrIter (m_handle, handlerId);

        while (xAttrIter.IsValid())
            {
            void* mem;
            size_t size;
            mem = (void*)xAttrIter.PeekData();
            size = xAttrIter.GetSize();
            m_checkSums[type][xAttrIter.GetId()] = GetCheckSum (mem, size);
            if (!xAttrIter.SearchNext (handlerId))
                break;
            }
        }
#endif
    }

template<class TYPE> bool DTMXAttributeHandler::FixUpDTMPartitionArray (DTMPartition partition, long originalMem, long originalNumPartitions, long& num, long& mem, long& numPartitions, long partitionSize, TYPE** partitionArray)
    {
    bool needsReload = false;
    if (num > originalMem)
        needsReload = true;
    else if (mem != originalMem)
        {
        if (numPartitions < originalNumPartitions)
            partitionArray = (TYPE **)realloc (partitionArray, sizeof (TYPE*) * originalNumPartitions);

        for(int n = originalNumPartitions; n < numPartitions; ++n)
            {
            if (partitionArray[n] != nullptr)
                {
                FreeMemory (partition, n, partitionArray[n]);
                partitionArray[n] = nullptr;
                }
            }
        if (numPartitions != 0)
            {
            for (int n = numPartitions - 1; n < originalNumPartitions - 1; ++n)
                partitionArray[n] = (TYPE*)ReallocateMemory (partition, n, partitionArray[n], partitionSize * sizeof(TYPE));
            }

            long remPartition = originalMem % partitionSize;
            if (remPartition != 0)
                partitionArray[originalNumPartitions - 1] = (TYPE*)ReallocateMemory (partition, originalNumPartitions - 1, partitionArray[originalNumPartitions - 1], remPartition * sizeof (TYPE));

            mem = originalMem;
            numPartitions = originalNumPartitions;
        }
    return needsReload;
    }
//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
bool DTMXAttributeHandler::CheckTemporary (bool goingToModify)
    {
    BC_DTM_OBJ* dtm = m_dtm->GetTinHandle();

    bool cdbg = false;
#ifdef DEBUG
    bool dbg = true;
#else
    bool dbg = false;
#endif
    bool needsReload = false;
    // Make sure that the memory is the same size.

    if (!goingToModify)
        {
        if (cdbg && bcdtmCheck_tinComponentDtmObject (dtm) != DTM_SUCCESS)
            bcdtmWrite_message (0, 0, 0, "DTM failed Check Before resize");
        if (dbg) bcdtmWrite_message (0, 0, 0, "Checking this header against the pre temporary change header.");
        m_writeCount++;

        if (dtm->numPoints > m_originalHeader.memPoints || dtm->numNodes > m_originalHeader.memNodes || dtm->numFeatures > m_originalHeader.memFeatures || dtm->numFlist > m_originalHeader.memFlist || dtm->cListPtr > m_originalHeader.memClist)
            {
            if (dtm->cListPtr > m_originalHeader.memClist)
                bcdtmTin_compactCircularListDtmObject (dtm);
            if (dtm->numFeatures > m_originalHeader.memFeatures)
                bcdtmTin_compactFeatureTableDtmObject (dtm);
            if (dtm->numFlist > m_originalHeader.memFlist)
                bcdtmTin_compactFeatureListDtmObject (dtm);
            if (dtm->numPoints > m_originalHeader.memPoints)
                bcdtmTin_compactPointAndNodeTablesDtmObject (dtm);
            }

        if (cdbg && bcdtmCheck_tinComponentDtmObject (dtm) != DTM_SUCCESS)
            bcdtmWrite_message (0, 0, 0, "DTM failed Check After resize");

        // Fix up the size of the partitions. If we need to reload then we dont need to.
        needsReload = needsReload | FixUpDTMPartitionArray<DPoint3d> (DTMPartition::Point, m_originalHeader.memPoints, m_originalHeader.numPointPartitions, dtm->numPoints, dtm->memPoints, dtm->numPointPartitions, dtm->pointPartitionSize, dtm->pointsPP);
        needsReload = needsReload | FixUpDTMPartitionArray<DTM_TIN_NODE> (DTMPartition::Node, m_originalHeader.memNodes, m_originalHeader.numNodePartitions, dtm->numNodes, dtm->memNodes, dtm->numNodePartitions, dtm->nodePartitionSize, dtm->nodesPP);
        needsReload = needsReload | FixUpDTMPartitionArray<BC_DTM_FEATURE> (DTMPartition::Feature, m_originalHeader.memFeatures, m_originalHeader.numFeaturePartitions, dtm->numFeatures, dtm->memFeatures, dtm->numFeaturePartitions, dtm->featurePartitionSize, dtm->fTablePP);
        needsReload = needsReload | FixUpDTMPartitionArray<DTM_FEATURE_LIST> (DTMPartition::FList, m_originalHeader.memFlist, m_originalHeader.numFlistPartitions, dtm->numFlist, dtm->memFlist, dtm->numFlistPartitions, dtm->flistPartitionSize, dtm->fListPP);
        needsReload = needsReload | FixUpDTMPartitionArray<DTM_CIR_LIST> (DTMPartition::CList, m_originalHeader.memClist, m_originalHeader.numClistPartitions, dtm->numClist, dtm->memClist, dtm->numClistPartitions, dtm->clistPartitionSize, dtm->cListPP);

        if (cdbg && bcdtmCheck_tinComponentDtmObject(dtm) != DTM_SUCCESS)
            bcdtmWrite_message (0, 0, 0, "DTM failed Check After moving to XAttribute memory");

        m_writeCount--;
        BeAssert (needsReload == false);
        if (needsReload)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "DTM doesn't match after temporary numFeature/hullPoint is out of sync");
            }

        //#ifdef DEBUG
        if (!needsReload)
            {
            int count = 0;
            for (int i = 0; i < NUMPARTITIONTYPES; i++)
                {
                memoryMapT::iterator iter = m_memory[i].begin();

                while (iter != m_memory[i].end())
                    {
                    switch(iter->second.state)
                        {
                        // Shouldn't need to do this!
                        case existingBlock:
                            break;
                        case reallocatedBlock:
                            if (dbg) bcdtmWrite_message (0, 0, 0, "A temporary change to the dtm partition reallocate %d %d", i, iter->first);
                            count++;
                            break;
                        case newBlock:
                            if (dbg) bcdtmWrite_message (0, 0, 0, "A temporary change to the dtm partition newBlock %d %d", i, iter->first);
                            count++;
                            break;
                        case deletedBlock:
                            if (dbg) bcdtmWrite_message (0, 0, 0, "A temporary change to the dtm partition deletedBlock %d %d", i, iter->first);
                            count++;
                            break;
                        }
                    iter++;
                    }
                }
            BeAssert (count == 0);
            if (count != 0)
                {
                needsReload = true;
                if (dbg) bcdtmWrite_message (0, 0, 0, "Something isn't quite right some memory is not in an existing state");
                }
            }
        //#endif
        }

    if (!needsReload)
        {
        // Dont care if the DTM is out of date
        dtm->modifiedTime = m_originalHeader.modifiedTime;
        dtm->lastModifiedTime = m_originalHeader.lastModifiedTime;

        XAttributeHandle xAttrIter (m_handle.GetElementRef(), XAttributeHandlerId(TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER), 0);

        // Update the header.
        if (xAttrIter.IsValid ())
            {
            BC_DTM_OBJ* xAttributeHeaderData = (BC_DTM_OBJ*)xAttrIter.PeekData();
            xAttributeHeaderData->dtmObjType = BC_DTM_ELM_TYPE;

            }
        }

#ifdef DEBUG
    // fListDelPtr?
    int cmp = memcmp (&m_originalHeader, dtm, DTMIOHeaderSize);

    if (cmp != 0)
        {
        write_to_log (L"DTM Element Header are different after temporary modification");
        if (dbg) bcdtmWrite_message (0, 0, 0, "DTM Element Header are different after temporary modification");
        }
#ifdef COMPLETE_TEMPORARY_CHECK
    for (int type = 0; type < NUMPARTITIONTYPES; type++)
        {
        UInt32 xAttrId = GetXAttrId(type);
        XAttributeHandlerId handlerId (TMElementMajorId, xAttrId);

        ElementHandle::XAttributeIter xAttrIter (m_handle, handlerId);

        while (xAttrIter.IsValid())
            {
            void* mem;
            size_t size;
            mem = (void*)xAttrIter.PeekData();
            size = xAttrIter.GetSize();
            if (m_checkSums[type][xAttrIter.GetId()] != GetCheckSum (mem, size))
                write_to_log (L"Data is Different");
            if (!xAttrIter.SearchNext (handlerId))
                break;
            }
        }
#endif
#endif
    return needsReload;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================
int DTMXAttributeHandler::SetMemoryAccess (BC_DTM_OBJ* dtm, DTMAccessMode accessMode)
    {
    BeAssert (!m_inCreate);
    BeAssert (m_dtm.IsNull () || dtm == m_dtm->GetTinHandle());

    if (accessMode == DTMAccessMode::Write)
        {
        if (m_handle.GetDgnModelP()->IsReadOnly ())
            return DTM_ERROR;

        if (m_isTemporary)
            {
#ifdef DEBUG
            CheckTemporary (true);
#endif
            m_isTemporary = false;
            }
        // If element is read only then fail.
        if (m_writeCount == 0)
            StartModify (false);
        }

    if (accessMode == DTMAccessMode::Temporary)
        {
        if (!m_isTemporary && m_writeCount == 0)
            {
            StoreSnapShot ();
            m_isTemporary = true;
            StartModify (true);
            }
        }

    return DTM_SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::StartModify (bool disableUndo)
    {
    write_to_log(L"StartModify %x (%d)", this, m_writeCount);
    if (m_writeCount == 0)
        {
        DTMTxnMonitor::GetInstance().AddToTxn (this);
        m_writeCount++;
        if (!disableUndo)
            AddXAttributesToUndoBuffer ();
        }
    else
        m_writeCount++;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 08/13
//=======================================================================================
void DTMXAttributeHandler::AddXAttributesToUndoBuffer ()
    {
    write_to_log(L"AddXAttributesToUndoBuffer %x", this);
    // Need to browse the XAttributes and add them to the undo stack.
    ElementHandle::XAttributeIter iter = ElementHandle::XAttributeIter (m_handle);
    while (iter.IsValid())
        {
        XAttributeHandlerId id = iter.GetHandlerId();
        if (id.GetMajorId() == TMElementMajorId)
            {
            switch(id.GetMinorId())
                {
                case XATTRIBUTES_SUBID_DTM_POINTARRAY:
                case XATTRIBUTES_SUBID_DTM_FEATUREARRAY:
                case XATTRIBUTES_SUBID_DTM_NODEARRAY:
                case XATTRIBUTES_SUBID_DTM_CLISTARRAY:
                case XATTRIBUTES_SUBID_DTM_FLISTARRAY:
                    {
                    XAttributeHandle* Xhandle = const_cast<XAttributeHandle*>((XAttributeHandleCP)iter.GetElementXAttributeIter());
                    if (Xhandle)
                        {
                        Xhandle->GetPtrForWrite();
                        ITxnManager::GetCurrentTxn().SaveXAttributeDataForDirectChange (*Xhandle, 0, iter.GetSize());
                        }
                    }
                    break;
                }
            }
        if (!iter.ToNext())
            break;  //ToDo LookAt
        }
    }

void DTMXAttributeHandler::StartTMPersist ()
    {
    DTMTxnMonitor::GetInstance ().StartTMPersist ();
    }

void DTMXAttributeHandler::EndTMPersist ()
    {
    DTMTxnMonitor::GetInstance ().EndTMPersist ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::EndModify ()
    {
    write_to_log(L"EndModify %x (%d)", this, m_writeCount);
    m_writeCount--;
    BeAssert(m_writeCount >= 0);
    if (m_writeCount < 0)
        m_writeCount = 0;

    if (m_writeCount == 0)
        {
        if (m_isTemporary)
            // If the Temporary Check failed we have to write this to avoid corrupt data.
            m_isTemporary = !CheckTemporary (false);

        if (!m_isTemporary)
            {
            if (m_handle.GetElementRef() && !m_inScheduleReplace)
                m_noSchedule = true;

            // Cheat for undo/redo we write the header twice, so that the header XAttribute is modified
            if (m_noSchedule)
                DTMTxnMonitor::GetInstance().StartTMPersist();

            UpdateXAttributes ();

            WriteHeader ((const byte*)m_dtm->GetTinHandle(), DTMIOHeaderSize);

            if (m_noSchedule)
                DTMTxnMonitor::GetInstance().EndTMPersist();

            if (m_handle.GetDisplayHandler ())
                m_handle.GetDisplayHandler ()->ValidateElementRange (m_handle, true);

            if (m_handle.GetElementRef() && !m_noSchedule)
                ReplaceInModel ();
            m_noSchedule = false;
            }

        DTMTxnMonitor::GetInstance().DropToTxn (this);
        m_isTemporary = false;
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::DeleteXAttributes()
    {
    write_to_log(L"DeleteXAttributes %x", this);

    //spu::201105:  First loop to delete the header. If the header is not the last XAttribute, deleting it
    //              in the general loop might modify the iterator while iterating...
    ElementHandle::XAttributeIter iter (m_handle);

    while (iter.IsValid())
        {
        XAttributeHandlerId id = iter.GetHandlerId();
        if (id.GetMajorId() == TMElementMajorId)
            {
            UInt32 attrId = iter.GetId();
            if (!iter.ToNext())
                break;  //ToDo LookAt
            if (id.GetMinorId() == XATTRIBUTES_SUBID_DTM_HEADER)
                {
                m_handle.ScheduleDeleteXAttribute(XAttributeHandlerId (TMElementMajorId, id.GetMinorId()),  attrId);
                break;
                }
            }
        else
            {
            if (!iter.ToNext())
                break;  //ToDo LookAt
            }
        }

    //spu::201105:  Second loop to schedule deletion of the rest of the dtm data
    iter = ElementHandle::XAttributeIter (m_handle);
    while (iter.IsValid())
        {
        XAttributeHandlerId id = iter.GetHandlerId();
        if (id.GetMajorId() == TMElementMajorId)
            {
            UInt32 attrId = iter.GetId();
            if (!iter.ToNext())
                break;  //ToDo LookAt
            switch(id.GetMinorId())
                {
                case XATTRIBUTES_SUBID_DTM_HEADER:
                    break;
                case XATTRIBUTES_SUBID_DTM_FEATUREARRAY:
                    FreeMemoryInternal(DTMPartition::Feature,attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_POINTARRAY:
                    FreeMemoryInternal(DTMPartition::Point,attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_NODEARRAY:
                    FreeMemoryInternal(DTMPartition::Node,attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_CLISTARRAY:
                    FreeMemoryInternal(DTMPartition::CList,attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_FLISTARRAY:
                    FreeMemoryInternal(DTMPartition::FList,attrId);
                    break;
                case XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP:
                    FreeMemoryInternal (DTMPartition::None, attrId);
                    break;
                }
            }
        else
            {
            if (!iter.ToNext())
                break;  //ToDo LookAt
            }
        }
    }


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 02/15
//=======================================================================================
void DTMXAttributeHandler::ReloadData (ElementRefP ref)
    {
    DTMTxnMonitor::GetInstance ().ReloadData (ref);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::UndoRedo ()
    {
    write_to_log (L"UndoRedo %x", this);

    // Clear out any modifications in the memory.
    for (int i = 0; i < NUMPARTITIONTYPES; i++)
        {
        memoryMapT::iterator iter = m_memory[i].begin();

        while (iter != m_memory[i].end())
            {
            switch (iter->second.state)
                {
                // Shouldn't need to do this!
                case existingBlock:
                    break;
                case reallocatedBlock:
                case newBlock:
                    free (iter->second.mem);
                    break;
                }
            iter++;
            }
        m_memory[i].clear();
        m_hasScanned[i] = false;
        }

    m_writeCount = 0;
    m_isTemporary = false;
    m_extendedMemoryDeleted.clear();
    DTMTxnMonitor::GetInstance().DropToTxn (this);

    UpdateDTMPointers ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::UpdateDTMPointers ()
    {
    write_to_log(L"UpdateDTMPointers %x, Modifed %d (inCreate %d)", this, this->m_writeCount, m_inCreate);

    BeAssert (!m_inCreate);

    if (m_writeCount)
        {
        BeAssert (m_isTemporary);
        // Dont do this.
        if (!m_isTemporary)
            return;
        }

    // Get the DTM iter
    XAttributeCollection m_dtmIter (m_handle.GetElementRef());
    ElementHandle::XAttributeIter iter (m_handle, XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER), 0);

//    BeAssert (iter.IsValid());

    if (iter.IsValid ())
        {
        BC_DTM_OBJ const * headerData  = (BC_DTM_OBJ const*)iter.PeekData();

        bvector<void*> featureArrays;
        bvector<void*> pointArrays;
        bvector<void*> nodeArrays;
        bvector<void*> cListArrays;
        bvector<void*> fListArrays;
        memStateT* t;
        featureArrays.resize (headerData->numFeaturePartitions);
        pointArrays.resize (headerData->numPointPartitions);
        nodeArrays.resize (headerData->numNodePartitions);
        cListArrays.resize (headerData->numClistPartitions);
        fListArrays.resize (headerData->numFlistPartitions);

        iter = ElementHandle::XAttributeIter (m_handle);
        while (iter.IsValid())
            {
            void* data = nullptr;
            XAttributeHandlerId id = iter.GetHandlerId();
            if (id.GetMajorId() == TMElementMajorId)
                {
                switch(id.GetMinorId())
                    {
                    case XATTRIBUTES_SUBID_DTM_POINTARRAY:
                    case XATTRIBUTES_SUBID_DTM_FEATUREARRAY:
                    case XATTRIBUTES_SUBID_DTM_NODEARRAY:
                    case XATTRIBUTES_SUBID_DTM_CLISTARRAY:
                    case XATTRIBUTES_SUBID_DTM_FLISTARRAY:
                        {
                        DTMPartition type;
                        bvector<void*>* partitionArray = NULL;
                        switch (id.GetMinorId ())
                            {
                            case XATTRIBUTES_SUBID_DTM_POINTARRAY:   type = DTMPartition::Point;   partitionArray = &pointArrays;   break;
                            case XATTRIBUTES_SUBID_DTM_FEATUREARRAY: type = DTMPartition::Feature; partitionArray = &featureArrays; break;
                            case XATTRIBUTES_SUBID_DTM_NODEARRAY:    type = DTMPartition::Node;    partitionArray = &nodeArrays;    break;
                            case XATTRIBUTES_SUBID_DTM_CLISTARRAY:   type = DTMPartition::CList;   partitionArray = &cListArrays;   break;
                            case XATTRIBUTES_SUBID_DTM_FLISTARRAY:   type = DTMPartition::FList;   partitionArray = &fListArrays;   break;
                            }

                        data = (void*)iter.PeekData();
                        t = GetMemoryMapT (type, iter.GetId());

                        if (t != nullptr && t->state != existingBlock)
                            data = t->mem;
                        else if (t != nullptr)
                            t->mem = data;

                        if (partitionArray->size() <= iter.GetId())
                            partitionArray->resize (iter.GetId() + 1);
                        (*partitionArray)[iter.GetId()] = data;

                        m_hasScanned[(int)type] = true;
                        memoryMapT::const_iterator memIter = m_memory[(int)type].find (iter.GetId ());

                        if (memIter == m_memory[(int)type].end ())
                            {
                            void* mem;
                            size_t size;
                            XAttributeHandle* Xhandle = const_cast<XAttributeHandle*>((XAttributeHandleCP)iter.GetElementXAttributeIter());

                            if (Xhandle)
                                {
                                mem = Xhandle->GetPtrForWrite();
                                size = Xhandle->GetSize();
                                }
                            else
                                {
                                mem = (void*)iter.PeekData();
                                size = iter.GetSize();
                                }

                            memStateT* state = &m_memory[(int)type][iter.GetId ()];
                            state->size = size;
                            state->xAttributeSize = size;
                            state->state = existingBlock;
                            state->mem = mem;
                            }
                        break;
                        }
                    case XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP:
                        if (m_nextIndex < iter.GetId())
                            m_nextIndex = iter.GetId();
                        break;
                    }
                }

            if (!iter.ToNext())
                break;  //ToDo LookAt
            }
        m_nextIndex++;
        if (headerData)
            bcdtmObject_updateDtmObjectForDtmElement (m_dtm->GetTinHandle(), (void*)headerData, &featureArrays[0], &pointArrays[0], &nodeArrays[0], &fListArrays[0], &cListArrays[0]);
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
    DTMXAttributeHandler::DTMXAttributeHandler (ElementHandleCR element, BcDTMR bcDTM, bool inCreate) : m_handle (element, false), m_dtm (&bcDTM), m_inCreate (inCreate)
    {
    write_to_log(L"DTMXAttributeHandler %x", this);

    DTMTxnMonitor::GetInstance().AddXAttributeHandler (this);
    for (int i = 0; i < _countof (m_hasScanned); i++)
        m_hasScanned[i] = false;

    m_noSchedule = false;
    m_writeCount = 0;
    m_nextIndex = 0;
    m_isTemporary = false;

    // Get the DTM iter
    DTMBinaryDataIter iter (m_handle, XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP);
    iter.Reset();
    while (iter.MoveNext())
        {
        if (m_nextIndex < iter.GetCurrentAttrId())
            m_nextIndex = iter.GetCurrentAttrId();
        }
    m_nextIndex++;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
// Returns if this element has been modified.
bool DTMXAttributeHandler::HasModified()
    {
    if (m_isTemporary)
        return false;
    for (int i = 0; i < NUMPARTITIONTYPES; i++)
        {
        memoryMapT::iterator iter = m_memory[i].begin();

        while (iter != m_memory[i].end())
            {
            if (iter->second.state != existingBlock)
                return true;
            iter++;
            }
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
// Deposes the memory allocated by the dtm and writes it to the XAttributes.
DTMXAttributeHandler::~DTMXAttributeHandler ()
    {
    if (HasModified())
        {
#ifdef COMPARE_DTM
    // Get the handles of the DTM and store binary data.
        void* headerP;
        int nHeader;
        void** featuresArrayP;
        int featuresNArray;
        int featuresArraySize;
        int featuresLastArraySize;
        void** pointArrayP;
        int pointNArray;
        int pointArraySize;
        int pointLastArraySize;
        void** nodeArrayP;
        int nodeNArray;
        int nodeArraySize;
        int nodeLastArraySize;
        void** clistArrayP;
        int clistNArray;
        int clistArraySize;
        int clistLastArraySize;
        void** flistArrayP;
        int flistNArray;
        int flistArraySize;
        int flistLastArraySize;

        m_dtm->getHandles(&headerP, &nHeader,
            &featuresArrayP, &featuresNArray, &featuresArraySize, &featuresLastArraySize,
            &pointArrayP, &pointNArray, &pointArraySize, &pointLastArraySize,
            &nodeArrayP, &nodeNArray, &nodeArraySize, &nodeLastArraySize,
            &clistArrayP, &clistNArray, &clistArraySize, &clistLastArraySize,
            &flistArrayP, &flistNArray, &flistArraySize, &flistLastArraySize
            );

        RememberDTM (headerP, featuresArrayP, pointArrayP, nodeArrayP, clistArrayP, flistArrayP);
#endif
        if (IsOpenedForWrite ())
            EndModify ();
            }

    write_to_log (L"~DTMXAttributeHandler %x", this);
    BeAssert (!HasModified());
    //    BeAssert(m_writeCount == 0);
    if (m_writeCount)
        EndModify ();

    if (!m_inCreate && m_dtm.IsValid())
        {
        bcdtmObject_setToDTMElement (m_dtm->GetTinHandle(), nullptr);
        }
    DTMTxnMonitor::GetInstance().RemoveXAttributeHandler (this);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::UpdateXAttributes ()
    {
    write_to_log(L"UpdateXAttributes %x", this);
    if (HasModified())
        {
        int count = 0;
        for (int i = (int)DTMPartition::None; i < NUMPARTITIONTYPES; i++)
            {
            DTMPartition partition= (DTMPartition)i;
            memoryMapT::iterator iter = m_memory[i].begin ();

            while (iter != m_memory[i].end ())
                {
                switch(iter->second.state)
                    {
                    // Shouldn't need to do this!
                    case existingBlock:
                        break;
                    case reallocatedBlock:
                        count++;
                        dh_allocXAttr (iter, partition);
                        break;
                    case newBlock:
                        count++;
                        dh_allocXAttr (iter, partition);
                        break;
                    case deletedBlock:
                        dh_freeXAttr (iter, partition);
                        iter = m_memory[i].erase (iter);
                        continue;
                        break;
                    }
                iter++;
                if (count >= 100)
                    count = 0;
                }
//            m_hasScanned[(int)i] = false;
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::ReplaceInModel()
    {
    if (m_handle.PeekElementDescrCP())
        m_handle.ReplaceInModel (m_handle.GetElementRef());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 11/11
//=======================================================================================
static void* GetWritableDataPointer (ElementHandle::XAttributeIter& iter)
    {
    XAttributeHandle* Xhandle = const_cast<XAttributeHandle*>((XAttributeHandle*)iter.GetElementXAttributeIter());
    if (Xhandle)
        return Xhandle->GetPtrForWrite();

    return (void*)iter.PeekData();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
// Allocates a new block of memory.
void* DTMXAttributeHandler::AllocateMemoryInternal (DTMPartition type, int num, size_t size)
    {
    write_to_log(L"AllocateMemoryInternal %x (%d/%d, %d)", this, type, num, size);
    BeAssert(m_writeCount != 0);
    BeAssert (size != 0);

    CheckScanXAttributes (type);
    memStateT* state = &m_memory[(int)type][num];

    if (state->state != unknown)
        {
        if (state->xAttributeSize == size)
            {
            UInt16 xAttrId = GetXAttrId(type);
            XAttributeHandlerId handlerId (TMElementMajorId, xAttrId);
            ElementHandle::XAttributeIter xAttrIter (m_handle, handlerId, num);
            BeAssert (size == xAttrIter.GetSize());

            state->mem = GetWritableDataPointer (xAttrIter);
            state->size = size;
            state->state = existingBlock;
            return state->mem;
            }
        }
    void* dataP = malloc(size);

    state->state = state->state == unknown ? newBlock : reallocatedBlock;
    state->mem = dataP;
    state->size = size;

    return dataP;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void* DTMXAttributeHandler::ReallocateMemoryInternal (DTMPartition type, int partitionNumber, size_t size)
    {
    write_to_log (L"ReallocateMemoryInternal %x (%d/%d, %d)", this, type, partitionNumber, size);
    BeAssert (m_writeCount != 0);
    BeAssert (size != 0);

    CheckScanXAttributes (type);
    memoryMapT::iterator iter = m_memory[(int)type].find (partitionNumber);
    if (iter != m_memory[(int)type].end ())
        {
        memStateT* state = &iter->second;
        if (size == state->size && state->state != deletedBlock)
            return state->mem;

        if (state->state == newBlock)
            {
            state->mem = realloc(state->mem, size);
            state->size = size;
            }
        else
            {
            if (state->state == reallocatedBlock)
                {
                if (state->xAttributeSize == size)
                    {
                    UInt16 xAttrId = GetXAttrId(type);
                    XAttributeHandlerId handlerId (TMElementMajorId, xAttrId);
                    ElementHandle::XAttributeIter xAttrIter (m_handle, handlerId, partitionNumber);
                    BeAssert (size == xAttrIter.GetSize());

                    void* originalMem = (void*)GetWritableDataPointer (xAttrIter);
                    memcpy(originalMem, state->mem, size < state->size ? size : state->size);
                    free (state->mem);

                    state->mem = originalMem;
                    state->size = size;
                    state->state = existingBlock;
                    }
                else
                    {
                    state->mem = realloc(state->mem, size);
                    state->size = size;
                    }
                }
            else if (state->state == existingBlock)
                {
                void* p = state->mem;
                state->mem = malloc(size);
                memcpy(state->mem, p, size < state->size ? size : state->size);
                state->size = size;
                state->state = reallocatedBlock;
                }
            else if (state->state == deletedBlock)
                {
                if (state->xAttributeSize == size)
                    {
                    UInt16 xAttrId = GetXAttrId(type);
                    XAttributeHandlerId handlerId (TMElementMajorId, xAttrId);
                    ElementHandle::XAttributeIter xAttrIter (m_handle, handlerId, partitionNumber);

                    BeAssert (size == xAttrIter.GetSize());
                    state->mem = GetWritableDataPointer (xAttrIter);
                    state->size = size;
                    state->state = existingBlock;
                    }
                else
                    {
                    state->mem = malloc(size);
                    state->size = size;
                    state->state = reallocatedBlock;
                    }
                }
            }
        return state->mem;
        }

    size_t prevSize = GetDataSize(type, partitionNumber);

    memStateT* state = &m_memory[(int)type][partitionNumber];
    state->mem = malloc(size);
    state->size = size;
    state->xAttributeSize = 0;
    state->state = reallocatedBlock;
    memcpy(state->mem, GetDataHandle(type, partitionNumber), size < prevSize ? size : prevSize);
    return state->mem;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::ScheduleWriteData (DTMPartition type, int partition, const byte* data, long size)
    {
    write_to_log(L"ScheduleWriteData %x (%d/%d %d)", this, type, partition, size);
    BeAssert (size != 0);
    if (size == 0)
        return;

    UInt16 xAttrId = GetXAttrId(type);

    m_handle.ScheduleWriteXAttribute (XAttributeHandlerId (TMElementMajorId, xAttrId), partition, size, data);

    memStateT& memState = m_memory[(int)type][partition];
    ElementHandle::XAttributeIter xAttr (m_handle, XAttributeHandlerId (TMElementMajorId, xAttrId), partition);
    BeAssert (xAttr.IsValid());
    memState.mem = const_cast<void*>(xAttr.PeekData());
    memState.size = size;
    memState.state = existingBlock;
    memState.xAttributeSize = size;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::FreeMemoryInternal(DTMPartition type, int partitionNumber)
    {
    write_to_log(L"FreeMemoryInternal %x (%d/%d)", this, type, partitionNumber);
    BeAssert(m_writeCount != 0);

    CheckScanXAttributes (type);
    memoryMapT::iterator iter = m_memory[(int)type].find(partitionNumber);

    if (iter != m_memory[(int)type].end())
        {
        if (iter->second.state == newBlock)
            {
            free(iter->second.mem);
            m_memory[(int)type].erase(partitionNumber);
            }
        else
            {
            if (iter->second.state != existingBlock)
                free(iter->second.mem);
            iter->second.state = deletedBlock;
            if (type == DTMPartition::None)
                m_extendedMemoryDeleted.push_back (partitionNumber);
            iter->second.mem = nullptr;
            }
        }
    else
        {
        memStateT* state = &m_memory[(int)type][partitionNumber];
        state->size = 0;
        state->state = deletedBlock;
        state->mem = nullptr;
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
UInt16 DTMXAttributeHandler::GetXAttrId (DTMPartition type)
    {

    UInt16 xAttrId = 0;
    switch (type)
        {
        case DTMPartition::Point:
            xAttrId = XATTRIBUTES_SUBID_DTM_POINTARRAY;
            break;
        case DTMPartition::Feature:
            xAttrId = XATTRIBUTES_SUBID_DTM_FEATUREARRAY;
            break;
        case DTMPartition::Node:
            xAttrId = XATTRIBUTES_SUBID_DTM_NODEARRAY;
            break;
        case DTMPartition::CList:
            xAttrId = XATTRIBUTES_SUBID_DTM_CLISTARRAY;
            break;
        case DTMPartition::FList:
            xAttrId = XATTRIBUTES_SUBID_DTM_FLISTARRAY;
            break;
        case DTMPartition::Header:
            xAttrId = XATTRIBUTES_SUBID_DTM_HEADER;
            break;
        case DTMPartition::None:
            xAttrId = XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP;
        }
    return xAttrId;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 11/11
//=======================================================================================
void DTMXAttributeHandler::ScanXAttributes (DTMPartition type)
    {
    if (!m_hasScanned[(int)type])
        {
        m_hasScanned[(int)type] = true;
        UInt16 xAttrId = GetXAttrId(type);
        XAttributeHandlerId handlerId (TMElementMajorId, xAttrId);

        ElementHandle::XAttributeIter xAttrIter (m_handle, handlerId);

        while (xAttrIter.IsValid())
            {
            memoryMapT::const_iterator iter = m_memory[(int)type].find (xAttrIter.GetId ());

            if (iter == m_memory[(int)type].end ())
                {
                void* mem;
                size_t size;
                XAttributeHandle* Xhandle = const_cast<XAttributeHandle*>((XAttributeHandle*)xAttrIter.GetElementXAttributeIter());

                if (Xhandle)
                    {
                    mem = Xhandle->GetPtrForWrite();
                    size = Xhandle->GetSize();
                    }
                else
                    {
                    mem = (void*)xAttrIter.PeekData();
                    size = xAttrIter.GetSize();
                    }

                memStateT* state = &m_memory[(int)type][xAttrIter.GetId ()];
                state->size = size;
                state->xAttributeSize = size;
                state->state = existingBlock;
                state->mem = mem;
                }
            if (!xAttrIter.SearchNext (handlerId))
                break;
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void* DTMXAttributeHandler::GetDataHandle (DTMPartition type, int index)
    {
    write_to_log(L"GetDataHandle %x (%d/%d)", this, type, index);
    CheckScanXAttributes (type);
    memoryMapT::const_iterator iter = m_memory[(int)type].find (index);

    if (iter != m_memory[(int)type].end ())
        {
        return iter->second.mem;
        }

    memStateT* state = &m_memory[(int)type][index];
    state->size = 0;
    state->xAttributeSize = 0;
    state->state = existingBlock;
    state->mem = nullptr;
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
size_t DTMXAttributeHandler::GetDataSize (DTMPartition type, int index)
    {
    memoryMapT::const_iterator iter = m_memory[(int)type].find (index);

    if (iter != m_memory[(int)type].end ())
        {
        return iter->second.size;
        }
    GetDataHandle(type, index);
    return m_memory[(int)type][index].size;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::dh_freeXAttr (memoryMapT::iterator& iter, DTMPartition type)
    {
    int index = iter->first;
    write_to_log(L"dh_freeXAttr %x (%d/%d)", this, type, index);
    UInt16 xAttrId = GetXAttrId(type);

    if (m_noSchedule)
        {
        XAttributeHandle xAttr (m_handle.GetElementRef(), XAttributeHandlerId (TMElementMajorId, xAttrId), index);
        BeAssert (xAttr.IsValid());
        ITxnManager::GetCurrentTxn().DeleteXAttribute (xAttr);
        }
    else
        m_handle.ScheduleDeleteXAttribute (XAttributeHandlerId (TMElementMajorId, xAttrId), index);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void DTMXAttributeHandler::dh_allocXAttr (memoryMapT::iterator& iter, DTMPartition type)
    {
    int index = iter->first;
    void* dataP = iter->second.mem;
    size_t dataSize = iter->second.size;

    write_to_log(L"dh_allocXAttr %x (%d/%d %d)", this, type, index, dataSize);
    BeAssert (dataSize != 0);
    if (dataSize == 0)
        return;

    UInt16 xAttrId = GetXAttrId(type);

    if (m_noSchedule)
        {
        if (iter->second.state == reallocatedBlock)
            {
            XAttributeHandle xAttr (m_handle.GetElementRef(), XAttributeHandlerId (TMElementMajorId, xAttrId), index);
            BeAssert (xAttr.IsValid());
            ITxnManager::GetCurrentTxn().ReplaceXAttributeData (xAttr, dataP, (UInt32)dataSize);
            iter->second.mem = const_cast<void*>(xAttr.PeekData());
            }
        else
            {
            ITxnManager::GetCurrentTxn().AddXAttribute (m_handle.GetElementRef (), XAttributeHandlerId (TMElementMajorId, xAttrId), index, dataP, (UInt32)dataSize);
            XAttributeHandle xAttr (m_handle.GetElementRef(), XAttributeHandlerId (TMElementMajorId, xAttrId), index);
            BeAssert (xAttr.IsValid());
            iter->second.mem = const_cast<void*>(xAttr.PeekData());
            }
        }
    else
        {
        m_handle.ScheduleWriteXAttribute (XAttributeHandlerId (TMElementMajorId, xAttrId), index, dataSize, dataP);
        ElementHandle::XAttributeIter xAttr (m_handle, XAttributeHandlerId (TMElementMajorId, xAttrId), index);
        BeAssert (xAttr.IsValid());
        iter->second.mem = const_cast<void*>(xAttr.PeekData());
        }
    iter->second.state = existingBlock;
    iter->second.xAttributeSize = dataSize;
    free (dataP);

    if (!m_inCreate)
        {
        // Update Pointer in the DTM arrays
        switch (type)
            {
            case DTMPartition::Point:   m_dtm->GetTinHandle ()->pointsPP[index] = (DPoint3d*)iter->second.mem; break;
            case DTMPartition::Feature: m_dtm->GetTinHandle ()->fTablePP[index] = (BC_DTM_FEATURE*)iter->second.mem; break;
            case DTMPartition::Node:    m_dtm->GetTinHandle ()->nodesPP[index] = (DTM_TIN_NODE*)iter->second.mem; break;
            case DTMPartition::CList:   m_dtm->GetTinHandle ()->cListPP[index] = (DTM_CIR_LIST*)iter->second.mem; break;
            case DTMPartition::FList:   m_dtm->GetTinHandle ()->fListPP[index] = (DTM_FEATURE_LIST*)iter->second.mem; break;
            }
        }
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMXAttributeHandler* DTMXAttributeHandler::Create (ElementHandleCR element, BcDTMR bcDTM, bool inCreate)
    {
    return new DTMXAttributeHandler (element, bcDTM, inCreate);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
bool DTMXAttributeHandler::HasDTMData (ElementHandleCR element)
    {
    if (dynamic_cast<DTMElementHandler*>(&element.GetHandler()) != 0 || dynamic_cast<DTMElement107Handler*>(&element.GetHandler()) != 0)
        {
        XAttributeHandlerId handlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER);
        ElementHandle::XAttributeIter xAttrHandle (element, handlerId, 0);
        return xAttrHandle.IsValid();
        }
    return false;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMXAttributeHandler* DTMXAttributeHandler::LoadDTM (ElementHandleCR element)
    {
    // Get the handles to the XAttribute
    BC_DTM_OBJ* headerData = nullptr;
    UInt32 headerSize  = 0;

    // Look at the XAttribute for a valid Header.
    // Get the DTM iter
    DTMBinaryDataIter iter1 (element, XATTRIBUTES_SUBID_DTM_HEADER);
    iter1.Reset();
    if (iter1.MoveNext())
        {
        headerData = (BC_DTM_OBJ*)iter1.GetCurrentData();
        headerSize = iter1.GetCurrentSize();
        }

    long* header = (long*)headerData;

    if (header[1] == 0xffffff38)
        return nullptr;

    BcDTMPtr bcDtm = BcDTM::Create (0, 100);
    if (bcDtm.IsNull())
        return nullptr;

    DTMXAttributeHandler* allocator = Create (element, *bcDtm);
    BC_DTM_OBJ* tinP = bcDtm->GetTinHandle();

    bcdtmObject_setToDTMElement (tinP, allocator);
    allocator->UpdateDTMPointers ();

    #ifdef DEBUG_MEMCHECK
    if (bcdtmCheck_tinComponentDtmObject (tinP) != DTM_SUCCESS)
        BeAssert (false);
    #endif

    return allocator;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
StatusInt DTMXAttributeHandler::ScheduleDtmData (EditElementHandleR elemHandle, BcDTMR bcDTM, bool disposeDTM, DTMXAttributeHandler* allocator)
    {
    // ToDo need to look at this, function.
    // Get the handles of the DTM and store binary data.
    void* headerP;
    int nHeader;
    void** featuresArrayP;
    int featuresNArray;
    int featuresArraySize;
    int featuresLastArraySize;
    void** pointArrayP;
    int pointNArray;
    int pointArraySize;
    int pointLastArraySize;
    void** nodeArrayP;
    int nodeNArray;
    int nodeArraySize;
    int nodeLastArraySize;
    void** clistArrayP;
    int clistNArray;
    int clistArraySize;
    int clistLastArraySize;
    void** flistArrayP;
    int flistNArray;
    int flistArraySize;
    int flistLastArraySize;

    bcDTM.GetHandles(&headerP, &nHeader,
        &featuresArrayP, &featuresNArray, &featuresArraySize, &featuresLastArraySize,
        &pointArrayP, &pointNArray, &pointArraySize, &pointLastArraySize,
        &nodeArrayP, &nodeNArray, &nodeArraySize, &nodeLastArraySize,
        &clistArrayP, &clistNArray, &clistArraySize, &clistLastArraySize,
        &flistArrayP, &flistNArray, &flistArraySize, &flistLastArraySize
       );

    BC_DTM_OBJ* tinP = (BC_DTM_OBJ*)bcDTM.GetTinHandle();

    if (nullptr != allocator &&  &bcDTM == allocator->GetDTM ())
        return SUCCESS;

    if (bcdtmMemory_getAllocator (tinP) != nullptr)
        disposeDTM = false;

    int index = 0;
    DTMXAttributeHandler* m_allocator = allocator;
    if (nullptr == allocator)
        m_allocator = Create (elemHandle, bcDTM, true);

    // If this is a DTMElement then we can't do a copy.
    m_allocator->StartModify (false);

    m_allocator->DeleteXAttributes();
    bool cancelAdd = false;

    // Need to change this to map the arrays accross to the allocation if we are disposing the TM.
    // Otherwise schedule the write and update the pointers.
    try
        {
        if (headerP != nullptr)
            m_allocator->WriteHeader ((const byte*)headerP, nHeader);

        for (index = 0; index < featuresNArray; index++)
            {
            int size = featuresArraySize;
            if (index == featuresNArray - 1)
                size = featuresLastArraySize;

            int count = size / sizeof(BC_DTM_FEATURE);

            BC_DTM_FEATURE* features = (BC_DTM_FEATURE*)m_allocator->AllocateMemory (DTMPartition::Feature, index, size);
            BC_DTM_FEATURE* dtmFeatureP = features;
            memcpy (features, (const byte*)featuresArrayP[index], size);
            for (int i = 0; i < count; dtmFeatureP++, i++)
                {
                if (dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
                    {
                    long pi = m_allocator->AllocateMemory (sizeof(DPoint3d) * dtmFeatureP->numDtmFeaturePts);
                    void* data = bcdtmMemory_getPointer (tinP, dtmFeatureP->dtmFeaturePts.pointsPI);
                    memcpy (m_allocator->GetMemoryPointer (pi), (void*)data, sizeof(DPoint3d) * dtmFeatureP->numDtmFeaturePts);
                    if (disposeDTM)
                        bcdtmMemory_free (tinP, dtmFeatureP->dtmFeaturePts.pointsPI);
                    dtmFeatureP->dtmFeaturePts.pointsPI = pi;
                    }
                else if (dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray)
                    {
                    long pi = m_allocator->AllocateMemory (sizeof(long) * dtmFeatureP->numDtmFeaturePts);
                    void* data = bcdtmMemory_getPointer (tinP, dtmFeatureP->dtmFeaturePts.offsetPI);
                    memcpy (m_allocator->GetMemoryPointer (pi), (void*)data, sizeof(long) * dtmFeatureP->numDtmFeaturePts);
                    if (disposeDTM)
                        bcdtmMemory_free (tinP, dtmFeatureP->dtmFeaturePts.pointsPI);
                    dtmFeatureP->dtmFeaturePts.offsetPI = pi;
                    }
                else if (dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError)
                    {
                    long pi = m_allocator->AllocateMemory (sizeof(DPoint3d) * dtmFeatureP->numDtmFeaturePts);
                    void* data = bcdtmMemory_getPointer (tinP, dtmFeatureP->dtmFeaturePts.pointsPI);
                    memcpy (m_allocator->GetMemoryPointer (pi), (void*)data, sizeof(DPoint3d) * dtmFeatureP->numDtmFeaturePts);
                    if (disposeDTM)
                        bcdtmMemory_free (tinP, dtmFeatureP->dtmFeaturePts.pointsPI);
                    dtmFeatureP->dtmFeaturePts.pointsPI = pi;
                    }
                else if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback)
                    {
                    long pi = m_allocator->AllocateMemory (sizeof(DPoint3d) * dtmFeatureP->numDtmFeaturePts);
                    void* data = bcdtmMemory_getPointer (tinP, dtmFeatureP->dtmFeaturePts.pointsPI);
                    memcpy (m_allocator->GetMemoryPointer (pi), (void*)data, sizeof(DPoint3d) * dtmFeatureP->numDtmFeaturePts);
                    if (disposeDTM)
                        bcdtmMemory_free (tinP, dtmFeatureP->dtmFeaturePts.pointsPI);
                    dtmFeatureP->dtmFeaturePts.pointsPI = pi;
                    }
                }

            if (disposeDTM)
                {
                bcdtmMemory_freePartition(tinP, DTMPartition::Feature, index, featuresArrayP[index]);
                featuresArrayP[index] = nullptr;
                }
            }

        for (index = 0; index < pointNArray; index++)
            {
            if (index == pointNArray - 1)
                m_allocator->ScheduleWriteData (DTMPartition::Point, index, (const byte*)pointArrayP[index], pointLastArraySize);
            else
                m_allocator->ScheduleWriteData (DTMPartition::Point, index, (const byte*)pointArrayP[index], pointArraySize);

            if (disposeDTM)
                {
                bcdtmMemory_freePartition(tinP, DTMPartition::Point, index, pointArrayP[index]);
                pointArrayP[index] = nullptr;
                }
            }
        for (index = 0; index < nodeNArray; index++)
            {
            if (index == nodeNArray - 1)
                m_allocator->ScheduleWriteData (DTMPartition::Node, index, (const byte*)nodeArrayP[index], nodeLastArraySize);
            else
                m_allocator->ScheduleWriteData (DTMPartition::Node, index, (const byte*)nodeArrayP[index], nodeArraySize);

            if (disposeDTM)
                {
                bcdtmMemory_freePartition(tinP, DTMPartition::Node, index, nodeArrayP[index]);
                nodeArrayP[index] = nullptr;
                }
            }
        for (index = 0; index < clistNArray; index++)
            {
            if (index == clistNArray - 1)
                m_allocator->ScheduleWriteData (DTMPartition::CList, index, (const byte*)clistArrayP[index], clistLastArraySize);
            else
                m_allocator->ScheduleWriteData (DTMPartition::CList, index, (const byte*)clistArrayP[index], clistArraySize);

            if (disposeDTM)
                {
                bcdtmMemory_freePartition(tinP, DTMPartition::CList, index, clistArrayP[index]);
                clistArrayP[index] = nullptr;
                }
            }
        for (index = 0; index < flistNArray; index++)
            {
            if (index == flistNArray - 1)
                m_allocator->ScheduleWriteData (DTMPartition::FList, index, (const byte*)flistArrayP[index], flistLastArraySize);
            else
                m_allocator->ScheduleWriteData (DTMPartition::FList, index, (const byte*)flistArrayP[index], flistArraySize);

            if (disposeDTM)
                {
                bcdtmMemory_freePartition (tinP, DTMPartition::FList, index, flistArrayP[index]);
                flistArrayP[index] = nullptr;
                }
            }
        }
    catch (...)
        {
        cancelAdd = true;
        }

    if (cancelAdd)
        {
        if (disposeDTM)
            {
            for (index = 0; index < featuresNArray; index++)
                {
                if (featuresArrayP[index])
                    bcdtmMemory_freePartition(tinP, DTMPartition::Feature, index, featuresArrayP[index]);
                featuresArrayP[index] = nullptr;
                }

            for (index = 0; index < pointNArray; index++)
                {
                if (pointArrayP[index])
                    bcdtmMemory_freePartition(tinP, DTMPartition::Point, index, pointArrayP[index]);
                pointArrayP[index] = nullptr;
                }
            for (index = 0; index < nodeNArray; index++)
                {
                if (nodeArrayP[index])
                    bcdtmMemory_freePartition(tinP, DTMPartition::Node, index, nodeArrayP[index]);
                nodeArrayP[index] = nullptr;
                }
            for (index = 0; index < clistNArray; index++)
                {
                if (clistArrayP[index])
                    bcdtmMemory_freePartition(tinP, DTMPartition::CList, index, clistArrayP[index]);
                clistArrayP[index] = nullptr;
                }
            for (index = 0; index < flistNArray; index++)
                {
                if (flistArrayP[index])
                    bcdtmMemory_freePartition (tinP, DTMPartition::FList, index, flistArrayP[index]);
                flistArrayP[index] = nullptr;
                }
            }
        m_allocator->DeleteXAttributes();
        delete m_allocator;
        return ERROR;
        }

    if (allocator != nullptr)
        m_allocator->m_inCreate = true;

        {
        AutoRestore<bool>  const savePoint (&m_allocator->m_inScheduleReplace);
        m_allocator->m_inScheduleReplace = true;
        m_allocator->EndModify ();
        }
    if (allocator != nullptr)
        m_allocator->m_inCreate = false;

    if (allocator == nullptr)
        {
        delete m_allocator;
        m_allocator = nullptr;
        }

    if (!disposeDTM)
        {
        if (nullptr != allocator)
            allocator->UpdateDTMPointers ();
        }
    else  // As we have disposed of this DTM data, free the arrays and set the nums to 0.
        {
        FREE_AND_CLEAR (tinP->cListPP);
        FREE_AND_CLEAR (tinP->fListPP);
        FREE_AND_CLEAR (tinP->fTablePP);
        FREE_AND_CLEAR (tinP->nodesPP);
        FREE_AND_CLEAR (tinP->pointsPP);
        tinP->numPointPartitions = 0; tinP->numPoints = 0;
        tinP->numNodePartitions = 0; tinP->numNodes = 0;
        tinP->numClistPartitions = 0; tinP->numClist = 0;
        tinP->numFeaturePartitions = 0; tinP->numFeatures = 0;
        tinP->numFlistPartitions = 0; tinP->numFlist = 0;
        }
    return SUCCESS;
    }
//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void* dh_allocAndStoreXAttr (int index, DTMPartition featureType, void* dataP, int dataSize, void* userP)
    {
    EditElementHandleP elementHandle = (EditElementHandleP)userP;

    UInt16 xAttrId = 0;
    switch (featureType)
        {
        case DTMPartition::Point:
            xAttrId = XATTRIBUTES_SUBID_DTM_POINTARRAY;
            break;
        case DTMPartition::Feature:
            xAttrId = XATTRIBUTES_SUBID_DTM_FEATUREARRAY;
            break;
        case DTMPartition::Node:
            xAttrId = XATTRIBUTES_SUBID_DTM_NODEARRAY;
            break;
        case DTMPartition::CList:
            xAttrId = XATTRIBUTES_SUBID_DTM_CLISTARRAY;
            break;
        case DTMPartition::FList:
            xAttrId = XATTRIBUTES_SUBID_DTM_FLISTARRAY;
            break;
        case DTMPartition::Header:
            xAttrId = XATTRIBUTES_SUBID_DTM_HEADER;
            break;
        case DTMPartition::None:
            xAttrId = XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP;
            break;
        }

    DTMBinaryData::ScheduleWriteData (elementHandle, xAttrId, index, (const byte*)dataP, dataSize);
    return (void*)1;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 12/10
//=======================================================================================
void    DTMHeaderXAttributeHandler::DeleteCache (XAttributeHandleCR xAttr, TransactionType type)
    {
    if (xAttr.GetElementRef())
        {
        ElementRefP refs[10];
        int numDep = xAttr.GetElementRef()->GetDependents (refs, 10);

        for (int i = 0; i < numDep; i++)
            {
            if (refs[i]->GetElementType() == 106)
                {
                EditElementHandle element (refs[i], nullptr);

                DTMElementDisplayHandler* handler = dynamic_cast<DTMElementDisplayHandler*>(element.GetDisplayHandler());
                if (handler)
                    {
                    DTMDisplayCacheManager::DeleteCacheElem (element);

                    if (type == TRANSACTIONTYPE_Action)
                        {
                        element.GetElementDescrP();

                        if (handler)
                            {
                            ScanRange rng = element.GetElementDescrCP()->el.hdr.dhdr.range;
                            if (handler->ValidateElementRange (element, false) == SUCCESS)
                                {
                                ScanRange rng2 = element.GetElementDescrCP()->el.hdr.dhdr.range;
                                if (memcmp(&rng, &rng2, sizeof(ScanRange)) != 0)
                                    element.ReplaceInModel (refs[i]);
                                }
                            }
                        }
                    }
                RedrawElement (element);
                }
            }
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 12/10
//=======================================================================================
void DTMHeaderXAttributeHandler::_OnPostAdd (XAttributeHandleCR xAttr, TransactionType type)
    {
    DeleteCache (xAttr, type);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 12/10
//=======================================================================================
void    DTMHeaderXAttributeHandler::_OnPostModifyData (XAttributeHandleCR xAttr, TransactionType type)
    {
    DeleteCache (xAttr, type);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 12/10
//=======================================================================================
void    DTMHeaderXAttributeHandler::_OnPostReplaceData (XAttributeHandleCR xAttr, TransactionType type)
    {
    DeleteCache (xAttr, type);
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
