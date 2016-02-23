#include <ScalableMeshPCH.h>
#include "SMSQLiteFile.h"

#ifdef VANCOUVER_API
#define WSTRING_FROM_CSTR(cstr) WString(cstr)
#else
#define WSTRING_FROM_CSTR(cstr) WString(cstr, BentleyCharEncoding::Utf8)
#define MAKE_COPY_NO Statement::MakeCopy::No
#define MAKE_COPY_YES Statement::MakeCopy::Yes
#define GET_VALUE_STR(stmt, id) stmt->GetValueText(id)
#define BIND_VALUE_STR(stmt, id, utf8str, copyval) stmt->BindText(id, utf8str, copyval)
#endif


SMSQLiteFile::SMSQLiteFile()
{
    m_database = nullptr;
}

SMSQLiteFile::~SMSQLiteFile()
{
    if (m_database != nullptr)
        delete m_database;
    m_database = nullptr;
}

bool SMSQLiteFile::Close()
{
    //DbResult result;
    /*result =*/ m_database->CloseDb();
    
    //assert(result == BE_SQLITE_OK);
    return true;// result == BE_SQLITE_OK;
}
bool SMSQLiteFile::Open(BENTLEY_NAMESPACE_NAME::Utf8CP filename, bool openReadOnly)
{
    if (m_database == nullptr)
        m_database = new BeSQLite::Db();
    DbResult result;
    if (m_database->IsDbOpen())
        m_database->CloseDb();
    result = m_database->OpenBeSQLiteDb(filename, Db::OpenParams(openReadOnly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite));

    //assert(result == BE_SQLITE_OK);
    return result == BE_SQLITE_OK;
}

bool SMSQLiteFile::Open(BENTLEY_NAMESPACE_NAME::WString& filename, bool openReadOnly)
    {
    Utf8String utf8FileName(filename);        
    return Open(utf8FileName.c_str(), openReadOnly);
    }

SMSQLiteFilePtr SMSQLiteFile::Open(const WString& filename, bool openReadOnly, StatusInt& status)
    {
    bool result;
    SMSQLiteFilePtr smSQLiteFile = new SMSQLiteFile();

    Utf8String utf8File(filename);

    result = smSQLiteFile->Open(utf8File.c_str(), openReadOnly);
    // need to check version file ?
    status = result ? 1 : 0;
    return smSQLiteFile;
    }

bool SMSQLiteFile::Create(BENTLEY_NAMESPACE_NAME::Utf8CP filename)
{
    if (m_database == nullptr)
        m_database = new BeSQLite::Db();
    DbResult result;
    result = m_database->CreateNewDb(filename);

    assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMMasterHeader", "MasterHeaderId INTEGER PRIMARY KEY,"
        "Balanced INTEGER,"
        "SplitTreshold INTEGER,"
        "RootNodeId INTEGER,"
        "Depth INTEGER,"
        "IsTextured INTEGER,"
        "SingleFile INTEGER,"
        "TerrainDepth INTEGER,"
        "GCS STRING,"
        "LastModifiedTime INTEGER,"
        "LastSyncTime INTEGER,"
        "CheckTime INTEGER,"
        "SerializedSourceFormatVersion INTEGER,"
        "ContentConfigFormatVersion INTEGER,"
        "ImportSequenceFormatVersion INTEGER,"
        "ImportConfigFormatVersion INTEGER");
        assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMPoint", "NodeId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "PointData BLOB,"
        "IndexData BLOB,"
        "SizePts INTEGER,"
        "SizeIndices INTEGER");
        assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMGraph", "NodeId INTEGER PRIMARY KEY,"
        "Data BLOB,"
        "Size UNSIGNED INT");
        assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMNodeHeader", "NodeId INTEGER PRIMARY KEY,"
        "ParentNodeId INTEGER,"
        "Resolution INTEGER,"
        "Filtered INTEGER,"
        "Extent BLOB,"
        "ContentExtent BLOB,"
        "TotalCount INTEGER,"
        "NodeCount INTEGER,"
        "ArePoints3d INTEGER,"
        "NbFaceIndexes INTEGER,"
        "NumberOfMeshComponents INTEGER,"
        "AllComponent BLOB,"
        "IsTextured INTEGER,"
        "TexID INTEGER,"
        "GraphID INTEGER,"
        "IndiceID INTEGER,"
        "SubNode BLOB,"
        "Neighbor BLOB");
        assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMTexture", "NodeId INTEGER PRIMARY KEY,"
                            "TexData BLOB,"
                            "UVData BLOB,"
                            "SizeTexture INTEGER,"
                            "SizeUVs INTEGER,"
                            "Codec INTEGER,"
                            "NOfChannels INTEGER");
                            assert(result == BE_SQLITE_OK);


   result = m_database->CreateTable("SMUVs", "NodeId INTEGER PRIMARY KEY,"
                            "UVData BLOB,"
                            "SizeUVs INTEGER");
                            assert(result == BE_SQLITE_OK);

    result = m_database->CreateTable("SMFeatures", "FeatureId INTEGER PRIMARY KEY,"
                                        "FeatureData BLOB,"
                                        "Size INTEGER");

    result = m_database->CreateTable("SMClipDefinitions", "PolygonId INTEGER PRIMARY KEY,"
                                     "PolygonData BLOB,"
                                     "Size INTEGER");

    result = m_database->CreateTable("SMDiffsets", "DiffsetId INTEGER PRIMARY KEY,"
                                     "DiffsetData BLOB,"
                                     "Size INTEGER");
    assert(result == BE_SQLITE_OK);


        result = m_database->CreateTable(m_sSourceTable.c_str(), "SourceId INTEGER PRIMARY KEY,"
        "SourceType INTEGER,"
        "DTMSourceID INTEGER,"
        "GroupID INTEGER,"
        "ModelId INTEGER,"
        "ModelName TEXT,"
        "LevelId INTEGER,"
        "LevelName TEXT,"
        "RootToRefPersistentPath TEXT,"
        "ReferenceName TEXT,"
        "ReferenceModelName TEXT,"
        "GCS TEXT,"
        "Flags INTEGER,"
        "TypeFamilyID INTEGER,"
        "OrgCount INTEGER,"
        "Layer INTEGER,"
        "ComponentCount INTEGER,"
        "ConfigComponentID INTEGER,"
        "MonikerType INTEGER,"
        "MonikerString INTEGER,"
        "CommandCount INTEGER,"
        "CommandID BLOB,"
        "TimeLastModified NUMERIC,"
        "SizeExtent INTEGER,"
        "Extent BLOB,"
        "UpToDateState INTEGER,"
        "Time NUMERIC,"
        "IsRepresenting3dData INTEGER,"
        "IsGroundDetection INTEGER,"
        "IsGISData INTEGER,"
        "ElevationProperty TEXT,"
        "LinearFeatureType INTEGER,"
        "PolygonFeatureType INTEGER,"
        "IsGridData INTEGER,"
        "DimensionCount BLOB,"
        "DimensionType BLOB,"
        "DimensionRole BLOB,"
        "DimensionName BLOB");
