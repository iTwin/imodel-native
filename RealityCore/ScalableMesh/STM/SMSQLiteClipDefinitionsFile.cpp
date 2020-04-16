/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "SMSQLiteClipDefinitionsFile.h"


#define WSTRING_FROM_CSTR(cstr) WString(cstr, BentleyCharEncoding::Utf8)
#define MAKE_COPY_NO Statement::MakeCopy::No
#define MAKE_COPY_YES Statement::MakeCopy::Yes
#define GET_VALUE_STR(stmt, id) stmt->GetValueText(id)
#define BIND_VALUE_STR(stmt, id, utf8str, copyval) stmt->BindText(id, utf8str, copyval)
#define READONLY Db::OpenMode::Readonly
#define READWRITE Db::OpenMode::ReadWrite

const BESQL_VERSION_STRUCT SMSQLiteClipDefinitionsFile::CURRENT_VERSION = BESQL_VERSION_STRUCT(1, 1, 0, 4);

const BESQL_VERSION_STRUCT s_listOfReleasedSchemasClip[5] = { BESQL_VERSION_STRUCT(1, 1, 0, 0), BESQL_VERSION_STRUCT(1, 1, 0, 1), BESQL_VERSION_STRUCT(1, 1, 0, 2), BESQL_VERSION_STRUCT(1, 1, 0, 3), BESQL_VERSION_STRUCT(1, 1, 0, 4) };
const size_t s_numberOfReleasedSchemasClip = 5;
double s_expectedTimeUpdateClip[4] = { 1.2*1e-5, 1e-6,1e-6,1e-6 };
std::function<void(BeSQLite::Db*)> s_databaseUpdateFunctionsClip[4] = {
    [](BeSQLite::Db* database)
        {
        database->DropTable("SMNodeHeader");
        database->DropTable("SMTexture");
        database->DropTable("SMPoint");
        database->DropTable("SMUVs");
        database->DropTable("SMDiffSets");
        database->DropTable("SMFeatures");
        database->DropTable("SMMasterHeader");
        database->DropTable("SMGraph");
        
        },
            [] (BeSQLite::Db* database)
            {
            database->CreateTable("SMCoverages", "PolygonId INTEGER PRIMARY KEY,"
                                   "PolygonData BLOB,"
                                   "Size INTEGER");

            }
                ,
                [] (BeSQLite::Db* database)
                {
                database->ExecuteSql("ALTER TABLE SMClipDefinitions ADD COLUMN ClipType INTEGER DEFAULT 0");
                database->ExecuteSql("ALTER TABLE SMClipDefinitions ADD COLUMN OnOff INTEGER DEFAULT 1");
                database->ExecuteSql("ALTER TABLE SMClipDefinitions ADD COLUMN ClipGeometryType INTEGER DEFAULT 0");
                }
            ,
            [](BeSQLite::Db* database)
            {
                database->ExecuteSql("ALTER TABLE SMCoverages ADD COLUMN Name STRING DEFAULT ''");
            }

    };

size_t SMSQLiteClipDefinitionsFile::GetNumberOfReleasedSchemas() { return s_numberOfReleasedSchemasClip; }
const BESQL_VERSION_STRUCT* SMSQLiteClipDefinitionsFile::GetListOfReleasedVersions() { return s_listOfReleasedSchemasClip; }
double* SMSQLiteClipDefinitionsFile::GetExpectedTimesForUpdateFunctions() { return s_expectedTimeUpdateClip; }
std::function<void(BeSQLite::Db*)>* SMSQLiteClipDefinitionsFile::GetFunctionsForAutomaticUpdate() { return s_databaseUpdateFunctionsClip; }

