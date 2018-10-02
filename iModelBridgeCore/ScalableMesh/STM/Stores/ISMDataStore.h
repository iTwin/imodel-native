/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include "../ImagePPHeaders.h"
#endif

#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HPMDataStore.h>

#include "../Edits/DifferenceSet.h"

#include <ScalableMesh/IClipDefinitionDataProvider.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH //NEEDS_WORK_SM : all this code here should be in this namespace instead.
USING_NAMESPACE_IMAGEPP
    
#ifndef VANCOUVER_API
class SMSQLiteFile;

typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<SMSQLiteFile> SMSQLiteFilePtr;
#endif

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
    CoveragePolygon,
    CoverageName,
    //Not persisted data type
    Mesh3D,
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

        virtual void GetIsClipActive(uint64_t id, bool& isActive) = 0;

        virtual void GetClipType(uint64_t id, SMNonDestructiveClipType& type) = 0;

        virtual void SetClipOnOrOff(uint64_t id, bool isActive) = 0;

        virtual void GetAllCoverageIDs(bvector<uint64_t>& allIds) = 0;

        virtual void StoreClipWithParameters(const bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) = 0;

        virtual void LoadClipWithParameters(bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive) = 0;

        virtual void StoreClipWithParameters(const ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) = 0;

        virtual void LoadClipWithParameters(ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive) = 0;


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
typedef RefCountedPtr<ISMNodeDataStore<Utf8String>>    ISMCoverageNameDataStorePtr;


//NEEDS_WORK_SM : Put that and all multiple item demo code in define 
typedef RefCountedPtr<ISMNodeDataStore<PointAndTriPtIndicesBase>> ISMPointTriPtIndDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<bvector<Byte>>>  ISMTileMeshDataStorePtr;
typedef RefCountedPtr<ISMNodeDataStore<Cesium3DTilesBase>>  ISMCesium3DTilesDataStorePtr;


template <class MasterHeaderType, class NodeHeaderType>  class ISMDataStore : public RefCountedBase
    {

    private:

#ifndef VANCOUVER_API
        virtual uint32_t _GetExcessiveRefCountThreshold() const override { return numeric_limits<uint32_t>::max(); } 
#endif

    protected:

        bool IsSisterFileType(SMStoreDataType dataType)
            {
            return (dataType == SMStoreDataType::DiffSet ||
                    dataType == SMStoreDataType::Graph ||
                    dataType == SMStoreDataType::LinearFeature ||
                    dataType == SMStoreDataType::ClipDefinition ||
                    dataType == SMStoreDataType::CoveragePolygon ||
                    dataType == SMStoreDataType::CoverageName ||
                    dataType == SMStoreDataType::Skirt);
            }

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
        virtual bool SetProjectFilesPath(BeFileName& projectFilesPath) = 0;

        /**----------------------------------------------------------------------------
         Determine if the files created for a given project (e.g. : dgndb file) are 
         written in the OS temp folder or in the path set by SetProjectFilesPath. 
        -----------------------------------------------------------------------------*/
        virtual bool SetUseTempPath(bool useTempPath) = 0;

        /**----------------------------------------------------------------------------
        Save the content of the project files.
        -----------------------------------------------------------------------------*/
        virtual void SaveProjectFiles() = 0;

		/**----------------------------------------------------------------------------
		Compact (vacuum) the content of the project files, if possible.
		-----------------------------------------------------------------------------*/
		virtual void CompactProjectFiles() = 0;

        /**----------------------------------------------------------------------------
        Preload data that will be required
        -----------------------------------------------------------------------------*/
        virtual void PreloadData(const bvector<DRange3d>& tileRanges) = 0;        
     
        /**----------------------------------------------------------------------------
        Preload data that will be required
        -----------------------------------------------------------------------------*/
        virtual void CancelPreloadData() = 0;

        /**----------------------------------------------------------------------------
        Compute raster tiles
        -----------------------------------------------------------------------------*/
        virtual void ComputeRasterTiles(bvector<SMRasterTile>& rasterTiles, const bvector<DRange3d>& tileRanges) = 0;

        /**----------------------------------------------------------------------------
        Is texture data is available. Should always return true but for streamed 
        texture source (e.g : BingMap)
        -----------------------------------------------------------------------------*/
        virtual bool IsTextureAvailable() = 0;

        /**----------------------------------------------------------------------------
        Register a scalable mesh to the store (useful for streaming type store).
        -----------------------------------------------------------------------------*/
        virtual void Register(const uint64_t& smID) {};

        /**----------------------------------------------------------------------------
        Unregister a scalable mesh from the store (useful for streaming type store).
        -----------------------------------------------------------------------------*/
        virtual void Unregister(const uint64_t& smID) {};

		virtual bool DoesClipFileExist() const = 0;

        virtual void EraseClipFile() const = 0;

		/**----------------------------------------------------------------------------
		Accept a way for the application to register its own callback for the clip polygons, instead of using file storage.
		-----------------------------------------------------------------------------*/
		virtual void SetClipDefinitionsProvider(const IClipDefinitionDataProviderPtr& provider) {};

		virtual void WriteClipDataToProjectFilePath() = 0;

        /**----------------------------------------------------------------------------
         Get the next node ID available.
        -----------------------------------------------------------------------------*/
        virtual uint64_t GetNextID() const = 0;

        virtual void Close () = 0;
                                        
        virtual bool GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType) = 0;        
        
        virtual bool GetNodeDataStore(ISMInt32DataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType) = 0;                

        virtual bool GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;
        
        virtual bool GetNodeDataStore(ISMTextureDataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType = SMStoreDataType::Texture) = 0;
        
        virtual bool GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType = SMStoreDataType::UvCoords) = 0;

        virtual bool GetSisterNodeDataStore(ISDiffSetDataStorePtr& dataStore, NodeHeaderType* nodeHeader, bool createSisterFile) = 0;
        
        virtual bool GetSisterNodeDataStore(ISMCoverageNameDataStorePtr& dataStore, NodeHeaderType* nodeHeader, bool createSisterFile) = 0;

        virtual bool GetSisterNodeDataStore(ISM3DPtDataStorePtr& dataStore, NodeHeaderType* nodeHeader, SMStoreDataType dataType, bool createSisterFile) = 0;
        
        
        //Multi-items loading store
        virtual bool GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;

        virtual bool GetNodeDataStore(ISMTileMeshDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;

        virtual bool GetNodeDataStore(ISMCesium3DTilesDataStorePtr& dataStore, NodeHeaderType* nodeHeader) = 0;       

        virtual SMSQLiteFilePtr GetSQLiteFilePtr(SMStoreDataType dataType) { return nullptr; }
    };