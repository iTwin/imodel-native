#include <ScalableMeshPCH.h>
#include "SMSQLiteFile.h"
#include <iostream>
#include "SMSQLiteClipDefinitionsFile.h"
#include "SMSQLiteDiffsetFile.h"
#include "SMSQLiteFeatureFile.h"

#define WSTRING_FROM_CSTR(cstr) WString(cstr, BentleyCharEncoding::Utf8)
#define MAKE_COPY_NO Statement::MakeCopy::No
#define MAKE_COPY_YES Statement::MakeCopy::Yes
#define GET_VALUE_STR(stmt, id) stmt->GetValueText(id)
#define BIND_VALUE_STR(stmt, id, utf8str, copyval) stmt->BindText(id, utf8str, copyval)
#define READONLY Db::OpenMode::Readonly
#define READWRITE Db::OpenMode::ReadWrite

const SchemaVersion SMSQLiteFile::CURRENT_VERSION = SchemaVersion(1, 1, 0, 3);

SMSQLiteFile::SMSQLiteFile()
{
    m_database = nullptr;
}

SMSQLiteFile::~SMSQLiteFile()
    {
    if (m_database != nullptr)
        {            
        if (m_database->IsDbOpen())
            {
            Close();
            }

        delete m_database;
        }

    m_database = nullptr;
    }

void SMSQLiteFile::Save()
    {
    if (m_database != nullptr) m_database->SaveChanges();
    }

bool SMSQLiteFile::Close()
    {
    m_database->SaveChanges();
    m_database->CloseDb();
    return true;
    }


const SchemaVersion s_listOfReleasedSchemas[4] = { SchemaVersion(1, 1, 0, 0), SchemaVersion(1, 1, 0, 1), SchemaVersion(1, 1, 0, 2), SchemaVersion(1, 1, 0, 3) };
const size_t s_numberOfReleasedSchemas = 4;
double s_expectedTimeUpdate[3] = { 1.2*1e-5, 1e-6,1e-6};
//all the functions for each schema transition. 
std::function<void(BeSQLite::Db*)> s_databaseUpdateFunctions[3] = {
    [](BeSQLite::Db* database)
        {
        assert(database->TableExists("SMMasterHeader"));
        assert(database->ColumnExists("SMMasterHeader", "TerrainDepth"));
        Savepoint s(*database, Utf8String("newTable").c_str());
        database->ExecuteSql("ALTER TABLE SMMasterHeader RENAME TO SMMasterHeader_2");
        database->CreateTable("SMMasterHeader", "MasterHeaderId INTEGER PRIMARY KEY,"
                                "Balanced INTEGER,"
                                "SplitTreshold INTEGER,"
                                "RootNodeId INTEGER,"
                                "Depth INTEGER,"
                                "IsTextured INTEGER,"
                                "SingleFile INTEGER,"
                                "MeshDataDepth INTEGER,"
                                "IsTerrain INTEGER,"
                                "GCS STRING,"
                                "LastModifiedTime INTEGER,"
                                "LastSyncTime INTEGER,"
                                "CheckTime INTEGER");
        database->ExecuteSql("INSERT INTO SMMasterHeader (MasterHeaderId,Balanced,SplitTreshold,RootNodeId,Depth,IsTextured,SingleFile,MeshDataDepth,IsTerrain,GCS,LastModifiedTime,LastSyncTime,CheckTime)"
                             " SELECT MasterHeaderId,Balanced,SplitTreshold,RootNodeId,Depth,IsTextured,SingleFile,TerrainDepth,IsTerrain,GCS,LastModifiedTime,LastSyncTime,CheckTime"
                             " FROM SMMasterHeader_2");
        s.Commit();
        Savepoint s2(*database, Utf8String("nodeTable").c_str());
        database->DropTable("SMMasterHeader_2");

        assert(database->TableExists("SMNodeHeader"));
        assert(database->ColumnExists("SMNodeHeader", "GraphID"));
        assert(database->ColumnExists("SMNodeHeader", "IndiceID"));

        database->ExecuteSql("ALTER TABLE SMNodeHeader RENAME TO SMNodeHeader_2");
        database->CreateTable("SMNodeHeader", "NodeId INTEGER PRIMARY KEY,"
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
                              "SubNode BLOB,"
                              "Neighbor BLOB");
        s2.Commit();
        database->ExecuteSql("INSERT INTO SMNodeHeader (NodeId,ParentNodeId,Resolution,Filtered,Extent,ContentExtent,TotalCount,NodeCount,ArePoints3d,NbFaceIndexes,NumberOfMeshComponents,AllComponent,IsTextured,TexID,SubNode,Neighbor)"
                             " SELECT SMNodeHeader_2.NodeId,SMNodeHeader_2.ParentNodeId,SMNodeHeader_2.Resolution,SMNodeHeader_2.Filtered,SMNodeHeader_2.Extent,SMNodeHeader_2.ContentExtent,SMNodeHeader_2.TotalCount,SMNodeHeader_2.NodeCount,SMNodeHeader_2.ArePoints3d,SMNodeHeader_2.NbFaceIndexes,SMNodeHeader_2.NumberOfMeshComponents,SMNodeHeader_2.AllComponent,SMNodeHeader_2.IsTextured,SMNodeHeader_2.TexID,SMNodeHeader_2.SubNode,SMNodeHeader_2.Neighbor"
                             " FROM SMNodeHeader_2");

        database->ExecuteSql("ALTER TABLE SMTexture RENAME TO SMTexture_2");
        database->CreateTable("SMTexture", "NodeId INTEGER PRIMARY KEY,"
                              "TexData BLOB,"
                              "SizeTexture INTEGER,"
                              "Codec INTEGER,"
                              "NOfChannels INTEGER");
        database->ExecuteSql("INSERT INTO SMTexture (NodeId,TexData,SizeTexture,Codec,NOfChannels)"
                             " SELECT SMTexture_2.NodeId,SMTexture_2.TexData,SMTexture_2.SizeTexture,SMTexture_2.Codec,SMTexture_2.NOfChannels"
                             " FROM SMTexture_2");
        database->ExecuteSql("ALTER TABLE SMUVs ADD COLUMN UVIndexData BLOB");
        database->ExecuteSql("ALTER TABLE SMUVs ADD COLUMN SizeUVIndex INTEGER DEFAULT 0");
        database->ExecuteSql("UPDATE SMUVs SET UVIndexData = (SELECT SMTexture_2.UVData FROM SMTexture_2 WHERE SMTexture_2.NodeId = SMUVs.NodeId) ,"
                             "SizeUVIndex = (SELECT SMTexture_2.SizeUVs FROM SMTexture_2 WHERE SMTexture_2.NodeId = SMUVs.NodeId)");
        
        Savepoint s3(*database, Utf8String("drops").c_str());
        database->DropTable("SMNodeHeader_2");
        database->DropTable("SMTexture_2");
        database->DropTable("SMSkirts");
        database->DropTable("SMDiffSets");
        database->DropTable("SMFeatures");
        database->DropTable("SMClipDefinitions");
        database->DropTable("SMGraph");
        s3.Commit();
        },

        [] (BeSQLite::Db* database)
        {
        database->ExecuteSql("ALTER TABLE SMMasterHeader ADD COLUMN DataResolution REAL DEFAULT 0.0");
        database->ExecuteSql("ALTER TABLE SMNodeHeader ADD COLUMN GeometryResolution REAL DEFAULT 0.0");
        database->ExecuteSql("ALTER TABLE SMNodeHeader ADD COLUMN TextureResolution REAL DEFAULT 0.0");
        },
            [](BeSQLite::Db* database)
        {
            database->ExecuteSql("ALTER TABLE SMFileMetadata ADD COLUMN Properties TEXT DEFAULT NULL");
        }
    };

