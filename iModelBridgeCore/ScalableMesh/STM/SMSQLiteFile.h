#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite/BeSQLite.h>
#include <ScalableMesh/Import/DataSQLite.h>
#include "ScalableMeshDb.h"
#include <json/json.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH

using namespace std;

class SMSQLiteFile;

typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<SMSQLiteFile> SMSQLiteFilePtr;

struct SQLiteNodeHeader
    {
    static const int NO_NODEID = -1;
    int m_nodeID;
    size_t      m_SplitTreshold;            // Holds the split treshold
    DRange3d      m_nodeExtent;               // The extent of the node (this extent is always defined immediately at the creation of the node)
    DRange3d      m_contentExtent;            // The extent of the content of the node ... this is not the same as the extent of the node.
    bool        m_contentExtentDefined;     // Indicates if the content extent has been initialised. Even if node is empty the content extent
    // can have been initialized since this extent includes sub-nodes content.
    size_t      m_level;                    // The level depth of the node in the index tree.
    size_t      m_numberOfSubNodesOnSplit;  // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.

    bool        m_totalCountDefined;         // Indicates if the total count of objects in node and subnode is up to date
    uint64_t      m_totalCount;                // This value indicates the total number of points in node all recursively all sub-nodes.
    bool        m_arePoints3d;               //Indicates if the node contains 3D points or 2.5D points only. 
    bool        m_isTextured;               // Indicates if the node contains Texture or not
    int  m_parentNodeID; //Required when loading 
    std::vector<int>  m_apSubNodeID;
    int  m_SubNodeNoSplitID;//not used?
    bool        m_filtered;    
    size_t      m_nbFaceIndexes;
    size_t        m_nbUvIndexes; //not used, we do metadata instead
    size_t        m_nbTextures; //not used, we do metadata instead
    int  m_graphID; //not used
    int  m_textureID; //ID of texture, if -1 means same as NodeID and texture cannot be shared with other nodes
    int  m_uvID; //not used
    std::vector<int>  m_ptsIndiceID; //not used same as id
    std::vector<int>  m_uvsIndicesID; //not used same as id
    size_t      m_numberOfMeshComponents;
    int*        m_meshComponents;
    size_t m_nodeCount;

    float m_geometricResolution; // 0 if not set
    float m_textureResolution; // 0 if not set


    std::vector<int> m_apNeighborNodeID[26];

    std::vector<int> m_clipSetsID; //Not used, I believe we just use the nodeID.

    SQLiteNodeHeader()
        {
        m_parentNodeID = SQLiteNodeHeader::NO_NODEID;
        m_SubNodeNoSplitID = SQLiteNodeHeader::NO_NODEID;
        m_uvID = SQLiteNodeHeader::NO_NODEID;
        m_graphID = SQLiteNodeHeader::NO_NODEID;
        m_geometricResolution = m_textureResolution = 0.0;
        }
    };



struct SQLiteIndexHeader
    {
    size_t                  m_SplitTreshold;                // Holds the split treshold
    DRange3d                m_MaxExtent;                    // Indicates the maximum extent if the spatial index is extent limited
    bool                    m_HasMaxExtent;                 // indicated if the index is extent limited.

    bool                    m_balanced;                     // Control variable that indicates if the tree must be balanced
    SMTextureType           m_textured;
    bool                    m_singleFile;
    size_t                  m_numberOfSubNodesOnSplit;      // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.
    size_t                  m_depth;                        // Cached (maximum) number of levels in the tree.
    int                     m_rootNodeBlockID;
    size_t                  m_terrainDepth;                 //Maximum number of LODs for terrain(mesh) data, set at generation time
    bool                    m_isTerrain;                    //can the Scalable Mesh be used as a terrain, e.g. for design applications
    float                   m_resolution;                   //Maximum resolution (of either texture or geometry) for the dataset. This may not be available in all areas. 0 if not computed.
    };

struct SQLiteSourcesData
{
    WString m_path;
};

class SMSQLiteFile : public BENTLEY_NAMESPACE_NAME::RefCountedBase
{
public:

    friend class ScalableMeshDb;

    SMSQLiteFile();
    virtual ~SMSQLiteFile();

    bool Open(BENTLEY_NAMESPACE_NAME::Utf8CP filename, bool openReadOnly = true, bool openShareable = false, SQLDatabaseType type = SQLDatabaseType::SM_MAIN_DB_FILE);
    bool Open(BENTLEY_NAMESPACE_NAME::WString& filename, bool openReadOnly = true, bool openShareable = false, SQLDatabaseType type = SQLDatabaseType::SM_MAIN_DB_FILE, bool createSisterIfMissing = false);
    bool Create(BENTLEY_NAMESPACE_NAME::Utf8CP filename, SQLDatabaseType type = SQLDatabaseType::SM_MAIN_DB_FILE);
    bool Create(BENTLEY_NAMESPACE_NAME::WString& filename, SQLDatabaseType type = SQLDatabaseType::SM_MAIN_DB_FILE);
    bool Close();
    bool IsOpen() { return m_database->IsDbOpen(); }
    bool IsReadOnly() { return m_database->IsReadonly(); }
    bool IsShared() { return m_isShared; }
    ScalableMeshDb* GetDb() { return m_database; }
    BENTLEY_SM_EXPORT void Save();

