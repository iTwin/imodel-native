#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite\BeSQLite.h>
#include <ScalableMesh/import/DataSQLite.h>
#include "ScalableMeshDb.h"

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
    int  m_SubNodeNoSplitID;
    bool        m_filtered;    
    size_t      m_nbFaceIndexes;
    size_t        m_nbUvIndexes;
    size_t        m_nbTextures;
    int  m_graphID;
    std::vector<int>  m_textureID;
    int  m_uvID;
    std::vector<int>  m_ptsIndiceID;
    std::vector<int>  m_uvsIndicesID;
    size_t      m_numberOfMeshComponents;
    int*        m_meshComponents;
    size_t m_nodeCount;


    std::vector<int> m_apNeighborNodeID[26];

    std::vector<int> m_clipSetsID;

    SQLiteNodeHeader()
        {
        m_parentNodeID = SQLiteNodeHeader::NO_NODEID;
        m_SubNodeNoSplitID = SQLiteNodeHeader::NO_NODEID;
        m_uvID = SQLiteNodeHeader::NO_NODEID;
        m_graphID = SQLiteNodeHeader::NO_NODEID;
        }
    };


struct SQLiteIndexHeader
    {
    size_t                  m_SplitTreshold;                // Holds the split treshold
    DRange3d                m_MaxExtent;                    // Indicates the maximum extent if the spatial index is extent limited
    bool                    m_HasMaxExtent;                 // indicated if the index is extent limited.

    bool                    m_balanced;                     // Control variable that indicates if the tree must be balanced
    bool                    m_textured;
    bool                    m_singleFile;
    size_t                  m_numberOfSubNodesOnSplit;      // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.
    size_t                  m_depth;                        // Cached (maximum) number of levels in the tree.
    int              m_rootNodeBlockID;
    size_t                  m_terrainDepth;                 //Maximum number of LODs for terrain(mesh) data, set at generation time
    bool                    m_isTerrain;                    //can the Scalable Mesh be used as a terrain, e.g. for design applications
    };

struct SQLiteSourcesData
{
    WString m_path;
};

class SMSQLiteFile : public BENTLEY_NAMESPACE_NAME::RefCountedBase
{
public:
    SMSQLiteFile();
    ~SMSQLiteFile();

    bool Open(BENTLEY_NAMESPACE_NAME::Utf8CP filename, bool openReadOnly = true);
    bool Open(BENTLEY_NAMESPACE_NAME::WString& filename, bool openReadOnly = true);
    bool Create(BENTLEY_NAMESPACE_NAME::Utf8CP filename);
    bool Create(BENTLEY_NAMESPACE_NAME::WString& filename);
    bool Close();
    bool IsOpen() { return m_database->IsDbOpen(); }

    void CommitAll();

    static SMSQLiteFilePtr Open(const WString& filename, bool openReadOnly, StatusInt& status);
    void SetSource();
    bool SetWkt(WCharCP extendedWkt);
    bool HasWkt();
    bool AddSource();
    bool SaveSource(SourcesDataSQLite& sourcesData);
    bool HasSources();
    bool HasMasterHeader();
    bool HasPoints();
    bool SetMasterHeader(const SQLiteIndexHeader& newHeader);
    bool SetNodeHeader(const SQLiteNodeHeader& newNodeHeader);
    bool SetSingleFile(bool isSingleFile);

    bool GetSource();
    bool GetGCS();
    bool GetWkt(WString& wktStr);
    bool GetMasterHeader(SQLiteIndexHeader& header);
    bool GetNodeHeader(SQLiteNodeHeader& nodeHeader);
    bool GetAccessMode() { return m_database->IsReadonly(); }
    bool IsSingleFile();

    //uint64_t GetLastInsertRowId() { return m_database->GetLastInsertRowId(); }
    uint64_t GetLastNodeId();


