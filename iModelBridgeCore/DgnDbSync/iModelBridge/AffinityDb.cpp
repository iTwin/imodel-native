/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult iModelBridgeAffinityDb::CreateSchema()
    {
    BeSQLite::DbResult res; 
    if (BeSQLite::BE_SQLITE_OK != (res = m_db.CreateTable("File", "id INTEGER PRIMARY KEY AUTOINCREMENT, Filename TEXT NOT NULL UNIQUE COLLATE NoCase"))
     || BeSQLite::BE_SQLITE_OK != (res = m_db.CreateTable("Model", "id INTEGER PRIMARY KEY AUTOINCREMENT, File INTEGER NOT NULL, SourceIdentifier TEXT, Description TEXT, JsonData TEXT, FOREIGN KEY(File) REFERENCES File(id), UNIQUE(File,SourceIdentifier)"))
     || BeSQLite::BE_SQLITE_OK != (res = m_db.CreateTable("Bridge", "id INTEGER PRIMARY KEY AUTOINCREMENT, Name TEXT NOT NULL UNIQUE COLLATE NoCase"))
     || BeSQLite::BE_SQLITE_OK != (res = m_db.CreateTable("Attachment", "Parent INTEGER NOT NULL, Child INTEGER NOT NULL, JsonData TEXT, FOREIGN KEY(Parent) REFERENCES Model(id), FOREIGN KEY(Child) REFERENCES Model(id), UNIQUE(Parent,Child)"))
     || BeSQLite::BE_SQLITE_OK != (res = m_db.CreateTable("Affinity", "File INTEGER NOT NULL, Bridge INTEGER NOT NULL, Level INTEGER NOT NULL, JsonData TEXT, FOREIGN KEY(File) REFERENCES File(id), FOREIGN KEY(Bridge) REFERENCES Bridge(id), UNIQUE(File,Bridge,Level)"))
     || BeSQLite::BE_SQLITE_OK != (res = m_db.SaveChanges())) // do this after changing the schema, to avoid performance problems when we prepare statements.
        {
        // The low-level code should have logged a detailed BeSQLite error
        LOG.errorv("%s - iModelBridgeAffinityDb::CreateSchema failed with %x", m_db.GetDbFileName(), res);
        BeAssert(false);
        }
    return res;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<iModelBridgeAffinityDb> iModelBridgeAffinityDb::Create(BeFileNameCR dbName)
    {
    if (BeFileName::DoesPathExist(dbName))
        return nullptr;

    auto db = new iModelBridgeAffinityDb();
    if (BeSQLite::BE_SQLITE_OK == db->m_db.CreateNewDb(dbName)
     && BeSQLite::BE_SQLITE_OK == db->CreateSchema())
        return db;

    delete db;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<iModelBridgeAffinityDb> iModelBridgeAffinityDb::Open(BeFileNameCR dbName)
    {
    auto db = new iModelBridgeAffinityDb();
    if (BeSQLite::BE_SQLITE_OK == db->m_db.OpenBeSQLiteDb(dbName, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::ReadWrite)))
        return db;

    delete db;
    return nullptr;;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t iModelBridgeAffinityDb::FindModel(Utf8StringP description, Utf8StringP jsonData, int64_t fileRowId, Utf8StringCR sourceIdentifier)
    {
    DbResult res;

    auto find = m_db.GetCachedStatement("select id, Description, JsonData from Model where File=? and SourceIdentifier=?");
    if (BeSQLite::BE_SQLITE_OK != (res = find->BindInt64(1, fileRowId))
     || BeSQLite::BE_SQLITE_OK != (res = find->BindText(2, sourceIdentifier, BeSQLite::Statement::MakeCopy::No)))
      {
      BeAssert(false);
      LOG.errorv("FindModel Bind failed with %x", res);
      return 0;
      }

    if (BeSQLite::BE_SQLITE_ROW != find->Step())
        return 0;
        
    auto id = find->GetValueInt64(0);
    if (description != nullptr)
        description->AssignOrClear(find->GetValueText(1));
    if (jsonData != nullptr)
        jsonData->AssignOrClear(find->GetValueText(2));

    return id;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t iModelBridgeAffinityDb::InsertModel(int64_t fileRowId, Utf8StringCR sourceIdentifier, Utf8StringCR description, JsonValueCP json)
    {
    DbResult res;
    auto insert = m_db.GetCachedStatement("insert into Model (File,SourceIdentifier,Description,JsonData) VALUES(?,?,?,?)");
    if (BeSQLite::BE_SQLITE_OK != (res = insert->BindInt64(1, fileRowId))
     || BeSQLite::BE_SQLITE_OK != (res = insert->BindText(2, sourceIdentifier, BeSQLite::Statement::MakeCopy::No))
     || BeSQLite::BE_SQLITE_OK != (res = insert->BindText(3, description, BeSQLite::Statement::MakeCopy::No))
     || BeSQLite::BE_SQLITE_OK != (res = (json ? insert->BindText(4, json->ToString(), BeSQLite::Statement::MakeCopy::Yes) : insert->BindNull(4))))
      {
      BeAssert(false);
      LOG.errorv("InsertModel Bind failed with %x", res);
      return 0;
      }

    if (BeSQLite::BE_SQLITE_DONE != insert->Step())
        return 0;

    m_db.SaveChanges(); // DEBUGGING PURPOSES ONLY
    return m_db.GetLastInsertRowId();
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t iModelBridgeAffinityDb::FindBridge(Utf8StringCR bridgeId)
    {
    DbResult res;

    auto find = m_db.GetCachedStatement("select id from Bridge where Name=?");
    if (BeSQLite::BE_SQLITE_OK != (res = find->BindText(1, bridgeId, BeSQLite::Statement::MakeCopy::No)))
      {
      BeAssert(false);
      LOG.errorv("FindBridge Bind failed with %x", res);
      return 0;
      }

    if (BeSQLite::BE_SQLITE_ROW == find->Step())
        return find->GetValueInt64(0);

    return 0;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t iModelBridgeAffinityDb::InsertBridge(Utf8StringCR bridgeId)
    {
    DbResult res;
    auto insert = m_db.GetCachedStatement("insert into Bridge (Name) VALUES(?)");
    insert->BindText(1, bridgeId, BeSQLite::Statement::MakeCopy::No);
    if (BeSQLite::BE_SQLITE_DONE != (res = insert->Step()))
        {
        LOG.errorv("InsertBridge failed with %x", res);
        BeAssert(false);
        return 0;
        }
    m_db.SaveChanges(); // DEBUGGING PURPOSES ONLY
    return m_db.GetLastInsertRowId();
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t iModelBridgeAffinityDb::FindFile(BeFileNameCR fileName)
    {
    DbResult res;

    auto find = m_db.GetCachedStatement("select id from File where Filename=?");
    if (BeSQLite::BE_SQLITE_OK != (res = find->BindText(1, Utf8String(fileName), BeSQLite::Statement::MakeCopy::Yes)))
      {
      BeAssert(false);
      LOG.errorv("FindFile Bind failed with %x", res);
      return BSIERROR;
      }
    if (BeSQLite::BE_SQLITE_ROW == find->Step())
        return find->GetValueInt64(0);

    return 0;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName iModelBridgeAffinityDb::GetFilename(int64_t fileRowId)
    {
    auto find = m_db.GetCachedStatement("select Filename from File where id=?");
    find->BindInt64(1, fileRowId);
    return (find->Step() == BE_SQLITE_ROW) ? BeFileName(find->GetValueText(0)) : BeFileName();
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t iModelBridgeAffinityDb::InsertFile(BeFileNameCR fileName)
    {
    DbResult res;

    auto insert = m_db.GetCachedStatement("insert into File (Filename) VALUES(?)");
    insert->BindText(1, Utf8String(fileName), BeSQLite::Statement::MakeCopy::Yes);
    if (BeSQLite::BE_SQLITE_DONE != (res = insert->Step()))
        {
        LOG.errorv("InsertFile failed with %x", res);
        BeAssert(false);
        return 0;
        }
    m_db.SaveChanges(); // DEBUGGING PURPOSES ONLY
    return m_db.GetLastInsertRowId();
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeAffinityDb::FindAttachment(Utf8StringP jsonData, int64_t parentModelRowId, int64_t childModelRowId)
    {
    auto find = m_db.GetCachedStatement("select JsonData from Attachment where Parent=? and Child=?");
    DbResult res;
    if (BeSQLite::BE_SQLITE_OK != (res = find->BindInt64(1, parentModelRowId))
     || BeSQLite::BE_SQLITE_OK != (res = find->BindInt64(2, childModelRowId)))
      {
      BeAssert(false);
      LOG.errorv("FindAttachment Bind failed with %x", res);
      return BSIERROR;
      }

    if (BeSQLite::BE_SQLITE_ROW != find->Step())
        return BSIERROR;

    if (jsonData != nullptr)
        jsonData->AssignOrClear(find->GetValueText(0));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeAffinityDb::InsertAttachment(int64_t parentModelRowId, int64_t childModelRowId, JsonValueCP json)
    {
    auto insert = m_db.GetCachedStatement("insert into Attachment (Parent, Child, JsonData) VALUES(?,?,?)");
    DbResult res;
    if (BeSQLite::BE_SQLITE_OK != (res = insert->BindInt64(1, parentModelRowId))
     || BeSQLite::BE_SQLITE_OK != (res = insert->BindInt64(2, childModelRowId))
     || BeSQLite::BE_SQLITE_OK != (res = (json ? insert->BindText(3, json->ToString(), BeSQLite::Statement::MakeCopy::Yes) : insert->BindNull(3))))
      {
      BeAssert(false);
      LOG.errorv("InsertAttachment Bind failed with %x", res);
      return BSIERROR;
      }

    if (BeSQLite::BE_SQLITE_DONE != (res = insert->Step()))
      {
      BeAssert(false);
      LOG.errorv("InsertAttachment Insert Step failed with %x", res);
      return BSIERROR;
      }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeAffinityDb::FindAffinity(iModelBridgeAffinityLevel* level, Utf8StringP jsonData, int64_t fileRowId, int64_t bridgeRowId)
    {
    auto find = m_db.GetCachedStatement("select Level, JsonData from Affinity where File=? and Bridge=?");

    DbResult res;
    if (BeSQLite::BE_SQLITE_OK != (res = find->BindInt64(1, fileRowId))
     || BeSQLite::BE_SQLITE_OK != (res = find->BindInt64(2, bridgeRowId)))
      {
      BeAssert(false);
      LOG.errorv("FindAffinity Bind failed with %x", res);
      return BSIERROR;
      }
     
    if (BeSQLite::BE_SQLITE_ROW != find->Step())
        return BSIERROR;

    if (level != nullptr)
        *level = static_cast<iModelBridgeAffinityLevel>(find->GetValueInt(0));

    if (jsonData != nullptr)
        jsonData->AssignOrClear(find->GetValueText(1));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeAffinityDb::InsertAffinity(int64_t sourceFileRowId, int64_t bridgeRowId, iModelBridgeAffinityLevel affinity, JsonValueCP json)
    {
    auto insert = m_db.GetCachedStatement("insert into Affinity (File,Bridge,Level,JsonData) VALUES(?,?,?,?)");
    DbResult res;
    if (BeSQLite::BE_SQLITE_OK != (res = insert->BindInt64(1, sourceFileRowId))
     || BeSQLite::BE_SQLITE_OK != (res = insert->BindInt64(2, bridgeRowId))
     || BeSQLite::BE_SQLITE_OK != (res = insert->BindInt(3, (int)affinity))
     || BeSQLite::BE_SQLITE_OK != (res = (json ? insert->BindText(4, json->ToString(), BeSQLite::Statement::MakeCopy::Yes) : insert->BindNull(4))))
      {
      BeAssert(false);
      LOG.errorv("InsertAffinity Bind failed with %x", res);
      return BSIERROR;
      }

    if (BeSQLite::BE_SQLITE_DONE != (res = insert->Step()))
        {
        BeAssert(false);
        LOG.errorv("iModelBridgeAffinityDb::InsertAffinity(%lld, %lld, %d) - failed with error %x", sourceFileRowId, bridgeRowId, affinity, (int)res);
        return BSIERROR;
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus checkJsonDataUnchanged(Utf8StringCR storedJsonData, JsonValueCP jsonData)
    {
    if (jsonData == nullptr && storedJsonData.empty())  // quick return in common case where there is no JSON data.
        return BSISUCCESS;

    auto stored = !storedJsonData.empty() ? Json::Value::From(storedJsonData) : Json::nullValue;
    auto proposed = jsonData ? *jsonData : Json::nullValue;
    if (stored == proposed)
        return BSISUCCESS;

    LOG.errorv("Already recorded with JsonData=%s. Rejected attempt to update to %s.",
        stored.toStyledString().c_str(),
        proposed.toStyledString().c_str());
    BeAssert(false);
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeAffinityDb::FindOrInsertAttachment(int64_t parentModelRowId, int64_t childModelRowId, JsonValueCP jsonData)
    {
    if (0 == parentModelRowId  || 0 == childModelRowId)
        {
        LOG.errorv("iModelBridgeAffinityDb::FindOrInsertAttachment(%lld, %lld) - Invalid argument", parentModelRowId, childModelRowId);
        BeAssert(false);
        return BSIERROR;
        }

    Utf8String storedJsonData;
    if (BSISUCCESS == FindAttachment(&storedJsonData, parentModelRowId, childModelRowId))
        {
        checkJsonDataUnchanged(storedJsonData, jsonData);
        return BSISUCCESS;
        }

    return InsertAttachment(parentModelRowId, childModelRowId, jsonData);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeAffinityDb::FindOrInsertAffinity(int64_t sourceFileRowId, int64_t bridgeRowId, iModelBridgeAffinityLevel affinity, JsonValueCP commandLineArguments)
    {
    if (0 == sourceFileRowId  || 0 == bridgeRowId || ((int)affinity < (int)iModelBridgeAffinityLevel::None || (int)iModelBridgeAffinityLevel::Override < (int)affinity))
        {
        LOG.errorv("iModelBridgeAffinityDb::FindOrInsertAffinity(%lld, %lld, %d) - Invalid argument", sourceFileRowId, bridgeRowId, (int)affinity);
        BeAssert(false);
        return BSIERROR;
        }

    Utf8String storedJsonData;
    iModelBridgeAffinityLevel storedAffinity;
    if (BSISUCCESS == FindAffinity(&storedAffinity, &storedJsonData, sourceFileRowId, bridgeRowId))
        {
        checkJsonDataUnchanged(storedJsonData, commandLineArguments);
        if (storedAffinity != affinity)
            LOG.warningv(L"iModelBridgeAffinityDb::FindOrInsertAffinity(%lld, %lld) - previously recorded an affinity of %d but now seeing an affinity of %d. The previously recorded affinity will be retained.", sourceFileRowId, bridgeRowId, (int)storedAffinity, affinity);
        return BSISUCCESS;
        }

    return InsertAffinity(sourceFileRowId, bridgeRowId, affinity, commandLineArguments);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeAffinityDb::ComputeAssignments(std::function<T_ProcessAssignment> const& proc)
    {
    auto stmt = m_db.GetCachedStatement("select bridge.name, file.filename, max(level) from affinity, file, bridge where affinity.bridge=bridge.id and affinity.file=file.id group by file");
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto bridge = stmt->GetValueText(0);
        auto file = stmt->GetValueText(1);
        auto affinity = stmt->GetValueInt(2);
        proc(BeFileName(file,true), bridge, (iModelBridgeAffinityLevel)affinity);
        }
    }

struct Affinity
    {
    int64_t file;
    int64_t bridge;
    int level;
    };

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeAffinityDb::DeleteCompetingAffinities(int64_t preferredBridge)
    {
    bmap<int, bset<int64_t>> affinityToFiles;
    if (true)
        {
        Statement stmt;
        stmt.Prepare(m_db, "SELECT Level, File from Affinity where Bridge=?");
        stmt.BindInt64(1, preferredBridge);
        while (BE_SQLITE_ROW == stmt.Step())
            {
            affinityToFiles[stmt.GetValueInt(0)].insert(stmt.GetValueInt64(1));
            }
        }
    if (affinityToFiles.empty())
        return;

    bvector<Affinity> affinitiesToDelete;
    if (true)
        {
        Statement stmt;
        stmt.Prepare(m_db, "SELECT Level, File, Bridge from Affinity where Bridge != ?");
        stmt.BindInt64(1, preferredBridge);
        while (BE_SQLITE_ROW == stmt.Step())
            {
            Affinity affinity;
            affinity.level = stmt.GetValueInt(0);
            affinity.file = stmt.GetValueInt64(1);
            affinity.bridge = stmt.GetValueInt64(2);
            auto iFiles = affinityToFiles.find(affinity.level);
            if (iFiles == affinityToFiles.end())
                continue;
            auto iFile = iFiles->second.find(affinity.file);
            if (iFile == iFiles->second.end())
                continue;
            affinitiesToDelete.push_back(affinity);
            }
        }
    
    if (affinitiesToDelete.empty())
        return;

    if (true)
        {
        Statement stmt;
        stmt.Prepare(m_db, "delete from Affinity WHERE File=? AND Bridge=? AND Level=?");
        for (auto const& affinity: affinitiesToDelete)
            {
            stmt.Reset();
            stmt.ClearBindings();
            stmt.BindInt64(1, affinity.file);
            stmt.BindInt64(2, affinity.bridge);
            stmt.BindInt(3, affinity.level);
            stmt.Step();
            }
        }
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeAffinityDb::QueryAttachmentsToFile(int64_t parentFileRowId, std::function<T_ProcessFile> const& proc)
    {
    bset<int64_t> filesOfChildModels;
    if (true)
        {
        auto stmt = m_db.GetCachedStatement(R"(
            select distinct model.file from model where model.id in (
                select a.child from attachment a where a.parent in (
                    select model.id from model where model.file = ?                     
                )
            )   
        )");
        stmt->BindInt64(1, parentFileRowId);
        while (BE_SQLITE_ROW == stmt->Step())
            {
            // NB: Do not call proc here. That will cause a stack overflow if proc calls iModelBridgeAffinityDb::QueryAttachmentsToFile
            //      recursively on child files to detect nested attachments. Instead, buffer up the results and then invoke the callback
            //      after releasing the statement.
            filesOfChildModels.insert(stmt->GetValueInt64(0));
            }
        }

    for (auto file : filesOfChildModels)
        {
        proc(file);
        }
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelBridgeAffinityDb::GetBridgename(int64_t bridgeId)
    {
    auto stmt = m_db.GetCachedStatement("select name from Bridge where ROWID=?");
    stmt->BindInt64(1, bridgeId);
    if (BE_SQLITE_ROW == stmt->Step())
        return stmt->GetValueText(0);
    return "";
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeAffinityDb::FindDuplicateAffinities(std::function<T_ProcessDuplicateAffinity> const& proc)
    {
    auto reportDupsStmt = m_db.GetCachedStatement("select File, Level, COUNT(*) c FROM Affinity GROUP BY File, Level HAVING c > 1");
    while (BE_SQLITE_ROW == reportDupsStmt->Step())
        {
        auto fileId = reportDupsStmt->GetValueInt64(0);
        auto level = (iModelBridgeAffinityLevel) reportDupsStmt->GetValueInt(1);

        bvector<int64_t> bridges;
        auto getBridgeIdsStmt = m_db.GetCachedStatement("select bridge from Affinity where File=? and Level=?");
        getBridgeIdsStmt->BindInt64(1, fileId);
        getBridgeIdsStmt->BindInt(2, level);
        while (BE_SQLITE_ROW == getBridgeIdsStmt->Step())
            bridges.push_back(getBridgeIdsStmt->GetValueInt64(0));

        proc(fileId, level, bridges);
        }
    }