size_t SMSQLiteFile::GetNumberOfReleasedSchemas() { return s_numberOfReleasedSchemas; }
const SchemaVersion* SMSQLiteFile::GetListOfReleasedVersions() { return s_listOfReleasedSchemas; }
double* SMSQLiteFile::GetExpectedTimesForUpdateFunctions() { return s_expectedTimeUpdate; }
std::function<void(BeSQLite::Db*)>* SMSQLiteFile::GetFunctionsForAutomaticUpdate() { return s_databaseUpdateFunctions; }

bool SMSQLiteFile::UpdateDatabase()
    {         
    CachedStatementPtr stmtTest;
    m_database->GetCachedStatement(stmtTest, "SELECT Version FROM SMFileMetadata");
    assert(stmtTest != nullptr);
    stmtTest->Step();
#ifndef VANCOUVER_API
    SchemaVersion databaseSchema(GET_VALUE_STR(stmtTest,0));    
    stmtTest->Finalize();
    for (size_t i = 0; i < GetNumberOfReleasedSchemas()- 1; ++i)
        {
        SchemaVersion databaseSchemaOld = GetListOfReleasedVersions()[i];

        if (databaseSchema.CompareTo(databaseSchemaOld) == 0)
            {
            Savepoint s(*m_database,(Utf8String("update ") + databaseSchemaOld.ToString()).c_str());
            clock_t start = clock();
            GetFunctionsForAutomaticUpdate()[i](m_database);
            databaseSchema = GetListOfReleasedVersions()[i + 1];
            double time = ((double)clock() - start) / CLOCKS_PER_SEC;
            std::cout << "Update to version " << databaseSchema.ToString() << " took " << time << "s" << std::endl;
            }
        }
    if (databaseSchema.CompareTo(GetCurrentVersion()) == 0)
        {
        CachedStatementPtr stmt;
        m_database->GetCachedStatement(stmt, "UPDATE SMFileMetadata SET Version=?");
        Utf8String versonJson(databaseSchema.ToJson());
#ifndef VANCOUVER_API
        stmt->BindText(1, versonJson.c_str(), Statement::MakeCopy::Yes);
#else
        stmt->BindUtf8String(1, versonJson, Statement::MAKE_COPY_Yes);
#endif
        DbResult status = stmt->Step();
        if (status == BE_SQLITE_DONE) return true;
        }
    #endif
    assert(!"ERROR - Unknown database schema version");
    return false;
    }


bool SMSQLiteFile::Open(BENTLEY_NAMESPACE_NAME::Utf8CP filename, bool openReadOnly, SQLDatabaseType type)
    {
    if (m_database == nullptr)
        m_database = new ScalableMeshDb(type);
    DbResult result;
    if (m_database->IsDbOpen())
        m_database->CloseDb();

    result = m_database->OpenBeSQLiteDb(filename, Db::OpenParams(openReadOnly ? READONLY: READWRITE));

//#ifndef VANCOUVER_API
    if (result == BE_SQLITE_SCHEMA)
//#endif
        {
        Db::OpenParams openParamUpdate(READWRITE);

        #ifndef VANCOUVER_API
        openParamUpdate.m_skipSchemaCheck = true;
        #endif

        result = m_database->OpenBeSQLiteDb(filename, openParamUpdate);

        assert(result == BE_SQLITE_OK);
        
        if (result == BE_SQLITE_OK)
        {
            UpdateDatabase();

            m_database->CloseDb();
        }

        result = m_database->OpenBeSQLiteDb(filename, Db::OpenParams(openReadOnly ? READONLY : READWRITE));
        }
    
    return result == BE_SQLITE_OK;
    }

