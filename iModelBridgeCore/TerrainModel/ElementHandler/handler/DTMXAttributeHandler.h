/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMXAttributeHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "DTMBinaryData.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
using namespace std;

enum memStateState : byte
    {
    unknown,
    newBlock,
    existingBlock,
    reallocatedBlock,
    deletedBlock,
    };

struct memStateT
    {
     void* mem;
     size_t size;
     size_t xAttributeSize;
     memStateState state;

     memStateT()
         {
         memset (this, 0, sizeof(memStateT));
         }
    };

class DTMXAttributeHandler : public IDTMElementMemoryAllocator
    {
    static const int NUMPARTITIONTYPES = ((int)DTMPartition::FList) + 1;

    BcDTMPtr m_dtm;

    BC_DTM_OBJ m_originalHeader;

#ifdef COMPLETE_TEMPORARY_CHECK
        typedef bmap<uint32_t, uint32_t> checkSumT;
        checkSumT m_checkSums[NUMPARTITIONTYPES];
#endif

        bool CheckTemporary (bool goingToModify);
        void StoreSnapShot ();

    public:
        uint32_t m_nextIndex;
        EditElementHandle m_handle;

        typedef bmap<uint32_t, memStateT> memoryMapT;
        memoryMapT m_memory [NUMPARTITIONTYPES];

        bvector<uint32_t> m_extendedMemoryDeleted;

        bool m_hasScanned [NUMPARTITIONTYPES];

        bool m_isTemporary;
        int m_writeCount;
        bool m_noSchedule;
        bool m_inCreate;

        bool IsOpenedForWrite()
            {
            return m_writeCount != 0 && !m_isTemporary;
            }

    public:
        virtual void WriteHeader (const byte* data, int size);

        virtual void* AllocateMemory (DTMPartition type, int partitionNumber, size_t size) override
            {
            return AllocateMemoryInternal (type, partitionNumber, size);
            }

        virtual void* ReallocateMemory (DTMPartition type, int partitionNumber, void *pointerOld, size_t size) override
            {
            return ReallocateMemoryInternal (type, partitionNumber, size);
            }

        virtual void FreeMemory (DTMPartition type, int partitionNumber, void* pointer) override
            {
            FreeMemoryInternal (type, partitionNumber);
            }

        virtual void* GetMemoryPointer (int partitionNumber) override
            {
            void* mem = GetDataHandle (DTMPartition::None, partitionNumber);
            if (mem == NULL)
                mem = NULL;
            return mem;
            }

        virtual int AllocateMemory (size_t size) override
            {
            int num;
            
            if (m_extendedMemoryDeleted.size())
                {
                num = m_extendedMemoryDeleted.back();
                m_extendedMemoryDeleted.pop_back();
                }
            else
                num = m_nextIndex++;
            AllocateMemoryInternal (DTMPartition::None, num, size);
            return num;
            }

        virtual int ReallocateMemory (int partitionNumber, size_t size) override
            {
            ReallocateMemoryInternal (DTMPartition::None, partitionNumber, size);
            return partitionNumber;
            }

        virtual void FreeMemory (int partitionNumber) override
            {
            return FreeMemoryInternal (DTMPartition::None, partitionNumber);
            }

        virtual void Release()
            {
            delete this;
            }

        virtual int SetMemoryAccess (BC_DTM_OBJ* dtm, DTMAccessMode accessMode);

        void UndoRedo ();
        void StartModify (bool disableUndo);
        void EndModify ();
        void UpdateDTMPointers ();
        void AddXAttributesToUndoBuffer ();
        void DeleteXAttributes();
        void ScheduleWriteData (DTMPartition type, int partition, const byte* data, long size);

        DTMXAttributeHandler (ElementHandleCR element, BcDTMR bcDTM, bool inCreate = false);

        BcDTMP GetDTM()
            {
            return m_dtm.get();
            }

        void UpdateXAttributes();
        void ReplaceInModel();
        bool HasModified();
        virtual ~DTMXAttributeHandler();
        ElementHandle GetElemHandle()
            {
            return m_handle;
            }

    private:
        template<class TYPE> bool FixUpDTMPartitionArray (DTMPartition partition, long originalMem, long originalNumPartitions, long& num, long& mem, long& numPartitions, long partitionSize, TYPE** partitionArray);

        void* AllocateMemoryInternal (DTMPartition type, int num, size_t size);
        void* ReallocateMemoryInternal (DTMPartition type, int partitionNumber, size_t size);
        void FreeMemoryInternal (DTMPartition type, int partitionNumber);
        UInt16 GetXAttrId (DTMPartition type);
        void* GetDataHandle (DTMPartition type, int index);
        size_t GetDataSize (DTMPartition type, int index);
        void dh_freeXAttr (memoryMapT::iterator& iter, DTMPartition type);
        void dh_allocXAttr (memoryMapT::iterator& iter, DTMPartition type);
        void ScanXAttributes (DTMPartition type);
        void CheckScanXAttributes (DTMPartition type)
            {
            if (!m_hasScanned[(int)type])
                ScanXAttributes (type);
            }
        memStateT* GetMemoryMapT (DTMPartition type, uint32_t id)
            {
            memoryMapT::iterator iter = m_memory[(int)type].find (id);

            if (iter == m_memory[(int)type].end ())
                return nullptr;
            return &iter->second;
            }

    public:
        static DTMXAttributeHandler* LoadDTM (ElementHandleCR element);
        static StatusInt ScheduleDtmData (EditElementHandleR element, BcDTMR bcDTM, bool disposeDTM, DTMXAttributeHandler* allocator = nullptr);
        static bool HasDTMData (ElementHandleCR element);
        static void StartTMPersist ();
        static void EndTMPersist ();
        static void ReloadData (ElementRefP ref);

        static DTMXAttributeHandler* Create (ElementHandleCR element, BcDTMR bcDTM, bool inCreate = false);
    };


class DTMHeaderXAttributeHandler : public XAttributeHandler, public IXAttributeTransactionHandler
    {
    public: static XAttributeHandlerId GetXAttributeHandlerId () {return XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER);}

    public: virtual IXAttributeTransactionHandler* _GetIXAttributeTransactionHandler() override { return this; }

    virtual void    _OnPostModifyData (XAttributeHandleCR xAttr, TransactionType type) override;
    virtual void    _OnPostReplaceData (XAttributeHandleCR xAttr, TransactionType type) override;
    virtual void _OnPostAdd (XAttributeHandleCR xAttr, TransactionType type) override;
    private:
        void DeleteCache (XAttributeHandleCR xAttr, TransactionType type);

    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
