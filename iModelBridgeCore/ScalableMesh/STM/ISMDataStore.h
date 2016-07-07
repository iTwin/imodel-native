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

enum class SMNodeDataStoreType
    {
    Points = 0,
    /*
    TriPtIndices,    
    TriUvIndices, 
    UvCoords, 
    DiffSet, 
    Graph,
    Texture,
    Display, //Use to represents data created for display purpose, like QV element. 
    LinearFeature,
    BcDTM,
    Unknown, 
    */
    };

template <typename DataType>  class ISMNodeDataStore : public RefCountedBase
    {
    private : 

        SMPointIndexHeader<EXTENT>* m_nodeHeader;
        ISMDataStore*               m_store;

    public : 

        ISMNodeDataStore(ISMDataStore* m_store, SMPointIndexHeader<EXTENT>* nodeHeader)
            {
            m_nodeHeader = nodeHeader;
            m_store = store;
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
        virtual void   SerializeHeaderToBinary(const TileHeaderType* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize, uint32_t pi_pMaxDataSize = 3000) const
            {
            HASSERT(false); // Not implemented;
            }
    };

typedef RefCountedPtr<ISMNodeDataStore<>> ISMNodeDataStorePtr;


template <typename PointType, typename MasterHeaderType, typename NodeHeaderType>  class ISMDataStore
    {
    public:

        ISMDataStore() {};
        virtual ~ISMDataStore() {};

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
        virtual size_t StoreHeader (NodeHeaderType* header, HPMBlockID blockID) = 0;

        /**----------------------------------------------------------------------------
         Loads the block header in the store. The block header is of an undefined type
         but should contain all information pertinent to the designated block except the
         block of data of type DataType.
        -----------------------------------------------------------------------------*/
        virtual size_t LoadHeader (NodeHeaderType* header, HPMBlockID blockID) = 0;

        virtual void Close () = 0;
        
        template<typename T>
        RefCountedPtr<ISMNodeDataStore<T>> GetNodeDataStore() = 0;
                            
    };
#endif