assert(result == BE_SQLITE_OK);

                            

    m_database->SaveChanges();

    return result == BE_SQLITE_OK;
}

bool SMSQLiteFile::Create(BENTLEY_NAMESPACE_NAME::WString& filename)
    {
    Utf8String utf8FileName(filename);            
    return Create(utf8FileName.c_str());
    }

bool SMSQLiteFile::SetMasterHeader(const SQLiteIndexHeader& newHeader)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmtTest;
    m_database->GetCachedStatement(stmtTest, "SELECT COUNT(MasterHeaderId) FROM SMMasterHeader WHERE MasterHeaderId=?");
    size_t id = 0;
    stmtTest->BindInt64(1, id);
    stmtTest->Step();
    size_t nRows = stmtTest->GetValueInt64(0);
    CachedStatementPtr stmt;
    if (nRows == 0)
    {
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, Balanced, RootNodeId, SplitTreshold, Depth, TerrainDepth, IsTextured) VALUES(?,?,?,?,?,?,?)");
    }
    else
    {
        m_database->GetCachedStatement(stmt, "UPDATE SMMasterHeader SET MasterHeaderId=?, Balanced=?, RootNodeId=?, SplitTreshold=?, Depth=?, TerrainDepth=?, IsTextured=?"
            " WHERE MasterHeaderId=?");
    }
    stmt->BindInt64(1, id);
    stmt->BindInt(2, newHeader.m_balanced ? 1: 0);
    stmt->BindInt64(3, newHeader.m_rootNodeBlockID);
    stmt->BindInt(4, (int)newHeader.m_SplitTreshold);
    stmt->BindInt64(5, newHeader.m_depth);
    stmt->BindInt64(6, newHeader.m_terrainDepth);
    stmt->BindInt(7, newHeader.m_textured ? 1 : 0);
    //stmt->BindInt(7, newHeader.m_singleFile ? 1 : 0);
    if (nRows != 0)
        stmt->BindInt64(8, id);
    DbResult status = stmt->Step();
    assert(status == BE_SQLITE_DONE);
    return status == BE_SQLITE_DONE;
    }

bool SMSQLiteFile::SetNodeHeader(const SQLiteNodeHeader& newNodeHeader)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "REPLACE INTO SMNodeHeader (NodeId, ParentNodeId, Resolution," 
                                  "Filtered, Extent, ContentExtent, TotalCount, ArePoints3d, NbFaceIndexes, "
                                  "NumberOfMeshComponents, AllComponent, GraphID, SubNode,Neighbor, IndiceID, TexID, IsTextured) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    stmt->BindInt64(1, newNodeHeader.m_nodeID);
    stmt->BindInt64(2, newNodeHeader.m_parentNodeID);
    stmt->BindInt64(3, newNodeHeader.m_level);
    stmt->BindInt(4, newNodeHeader.m_filtered ? 1 : 0);
    stmt->BindBlob(5, &newNodeHeader.m_nodeExtent, 6 * sizeof(double), MAKE_COPY_NO);
    stmt->BindBlob(6, &newNodeHeader.m_contentExtent, 6 * sizeof(double), MAKE_COPY_NO);
    stmt->BindInt64(7, newNodeHeader.m_totalCount);
    stmt->BindInt(8, newNodeHeader.m_arePoints3d ? 1 : 0);
    stmt->BindInt64(9, newNodeHeader.m_nbFaceIndexes);
    stmt->BindInt64(10, newNodeHeader.m_numberOfMeshComponents);
    stmt->BindBlob(11, newNodeHeader.m_meshComponents, (int)newNodeHeader.m_numberOfMeshComponents * sizeof(int), MAKE_COPY_NO);
    stmt->BindInt64(12, newNodeHeader.m_graphID);
    stmt->BindBlob(13, (newNodeHeader.m_apSubNodeID.size() > 0) ? &newNodeHeader.m_apSubNodeID[0] : nullptr, (int)newNodeHeader.m_apSubNodeID.size()  * sizeof(int), MAKE_COPY_NO);
    size_t nOfNeighbors = 0;
    for (size_t i = 0; i < 26; ++i)
        nOfNeighbors += newNodeHeader.m_apNeighborNodeID[i].size();
    int* neighbors = new int[26 + nOfNeighbors];
    int offset = 0;
    for (size_t i = 0; i < 26; ++i)
        {
        neighbors[i] = offset;
        memcpy(&neighbors[26 + offset], &newNodeHeader.m_apNeighborNodeID[i][0], newNodeHeader.m_apNeighborNodeID[i].size()*sizeof(int));
        offset += (int)newNodeHeader.m_apNeighborNodeID[i].size();
        }
    stmt->BindBlob(14, (void*)neighbors, (int)nOfNeighbors*sizeof(int) + 26 * sizeof(int), MAKE_COPY_NO);
    int64_t idx = newNodeHeader.m_ptsIndiceID.size() > 0 ? newNodeHeader.m_ptsIndiceID[0] : -1;
    stmt->BindInt64(15, idx);
    if (newNodeHeader.m_textureID.size() > 0)
        stmt->BindInt64(16, newNodeHeader.m_textureID[0]);
    else
        {
        size_t texID = SQLiteNodeHeader::NO_NODEID;
        stmt->BindInt64(16, texID);
        }
    stmt->BindInt(17, newNodeHeader.m_areTextured ? 1 : 0);
    DbResult status = stmt->Step();
    stmt->ClearBindings();
    delete[]neighbors;
    assert(status == BE_SQLITE_DONE);
    return status == BE_SQLITE_DONE;
    }


bool SMSQLiteFile::GetMasterHeader(SQLiteIndexHeader& header)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Balanced, RootNodeId, SplitTreshold, Depth,IsTextured, SingleFile FROM SMMasterHeader WHERE MasterHeaderId = ?");
    size_t id = 0;
    stmt->BindInt64(1, id);
    DbResult status = stmt->Step();
    assert(status == BE_SQLITE_DONE || status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE) return false;
    header.m_balanced = stmt->GetValueInt(0) ? true : false;
    header.m_rootNodeBlockID = stmt->GetValueInt(1);
    header.m_SplitTreshold = stmt->GetValueInt(2);
    header.m_depth = (size_t)stmt->GetValueInt(3);
    header.m_textured = stmt->GetValueInt(4) ? true : false;
    header.m_singleFile = stmt->GetValueInt(5) ? true : false;
    return true;
    }


