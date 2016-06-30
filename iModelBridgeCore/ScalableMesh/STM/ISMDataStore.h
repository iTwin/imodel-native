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

        virtual HPMBlockID StoreNewPointBlock (DataType* DataTypeArray, size_t countData) = 0;

        virtual HPMBlockID StorePointBlock (DataType* DataTypeArray, size_t countData, HPMBlockID blockID) = 0;

        virtual size_t GetPointBlockDataCount (HPMBlockID blockID) const = 0;

        virtual size_t LoadPointBlock (DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID) = 0;

        virtual bool DestroyPointBlock (HPMBlockID blockID) = 0;

       
        /**----------------------------------------------------------------------------
        Stores an existing block ... if the current block size is not sufficient to hold the data, then
        a new block can be allocated elsewhere. The new or previous block ID is returned. No compression
        is performed prior to store the block and the block is assumed to be already compressed.
        -----------------------------------------------------------------------------*/
        virtual HPMBlockID StoreCompressedTextureBlock(DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
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
        virtual size_t LoadCompressedTextureBlock(bvector<DataType>& DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            HASSERT(false); // Not implemented;
            return 0;
            }

        /**----------------------------------------------------------------------------
        This method serializes the node for streaming.

        @param
        -----------------------------------------------------------------------------*/
        //NEEDS_WORK_SM_STORE
        virtual void   SerializeHeaderToBinary(const NodeHeaderType* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize, uint32_t pi_pMaxDataSize = 3000) const
            {
            HASSERT(false); // Not implemented;
            }       
    };