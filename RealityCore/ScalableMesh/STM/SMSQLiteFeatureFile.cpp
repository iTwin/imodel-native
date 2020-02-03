/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "SMSQLiteFeatureFile.h"


#define WSTRING_FROM_CSTR(cstr) WString(cstr, BentleyCharEncoding::Utf8)
#define MAKE_COPY_NO Statement::MakeCopy::No
#define MAKE_COPY_YES Statement::MakeCopy::Yes
#define GET_VALUE_STR(stmt, id) stmt->GetValueText(id)
#define BIND_VALUE_STR(stmt, id, utf8str, copyval) stmt->BindText(id, utf8str, copyval)
#define READONLY Db::OpenMode::Readonly
#define READWRITE Db::OpenMode::ReadWrite

const BESQL_VERSION_STRUCT SMSQLiteFeatureFile::CURRENT_VERSION = BESQL_VERSION_STRUCT(1, 1, 0, 2);

const BESQL_VERSION_STRUCT s_listOfReleasedSchemasFeature[3] = { BESQL_VERSION_STRUCT(1, 1, 0, 0), BESQL_VERSION_STRUCT(1, 1, 0, 1), BESQL_VERSION_STRUCT(1, 1, 0, 2) };
const size_t s_numberOfReleasedSchemasFeature = 3;
double s_expectedTimeUpdateFeature[1] = { 1.2*1e-5 };
std::function<void(BeSQLite::Db*)> s_databaseUpdateFunctionsFeature[2] = {
    [](BeSQLite::Db* database)
        {
        database->DropTable("SMNodeHeader");
        database->DropTable("SMTexture");
        database->DropTable("SMPoint");
        database->DropTable("SMUVs");
        database->DropTable("SMDiffSets");
        database->DropTable("SMMasterHeader");
        database->DropTable("SMSkirts");
        database->DropTable("SMClipDefinitions");

        },
    [] (BeSQLite::Db* database)
        {
        DbResult result;
        result = database->CreateTable("SMFeatureList", 
                                       "FeatureId INTEGER PRIMARY KEY,"
                                       "FeatureData BLOB,"
                                       "Size INTEGER");

        assert(result == BE_SQLITE_OK);
        }
    };
size_t SMSQLiteFeatureFile::GetNumberOfReleasedSchemas() { return s_numberOfReleasedSchemasFeature; }
const BESQL_VERSION_STRUCT* SMSQLiteFeatureFile::GetListOfReleasedVersions() { return s_listOfReleasedSchemasFeature; }
double* SMSQLiteFeatureFile::GetExpectedTimesForUpdateFunctions() { return s_expectedTimeUpdateFeature; }
std::function<void(BeSQLite::Db*)>* SMSQLiteFeatureFile::GetFunctionsForAutomaticUpdate() { return s_databaseUpdateFunctionsFeature; }

DbResult SMSQLiteFeatureFile::CreateTables()
    {
    DbResult result;


    result = m_database->CreateTable("SMGraph", "NodeId INTEGER PRIMARY KEY,"
                                     "Data BLOB,"
                                     "Size UNSIGNED INT");
    assert(result == BE_SQLITE_OK);

    result = m_database->CreateTable("SMFeatures", "FeatureId INTEGER PRIMARY KEY,"
                                     "FeatureData BLOB,"
                                     "Size UNSIGNED INT");

    assert(result == BE_SQLITE_OK);

    result = m_database->CreateTable("SMFeaturesList",
                                     "FeatureId INTEGER PRIMARY KEY,"
                                     "Type INTEGER,"
                                     "FeatureData BLOB,"
                                     "Size UNSIGNED INT");

    assert(result == BE_SQLITE_OK);
    return result;
    }

void SMSQLiteFeatureFile::StoreGraph(int64_t& nodeID, const bvector<uint8_t>& graph, size_t uncompressedSize)
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

void SMSQLiteFeatureFile::StoreFeature(int64_t& featureID, const bvector<uint8_t>& featureData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(FeatureId) FROM SMFeatures WHERE FeatureId=?");
    stmt3->BindInt64(1, featureID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);

    if (featureID == SQLiteNodeHeader::NO_NODEID || nRows == 0)
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

void SMSQLiteFeatureFile::GetFeatureDefinition(int64_t featureID, uint32_t& type, bvector<uint8_t>& featureData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT FeatureData, length(FeatureData), Size FROM SMFeaturesList WHERE FeatureId=?");
    stmt->BindInt64(1, featureID);
    DbResult status = stmt->Step();

    if(status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    featureData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&featureData[0], stmt->GetValueBlob(0), featureData.size());
    }

void SMSQLiteFeatureFile::StoreFeatureDefinition(int64_t& featureID, uint32_t type, const bvector<uint8_t>& featureData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;

    Savepoint insertTransaction(*m_database, "insert");
    m_database->GetCachedStatement(stmt, "INSERT INTO SMFeaturesList (Type,FeatureData,Size) VALUES(?,?,?)");
    stmt->BindInt(2, (int)type);
    stmt->BindBlob(3, &featureData[0], (int)featureData.size(), MAKE_COPY_NO);
    stmt->BindInt64(4, uncompressedSize);
    DbResult status = stmt->Step();
    assert(status == BE_SQLITE_DONE);
    stmt->ClearBindings();

    CachedStatementPtr stmt2;
    m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
    //m_database->GetCachedStatement(stmt2, "SELECT COUNT(FeatureId) FROM SMFeaturesList");
    status = stmt2->Step();
    featureID = stmt2->GetValueInt64(0);
    }

void SMSQLiteFeatureFile::GetGraph(int64_t nodeID, bvector<uint8_t>& graph, size_t& uncompressedSize)
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

void SMSQLiteFeatureFile::GetFeature(int64_t featureID, bvector<uint8_t>& featureData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT FeatureData, length(FeatureData), Size FROM SMFeatures WHERE FeatureId=?");
    stmt->BindInt64(1, featureID);
    DbResult status = stmt->Step();

    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    featureData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&featureData[0], stmt->GetValueBlob(0), featureData.size());
    }

size_t SMSQLiteFeatureFile::GetNumberOfFeaturePoints(int64_t featureID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMFeatures WHERE FeatureId=?");
    stmt->BindInt64(1, featureID);
    DbResult status = stmt->Step();
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0) / sizeof(int32_t);
    }