bool SMSQLiteFile::GetNodeHeader(SQLiteNodeHeader& nodeHeader)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT ParentNodeId, Resolution, Filtered, Extent,"
                                  "ContentExtent, TotalCount, ArePoints3d, NbFaceIndexes, "
                                  "NumberOfMeshComponents, AllComponent, GraphID, SubNode, Neighbor, IndiceId, TexID, IsTextured FROM SMNodeHeader WHERE NodeId=?");
    stmt->BindInt64(1, nodeHeader.m_nodeID);


    DbResult status = stmt->Step();
    assert(status == BE_SQLITE_DONE || status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE) return false;
    nodeHeader.m_parentNodeID = stmt->GetValueInt64(0);
    nodeHeader.m_level = stmt->GetValueInt64(1);
    nodeHeader.m_filtered = stmt->GetValueInt(2) ? true : false;
    const void* extentTmp = stmt->GetValueBlob(3);
    const void* contentExtentTmp = stmt->GetValueBlob(4);
    nodeHeader.m_totalCount = stmt->GetValueInt64(5);
    nodeHeader.m_totalCountDefined = true;
    nodeHeader.m_arePoints3d = stmt->GetValueInt(6) ? true : false;
    nodeHeader.m_nbFaceIndexes = stmt->GetValueInt64(7);
    nodeHeader.m_nbTextures = 0;
    nodeHeader.m_nbUvIndexes = 0;
    nodeHeader.m_numberOfMeshComponents = stmt->GetValueInt64(8);
    const void* allComponentTmp = stmt->GetValueBlob(9);
    nodeHeader.m_graphID = stmt->GetValueInt64(10);

    const void* childrenTmp = stmt->GetValueBlob(11);
    size_t nofNodes = stmt->GetColumnBytes(11) / sizeof(int);
    nodeHeader.m_apSubNodeID.resize(nofNodes);
    memcpy(&nodeHeader.m_apSubNodeID[0], childrenTmp, stmt->GetColumnBytes(11));
    const void* neighborTmp = stmt->GetValueBlob(12);
    if (stmt->GetColumnBytes(12) >= 26*sizeof(int))
        {
        const int* neighbors = (const int*)neighborTmp;
        for (size_t i = 0; i < 26; ++i)
            {
            int nNeighbors = i + 1 < 26 ? (neighbors[i + 1] - neighbors[i]) : (stmt->GetColumnBytes(12)/sizeof(int) - (neighbors[i]+26));
            nodeHeader.m_apNeighborNodeID[i].resize(nNeighbors);
            memcpy(&nodeHeader.m_apNeighborNodeID[i][0], &neighbors[26 + neighbors[i]], nNeighbors*sizeof(int));
            }
        }
    int64_t idx = stmt->GetValueInt64(13);
    if (idx != SQLiteNodeHeader::NO_NODEID)
        {
        nodeHeader.m_ptsIndiceID.resize(1);
        nodeHeader.m_ptsIndiceID[0] = (int)idx;
        }

    memcpy(&nodeHeader.m_nodeExtent, extentTmp, sizeof(double) * 6);
    memcpy(&nodeHeader.m_contentExtent, contentExtentTmp, sizeof(double) * 6);
    nodeHeader.m_meshComponents = new int[nodeHeader.m_numberOfMeshComponents];
    memcpy(nodeHeader.m_meshComponents, allComponentTmp, sizeof(int) * nodeHeader.m_numberOfMeshComponents);
    nodeHeader.m_clipSetsID = std::vector<int>();
    nodeHeader.m_numberOfSubNodesOnSplit = nodeHeader.m_apSubNodeID.size();
    int64_t texIdx = stmt->GetValueInt64(14);
    if (texIdx != SQLiteNodeHeader::NO_NODEID)
        {
        nodeHeader.m_textureID.resize(1);
        nodeHeader.m_textureID[0] = texIdx;
        nodeHeader.m_ptsIndiceID.resize(2);
        nodeHeader.m_ptsIndiceID[1] = (int)idx;
        nodeHeader.m_ptsIndiceID[0] = SQLiteNodeHeader::NO_NODEID;
        nodeHeader.m_nbTextures = 1;
        nodeHeader.m_uvsIndicesID.resize(1);
        nodeHeader.m_uvsIndicesID[0] = texIdx;
        }
    nodeHeader.m_areTextured = stmt->GetValueInt(15) ? true : false;
    stmt->ClearBindings();
    return true;
    }

void SMSQLiteFile::GetPoints(int64_t nodeID, bvector<uint8_t>& pts, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PointData, length(PointData), SizePts FROM SMPoint WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
   // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW)
        {
        uncompressedSize = 0;
        return;
        }
    pts.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    if(pts.size() > 0) memcpy(&pts[0], stmt->GetValueBlob(0), pts.size());
    }

void SMSQLiteFile::GetIndices(int64_t nodeID, bvector<uint8_t>& indices, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT IndexData, length(IndexData), SizeIndices FROM SMPoint WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
   // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    indices.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&indices[0], stmt->GetValueBlob(0), indices.size());
    }


void SMSQLiteFile::GetUVIndices(int64_t nodeID, bvector<uint8_t>& uvs, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT UVData, length(UVData), SizeUVs FROM SMTexture WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    uvs.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&uvs[0], stmt->GetValueBlob(0), uvs.size());
    }

void SMSQLiteFile::GetTexture(int64_t nodeID, bvector<uint8_t>& texture, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT TexData, length(TexData), SizeTexture FROM SMTexture WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    texture.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&texture[0], stmt->GetValueBlob(0), texture.size());
    }

void SMSQLiteFile::GetUVs(int64_t nodeID, bvector<uint8_t>& uvs, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT UVData, length(UVData), SizeUVs FROM SMUVs WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    uvs.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&uvs[0], stmt->GetValueBlob(0), uvs.size());
    }

void SMSQLiteFile::GetGraph(int64_t nodeID, bvector<uint8_t>& graph, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Data, length(Data), Size FROM SMGraph WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    graph.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&graph[0], stmt->GetValueBlob(0), graph.size());
    }

void SMSQLiteFile::GetFeature(int64_t featureID, bvector<uint8_t>& featureData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT FeatureData, length(FeatureData), Size FROM SMFeatures WHERE FeatureId=?");
    stmt->BindInt64(1, featureID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    featureData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&featureData[0], stmt->GetValueBlob(0), featureData.size());
    }

void SMSQLiteFile::GetClipPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PolygonData, length(PolygonData), Size FROM SMClipDefinitions WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    clipData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&clipData[0], stmt->GetValueBlob(0), clipData.size());
    }

void SMSQLiteFile::GetDiffSet(int64_t diffsetID, bvector<uint8_t>& diffsetData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT DiffsetData, length(DiffsetData), Size FROM SMDiffsets WHERE DiffsetId=?");
    stmt->BindInt64(1, diffsetID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    diffsetData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&diffsetData[0], stmt->GetValueBlob(0), diffsetData.size());
    }