bool SMSQLiteFile::Open(BENTLEY_NAMESPACE_NAME::WString& filename, bool openReadOnly, SQLDatabaseType type)
    {
    Utf8String utf8FileName(filename);        
    return Open(utf8FileName.c_str(), openReadOnly, type);
    }

SMSQLiteFilePtr SMSQLiteFile::Open(const WString& filename, bool openReadOnly, StatusInt& status, SQLDatabaseType type)
    {
    bool result;
    SMSQLiteFilePtr smSQLiteFile;
    switch (type)
        {
        case SQLDatabaseType::SM_CLIP_DEF_FILE:
            smSQLiteFile = new SMSQLiteClipDefinitionsFile();
            break;
        case SQLDatabaseType::SM_DIFFSETS_FILE:
            smSQLiteFile = new SMSQLiteDiffsetFile();
            break;
        case SQLDatabaseType::SM_GENERATION_FILE:
            smSQLiteFile = new SMSQLiteFeatureFile();
            break;
        default:
            smSQLiteFile = new SMSQLiteFile();
        }

    Utf8String utf8File(filename);

    result = smSQLiteFile->Open(utf8File.c_str(), openReadOnly, type);
    // need to check version file ?
    status = result ? 1 : 0;
    return smSQLiteFile;
    }

bool SMSQLiteFile::GetFileName(Utf8String& fileName) const
    {
    if (m_database == 0)
        {
        return false;
        }

    fileName = Utf8String(m_database->GetDbFileName());

    return true;
    }        

DbResult SMSQLiteFile::CreateTables()
    {
    DbResult result;
    result = m_database->CreateTable("SMMasterHeader", "MasterHeaderId INTEGER PRIMARY KEY,"
                                     "Balanced INTEGER,"
                                     "SplitTreshold INTEGER,"
                                     "RootNodeId INTEGER,"
                                     "Depth INTEGER,"
                                     "IsTextured INTEGER,"
                                     "SingleFile INTEGER,"
                                     "MeshDataDepth INTEGER,"
                                     "IsTerrain INTEGER,"
                                     "GCS STRING,"
                                     "LastModifiedTime INTEGER,"
                                     "LastSyncTime INTEGER,"
                                     "CheckTime INTEGER,"
                                     "DataResolution REAL");
    assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMPoint", "NodeId INTEGER PRIMARY KEY AUTOINCREMENT,"
                                     "PointData BLOB,"
                                     "IndexData BLOB,"
                                     "SizePts INTEGER,"
                                     "SizeIndices INTEGER");
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
                                     "SubNode BLOB,"
                                     "Neighbor BLOB,"
                                     "GeometryResolution REAL,"
                                     "TextureResolution REAL");
    assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMTexture", "NodeId INTEGER PRIMARY KEY,"
                                     "TexData BLOB,"
                                     "SizeTexture INTEGER,"
                                     "Codec INTEGER,"
                                     "NOfChannels INTEGER");
    assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMUVs", "NodeId INTEGER PRIMARY KEY,"
                                     "UVData BLOB,"
                                     "SizeUVs INTEGER,"
                                     "UVIndexData BLOB,"
                                     "SizeUVIndex INTEGER");


#ifdef WIP_MESH_IMPORT

    result = m_database->CreateTable("SMMeshParts", "NodeId INTEGER PRIMARY KEY,"
                                     "Data BLOB,"
                                     "Size INTEGER");

    result = m_database->CreateTable("SMMeshMetadata", "NodeId INTEGER PRIMARY KEY,"
                                     "Data BLOB,"
                                     "Size INTEGER");
#endif

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
                                     "TypeID INTEGER,"
                                     "Layer INTEGER,"
                                     "MonikerType INTEGER,"
                                     "MonikerString INTEGER,"
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
                                     "IsGridData INTEGER");
    assert(result == BE_SQLITE_OK);


    result = m_database->CreateTable("SMImportSequences", "CommandID INTEGER PRIMARY KEY,"
                                     "SourceID INTEGER,"
                                     "CommandPosition INTEGER,"
                                     "SourceLayer INTEGER,"
                                     "TargetLayer INTEGER,"
                                     "SourceType INTEGER,"
                                     "TargetType INTEGER");
    assert(result == BE_SQLITE_OK);
    return result;
    }

    bool SMSQLiteFile::Create(BENTLEY_NAMESPACE_NAME::Utf8CP filename, SQLDatabaseType type)
{
    if (m_database == nullptr)

        m_database = new ScalableMeshDb(type);

    DbResult result;
    result = m_database->CreateNewDb(filename);

    assert(result == BE_SQLITE_OK);

    result = CreateTables();

                            

    m_database->SaveChanges();

    return result == BE_SQLITE_OK;
}