void SMSQLiteClipDefinitionsFile::StoreClipPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
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
        m_database->GetCachedStatement(stmt, "INSERT INTO SMClipDefinitions (PolygonData,Size, Importance, NDimensions, ClipType, OnOff, ClipGeometryType) VALUES(?,?, ?, ?, ?, ?, ?)");
        stmt->BindBlob(1, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindDouble(3, 0);
        stmt->BindInt(4, 0);
        stmt->BindInt(5, (int)type);
        stmt->BindInt(6, isActive? 1 : 0);
        stmt->BindInt(7, (int)geom);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        clipID = stmt2->GetValueInt64(0);
        if (m_autocommit) m_database->SaveChanges();
        }
    else if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMClipDefinitions (PolygonId, PolygonData,Size, Importance, NDimensions, ClipType, OnOff, ClipGeometryType) VALUES(?, ?,?,?,?,?,?,?)");
        stmt->BindInt64(1, clipID);
        stmt->BindBlob(2, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        stmt->BindDouble(4, 0);
        stmt->BindInt(5, 0);
        stmt->BindInt(6, (int)type);
        stmt->BindInt(7, isActive ? 1 : 0);
        stmt->BindInt(8, (int)geom);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMClipDefinitions SET PolygonData=?, Size=?, ClipType=?, OnOff=?, ClipGeometryType=? WHERE PolygonId=?");
        stmt->BindBlob(1, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt(3, (int)type);
        stmt->BindInt(4, isActive ? 1 : 0);
        stmt->BindInt(5, (int)geom);
        stmt->BindInt64(6, clipID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    }

void SMSQLiteClipDefinitionsFile::SetClipPolygonMetadata(uint64_t& clipID, double importance, int nDimensions)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(PolygonId) FROM SMClipDefinitions WHERE PolygonId=?");
    stmt3->BindInt64(1, clipID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);

    if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMClipDefinitions (PolygonId, PolygonData,Size, Importance, NDimensions) VALUES(?, ?,?,?,?)");
        stmt->BindInt64(1, clipID);
        stmt->BindBlob(2, 0, 0, MAKE_COPY_NO);
        stmt->BindInt64(3, 0);
        stmt->BindDouble(4, importance);
        stmt->BindInt(5, nDimensions);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMClipDefinitions SET Importance=?, NDimensions=? WHERE PolygonId=?");
        stmt->BindDouble(1, importance);
        stmt->BindInt(2, nDimensions);
        stmt->BindInt64(3, clipID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    }

void SMSQLiteClipDefinitionsFile::GetClipPolygonMetadata(uint64_t clipID, double& importance, int& nDimensions)
    {
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Importance, NDimensions FROM SMClipDefinitions WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    DbResult status = stmt->Step();

    if (status != BE_SQLITE_ROW) return;
    importance = stmt->GetValueDouble(0);
    nDimensions = stmt->GetValueInt(1);
    }

void SMSQLiteClipDefinitionsFile::StoreSkirtPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(PolygonId) FROM SMSkirts WHERE PolygonId=?");
    stmt3->BindInt64(1, clipID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (clipID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMSkirts (PolygonData,Size) VALUES(?,?)");
        stmt->BindBlob(1, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        clipID = stmt2->GetValueInt64(0);
        if (m_autocommit) m_database->SaveChanges();
        }
    else if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMSkirts (PolygonId, PolygonData,Size) VALUES(?, ?,?)");
        stmt->BindInt64(1, clipID);
        stmt->BindBlob(2, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMSkirts SET PolygonData=?, Size=? WHERE PolygonId=?");
        stmt->BindBlob(1, &clipData[0], (int)clipData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, clipID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    }

size_t SMSQLiteClipDefinitionsFile::GetClipPolygonByteCount(int64_t clipID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMClipDefinitions WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    DbResult status = stmt->Step();
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteClipDefinitionsFile::GetSkirtPolygonByteCount(int64_t clipID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMSkirts WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    DbResult status = stmt->Step();
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

void SMSQLiteClipDefinitionsFile::GetClipPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PolygonData, length(PolygonData), Size, ClipType, OnOff, ClipGeometryType FROM SMClipDefinitions WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    DbResult status = stmt->Step();

    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    clipData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    type = (SMNonDestructiveClipType)stmt->GetValueInt(3);
    isActive = stmt->GetValueInt(4) > 0;
    geom = (SMClipGeometryType)stmt->GetValueInt(5);
    memcpy(&clipData[0], stmt->GetValueBlob(0), clipData.size());
    }

void SMSQLiteClipDefinitionsFile::GetSkirtPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PolygonData, length(PolygonData), Size FROM SMSkirts WHERE PolygonId=?");
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


void SMSQLiteClipDefinitionsFile::GetCoverageName(int64_t coverageID, Utf8String* name, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Name FROM SMCoverages WHERE PolygonId=?");
    stmt->BindInt64(1, coverageID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }

    *name = GET_VALUE_STR(stmt, 0);
    uncompressedSize = name->SizeInBytes();        
    }


void SMSQLiteClipDefinitionsFile::GetCoveragePolygon(int64_t coverageID, bvector<uint8_t>& coverageData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PolygonData, length(PolygonData), Size FROM SMCoverages WHERE PolygonId=?");
    stmt->BindInt64(1, coverageID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    coverageData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&coverageData[0], stmt->GetValueBlob(0), coverageData.size());
    }

void SMSQLiteClipDefinitionsFile::StoreCoveragePolygon(int64_t& coverageID, const bvector<uint8_t>& coverageData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(PolygonId) FROM SMCoverages WHERE PolygonId=?");
    stmt3->BindInt64(1, coverageID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (coverageID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMCoverages (PolygonData,Size) VALUES(?,?)");
        stmt->BindBlob(1, &coverageData[0], (int)coverageData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        coverageID = stmt2->GetValueInt64(0);
        if (m_autocommit) m_database->SaveChanges();
        }
    else if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMCoverages (PolygonId, PolygonData,Size) VALUES(?, ?,?)");
        stmt->BindInt64(1, coverageID);
        stmt->BindBlob(2, &coverageData[0], (int)coverageData.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMCoverages SET PolygonData=?, Size=? WHERE PolygonId=?");
        stmt->BindBlob(1, &coverageData[0], (int)coverageData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, coverageID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
        }
    }


void SMSQLiteClipDefinitionsFile::StoreCoverageName(int64_t& coverageID, Utf8String& coverageName, size_t uncompressedSize)
{
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    CachedStatementPtr stmt3;
    m_database->GetCachedStatement(stmt3, "SELECT COUNT(PolygonId) FROM SMCoverages WHERE PolygonId=?");
    stmt3->BindInt64(1, coverageID);
    stmt3->Step();
    size_t nRows = stmt3->GetValueInt64(0);
    if (coverageID == SQLiteNodeHeader::NO_NODEID)
    {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMCoverages (Name) VALUES(?)");        
        BIND_VALUE_STR(stmt, 1, coverageName, MAKE_COPY_NO);        
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        coverageID = stmt2->GetValueInt64(0);
        if (m_autocommit) m_database->SaveChanges();
    }
    else if (nRows == 0)
    {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMCoverages (Name) VALUES(?)");
        BIND_VALUE_STR(stmt, 1, coverageName, MAKE_COPY_NO);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
    }
    else
    {
        m_database->GetCachedStatement(stmt, "UPDATE SMCoverages SET Name=? WHERE PolygonId=?");
        BIND_VALUE_STR(stmt, 1, coverageName, MAKE_COPY_NO);
        stmt->BindInt64(2, coverageID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        if (m_autocommit) m_database->SaveChanges();
    }
}

size_t SMSQLiteClipDefinitionsFile::GetCoveragePolygonByteCount(int64_t coverageID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Size FROM SMCoverages WHERE PolygonId=?");
    stmt->BindInt64(1, coverageID);
    DbResult status = stmt->Step();
    if (status != BE_SQLITE_ROW) return 0;
    return stmt->GetValueInt64(0);
    }

size_t SMSQLiteClipDefinitionsFile::GetCoverageNameByteCount(int64_t coverageID)
    {
    Utf8String name;
    size_t     uncompressedSize;
    GetCoverageName(coverageID, &name, uncompressedSize);
    return uncompressedSize;    
    }

void SMSQLiteClipDefinitionsFile::DeleteClipPolygon(int64_t clipID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "DELETE FROM SMClipDefinitions WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    stmt->Step();
    }

void SMSQLiteClipDefinitionsFile::DeleteSkirtPolygon(int64_t clipID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "DELETE FROM SMSkirts WHERE PolygonId=?");
    stmt->BindInt64(1, clipID);
    stmt->Step();
    }

void SMSQLiteClipDefinitionsFile::DeleteCoveragePolygon(int64_t coverageID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "DELETE FROM SMCoverages WHERE PolygonId=?");
    stmt->BindInt64(1, coverageID);
    stmt->Step();
    }

void SMSQLiteClipDefinitionsFile::GetAllPolys(bvector<bvector<uint8_t>>& polys, bvector<size_t>& sizes)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PolygonData, length(PolygonData), Size FROM SMCoverages");
    DbResult status;
    do
        {
        status = stmt->Step();
        // assert(status == BE_SQLITE_ROW);
        if (status == BE_SQLITE_DONE)
            {
            return;
            }
        bvector<uint8_t> coverageData;
        size_t uncompressedSize;
        coverageData.resize(stmt->GetValueInt64(1));
        uncompressedSize = stmt->GetValueInt64(2);
        memcpy(&coverageData[0], stmt->GetValueBlob(0), coverageData.size());
        polys.push_back(coverageData);
        sizes.push_back(uncompressedSize);
        }
    while (status == BE_SQLITE_ROW);
    }

DbResult SMSQLiteClipDefinitionsFile::CreateTables()
    {
    DbResult result;

    result = m_database->CreateTable("SMClipDefinitions", "PolygonId INTEGER PRIMARY KEY,"
                                     "PolygonData BLOB,"
                                     "Size INTEGER,"
                                     "Importance DOUBLE,"
                                     "NDimensions INTEGER,"
                                     "ClipType    INTEGER,"
                                     "OnOff       INTEGER,"
                                     "ClipGeometryType INTEGER");
                                     
    assert(result == BE_SQLITE_OK);

    result = m_database->CreateTable("SMSkirts", "PolygonId INTEGER PRIMARY KEY,"
                                     "PolygonData BLOB,"
                                     "Size INTEGER");

    assert(result == BE_SQLITE_OK);

    result = m_database->CreateTable("SMCoverages", "PolygonId INTEGER PRIMARY KEY,"
                                     "PolygonData BLOB,"
                                     "Size INTEGER,"
                                     "Name STRING");


    assert(result == BE_SQLITE_OK);

    return result;
    }


void SMSQLiteClipDefinitionsFile::GetAllCoverageIDs(bvector<uint64_t>& ids)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PolygonId FROM SMCoverages");
    DbResult status = stmt->Step();
    while (status == BE_SQLITE_ROW)
        {
        ids.push_back(stmt->GetValueInt64(0));
        status = stmt->Step();
        }
    }

void SMSQLiteClipDefinitionsFile::GetAllClipIDs(bvector<uint64_t>& allIds)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT PolygonId FROM SMClipDefinitions");
    DbResult status = stmt->Step();
    while (status == BE_SQLITE_ROW)
        {
        allIds.push_back(stmt->GetValueInt64(0));
        status = stmt->Step();
        }
    }

void SMSQLiteClipDefinitionsFile::GetIsClipActive(uint64_t id, bool& isActive)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT OnOff FROM SMClipDefinitions where PolygonId=?");
    stmt->BindInt64(1, id);
    DbResult status = stmt->Step();
    if (status == BE_SQLITE_ROW)
        isActive = stmt->GetValueInt(0) > 0;
    }

void SMSQLiteClipDefinitionsFile::GetClipType(uint64_t id, SMNonDestructiveClipType& type)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT ClipType FROM SMClipDefinitions where PolygonId=?");
    stmt->BindInt64(1, id);
    DbResult status = stmt->Step();
    if (status == BE_SQLITE_ROW)
        type = (SMNonDestructiveClipType)stmt->GetValueInt(0);
    }

void SMSQLiteClipDefinitionsFile::SetClipOnOrOff(uint64_t id, bool isActive)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "UPDATE SMClipDefinitions SET OnOff=? where PolygonId=?");
    stmt->BindInt64(1, isActive? 1 : 0);
    stmt->BindInt64(2, id);
    stmt->Step();
    }
