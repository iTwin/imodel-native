/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/

#pragma once

/** --------------------------------------------------------------------------------------------------------
This interface defines the methods required for a scalable mesh data store. In short, a scalable mesh 
data store uses block IDs (HPMBlockID) to identify blocks of data of type DataType. Blocks of data can 
be stored, loaded or removed from store. Data can also be stored/loaded as (already) compressed blocks if 
need be.
--------------------------------------------------------------------------------------------------------
*/


////MST_TS
#include <ImagePP\all\h\HFCPtr.h>
#include <ImagePP\all\h\HPMDataStore.h>
USING_NAMESPACE_IMAGEPP

//template <class MasterHeaderType, class NodeHeaderType> class ISMDataStore;      

template <class DataType, class NodeHeaderType> class ISMNodeDataStore : public RefCountedBase
    {
    private : 
        
        NodeHeaderType* m_nodeHeader;
        //ISMDataStore*   m_store;
        
    public : 

        ISMNodeDataStore(/*ISMDataStore* m_store, */NodeHeaderType* nodeHeader)
            {
            m_nodeHeader = nodeHeader;
            //m_store = store;
            }    

        ~ISMNodeDataStore()
            {
            }

        virtual HPMBlockID StoreNewBlock(DataType* DataTypeArray, size_t countData) = 0;

        virtual HPMBlockID StoreBlock(DataType* DataTypeArray, size_t countData, HPMBlockID blockID) = 0;

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const = 0;

        virtual size_t LoadBlock(DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID) = 0;

        virtual bool DestroyBlock(HPMBlockID blockID) = 0;

         /**----------------------------------------------------------------------------
        Stores an existing block ... if the current block size is not sufficient to hold the data, then
        a new block can be allocated elsewhere. The new or previous block ID is returned. No compression
        is performed prior to store the block and the block is assumed to be already compressed.
        -----------------------------------------------------------------------------*/
        virtual HPMBlockID StoreCompressedBlock(DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            HASSERT(false); // Not implemented;
            return HPMBlockID();
            }

        /**----------------------------------------------------------------------------
        Loads data type block designated. The method will not load more data
        than can be held in provided buffer. To know the required buffer size prior
        to loading the block, the method GetBlockDataCount() can be used. No decompression
        prior to loading will be performed before loading the block.
        -----------------------------------------------------------------------------*/
        virtual size_t LoadCompressedBlock(bvector<DataType>& DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            HASSERT(false); // Not implemented;
            return 0;
            }

        /**----------------------------------------------------------------------------
        This method serializes the node for streaming.        
        -----------------------------------------------------------------------------*/
        virtual void SerializeHeaderToBinary(const NodeHeaderType* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize, uint32_t pi_pMaxDataSize = 3000) const
            {
            HASSERT(false); // Not implemented;
            }
    };


template <class MasterHeaderType, class NodeHeaderType>  class ISMDataStore : public RefCountedBase
    {
    public:

        ISMDataStore() {};
        virtual ~ISMDataStore() {};

        typedef RefCountedPtr<ISMDataStore<MasterHeaderType, NodeHeaderType>> Ptr; 

        /**----------------------------------------------------------------------------
         Stores the master header in the store. The master header is of an undefined type
         but should contain all information pertinent to the whole store.         
        -----------------------------------------------------------------------------*/
        virtual bool StoreMasterHeader (MasterHeaderType* header, size_t headerSize) = 0;

        /**----------------------------------------------------------------------------
         Loads the master header from the store. The master header is of an undefined type
         but should contain all information pertinent to the whole store.         
        -----------------------------------------------------------------------------*/
        virtual size_t LoadMasterHeader (MasterHeaderType* header, size_t maxHeaderSize) = 0;

        /**----------------------------------------------------------------------------
         Stores the block header in the store. The block header is of an undefined type
         but should contain all information pertinent to the designated block except the
         block of data of type DataType.
        -----------------------------------------------------------------------------*/
        virtual size_t StoreNodeHeader (NodeHeaderType* header, HPMBlockID blockID) = 0;

        /**----------------------------------------------------------------------------
         Loads the block header in the store. The block header is of an undefined type
         but should contain all information pertinent to the designated block except the
         block of data of type DataType.
        -----------------------------------------------------------------------------*/
        virtual size_t LoadNodeHeader (NodeHeaderType* header, HPMBlockID blockID) = 0;

        /**----------------------------------------------------------------------------
         Get the next node ID available.
        -----------------------------------------------------------------------------*/
        virtual uint64_t GetNextID() const = 0;

        virtual void Close () = 0;
                
        virtual RefCountedPtr<ISMNodeDataStore<DPoint3d, NodeHeaderType>> GetNodeDataStore(NodeHeaderType* nodeHeader) = 0;
                            
    };