bool SMSQLiteFile::Create(BENTLEY_NAMESPACE_NAME::WString& filename, SQLDatabaseType type)
    {
    BeFileName sqlFileName(filename);

    if (!sqlFileName.GetDirectoryName().DoesPathExist())
        BeFileName::CreateNewDirectory(sqlFileName.GetDirectoryName().GetWCharCP());

    Utf8String utf8FileName(filename);            
    return Create(utf8FileName.c_str(), type);
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
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, Balanced, RootNodeId, SplitTreshold, Depth, MeshDataDepth, IsTextured, IsTerrain, DataResolution) VALUES(?,?,?,?,?,?,?,?,?)");
    }
    else
    {
        m_database->GetCachedStatement(stmt, "UPDATE SMMasterHeader SET MasterHeaderId=?, Balanced=?, RootNodeId=?, SplitTreshold=?, Depth=?, MeshDataDepth=?, IsTextured=?, IsTerrain=?, DataResolution=?"
            " WHERE MasterHeaderId=?");
    }
    stmt->BindInt64(1, id);
    stmt->BindInt(2, newHeader.m_balanced ? 1: 0);
    stmt->BindInt64(3, newHeader.m_rootNodeBlockID);
    stmt->BindInt(4, (int)newHeader.m_SplitTreshold);
    stmt->BindInt64(5, newHeader.m_depth);
    stmt->BindInt64(6, newHeader.m_terrainDepth);
    stmt->BindInt(7, (int)newHeader.m_textured);
    stmt->BindInt(8, newHeader.m_isTerrain ? 1 : 0);
    stmt->BindDouble(9, (double)newHeader.m_resolution);
    if (nRows != 0)
        stmt->BindInt64(10, id);
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
                                  "NumberOfMeshComponents, AllComponent,SubNode,Neighbor, TexID, IsTextured, NodeCount, GeometryResolution, TextureResolution) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    stmt->BindInt64(1, newNodeHeader.m_nodeID);
    stmt->BindInt64(2, newNodeHeader.m_parentNodeID);
    stmt->BindInt64(3, newNodeHeader.m_level);
    stmt->BindInt(4, newNodeHeader.m_filtered ? 1 : 0);
    stmt->BindBlob(5, &newNodeHeader.m_nodeExtent, 6 * sizeof(double), MAKE_COPY_NO);
    stmt->BindBlob(6, newNodeHeader.m_contentExtentDefined ? &newNodeHeader.m_contentExtent : NULL, 6 * sizeof(double), MAKE_COPY_NO);
    stmt->BindInt64(7, newNodeHeader.m_totalCount);
    stmt->BindInt(8, newNodeHeader.m_arePoints3d ? 1 : 0);
    stmt->BindInt64(9, newNodeHeader.m_nbFaceIndexes);
    stmt->BindInt64(10, newNodeHeader.m_numberOfMeshComponents);
    stmt->BindBlob(11, newNodeHeader.m_meshComponents, (int)newNodeHeader.m_numberOfMeshComponents * sizeof(int), MAKE_COPY_NO);
    stmt->BindBlob(12, (newNodeHeader.m_apSubNodeID.size() > 0) ? &newNodeHeader.m_apSubNodeID[0] : nullptr, (int)newNodeHeader.m_apSubNodeID.size()  * sizeof(int), MAKE_COPY_NO);
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
    stmt->BindBlob(13, (void*)neighbors, (int)nOfNeighbors*sizeof(int) + 26 * sizeof(int), MAKE_COPY_NO);

    stmt->BindInt64(14, newNodeHeader.m_textureID);


    stmt->BindInt(15, newNodeHeader.m_isTextured ? 1 : 0); 
    stmt->BindInt(16, (int)newNodeHeader.m_nodeCount);
    stmt->BindDouble(17, (double)newNodeHeader.m_geometricResolution);
    stmt->BindDouble(18, (double)newNodeHeader.m_textureResolution);
    DbResult status = stmt->Step();
    stmt->ClearBindings();
    delete[]neighbors;
    assert(status == BE_SQLITE_DONE);
    return status == BE_SQLITE_DONE;
    }


bool SMSQLiteFile::GetMasterHeader(SQLiteIndexHeader& header)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Balanced, RootNodeId, SplitTreshold, Depth,IsTextured, SingleFile, MeshDataDepth, IsTerrain, DataResolution FROM SMMasterHeader WHERE MasterHeaderId = ?");
    size_t id = 0;
    stmt->BindInt64(1, id);
    DbResult status = stmt->Step();
    assert(status == BE_SQLITE_DONE || status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE) return false;
    header.m_balanced = stmt->GetValueInt(0) ? true : false;
    header.m_rootNodeBlockID = stmt->GetValueInt(1);
    header.m_SplitTreshold = stmt->GetValueInt(2);
    header.m_depth = (size_t)stmt->GetValueInt(3);
    header.m_textured = (IndexTexture) stmt->GetValueInt(4);
    header.m_singleFile = stmt->GetValueInt(5) ? true : false;
    header.m_terrainDepth = stmt->GetValueInt64(6);
    header.m_isTerrain = stmt->GetValueInt(7) ? true : false;
    header.m_resolution = (float)stmt->GetValueDouble(8);
    return true;
    }