    static SMSQLiteFilePtr Open(const WString& filename, bool openReadOnly, StatusInt& status, bool openShareable = false, SQLDatabaseType type = SQLDatabaseType::SM_MAIN_DB_FILE, bool createSisterIfMissing = false);
    void SetSource();
    bool SetWkt(WCharCP extendedWkt);
    bool HasWkt();
    bool AddSource();
    BENTLEY_SM_EXPORT bool SaveSource(SourcesDataSQLite& sourcesData);
    BENTLEY_SM_EXPORT bool HasSources();
    bool HasMasterHeader();
    bool HasPoints();
    bool SetMasterHeader(const SQLiteIndexHeader& newHeader);
    bool SetNodeHeader(const SQLiteNodeHeader& newNodeHeader);
    BENTLEY_SM_EXPORT bool SetSingleFile(bool isSingleFile);
    bool SetProperties(const Json::Value& properties);

    bool GetSource();
    bool GetGCS();
    BENTLEY_SM_EXPORT bool GetWkt(WString& wktStr);
    bool GetMasterHeader(SQLiteIndexHeader& header);
    bool GetNodeHeader(SQLiteNodeHeader& nodeHeader);
    bool GetAccessMode() { return m_database->IsReadonly(); }
    BENTLEY_SM_EXPORT bool IsSingleFile();
    bool GetProperties(Json::Value& properties);

    //uint64_t GetLastInsertRowId() { return m_database->GetLastInsertRowId(); }
    uint64_t GetLastNodeId();


    void GetPoints(int64_t nodeID, bvector<uint8_t>& pts, size_t& uncompressedSize);
    void GetIndices(int64_t nodeID, bvector<uint8_t>& indices, size_t& uncompressedSize);
    void GetPointsAndIndices(int64_t nodeID, bvector<uint8_t>& pts, size_t& uncompressedSizePts, bvector<uint8_t>& indices, size_t& uncompressedSizeIndices);    
    void GetUVs(int64_t nodeID, bvector<uint8_t>& uvCoords, size_t& uncompressedSize);
    BENTLEY_SM_EXPORT bool LoadSources(SourcesDataSQLite& sourcesData);
    void GetUVIndices(int64_t nodeID, bvector<uint8_t>& uvIndices, size_t& uncompressedSize);
    void GetTexture(int64_t nodeID, bvector<uint8_t>& texture, size_t& uncompressedSize);

#ifdef WIP_MESH_IMPORT
    void GetMeshParts(int64_t nodeID, bvector<uint8_t>& data, size_t& uncompressedSize);
    void GetMetadata(int64_t nodeID, bvector<uint8_t>& metadata, size_t& uncompressedSize);
#endif


    void StorePoints(int64_t& nodeID, const bvector<uint8_t>& pts, size_t uncompressedSize);
    void StoreIndices(int64_t& nodeID, const bvector<uint8_t>& indices, size_t uncompressedSize);
    void StoreUVs(int64_t& nodeID, const bvector<uint8_t>& uvCoords, size_t uncompressedSize);
    void StoreUVIndices(int64_t& nodeID, const bvector<uint8_t>& uvIndices, size_t uncompressedSize);
    void StoreTexture(int64_t& nodeID, const bvector<uint8_t>& texture, size_t uncompressedSize);

#ifdef WIP_MESH_IMPORT
    void StoreMeshParts(int64_t& nodeID, const bvector<uint8_t>& data, size_t uncompressedSize);
    void StoreMetadata(int64_t& nodeID, const bvector<uint8_t>& metadata, size_t uncompressedSize);
#endif


    size_t GetNumberOfPoints(int64_t nodeID);
    size_t GetNumberOfIndices(int64_t nodeID);
    size_t GetNumberOfUVs(int64_t nodeID);
    size_t GetNumberOfUVIndices(int64_t nodeID);
    size_t GetTextureByteCount(int64_t nodeID);
    size_t GetTextureCompressedByteCount(int64_t nodeID);

#ifdef WIP_MESH_IMPORT
    size_t CountTextures();
    size_t GetNumberOfMeshParts(int64_t nodeId);
    size_t GetNumberOfMetadataChars(int64_t nodeId);
#endif

    virtual void GetAllClipIDs(bvector<uint64_t>& allIds) { assert(false); };