void SMSQLiteFile::StorePoints(int64_t& nodeID, const bvector<uint8_t>& pts, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMPoint WHERE NodeId=?");
    stmt3->BindInt64(1, nodeID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (nodeID == SQLiteNodeHeader::NO_NODEID || nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMPoint (NodeId, PointData, IndexData, SizePts, SizeIndices) VALUES(?,?,?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, pts.data(), (int)pts.size(), MAKE_COPY_NO);
        stmt->BindBlob(3, nullptr, 0, MAKE_COPY_NO);
        stmt->BindInt64(4, uncompressedSize);
        stmt->BindInt64(5, 0);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        /*CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);*/
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMPoint SET PointData=?, SizePts=? WHERE NodeId=?");
        stmt->BindBlob(1, &pts[0], (int)pts.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        }
    }

void SMSQLiteFile::StoreIndices(int64_t& nodeID, const bvector<uint8_t>& indices, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMPoint WHERE NodeId=?");
    stmt3->BindInt64(1, nodeID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);

    if (nodeID != SQLiteNodeHeader::NO_NODEID && nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMPoint (NodeId,PointData, IndexData, SizePts, SizeIndices) VALUES(?,?,?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, nullptr, 0, MAKE_COPY_NO);
        stmt->BindBlob(3, &indices[0], (int)indices.size(), MAKE_COPY_NO);
        stmt->BindInt64(4, 0);
        stmt->BindInt64(5, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
       /* CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);*/
        stmt->ClearBindings();
        }
    else if (nodeID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMPoint (PointData, IndexData, SizePts, SizeIndices) VALUES(?,?,?,?)");
        //stmt->BindInt64(1, nodeID);
        stmt->BindBlob(1, nullptr, 0, MAKE_COPY_NO);
        stmt->BindBlob(2, &indices[0], (int)indices.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, 0);
        stmt->BindInt64(4, uncompressedSize);
        DbResult status = stmt->Step();
        if (status != BE_SQLITE_DONE)
            {
            std::string s;
            s = std::to_string(status);
            }
        assert(status == BE_SQLITE_DONE);
        /* CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);*/
        stmt->ClearBindings();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMPoint SET IndexData=?, SizeIndices=? WHERE NodeId=?");
        stmt->BindBlob(1, &indices[0], (int)indices.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }

void SMSQLiteFile::StoreUVIndices(int64_t& nodeID, const bvector<uint8_t>& uvs, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMTexture WHERE NodeId=?");
    stmt3->BindInt64(1, nodeID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (nodeID == SQLiteNodeHeader::NO_NODEID || nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMTexture (NodeId,TexData, UVData, SizeTexture, SizeUVs) VALUES(?,?,?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, nullptr, 0, MAKE_COPY_NO);
        stmt->BindBlob(3, &uvs[0], (int)uvs.size(), MAKE_COPY_NO);
        stmt->BindInt64(4, 0);
        stmt->BindInt64(5, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        /* CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);*/
        stmt->ClearBindings();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMTexture SET UVData=?, SizeUVs=? WHERE NodeId=?");
        stmt->BindBlob(1, &uvs[0], (int)uvs.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }

void SMSQLiteFile::StoreTexture(int64_t& nodeID, const bvector<uint8_t>& texture, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMTexture WHERE NodeId=?");
    stmt3->BindInt64(1, nodeID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (nodeID == SQLiteNodeHeader::NO_NODEID || nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMTexture (NodeId,TexData, UVData, SizeTexture, SizeUVs) VALUES(?,?,?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, &texture[0], (int)texture.size(), MAKE_COPY_NO);
        stmt->BindBlob(3, nullptr, 0, MAKE_COPY_NO);
        stmt->BindInt64(4, uncompressedSize);
        stmt->BindInt64(5, 0);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        /* CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);*/
        stmt->ClearBindings();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMTexture SET TexData=?, SizeTexture=? WHERE NodeId=?");
        stmt->BindBlob(1, &texture[0], (int)texture.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }

void SMSQLiteFile::StoreUVs(int64_t& nodeID, const bvector<uint8_t>& uvs, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMUVs WHERE NodeId=?");
    stmt3->BindInt64(1, nodeID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (nodeID == SQLiteNodeHeader::NO_NODEID || nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMUVs (NodeId,UVData,SizeUVs) VALUES(?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, &uvs[0], (int)uvs.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMUVs SET UvData=?, SizeUVs=? WHERE NodeId=?");
        stmt->BindBlob(1, &uvs[0], (int)uvs.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }

void SMSQLiteFile::StoreGraph(int64_t& nodeID, const bvector<uint8_t>& graph, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMGraph WHERE NodeId=?");
    stmt3->BindInt64(1, nodeID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (nodeID == SQLiteNodeHeader::NO_NODEID || nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMGraph (NodeId,Data,Size) VALUES(?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, &graph[0], (int)graph.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMGraph SET Data=?, Size=? WHERE NodeId=?");
        stmt->BindBlob(1, &graph[0], (int)graph.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }

void SMSQLiteFile::StoreFeature(int64_t& featureID, const bvector<uint8_t>& featureData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    if (featureID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMFeatures (FeatureData,Size) VALUES(?,?)");
        stmt->BindBlob(1, &featureData[0], (int)featureData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        featureID = stmt2->GetValueInt64(0);
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMFeatures SET FeatureData=?, Size=? WHERE FeatureId=?");
        stmt->BindBlob(1, &featureData[0], (int)featureData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, featureID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }


void SMSQLiteFile::StoreClipPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(PolygonId) FROM SMClipDefinitions WHERE PolygonId=?");
    stmt3->BindInt64(1, clipID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (clipID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMClipDefinitions (PolygonData,Size) VALUES(?,?)");
        stmt->BindBlob(1, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        clipID = stmt2->GetValueInt64(0);
        m_database->SaveChanges();
        }
    else if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMClipDefinitions (PolygonId, PolygonData,Size) VALUES(?, ?,?)");
        stmt->BindInt64(1, clipID);
        stmt->BindBlob(2, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        m_database->SaveChanges();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMClipDefinitions SET PolygonData=?, Size=? WHERE PolygonId=?");
        stmt->BindBlob(1, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, clipID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        m_database->SaveChanges();
        }
    }

void SMSQLiteFile::StoreDiffSet(int64_t& diffsetID, const bvector<uint8_t>& diffsetData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    if (diffsetID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMDiffSets (Data,Size) VALUES(?,?)");
        stmt->BindBlob(1, &diffsetData[0], (int)diffsetData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        diffsetID = stmt2->GetValueInt64(0);
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMDiffSets SET Data=?, Size=? WHERE DiffsetId=?");
        stmt->BindBlob(1, &diffsetData[0], (int)diffsetData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, diffsetID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }

size_t SMSQLiteFile::GetNumberOfPoints(int64_t nodeID)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SizePts FROM SMPoint WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    //assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetNumberOfIndices(int64_t nodeID)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SizeIndices FROM SMPoint WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
   // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetNumberOfUVIndices(int64_t nodeID)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SizeUVs FROM SMTexture WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetTextureByteCount(int64_t nodeID)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SizeTexture FROM SMTexture WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetNumberOfUVs(int64_t nodeID)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SizeUVs FROM SMUVs WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetNumberOfFeaturePoints(int64_t featureID)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMFeatures WHERE FeatureId=?");
    stmt->BindInt64(1, featureID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0) / sizeof(int32_t);
    }

size_t SMSQLiteFile::GetClipPolygonByteCount(int64_t clipID)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMClipDefinitions WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }


    bool SMSQLiteFile::SetWkt(WCharCP extendedWkt)
{
    /*char* tmpCharP = new char[extendedWkt.GetMaxLocaleCharBytes()];
    filename.ConvertToLocaleChars(tmpCharP, extendedWkt.GetMaxLocaleCharBytes());*/

    Utf8String extendedWktUtf8String;
    BeStringUtilities::WCharToUtf8(extendedWktUtf8String, extendedWkt);

    CachedStatementPtr stmtTest;
    m_database->GetCachedStatement(stmtTest, "SELECT COUNT(MasterHeaderId) FROM SMMasterHeader WHERE MasterHeaderId=?");
    size_t id = 0;
    stmtTest->BindInt64(1, id);
    stmtTest->Step();
    size_t nRows = stmtTest->GetValueInt64(0);
    CachedStatementPtr stmt;
    if (nRows == 0)
    {
        //Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, GCS, RootNodeId) VALUES(?,?,?)");
        //stmt.Prepare(*m_database, "INSERT INTO SMMasterHeader (MasterHeaderId, GCS) VALUES(?,?)");
    }
    else
    {
        //Savepoint insertTransaction(*m_database, "update");
        m_database->GetCachedStatement(stmt, "UPDATE SMMasterHeader SET MasterHeaderId=?, GCS=? WHERE MasterHeaderID=?");
        //stmt.Prepare(*m_database, "UPDATE SMMasterHeader SET MasterHeaderId=?, GCS=? WHERE MasterHeaderID=?");
    }
    stmt->BindInt64(1, id);
    BIND_VALUE_STR(stmt, 2, extendedWktUtf8String, MAKE_COPY_NO);
    if (nRows != 0)
        stmt->BindInt64(3, id);
    else
        stmt->BindInt64(3, SQLiteNodeHeader::NO_NODEID);
    /*stmt.Prepare(*m_database, "REPLACE INTO SMSources "
        "(SourceId, SourceType, ModelId, ModelName, LevelId, LevelName, RootToRefPersistentPath, ReferenceName,"
        " ReferenceModelName, GCS, Flags, TypeFamilyID, OrgCount, Layer, ComponentCount, MonikerType, CommandCount, CommandID, TimeLastModified) "
        " VALUES(1, 2, 6553695, '', 3014702, '', '', '', '', 'LOCAL_CS["", "
        "LOCAL_DATUM[\"AnywhereXYZ\", 11000, AUTHORITY[\"BENTLEY_SYSTEMS\", \"11000\"]], UNIT[\"m\", 1], AXIS[\"X\", OTHER], AXIS[\"Y\", OTHER], AXIS[\"Z\", OTHER], AUTHORITY[\"BENTLEY_SYSTEMS\", \"0\"]]', "
        "1, '', 7340148, 4390944, 2, 1, 1, 12, 1452028924)");*/
    DbResult status = stmt->Step();
    //m_database->SaveChanges();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    return ((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
}

bool SMSQLiteFile::HasWkt()
{
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT GCS FROM SMMasterHeader WHERE MasterHeaderId=?");
    stmt->BindInt64(1, 0);


    DbResult status = stmt->Step();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW)); // can be BE_SQLITE_RAW => step has another raw ready (but the true question is why ?
    Utf8String wktStringUtf8 = GET_VALUE_STR(stmt, 0);
    return !wktStringUtf8.empty();
}

bool SMSQLiteFile::HasMasterHeader()
{
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT count(MasterHeaderId) FROM SMMasterHeader");
    DbResult status = stmt->Step();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    int masterHeaderCount = stmt->GetValueInt(0);
    return masterHeaderCount > 0 ? true : false;
}

bool SMSQLiteFile::HasPoints()
{
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT count(NodeId) FROM SMPoint");
    DbResult status = stmt->Step();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    int nodeIdCount = stmt->GetValueInt(0);
    return nodeIdCount > 0 ? true : false;
}

bool SMSQLiteFile::IsSingleFile()
{
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SingleFile  FROM SMMasterHeader WHERE MasterHeaderId=?");
    stmt->BindInt64(1, 0);

    DbResult status = stmt->Step();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW)); // can be BE_SQLITE_RAW => step has another raw ready (but the true question is why ?
    int singleFile = stmt->GetValueInt(0);
    return singleFile > 0 ? true : false;
}

bool SMSQLiteFile::GetWkt(WString& wktStr)
{
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT GCS FROM SMMasterHeader WHERE MasterHeaderId=?");
    stmt->BindInt64(1, 0);


    DbResult status = stmt->Step();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    Utf8String wktStringUtf8 = GET_VALUE_STR(stmt,0);
    wktStr = WSTRING_FROM_CSTR(wktStringUtf8.c_str());
    return wktStringUtf8.empty();
}

bool SMSQLiteFile::SaveSource(SourcesDataSQLite& sourcesData)
{
    BeAssert(m_database->IsTransactionActive());
    std::vector<SourceDataSQLite> vecSourceDataSQLite = sourcesData.GetSourceDataSQLite();

    for (SourceDataSQLite& sourceData : vecSourceDataSQLite)
    {
        Utf8String extendedWktUtf8String;
        BeStringUtilities::WCharToUtf8(extendedWktUtf8String, sourceData.GetGCS().GetWCharCP());
        // Logic to reconstruct all dimensionCount and company
        /*std::vector<uint32_t> m_dimensionCount = sourceData.GetDimensionCount();
        std::vector<byte> m_dimensionType = sourceData.GetDimensionType();
        std::vector<byte> m_dimensionRole = sourceData.GetDimensionRole();
        std::vector<WString> m_dimTypeName = sourceData.GetDimensionTypeName();*/
        //uint32_t componentID = sourceData.GetComponentID();
        ScalableMeshData smData = sourceData.GetScalableMeshData();

        Utf8String modelNameUtf8String;
        BeStringUtilities::WCharToUtf8(modelNameUtf8String, sourceData.GetModelName().GetWCharCP());
        //WString levelName = sourceData.GetLevelName();
        Utf8String levelNameUtf8String;
        BeStringUtilities::WCharToUtf8(levelNameUtf8String, sourceData.GetLevelName().GetWCharCP());
        //WString rootToRefPersistentPath = sourceData.GetRootToRefPersistentPath();
        Utf8String rootToRefPersistentPathUtf8String;
        BeStringUtilities::WCharToUtf8(rootToRefPersistentPathUtf8String, sourceData.GetRootToRefPersistentPath().GetWCharCP());
        //WString referenceName = sourceData.GetReferenceName();
        Utf8String referenceNameUtf8String;
        BeStringUtilities::WCharToUtf8(referenceNameUtf8String, sourceData.GetReferenceName().GetWCharCP());
        //WString referenceModelName = sourceData.GetReferenceModelName();
        Utf8String referenceModelNameUtf8String;
        BeStringUtilities::WCharToUtf8(referenceModelNameUtf8String, sourceData.GetReferenceModelName().GetWCharCP());

        Utf8String ElevationPropertyNameUtf8String;
        BeStringUtilities::WCharToUtf8(ElevationPropertyNameUtf8String, smData.ElevationPropertyName().GetWCharCP());

        Utf8String monikerStringUtf8String;
        BeStringUtilities::WCharToUtf8(monikerStringUtf8String, sourceData.GetMonikerString().GetWCharCP());

        CachedStatementPtr stmt;
        //Savepoint insertTransaction(*m_database, "replace");
        m_database->GetCachedStatement(stmt, "REPLACE INTO SMSources (SourceId, SourceType, DTMSourceID, GroupID, ModelId, ModelName, LevelId, LevelName, RootToRefPersistentPath, "
            "ReferenceName, ReferenceModelName, GCS, Flags, TypeFamilyID, OrgCount, Layer, ComponentCount, ConfigComponentID, MonikerType, MonikerString, CommandCount, CommandID, TimeLastModified, "
            "SizeExtent, Extent, UpToDateState, Time, IsRepresenting3dData, IsGroundDetection, IsGISData, ElevationProperty, LinearFeatureType, PolygonFeatureType, IsGridData,"
            "DimensionCount, DimensionType, DimensionRole, DimensionName) "
            "VALUES(?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?)");
/*        stmt.Prepare(*m_database, "REPLACE INTO SMSources (SourceId, SourceType, ModelId, ModelName, LevelId, LevelName, RootToRefPersistentPath, "
            "ReferenceName, ReferenceModelName, GCS, Flags, TypeFamilyID, OrgCount, Layer, ComponentCount, MonikerType, CommandCount, CommandID, TimeLastModified) "
            "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
        /*stmt.Prepare(*m_database, "REPLACE INTO SMSources "
            "(SourceId, SourceType, ModelId, ModelName, LevelId, LevelName, RootToRefPersistentPath, ReferenceName,"
            " ReferenceModelName, GCS, Flags, TypeFamilyID, OrgCount, Layer, ComponentCount, MonikerType, CommandCount, CommandID, TimeLastModified) "
            " VALUES(1, 2, 6553695, '', 3014702, '', '', '', '', 'LOCAL_CS["", "
            "LOCAL_DATUM[\"AnywhereXYZ\", 11000, AUTHORITY[\"BENTLEY_SYSTEMS\", \"11000\"]], UNIT[\"m\", 1], AXIS[\"X\", OTHER], AXIS[\"Y\", OTHER], AXIS[\"Z\", OTHER], AUTHORITY[\"BENTLEY_SYSTEMS\", \"0\"]]', "
            "1, '', 7340148, 4390944, 2, 1, 1, 12, 1452028924)");*/
        stmt->BindInt64(1, sourceData.GetSourceID());
        stmt->BindInt(2, sourceData.GetSourceType());
        stmt->BindInt(3, sourceData.GetDTMSourceID());
        stmt->BindInt64(4, sourceData.GetGroupID());
        stmt->BindInt64(5, sourceData.GetModelID());
        BIND_VALUE_STR(stmt, 6, modelNameUtf8String, MAKE_COPY_NO);
        stmt->BindInt64(7, sourceData.GetLevelID());
        BIND_VALUE_STR(stmt, 8, levelNameUtf8String, MAKE_COPY_NO);
        BIND_VALUE_STR(stmt, 9, rootToRefPersistentPathUtf8String, MAKE_COPY_NO);
        BIND_VALUE_STR(stmt, 10, referenceNameUtf8String, MAKE_COPY_NO);
        BIND_VALUE_STR(stmt, 11, referenceModelNameUtf8String, MAKE_COPY_NO);
        BIND_VALUE_STR(stmt, 12, extendedWktUtf8String, MAKE_COPY_NO);
        stmt->BindInt64(13, sourceData.GetFlags());
        stmt->BindInt64(14, sourceData.GetTypeFamilyID());
        stmt->BindInt64(15, sourceData.GetOrgCount());
        stmt->BindInt64(16, sourceData.GetLayer());
        stmt->BindInt64(17, sourceData.GetComponentCount());
        
        stmt->BindBlob(18, &sourceData.GetConfigComponentID()[0], sizeof(byte)*(int)(sourceData.GetComponentCount()), MAKE_COPY_YES);

        stmt->BindInt(19, sourceData.GetMonikerType());
        BIND_VALUE_STR(stmt, 20, monikerStringUtf8String, MAKE_COPY_NO);
        stmt->BindInt64(21, sourceData.GetCommandCount());

        stmt->BindBlob(22, &sourceData.GetCommandID()[0], sizeof(byte)*(int)(sourceData.GetCommandCount()), MAKE_COPY_YES);

        stmt->BindInt64(23, sourceData.GetTimeLastModified());
        stmt->BindInt64(24, smData.GetExtent().size());
        //if(smData.GetExtent().size()>0)
        {
            stmt->BindBlob(25, &smData.GetExtent()[0], sizeof(DRange3d)*(int)smData.GetExtent().size(), MAKE_COPY_YES);
        }
        stmt->BindInt(26, smData.GetUpToDateState());
        stmt->BindInt64(27, smData.GetTimeFile());
        stmt->BindInt(28, (int)smData.IsRepresenting3dData());
        stmt->BindInt(29, smData.IsGroundDetection() ? 1 : 0);
        stmt->BindInt(30, smData.IsGISDataType() ? 1 : 0);
        BIND_VALUE_STR(stmt, 31, ElevationPropertyNameUtf8String, MAKE_COPY_NO);
        stmt->BindInt(32, (int)smData.GetLinearFeatureType());
        stmt->BindInt(33, (int)smData.GetPolygonFeatureType());
        stmt->BindInt(34, smData.IsGridData() ? 1 : 0);
        size_t size = sourceData.GetVecDimensionCount().size();
        stmt->BindBlob(35, &sourceData.GetVecDimensionCount()[0], sizeof(uint32_t)*(int)(size), MAKE_COPY_YES);
        int sizeTot = 0;
        for (int i = 0; i < size; i++) sizeTot += sourceData.GetDimensionCount(i);
        byte* ptrType = new byte[sizeTot];
        byte* ptrRole = new byte[sizeTot];
        WString* ptrName = new WString[sizeTot];
        for (size_t i = 0; i < sourceData.GetOrgCount(); ++i)
            for (size_t j = 0; j < sourceData.GetVecDimensionType()[i].size(); ++j)
            {
                ptrType[i*sourceData.GetVecDimensionType()[i].size()+j] = sourceData.GetVecDimensionType()[i][j];
                ptrRole[i*sourceData.GetVecDimensionRole()[i].size()+j] = sourceData.GetVecDimensionRole()[i][j];
                ptrName[i*sourceData.GetVecDimensionName()[i].size()+j] = sourceData.GetVecDimensionName()[i][j];
            }
        /*stmt->BindBlob(36, &sourceData.GetVecDimensionType()[0][0], sizeof(byte)*(int)(sizeTot), MAKE_COPY_YES);
        stmt->BindBlob(37, &sourceData.GetVecDimensionRole()[0][0], sizeof(byte)*(int)(sizeTot), MAKE_COPY_YES);
        stmt->BindBlob(38, &sourceData.GetVecDimensionName()[0][0], sizeof(WString)*(int)(sizeTot), MAKE_COPY_YES);*/
        stmt->BindBlob(36, &ptrType[0], sizeof(byte)*(int)(sizeTot), MAKE_COPY_YES);
        stmt->BindBlob(37, &ptrRole[0], sizeof(byte)*(int)(sizeTot), MAKE_COPY_YES);
        stmt->BindBlob(38, &ptrName[0], sizeof(WString)*(int)(sizeTot), MAKE_COPY_YES);
        delete[] ptrType;
        delete[] ptrRole;
        delete[] ptrName;
        DbResult status = stmt->Step();
        assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
        m_database->SaveChanges();
    }
    CachedStatementPtr stmtTest;
    m_database->GetCachedStatement(stmtTest, "SELECT COUNT(MasterHeaderId) FROM SMMasterHeader WHERE MasterHeaderId=?");
    size_t id = 0;
    stmtTest->BindInt64(1, id);
    stmtTest->Step();
    size_t nRows = stmtTest->GetValueInt64(0);
    CachedStatementPtr stmt;
    if (nRows == 0)
    {
        /*Savepoint insertTransaction(*m_database, "insert");*/
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, LastModifiedTime, LastSyncTime, CheckTime,"
            "SerializedSourceFormatVersion, ContentConfigFormatVersion, ImportSequenceFormatVersion, ImportConfigFormatVersion, RootNodeId) VALUES(?,?,?,?,?,?,?,?,?)");
        /*stmt.Prepare(*m_database, "INSERT INTO SMMasterHeader (MasterHeaderId, LastModifiedTime, LastSyncTime, CheckTime,"
            "SerializedSourceFormatVersion, ContentConfigFormatVersion, ImportSequenceFormatVersion, ImportConfigFormatVersion) VALUES(?,?,?,?,?,?,?,?)");*/
    }
    else
    {
        /*Savepoint insertTransaction(*m_database, "update");*/
        m_database->GetCachedStatement(stmt, "UPDATE SMMasterHeader SET MasterHeaderId=?, LastModifiedTime=?, LastSyncTime=?, CheckTime=?,"
            "SerializedSourceFormatVersion=?, ContentConfigFormatVersion=?, ImportSequenceFormatVersion=?, ImportConfigFormatVersion=? WHERE MasterHeaderId=?");
        /*stmt.Prepare(*m_database, "UPDATE SMMasterHeader SET MasterHeaderId=?, LastModifiedTime=?, LastSyncTime=?, CheckTime=?,"
            "SerializedSourceFormatVersion=?, ContentConfigFormatVersion=?, ImportSequenceFormatVersion=?, ImportConfigFormatVersion=? WHERE MasterHeaderId=?");*/
    }
    
    stmt->BindInt64(1, id);
    stmt->BindInt64(2, sourcesData.GetLastModifiedCheckTime());
    stmt->BindInt64(3, sourcesData.GetLastModifiedTime());
    stmt->BindInt64(4, sourcesData.GetLastSyncTime());
    stmt->BindInt64(5, sourcesData.GetSerializedSourceFormatVersion());
    stmt->BindInt64(6, sourcesData.GetContentConfigFormatVersion());
    stmt->BindInt64(7, sourcesData.GetImportSequenceFormatVersion());
    stmt->BindInt64(8, sourcesData.GetImportConfigFormatVersion());
    if (nRows != 0)
        stmt->BindInt64(9, id);
    else
        stmt->BindInt64(9, SQLiteNodeHeader::NO_NODEID);
    DbResult status = stmt->Step();
    m_database->SaveChanges();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    return ((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
}

bool SMSQLiteFile::HasSources()
{
    CachedStatementPtr stmtTest;
    m_database->GetCachedStatement(stmtTest, "SELECT COUNT(SourceId) FROM SMSources");
    stmtTest->Step();
    return stmtTest->GetValueInt64(0) > 0 ? true : false;
}

bool SMSQLiteFile::LoadSources(SourcesDataSQLite& sourcesData)
{
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SourceId, SourceType, DTMSourceID, GroupID, ModelId, ModelName, LevelId, LevelName, RootToRefPersistentPath, "
        "ReferenceName, ReferenceModelName, GCS, Flags, TypeFamilyID, OrgCount, Layer, ComponentCount, ConfigComponentID, MonikerType, MonikerString, CommandCount, CommandID, TimeLastModified, "
        "SizeExtent, Extent, UpToDateState, Time, IsRepresenting3dData, IsGroundDetection, IsGISData, ElevationProperty, LinearFeatureType, PolygonFeatureType, IsGridData, "
        "DimensionCount, DimensionType, DimensionRole, DimensionName FROM SMSources");
    while (stmt->Step() == BE_SQLITE_ROW)
    {
        SourceDataSQLite sourceData(SourceDataSQLite::GetNull());
        sourceData.SetSourceID(stmt->GetValueInt64(0));
        sourceData.SetSourceType(stmt->GetValueInt64(1));
        sourceData.SetDTMSourceID(stmt->GetValueInt(2));
        sourceData.SetGroupID(stmt->GetValueInt64(3));
        sourceData.SetModelID(stmt->GetValueInt64(4));
        sourceData.SetModelName(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 5)).c_str()));
        sourceData.SetLevelID(stmt->GetValueInt64(6));
        sourceData.SetLevelName(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 7)).c_str()));
        sourceData.SetRootToRefPersistentPath(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 8)).c_str()));
        sourceData.SetReferenceName(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 9)).c_str()));
        sourceData.SetReferenceModelName(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 10)).c_str()));
        sourceData.SetGCS(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 11)).c_str()));
        sourceData.SetFlags(stmt->GetValueInt64(12));
        sourceData.SetTypeFamilyID(stmt->GetValueInt64(13));
        sourceData.SetOrgCount(stmt->GetValueInt64(14));
        sourceData.SetLayer(stmt->GetValueInt64(15));
        sourceData.SetComponentCount(stmt->GetValueInt64(16));

        std::vector<byte> configComponentID(sourceData.GetComponentCount());
        std::memcpy(&configComponentID[0], stmt->GetValueBlob(17), sizeof(byte) * (int)sourceData.GetComponentCount());
        sourceData.SetConfigComponentID(configComponentID);

        sourceData.SetMonikerType(stmt->GetValueInt(18));
        sourceData.SetMonikerString(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 19)).c_str()));
        sourceData.SetCommandCount(stmt->GetValueInt64(20));

        std::vector<byte> commandID(sourceData.GetCommandCount());
        std::memcpy(&commandID[0], stmt->GetValueBlob(21), sizeof(byte) * (int)sourceData.GetCommandCount());
        sourceData.SetCommandID(commandID);

        sourceData.SetTimeLastModified(stmt->GetValueInt64(22));

        ScalableMeshData smData(ScalableMeshData::GetNull());
        size_t nLayer = stmt->GetValueInt64(23);
        //if(nLayer>0)
        {
            //stmt->BindBlob(21, reinterpret_cast<const byte*>(&smData.GetExtent()[0]), sizeof(DRange3d)*(int)smData.GetExtent().size(), MAKE_COPY_YES);
            std::vector<DRange3d> extents(nLayer);
            std::memcpy(&extents[0], stmt->GetValueBlob(24), sizeof(DRange3d)*(int)nLayer);
            smData.SetExtents(extents);
        }
        smData.SetUpToDateState(UpToDateState(stmt->GetValueInt(25)));
        smData.SetTimeFile(stmt->GetValueInt64(26));
        smData.SetRepresenting3dData(SMis3D(stmt->GetValueInt(27)));
        smData.SetIsGroundDetection(stmt->GetValueInt(28) ? true : false);
        smData.SetIsGISDataType(stmt->GetValueInt(29) ? true : false);
        WString tmp = WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 30)).c_str());
        smData.SetElevationPropertyName(tmp);
        smData.SetLinearFeatureType(DTMFeatureType(stmt->GetValueInt(31)));
        smData.SetPolygonFeatureType(DTMFeatureType(stmt->GetValueInt(32)));
        smData.SetIsGridData(stmt->GetValueInt(33) ? true : false);

        std::vector<uint32_t> dimensionCount(sourceData.GetOrgCount());
        std::memcpy(&dimensionCount[0], stmt->GetValueBlob(34), sizeof(uint32_t) * (int)sourceData.GetOrgCount());
        sourceData.SetDimensionCount(dimensionCount);

        std::vector<std::vector<byte>> dimensionType(sourceData.GetOrgCount());
        int sizeTot = 0;
        for (int i = 0; i < dimensionType.size(); i++)
        {
            dimensionType[i].resize(sourceData.GetDimensionCount(i));
            sizeTot += sourceData.GetDimensionCount(i);
        }

        byte* ptrType = new byte[sizeTot];
        byte* ptrRole = new byte[sizeTot];
        WString* ptrName = new WString[sizeTot];

        std::memcpy(&ptrType[0], stmt->GetValueBlob(35), sizeof(byte) * (int)sizeTot);
        std::vector<std::vector<byte>> dimensionRole (sourceData.GetOrgCount());
        for (int i = 0; i < dimensionRole.size(); i++) dimensionRole[i].resize(sourceData.GetDimensionCount(i));
        std::memcpy(&ptrRole[0], stmt->GetValueBlob(36), sizeof(byte) * (int)sizeTot);
        std::vector<std::vector<WString>> dimensionName(sourceData.GetOrgCount());
        for (int i = 0; i < dimensionName.size(); i++) dimensionName[i].resize(sourceData.GetDimensionCount(i));
        std::memcpy(&ptrName[0], stmt->GetValueBlob(37), sizeof(WString) * (int)sizeTot);

        for (size_t i = 0; i < dimensionType.size(); i++)
            for (size_t j = 0; j < dimensionType[i].size(); j++)
            {
                dimensionType[i][j] = ptrType[i*dimensionType[i].size() + j];
                dimensionRole[i][j] = ptrRole[i*dimensionRole[i].size() + j];
                dimensionName[i][j] = ptrName[i*dimensionName[i].size() + j];
            }
        sourceData.SetDimensionType(dimensionType);
        sourceData.SetDimensionRole(dimensionRole);
        sourceData.SetScalableMeshData(smData);
        sourceData.SetDimensionName(dimensionName);

        sourcesData.AddSourcesNode(sourceData);
        delete[] ptrType;
        delete[] ptrRole;
        delete[] ptrName;
    }

    CachedStatementPtr stmt2;
    m_database->GetCachedStatement(stmt2, "SELECT LastModifiedTime, LastSyncTime, CheckTime,"
        "SerializedSourceFormatVersion, ContentConfigFormatVersion, ImportSequenceFormatVersion, ImportConfigFormatVersion FROM SMMasterHeader WHERE MasterHEaderId=?");
    size_t id = 0;
    stmt2->BindInt64(1, id);
    stmt2->Step();

    sourcesData.SetLastModifiedCheckTime(stmt->GetValueInt64(0));
    sourcesData.SetLastModifiedTime(stmt->GetValueInt64(1));
    sourcesData.SetLastSyncTime(stmt->GetValueInt64(2));
    sourcesData.SetSerializedSourceFormatVersion(stmt->GetValueInt64(3));
    sourcesData.SetContentConfigFormatVersion(stmt->GetValueInt64(4));
    sourcesData.SetImportSequenceFormatVersion(stmt->GetValueInt64(5));
    sourcesData.SetImportConfigFormatVersion(stmt->GetValueInt64(6));

    return true;
}

bool SMSQLiteFile::SetSingleFile(bool isSingleFile)
{
    CachedStatementPtr stmtTest;
    m_database->GetCachedStatement(stmtTest, "SELECT COUNT(MasterHeaderId) FROM SMMasterHeader WHERE MasterHeaderId=?");
    size_t id = 0;
    stmtTest->BindInt64(1, id);
    stmtTest->Step();
    size_t nRows = stmtTest->GetValueInt64(0);
    CachedStatementPtr stmt;
    if (nRows == 0)
    {
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, SingleFile, RootNodeId) VALUES(?,?,?)");
    }
    else
    {
        m_database->GetCachedStatement(stmt, "UPDATE SMMasterHeader SET MasterHeaderId=?, SingleFile=? WHERE MasterHeaderID=?");
    }
    stmt->BindInt64(1, id);
    stmt->BindInt(2, isSingleFile ? 1 : 0);
    if (nRows != 0)
        stmt->BindInt64(3, id);
    else
        stmt->BindInt64(3, SQLiteNodeHeader::NO_NODEID);

    DbResult status = stmt->Step();

    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    return ((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
}