bool SMSQLiteFile::GetNodeHeader(SQLiteNodeHeader& nodeHeader)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT ParentNodeId, Resolution, Filtered, Extent,"
                                  "ContentExtent, TotalCount, ArePoints3d, NbFaceIndexes, "
                                  "NumberOfMeshComponents, AllComponent,SubNode, Neighbor, TexID, IsTextured, NodeCount, GeometryResolution, TextureResolution, length(SubNode), length(Neighbor) FROM SMNodeHeader WHERE NodeId=?");
    stmt->BindInt64(1, nodeHeader.m_nodeID);


    DbResult status = stmt->Step();
    assert(status == BE_SQLITE_DONE || status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE) 
        {
        assert(!"Node header not found");
        return false;
        }
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

    const void* childrenTmp = stmt->GetValueBlob(10);
    size_t nofNodes = stmt->GetValueInt(17) / sizeof(int);
    nodeHeader.m_apSubNodeID.resize(nofNodes);
    memcpy(&nodeHeader.m_apSubNodeID[0], childrenTmp, stmt->GetValueInt(17));
    const void* neighborTmp = stmt->GetValueBlob(11);
    if (stmt->GetValueInt(18) >= 26 * sizeof(int))
        {
        const int* neighbors = (const int*)neighborTmp;
        for (size_t i = 0; i < 26; ++i)
            {
            int nNeighbors = i + 1 < 26 ? (neighbors[i + 1] - neighbors[i]) : (stmt->GetValueInt(18)/ sizeof(int) - (neighbors[i] + 26));
            nodeHeader.m_apNeighborNodeID[i].resize(nNeighbors);
            memcpy(&nodeHeader.m_apNeighborNodeID[i][0], &neighbors[26 + neighbors[i]], nNeighbors*sizeof(int));
            }
        }

        nodeHeader.m_ptsIndiceID.resize(1);


    memcpy(&nodeHeader.m_nodeExtent, extentTmp, sizeof(double) * 6);
    nodeHeader.m_contentExtentDefined = contentExtentTmp != NULL;
    if (nodeHeader.m_contentExtentDefined) memcpy(&nodeHeader.m_contentExtent, contentExtentTmp, sizeof(double) * 6);
    nodeHeader.m_meshComponents = new int[nodeHeader.m_numberOfMeshComponents];
    memcpy(nodeHeader.m_meshComponents, allComponentTmp, sizeof(int) * nodeHeader.m_numberOfMeshComponents);
    nodeHeader.m_clipSetsID = std::vector<int>();
    nodeHeader.m_numberOfSubNodesOnSplit = nodeHeader.m_apSubNodeID.size();
    int64_t texIdx = stmt->GetValueInt64(12);
    nodeHeader.m_isTextured = stmt->GetValueInt(13) ? true : false;
    if (nodeHeader.m_isTextured)
        {
        nodeHeader.m_textureID = texIdx;
        nodeHeader.m_nbTextures = 1;
        nodeHeader.m_uvsIndicesID.resize(1);
        nodeHeader.m_uvsIndicesID[0] = texIdx;
        }
    nodeHeader.m_nodeCount = stmt->GetValueInt(14);
    nodeHeader.m_geometricResolution = (float)stmt->GetValueDouble(15);
    nodeHeader.m_textureResolution = (float)stmt->GetValueDouble(16);
    stmt->ClearBindings();
    return true;
    }

bool SMSQLiteFile::SetProperties(const Json::Value& properties)
{
    CachedStatementPtr stmt;
    Utf8String propertiesStr(Json::FastWriter().write(properties));
    m_database->GetCachedStatement(stmt, "UPDATE SMFileMetadata SET Properties=?");

    BIND_VALUE_STR(stmt, 1, propertiesStr, MAKE_COPY_NO);

    DbResult status = stmt->Step();

    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    return ((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
}

bool SMSQLiteFile::GetProperties(Json::Value& properties)
{
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Properties  FROM SMFileMetadata");
    DbResult status = stmt->Step();

    Utf8String propertiesUtf8 = GET_VALUE_STR(stmt, 0);
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW)); 

    Json::Reader reader;
    reader.parse(propertiesUtf8, properties);

    return !propertiesUtf8.empty();
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


void SMSQLiteFile::GetPointsAndIndices(int64_t nodeID, bvector<uint8_t>& pts, size_t& uncompressedSizePts, bvector<uint8_t>& indices, size_t& uncompressedSizeIndices)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;    
    m_database->GetCachedStatement(stmt, "SELECT PointData, length(PointData), SizePts, IndexData, length(IndexData), SizeIndices FROM SMPoint WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
   // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW)
        {
        uncompressedSizePts = 0;
        uncompressedSizeIndices = 0;
        return;
        }

    pts.resize(stmt->GetValueInt64(1));
    uncompressedSizePts = stmt->GetValueInt64(2);
    if(pts.size() > 0) memcpy(&pts[0], stmt->GetValueBlob(0), pts.size());

    indices.resize(stmt->GetValueInt64(4));
    uncompressedSizeIndices = stmt->GetValueInt64(5);
    if(indices.size() > 0) memcpy(&indices[0], stmt->GetValueBlob(3), indices.size());

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


void SMSQLiteFile::GetUVIndices(int64_t nodeID, bvector<uint8_t>& uvCoords, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT UVIndexData, length(UVIndexData), SizeUVIndex FROM SMUVs WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    uvCoords.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&uvCoords[0], stmt->GetValueBlob(0), uvCoords.size());
    }

void SMSQLiteFile::GetTexture(int64_t nodeID, bvector<uint8_t>& texture, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT TexData, length(TexData), SizeTexture FROM SMTexture WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE || stmt->GetValueInt64(1) == 0)
        {
        uncompressedSize = 0;
        return;
        }
    texture.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&texture[0], stmt->GetValueBlob(0), texture.size());
    }

void SMSQLiteFile::GetUVs(int64_t nodeID, bvector<uint8_t>& uvCoords, size_t& uncompressedSize)
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
    uvCoords.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&uvCoords[0], stmt->GetValueBlob(0), uvCoords.size());
    }



#ifdef WIP_MESH_IMPORT
void SMSQLiteFile::GetMeshParts(int64_t nodeID, bvector<uint8_t>& data, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Data, length(Data), Size FROM SMMeshParts WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    data.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&data[0], stmt->GetValueBlob(0), data.size());
    }

void SMSQLiteFile::GetMetadata(int64_t nodeID, bvector<uint8_t>& metadata, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Data, length(Data), Size FROM SMMeshMetadata WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    metadata.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&metadata[0], stmt->GetValueBlob(0), metadata.size());
}
#endif

uint64_t SMSQLiteFile::GetLastNodeId()
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT NodeId FROM SMNodeHeader ORDER BY NodeId DESC LIMIT 1");
    stmt->Step();
    auto numResults = stmt->GetColumnCount();

    auto lastID = numResults > 0 ? stmt->GetValueInt64(0) : 0;
    return lastID;
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
        stmt->ClearBindings();
        }
    else if (nodeID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMPoint (PointData, IndexData, SizePts, SizeIndices) VALUES(?,?,?,?)");
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
         CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);
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