    bool GetFileName(Utf8String& fileName) const; 

    virtual void GetGraph(int64_t nodeID, bvector<uint8_t>& graph, size_t& uncompressedSize) { assert(false); }
    virtual void GetFeature(int64_t featureID, bvector<uint8_t>& featureData, size_t& uncompressedSize) { assert(false); }

    virtual void StoreGraph(int64_t& nodeID, const bvector<uint8_t>& graph, size_t uncompressedSize) { assert(false); }
    virtual void StoreFeature(int64_t& featureID, const bvector<uint8_t>& featureData, size_t uncompressedSize) { assert(false); }

    virtual size_t GetNumberOfFeaturePoints(int64_t featureID) { assert(false); return 0; }

    virtual void StoreClipPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize, SMClipGeometryType geom = SMClipGeometryType::Polygon, SMNonDestructiveClipType type = SMNonDestructiveClipType::Mask, bool isActive = true) { assert(false); }
    virtual void SetClipPolygonMetadata(uint64_t& clipID, double importance, int nDimensions) { assert(false); }
    virtual void GetClipPolygonMetadata(uint64_t clipID, double& importance, int& nDimensions) { assert(false); }
    virtual void StoreSkirtPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize) { assert(false); }

    virtual void GetClipPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive) { assert(false); }
    virtual void GetSkirtPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize) { assert(false); }

    virtual void SetClipOnOrOff(uint64_t id, bool isActive) { assert(false); }

    virtual void GetIsClipActive(uint64_t id, bool& isActive) { assert(false); }

    virtual void GetClipType(uint64_t id, SMNonDestructiveClipType& type) { assert(false); }

    virtual size_t GetClipPolygonByteCount(int64_t clipID) { assert(false); return 0; }
    virtual size_t GetSkirtPolygonByteCount(int64_t skirtID) { assert(false); return 0; }
    
    virtual void GetCoverageName(int64_t coverageID, Utf8String* name, size_t& uncompressedSize) { assert(false); }
    virtual size_t GetCoverageNameByteCount(int64_t coverageID) { assert(false); return 0; }
    virtual void GetCoveragePolygon(int64_t coverageID, bvector<uint8_t>& coverageData, size_t& uncompressedSize) { assert(false); }
    virtual void StoreCoveragePolygon(int64_t& coverageID, const bvector<uint8_t>& coverageData, size_t uncompressedSize) { assert(false); }
    virtual void StoreCoverageName(int64_t& coverageID, Utf8String& coverageName, size_t uncompressedSize) { assert(false); }
    virtual size_t GetCoveragePolygonByteCount(int64_t coverageID) { assert(false); return 0; }

    virtual void GetAllCoverageIDs(bvector<uint64_t>& ids) { assert(false); }

    virtual void GetAllPolys(bvector<bvector<uint8_t>>& polys, bvector<size_t>& sizes) { assert(false); }

    virtual void GetDiffSet(int64_t diffsetID, bvector<uint8_t>& diffsetData, size_t& uncompressedSize) { assert(false); }
    virtual void StoreDiffSet(int64_t& diffsetID, const bvector<uint8_t>& diffsetData, size_t uncompressedSize) { assert(false); }
    virtual void DeleteDiffSet(int64_t diffsetID) { assert(false); }

    virtual void DeleteCoveragePolygon(int64_t coverageID) { assert(false); }
    virtual void DeleteClipPolygon(int64_t clipID) { assert(false); }
    virtual void DeleteSkirtPolygon(int64_t clipID) { assert(false); }

	void Compact();

    bool m_autocommit = true;    

    static const BESQL_VERSION_STRUCT CURRENT_VERSION;
protected:
    ScalableMeshDb* m_database;
    std::mutex dbLock;

    virtual BESQL_VERSION_STRUCT GetCurrentVersion()
        {
        return SMSQLiteFile::CURRENT_VERSION;
        }

    virtual DbResult CreateTables();

    virtual size_t GetNumberOfReleasedSchemas();
    virtual const BESQL_VERSION_STRUCT* GetListOfReleasedVersions();
    virtual double* GetExpectedTimesForUpdateFunctions();
    virtual std::function<void(BeSQLite::Db*)>* GetFunctionsForAutomaticUpdate();

private:

#ifndef VANCOUVER_API
    //Avoid assert added on Bim02    
    virtual uint32_t _GetExcessiveRefCountThreshold() const override { return std::numeric_limits<uint32_t>::max(); }
#endif

    bool m_isShared;

    // string table name
    const std::string m_sMasterHeaderTable = "SMMasterHeader";
    const std::string m_sNodeHeaderTable = "SMNodeHeader";
    const std::string m_sGraphTable = "SMGraph";
    const std::string m_sSourceTable = "SMSources";


    DbResult UpdateDatabase();

};