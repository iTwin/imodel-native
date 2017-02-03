/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#ifdef VANCOUVER_API
#include "..\ImagePPHeaders.h"
#endif

#include <ImagePP\all\h\HFCPtr.h>
#include <ImagePP\all\h\HPMDataStore.h>

#include "..\Edits\DifferenceSet.h"
#include <TilePublisher\MeshTile.h>


USING_NAMESPACE_BENTLEY_SCALABLEMESH //NEEDS_WORK_SM : all this code here should be in this namespace instead.
USING_NAMESPACE_IMAGEPP
    
enum class SMStoreDataType
    {
    Points = 0,
    TriPtIndices,    
    TriUvIndices, 
    UvCoords, 
    DiffSet, 
    Graph,
    Texture,    
    TextureCompressed,
    LinearFeature,    
    Skirt,     
    ClipDefinition,     
    //Not persisted data type
    Display, //NEEDS_WORK_SM : Replace by displaymesh and displaytexture, Need to be removed.
    //Not persisted data type
    BcDTM,    
    //Composite datatype - allows to treat different data as an atomic pool item.
    PointAndTriPtIndices, 
    Cesium3DTiles,
    MeshParts,
    Metadata,
    //Not persisted data type
    DisplayMesh,
    //Not persisted data type
    DisplayTexture,
    Coverage,
    Unknown, 
    };


class IClipDefinitionExtOps : public RefCountedBase
    {
    public : 

        virtual void GetMetadata(uint64_t id, double& importance, int& nDimensions) = 0;

        virtual void SetMetadata(uint64_t id, double importance, int nDimensions) = 0;

        virtual void GetAllIDs(bvector<uint64_t>& allIds) = 0;

        virtual void SetAutoCommit(bool autoCommit) = 0;

        virtual void GetAllPolys(bvector<bvector<DPoint3d>>& polys) = 0;
        
    };

typedef RefCountedPtr<IClipDefinitionExtOps> IClipDefinitionExtOpsPtr;


template <class DataType> class ISMNodeDataStore : public RefCountedBase
    {    
    public : 
                
        typedef RefCountedPtr<ISMNodeDataStore<DataType>> Ptr;                               

        ISMNodeDataStore()
            {
            }    

        ~ISMNodeDataStore()
            {
            }        

        virtual HPMBlockID StoreBlock(DataType* DataTypeArray, size_t countData, HPMBlockID blockID) = 0;

        //Valid only for store handling only one type of data.
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const = 0;

        //Valid only for store handling only multiple type of data.
        virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const = 0;

        //Valid only for store handling only one type of data.
        virtual void   ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) = 0;

        //Valid only for store handling only multiple type of data.
        virtual void   ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) = 0;

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
        virtual size_t LoadCompressedBlock(bvector<uint8_t>& DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            HASSERT(false); // Not implemented;
            return 0;
            }   

        virtual bool GetClipDefinitionExtOps(IClipDefinitionExtOpsPtr& clipDefinitionExOpsPtr)
            {
            HASSERT(!"Unexpected call");
            return false;
            }                   
    };


struct PointAndTriPtIndicesBase
    {        
    DPoint3d* m_pointData;
    int32_t*  m_indicesData;
    };

struct Cesium3DTilesBase
    {
    DPoint3d* m_pointData = nullptr;
    int32_t*  m_indicesData = nullptr;
    int32_t*  m_uvIndicesData = nullptr;
    DPoint2d* m_uvData = nullptr;
    Byte*     m_textureData = nullptr;
    };

typedef RefCountedPtr<ISMNodeDataStore<DPoint3d>>      ISM3DPtDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<DifferenceSet>> ISDiffSetDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<int32_t>>       ISMInt32DataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<MTGGraph>>      ISMMTGGraphDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<Byte>>          ISMTextureDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<DPoint2d>>      ISMUVCoordsDataStorePtr;


//NEEDS_WORK_SM : Put that and all multiple item demo code in define 
typedef RefCountedPtr<ISMNodeDataStore<PointAndTriPtIndicesBase>> ISMPointTriPtIndDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<bvector<Byte>>>  ISMTileMeshDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<Cesium3DTilesBase>>  ISMCesium3DTilesDataStorePtr;



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
         Set the path of the files created for a given project (e.g. : dgndb file). 
        -----------------------------------------------------------------------------*/
        virtual bool SetProjectFilesPath(BeFileName& projectFilesPath, bool inCreation) = 0;

        /**----------------------------------------------------------------------------
         Get the next node ID available.
        -----------------------------------------------------------------------------*/
        virtual uint64_t GetNextID() const = 0;

        virtual void Close () = 0;
                                        
        virtual bool GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType) = 0;

        virtual bool GetNodeDataStore(ISDiffSetDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;

        virtual bool GetNodeDataStore(ISMInt32DataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType) = 0;                

        virtual bool GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;
        
        virtual bool GetNodeDataStore(ISMTextureDataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType = SMStoreDataType::Texture) = 0;
        
        virtual bool GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType = SMStoreDataType::UvCoords) = 0;

        //Multi-items loading store
        virtual bool GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;

        virtual bool GetNodeDataStore(ISMTileMeshDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;

        virtual bool GetNodeDataStore(ISMCesium3DTilesDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;
    };