    void GetPoints(int64_t nodeID, bvector<uint8_t>& pts, size_t& uncompressedSize);
    void GetIndices(int64_t nodeID, bvector<uint8_t>& indices, size_t& uncompressedSize);
    void GetPointsAndIndices(int64_t nodeID, bvector<uint8_t>& pts, size_t& uncompressedSizePts, bvector<uint8_t>& indices, size_t& uncompressedSizeIndices);    
    void GetUVs(int64_t nodeID, bvector<uint8_t>& uvCoords, size_t& uncompressedSize);
    bool LoadSources(SourcesDataSQLite& sourcesData);
    void GetUVIndices(int64_t nodeID, bvector<uint8_t>& uvIndices, size_t& uncompressedSize);
    void GetTexture(int64_t nodeID, bvector<uint8_t>& texture, size_t& uncompressedSize);
    void GetGraph(int64_t nodeID, bvector<uint8_t>& graph, size_t& uncompressedSize);
    void GetFeature(int64_t featureID, bvector<uint8_t>& featureData, size_t& uncompressedSize);
    void GetClipPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize);
    void GetSkirtPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize);
    void GetDiffSet(int64_t diffsetID, bvector<uint8_t>& diffsetData, size_t& uncompressedSize);
#ifdef WIP_MESH_IMPORT
    void GetMeshParts(int64_t nodeID, bvector<uint8_t>& data, size_t& uncompressedSize);
    void GetMetadata(int64_t nodeID, bvector<uint8_t>& metadata, size_t& uncompressedSize);
#endif


    void StorePoints(int64_t& nodeID, const bvector<uint8_t>& pts, size_t uncompressedSize);
    void StoreIndices(int64_t& nodeID, const bvector<uint8_t>& indices, size_t uncompressedSize);
    void StoreUVs(int64_t& nodeID, const bvector<uint8_t>& uvCoords, size_t uncompressedSize);
    void StoreUVIndices(int64_t& nodeID, const bvector<uint8_t>& uvIndices, size_t uncompressedSize);
    void StoreTexture(int64_t& nodeID, const bvector<uint8_t>& texture, size_t uncompressedSize);
    void StoreGraph(int64_t& nodeID, const bvector<uint8_t>& graph, size_t uncompressedSize);
    void StoreFeature(int64_t& featureID, const bvector<uint8_t>& featureData, size_t uncompressedSize);
    void StoreClipPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize);
    void SetClipPolygonMetadata(uint64_t& clipID, double importance, int nDimensions);
    void GetClipPolygonMetadata(uint64_t clipID, double& importance, int& nDimensions);
    void StoreSkirtPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize);
    void StoreDiffSet(int64_t& diffsetID, const bvector<uint8_t>& diffsetData, size_t uncompressedSize);
#ifdef WIP_MESH_IMPORT
    void StoreMeshParts(int64_t& nodeID, const bvector<uint8_t>& data, size_t uncompressedSize);
    void StoreMetadata(int64_t& nodeID, const bvector<uint8_t>& metadata, size_t uncompressedSize);
#endif


    size_t GetNumberOfPoints(int64_t nodeID);
    size_t GetNumberOfIndices(int64_t nodeID);
    size_t GetNumberOfUVs(int64_t nodeID);
    size_t GetNumberOfUVIndices(int64_t nodeID);
    size_t GetTextureByteCount(int64_t nodeID);
    size_t GetNumberOfFeaturePoints(int64_t featureID);
    size_t GetClipPolygonByteCount(int64_t clipID);
    size_t GetSkirtPolygonByteCount(int64_t skirtID);
#ifdef WIP_MESH_IMPORT
    size_t GetNumberOfMeshParts(int64_t nodeId);
    size_t GetNumberOfMetadataChars(int64_t nodeId);
#endif

    void GetAllClipIDs(bvector<uint64_t>& allIds); 

    bool GetFileName(Utf8String& fileName) const; 
    
    bool m_autocommit = true;
private:
    ScalableMeshDb* m_database;

    // string table name
    const std::string m_sMasterHeaderTable = "SMMasterHeader";
    const std::string m_sNodeHeaderTable = "SMNodeHeader";
    const std::string m_sGraphTable = "SMGraph";
    const std::string m_sSourceTable = "SMSources";
    std::mutex dbLock;


    bool UpdateDatabase();

};