void SMSQLiteFile::StoreUVIndices(int64_t& nodeID, const bvector<uint8_t>& uvCoords, size_t uncompressedSize)
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
        m_database->GetCachedStatement(stmt, "INSERT INTO SMUVs (NodeId,UVData,UVIndexData, SizeUVs, SizeUVIndex) VALUES(?,?,?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, nullptr, 0, MAKE_COPY_NO);
        stmt->BindBlob(3, &uvCoords[0], (int)uvCoords.size(), MAKE_COPY_NO);
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
        m_database->GetCachedStatement(stmt, "UPDATE SMUVs SET UVIndexData=?, SizeUVIndex=? WHERE NodeId=?");
        stmt->BindBlob(1, &uvCoords[0], (int)uvCoords.size(), MAKE_COPY_NO);
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
        m_database->GetCachedStatement(stmt, "INSERT INTO SMTexture (NodeId,TexData, SizeTexture) VALUES(?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, &texture[0], (int)texture.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
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

void SMSQLiteFile::StoreUVs(int64_t& nodeID, const bvector<uint8_t>& uvCoords, size_t uncompressedSize)
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
        m_database->GetCachedStatement(stmt, "INSERT INTO SMUVs (NodeId,UVData,UVIndexData,SizeUVs, SizeUVIndex) VALUES(?,?,?,?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, &uvCoords[0], (int)uvCoords.size(), MAKE_COPY_NO);
        stmt->BindBlob(3, nullptr, 0, MAKE_COPY_NO);
        stmt->BindInt64(4, uncompressedSize);
        stmt->BindInt64(5, 0);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMUVs SET UvData=?, SizeUVs=? WHERE NodeId=?");
        stmt->BindBlob(1, &uvCoords[0], (int)uvCoords.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }






#ifdef WIP_MESH_IMPORT
void SMSQLiteFile::StoreMeshParts(int64_t& nodeID, const bvector<uint8_t>& data, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt; 
    size_t nRows = 0;
    if (nodeID != SQLiteNodeHeader::NO_NODEID)
        {
        CachedStatementPtr stmt3;
        m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMMeshParts WHERE NodeId=?");
        stmt3->BindInt64(1, nodeID);
        stmt3->Step();
        nRows = stmt3->GetValueInt64(0);
        }
    if (nodeID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMeshParts (Data,Size) VALUES(?,?)");
        stmt->BindBlob(1, &data[0], (int)data.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);
        }
    else if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMeshParts (NodeId, Data,Size) VALUES(?, ?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, &data[0], (int)data.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        m_database->SaveChanges();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMMeshParts SET Data=?, Size=? WHERE NodeId=?");
        stmt->BindBlob(1, &data[0], (int)data.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
    }

void SMSQLiteFile::StoreMetadata(int64_t& nodeID, const bvector<uint8_t>& metadata, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    size_t nRows = 0;
    if (nodeID != SQLiteNodeHeader::NO_NODEID)
        {
        CachedStatementPtr stmt3;
        m_database->GetCachedStatement(stmt3, "SELECT COUNT(NodeId) FROM SMMeshMetadata WHERE NodeId=?");
        stmt3->BindInt64(1, nodeID);
        stmt3->Step();
        nRows = stmt3->GetValueInt64(0);
        }
    if (nodeID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMeshMetadata (Data,Size) VALUES(?,?)");
        stmt->BindBlob(1, &metadata[0], (int)metadata.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        nodeID = stmt2->GetValueInt64(0);
        }
    else if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMeshMetadata (NodeId, Data,Size) VALUES(?, ?,?)");
        stmt->BindInt64(1, nodeID);
        stmt->BindBlob(2, &metadata[0], (int)metadata.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        m_database->SaveChanges();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMMeshMetadata SET Data=?, Size=? WHERE NodeId=?");
        stmt->BindBlob(1, &metadata[0], (int)metadata.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, nodeID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        }
}
#endif

size_t SMSQLiteFile::GetNumberOfPoints(int64_t nodeID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    DbResult rc = m_database->GetCachedStatement(stmt, "SELECT SizePts FROM SMPoint WHERE NodeId=?");
    if (rc != BE_SQLITE_OK)
        {
        assert(!"Can't get number of points");
        return 0;
        }
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    //assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetNumberOfIndices(int64_t nodeID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    DbResult rc = m_database->GetCachedStatement(stmt, "SELECT SizeIndices FROM SMPoint WHERE NodeId=?");
    if (rc != BE_SQLITE_OK)
        {
        assert(!"Can't get number of indices");
        return 0;
        }
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
   // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetNumberOfUVIndices(int64_t nodeID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    DbResult rc = m_database->GetCachedStatement(stmt, "SELECT SizeUVIndex FROM SMUVs WHERE NodeId=?");
    if (rc != BE_SQLITE_OK)
        {
        assert(!"Can't get number of UV indices");
        return 0;
        }
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetTextureByteCount(int64_t nodeID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SizeTexture FROM SMTexture WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetTextureCompressedByteCount(int64_t nodeID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT length(TexData) FROM SMTexture WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE || stmt->GetValueInt64(0) == 0)
        {
        return 0;
        }
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteFile::GetNumberOfUVs(int64_t nodeID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT SizeUVs FROM SMUVs WHERE NodeId=?");
    stmt->BindInt64(1, nodeID);
    DbResult status = stmt->Step();
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }


#ifdef WIP_MESH_IMPORT
size_t SMSQLiteFile::CountTextures()
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT MAX(_ROWID_) FROM SMTexture LIMIT 1"); 
    DbResult status = stmt->Step();
    assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));
    int texCount = stmt->GetValueInt(0);
    return (size_t) texCount;
    }

size_t SMSQLiteFile::GetNumberOfMeshParts(int64_t nodeId)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMMeshParts WHERE NodeId=?");
    stmt->BindInt64(1, nodeId);
    DbResult status = stmt->Step();
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
}

size_t SMSQLiteFile::GetNumberOfMetadataChars(int64_t nodeId)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMMeshMetadata WHERE NodeId=?");
    stmt->BindInt64(1, nodeId);
    DbResult status = stmt->Step();
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }
#endif




    bool SMSQLiteFile::SetWkt(WCharCP extendedWkt)
{

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
        
        m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, GCS, RootNodeId) VALUES(?,?,?)");

    }
    else
    {
        
        m_database->GetCachedStatement(stmt, "UPDATE SMMasterHeader SET MasterHeaderId=?, GCS=? WHERE MasterHeaderID=?");
    }
    stmt->BindInt64(1, id);
    BIND_VALUE_STR(stmt, 2, extendedWktUtf8String, MAKE_COPY_NO);
    if (nRows != 0)
        stmt->BindInt64(3, id);
    else
        stmt->BindInt64(3, SQLiteNodeHeader::NO_NODEID);

    DbResult status = stmt->Step();
  
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
    m_database->GetCachedStatement(stmt, "SELECT MAX(_ROWID_) FROM SMPoint LIMIT 1"); //select count() is not optimized on sqlite
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
    Savepoint s(*m_database, "sources");
    s.Begin();
    std::vector<SourceDataSQLite> vecSourceDataSQLite = sourcesData.GetSourceDataSQLite();

    for (SourceDataSQLite& sourceData : vecSourceDataSQLite)
    {
        Utf8String extendedWktUtf8String;
        BeStringUtilities::WCharToUtf8(extendedWktUtf8String, sourceData.GetGCS().GetWCharCP());


        ScalableMeshData smData = sourceData.GetScalableMeshData();

        Utf8String modelNameUtf8String;
        BeStringUtilities::WCharToUtf8(modelNameUtf8String, sourceData.GetModelName().GetWCharCP());

        Utf8String levelNameUtf8String;
        BeStringUtilities::WCharToUtf8(levelNameUtf8String, sourceData.GetLevelName().GetWCharCP());

        Utf8String rootToRefPersistentPathUtf8String;
        BeStringUtilities::WCharToUtf8(rootToRefPersistentPathUtf8String, sourceData.GetRootToRefPersistentPath().GetWCharCP());
       
        Utf8String referenceNameUtf8String;
        BeStringUtilities::WCharToUtf8(referenceNameUtf8String, sourceData.GetReferenceName().GetWCharCP());
   
        Utf8String referenceModelNameUtf8String;
        BeStringUtilities::WCharToUtf8(referenceModelNameUtf8String, sourceData.GetReferenceModelName().GetWCharCP());

        Utf8String ElevationPropertyNameUtf8String;
        BeStringUtilities::WCharToUtf8(ElevationPropertyNameUtf8String, smData.ElevationPropertyName().GetWCharCP());

        Utf8String monikerStringUtf8String;
        BeStringUtilities::WCharToUtf8(monikerStringUtf8String, sourceData.GetMonikerString().GetWCharCP());

        CachedStatementPtr stmt;
        //Savepoint insertTransaction(*m_database, "replace");
        m_database->GetCachedStatement(stmt, "REPLACE INTO SMSources (SourceId, SourceType, DTMSourceID, GroupID, ModelId, ModelName, LevelId, LevelName, RootToRefPersistentPath, "
            "ReferenceName, ReferenceModelName, GCS, Flags, TypeFamilyID, TypeID, MonikerType, MonikerString, TimeLastModified, "
            "SizeExtent, Extent, UpToDateState, Time, IsRepresenting3dData, IsGroundDetection, IsGISData, ElevationProperty, LinearFeatureType, PolygonFeatureType, IsGridData)"
            "VALUES(?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?,?,"
            "?,?,?,?)");
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
        stmt->BindInt64(15, sourceData.GetTypeID());


        stmt->BindInt(16, sourceData.GetMonikerType());
        BIND_VALUE_STR(stmt, 17, monikerStringUtf8String, MAKE_COPY_NO);


        stmt->BindInt64(18, sourceData.GetTimeLastModified());
        stmt->BindInt64(19, smData.GetExtent().size());

        {
            stmt->BindBlob(20, &smData.GetExtent()[0], sizeof(DRange3d)*(int)smData.GetExtent().size(), MAKE_COPY_YES);
        }
        stmt->BindInt(21, smData.GetUpToDateState());
        stmt->BindInt64(22, smData.GetTimeFile());
        stmt->BindInt(23, (int)smData.IsRepresenting3dData());
        stmt->BindInt(24, smData.IsGroundDetection() ? 1 : 0);
        stmt->BindInt(25, smData.IsGISDataType() ? 1 : 0);
        BIND_VALUE_STR(stmt, 26, ElevationPropertyNameUtf8String, MAKE_COPY_NO);
        stmt->BindInt(27, (int)smData.GetLinearFeatureType());
        stmt->BindInt(28, (int)smData.GetPolygonFeatureType());
        stmt->BindInt(29, smData.IsGridData() ? 1 : 0);
        DbResult status = stmt->Step();
        assert((status == BE_SQLITE_DONE) || (status == BE_SQLITE_ROW));


        for (auto& cmdData : sourceData.GetOrderedCommands())
            {
            CachedStatementPtr stmtTest;
            m_database->GetCachedStatement(stmtTest, "SELECT COUNT(CommandID) FROM SMImportSequences WHERE SourceID=? AND CommandPosition=?");
            int pos = &cmdData - &sourceData.GetOrderedCommands().front();
            stmtTest->BindInt64(1, sourceData.GetSourceID());
            stmtTest->BindInt(2, pos);
            stmtTest->Step();
            size_t nRows = stmtTest->GetValueInt64(0);
            CachedStatementPtr stmtSeq;
            if (nRows > 0)
                {
                m_database->GetCachedStatement(stmtSeq, "UPDATE SMImportSequences SET SourceLayer=?, TargetLayer=?, SourceType=?, TargetType=? WHERE SourceID=? AND CommandPosition=?");
                }
            else
                {
                m_database->GetCachedStatement(stmtSeq, "INSERT INTO SMImportSequences (SourceLayer,TargetLayer,SourceType,TargetType,SourceID,CommandPosition) VALUES (?,?,?,?,?,?)");
                }
            if (cmdData.sourceLayerSet) stmtSeq->BindInt(1, cmdData.sourceLayerID);
            if (cmdData.targetLayerSet) stmtSeq->BindInt(2, cmdData.targetLayerID);
            if (cmdData.sourceTypeSet) stmtSeq->BindInt(3, cmdData.sourceTypeID);
            if (cmdData.targetTypeSet) stmtSeq->BindInt(4, cmdData.targetTypeID);
            stmtSeq->BindInt64(5, sourceData.GetSourceID());
            stmtSeq->BindInt(6, pos);
            stmtSeq->Step();
            }
        s.Save("newSource");
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

        m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, LastModifiedTime, LastSyncTime, CheckTime,"
            "RootNodeId) VALUES(?,?,?,?,?)");

    }
    else
    {

        m_database->GetCachedStatement(stmt, "UPDATE SMMasterHeader SET MasterHeaderId=?, LastModifiedTime=?, LastSyncTime=?, CheckTime=?"
            " WHERE MasterHeaderId=?");

    }
    
    stmt->BindInt64(1, id);
    stmt->BindInt64(2, sourcesData.GetLastModifiedCheckTime());
    stmt->BindInt64(3, sourcesData.GetLastModifiedTime());
    stmt->BindInt64(4, sourcesData.GetLastSyncTime());

    if (nRows != 0)
        stmt->BindInt64(5, id);
    else
        stmt->BindInt64(5, SQLiteNodeHeader::NO_NODEID);
    DbResult status = stmt->Step();
    s.Commit();
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
        "ReferenceName, ReferenceModelName, GCS, Flags, TypeFamilyID, TypeID, MonikerType, MonikerString, TimeLastModified, "
        "SizeExtent, Extent, UpToDateState, Time, IsRepresenting3dData, IsGroundDetection, IsGISData, ElevationProperty, LinearFeatureType, PolygonFeatureType, IsGridData "
        "FROM SMSources");
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
        sourceData.SetTypeID(stmt->GetValueInt64(14));


        sourceData.SetMonikerType(stmt->GetValueInt(15));
        sourceData.SetMonikerString(WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 16)).c_str()));


        sourceData.SetTimeLastModified(stmt->GetValueInt64(17));

        ScalableMeshData smData(ScalableMeshData::GetNull());
        size_t nLayer = stmt->GetValueInt64(18);

        {
            std::vector<DRange3d> extents(nLayer);
            std::memcpy(&extents[0], stmt->GetValueBlob(19), sizeof(DRange3d)*(int)nLayer);
            smData.SetExtents(extents);
        }
        smData.SetUpToDateState(UpToDateState(stmt->GetValueInt(20)));
        smData.SetTimeFile(stmt->GetValueInt64(21));
        smData.SetRepresenting3dData(SMis3D(stmt->GetValueInt(22)));
        smData.SetIsGroundDetection(stmt->GetValueInt(23) ? true : false);
        smData.SetIsGISDataType(stmt->GetValueInt(24) ? true : false);
        WString tmp = WSTRING_FROM_CSTR(Utf8String(GET_VALUE_STR(stmt, 25)).c_str());
        smData.SetElevationPropertyName(tmp);
        smData.SetLinearFeatureType(DTMFeatureType(stmt->GetValueInt(26)));
        smData.SetPolygonFeatureType(DTMFeatureType(stmt->GetValueInt(27)));
        smData.SetIsGridData(stmt->GetValueInt(28) ? true : false);
        sourceData.SetScalableMeshData(smData);
      

        CachedStatementPtr stmtSequence;
        bvector<ImportCommandData> sequenceData;
        m_database->GetCachedStatement(stmtSequence, "SELECT SourceLayer, TargetLayer, SourceType, TargetType FROM SMImportSequences WHERE SourceID=? ORDER BY CommandPosition ASC");
        stmtSequence->BindInt64(1, sourceData.GetSourceID());
        while (stmtSequence->Step() == BE_SQLITE_ROW)
            {
            ImportCommandData data;
            if (!stmt->IsColumnNull(0))
                {
                data.sourceLayerID = stmt->GetValueInt(0);
                data.sourceLayerSet = true;
                }
            if (!stmt->IsColumnNull(1))
                {
                data.targetLayerID = stmt->GetValueInt(1);
                data.targetLayerSet = true;
                }
            if (!stmt->IsColumnNull(2))
                {
                data.sourceTypeID = stmt->GetValueInt(2);
                data.sourceTypeSet = true;
                }
            if (!stmt->IsColumnNull(3))
                {
                data.targetTypeID = stmt->GetValueInt(3);
                data.targetTypeSet = true;
                }
            sequenceData.push_back(data);
            }
        sourceData.SetOrderedCommands(sequenceData);
        sourcesData.AddSourcesNode(sourceData);
    }

    CachedStatementPtr stmt2;
    m_database->GetCachedStatement(stmt2, "SELECT LastModifiedTime, LastSyncTime, CheckTime"
        " FROM SMMasterHeader WHERE MasterHEaderId=?");
    size_t id = 0;
    stmt2->BindInt64(1, id);
    stmt2->Step();

    sourcesData.SetLastModifiedCheckTime(stmt->GetValueInt64(0));
    sourcesData.SetLastModifiedTime(stmt->GetValueInt64(1));
    sourcesData.SetLastSyncTime(stmt->GetValueInt64(2));

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

