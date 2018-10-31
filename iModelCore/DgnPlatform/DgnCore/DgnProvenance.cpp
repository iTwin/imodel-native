/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnProvenance.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8FileProvenance::CreateTable(DgnDbR dgndb)
    {
    dgndb.CreateTable(DGN_TABLE_ProvenanceFile, "V8FileId TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY, V8Name CHAR NOT NULL, V8UniqueName CHAR NOT NULL UNIQUE");
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceFile "_V8UniqueName ON "  DGN_TABLE_ProvenanceFile "(V8UniqueName)");
        // DgnDb61 resolution of URIs requires lookups based on unique name
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceFile "_V8Name ON "  DGN_TABLE_ProvenanceFile "(V8Name)");
        // Graphite05 resolution of URIs requires lookups based on (non unique) name
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
Utf8String DgnV8FileProvenance::ExtractEmbeddedFileName(Utf8StringCR v8Pathname)
    {
    BeFileName pathname(v8Pathname);
    Utf8String fileName(pathname.GetFileNameAndExtension());

    Utf8CP packageEnd = ::strchr(fileName.c_str(), '>');
    if (!packageEnd)
        return fileName;

    return Utf8String(packageEnd + 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8FileProvenance::Insert(BeSQLite::BeGuidCR v8FileId, Utf8StringCR v8Pathname, Utf8StringCR v8UniqueName, DgnDbR dgndb)
    {
    Utf8String v8Name = ExtractEmbeddedFileName(v8Pathname);

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "INSERT INTO " DGN_TABLE_ProvenanceFile " (V8FileId,V8Name,V8UniqueName) VALUES (?,?,?)");
    Utf8String guidString = v8FileId.ToString();
    stmt->BindText(1, guidString.c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, v8Name.c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, v8UniqueName.c_str(), Statement::MakeCopy::No);
    
    DbResult result = stmt->Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8FileProvenance::Delete(BeSQLite::BeGuidCR v8FileId, DgnDbR dgndb)
    {
    Utf8String guidString = v8FileId.ToString();

    Statement stmt;
    stmt.Prepare(dgndb, "DELETE FROM " DGN_TABLE_ProvenanceFile " WHERE V8FileId=?");
    stmt.BindText(1, guidString.c_str(), Statement::MakeCopy::No);

    DbResult result = stmt.Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnV8FileProvenance::Find(Utf8StringP v8Name, Utf8StringP v8UniqueName, BeSQLite::BeGuidCR v8FileId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceFile))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT V8Name,V8UniqueName FROM " DGN_TABLE_ProvenanceFile " WHERE V8FileId=?");
    Utf8String guidString = v8FileId.ToString();
    stmt->BindText(1, guidString.c_str(), Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    if (v8Name)
        *v8Name = stmt->GetValueText(0);

    if (v8UniqueName)
        *v8UniqueName = stmt->GetValueText(1);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnV8FileProvenance::FindFirst(BeSQLite::BeGuid* v8FileId, Utf8CP v8NameOrUniqueName, bool findByUniqueName, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceFile))
        return ERROR; // Provenance not supported

    Utf8CP sql = findByUniqueName ? "SELECT V8FileId FROM " DGN_TABLE_ProvenanceFile " WHERE V8UniqueName=?" :
        "SELECT V8FileId FROM " DGN_TABLE_ProvenanceFile " WHERE V8Name=?";

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, sql);
    stmt->BindText(1, v8NameOrUniqueName, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    
    if (v8FileId)
        {
        Utf8String guidString = stmt->GetValueText(0);
        return v8FileId->FromString(guidString.c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ModelProvenance::CreateTable(DgnDbR dgndb)
    {
    dgndb.CreateTable(DGN_TABLE_ProvenanceModel,
                      "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "ModelId BIGINT,"//Maps to ModelId in SYNCINFO_ATTACH(SYNC_TABLE_Model),
                      "V8FileId TEXT COLLATE NoCase REFERENCES " DGN_TABLE_ProvenanceFile "(V8FileId) ON DELETE CASCADE," //Maps to V8FileSyncInfoId
                      "V8ModelId INT,"//Maps to V8Id
                      "V8ModelName CHAR NOT NULL,"
                      "Transform BLOB,"
                      "CONSTRAINT FileModelId UNIQUE(ModelId, V8FileId, V8ModelId,Transform)"
                      );//Maps to V8Name
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceModel "_ModelId ON "  DGN_TABLE_ProvenanceModel "(ModelId)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ModelProvenance::Insert(BeSQLite::BeGuidCR v8FileId, ModelProvenanceEntry const& entry, DgnDbR dgndb)
    {
    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "INSERT INTO " DGN_TABLE_ProvenanceModel " (ModelId,V8FileId,V8ModelId,V8ModelName,Transform) VALUES (?,?,?,?,?)");
    stmt->BindId(1, entry.m_modelId);
    Utf8String guidString = v8FileId.ToString();
    stmt->BindText(2, guidString.c_str(), Statement::MakeCopy::No);
    stmt->BindInt(3, entry.m_dgnv8ModelId);
    stmt->BindText(4, entry.m_modelName.c_str(), Statement::MakeCopy::No);
    if (entry.m_trans.IsIdentity())
        stmt->BindNull(5);
    else
        stmt->BindBlob(5, &entry.m_trans, sizeof(entry.m_trans), Statement::MakeCopy::No);
    DbResult result = stmt->Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ModelProvenance::Delete(DgnModelId modelId, DgnDbR dgndb)
    {
    Statement stmt;
    stmt.Prepare(dgndb, "DELETE FROM " DGN_TABLE_ProvenanceModel " WHERE ModelId=?");
    stmt.BindId(1, modelId);

    DbResult result = stmt.Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnV8ModelProvenance::FindFirst(BeSQLite::BeGuid* v8FileId, int* v8ModelId, Utf8StringP v8ModelName, DgnModelId modelId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceModel))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT V8FileId,V8ModelId,V8ModelName FROM " DGN_TABLE_ProvenanceModel " WHERE ModelId=?");
    stmt->BindId(1, modelId);
    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    if (v8FileId)
        {
        Utf8String guidString = stmt->GetValueText(0);
        v8FileId->FromString(guidString.c_str());
        }

    if (v8ModelId)
        *v8ModelId = stmt->GetValueInt(1);

    if (v8ModelName)
        *v8ModelName = stmt->GetValueText(2);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnV8ModelProvenance::FindAll(bvector <ModelProvenanceEntry> &entries, BeSQLite::BeGuidCR v8FileId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceModel))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT ModelId,V8ModelId,V8ModelName,Transform FROM " DGN_TABLE_ProvenanceModel " WHERE V8FileId=?");
    Utf8String guidString = v8FileId.ToString();
    stmt->BindText(1, guidString.c_str(), Statement::MakeCopy::No);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ModelProvenanceEntry entry;
        entry.m_modelId = stmt->GetValueId<DgnModelId>(0);
        entry.m_dgnv8ModelId = stmt->GetValueInt(1);
        entry.m_modelName = stmt->GetValueText(2);
        memcpy(&entry.m_trans, stmt->GetValueBlob(3), sizeof(entry.m_trans));
        entries.push_back(entry);
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ElementProvenance::CreateTable(DgnDbR dgndb)
    {
    dgndb.CreateTable(DGN_TABLE_ProvenanceElement, "Id INTEGER PRIMARY KEY NOT NULL, ElementId BIGINT, V8ElementId BIGINT, V8ModelId INT, V8FileId TEXT NOT NULL COLLATE NoCase");
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceElement "_ElementId ON "  DGN_TABLE_ProvenanceElement "(ElementId)");
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceElement "_V8ElementId ON "  DGN_TABLE_ProvenanceElement "(V8FileId,V8ElementId)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ElementProvenance::Insert(DgnElementId elementId, BeSQLite::BeGuidCR v8FileId, int v8ModelId, int64_t v8ElementId, DgnDbR dgndb)
    {
    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "INSERT INTO " DGN_TABLE_ProvenanceElement " (ElementId,V8ElementId,V8ModelId,V8FileId) VALUES (?,?,?,?)");
    stmt->BindId(1, elementId);
    stmt->BindInt64(2, v8ElementId);
    stmt->BindInt(3, v8ModelId);
    
    Utf8String guidString = v8FileId.ToString();
    stmt->BindText(4, guidString.c_str(), Statement::MakeCopy::No);

    DbResult result = stmt->Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ElementProvenance::Delete(DgnElementId elementId, DgnDbR dgndb)
    {
    Statement stmt;
    stmt.Prepare(dgndb, "DELETE FROM " DGN_TABLE_ProvenanceElement " WHERE ElementId=?");
    stmt.BindId(1, elementId);

    DbResult result = stmt.Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnV8ElementProvenance::FindFirst(BeSQLite::BeGuid* v8FileId, int* v8ModelId, int64_t* v8ElementId, DgnElementId elementId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceElement))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT V8FileId,V8ModelId,V8ElementId FROM " DGN_TABLE_ProvenanceElement " WHERE ElementId=?");
    stmt->BindId(1, elementId);
    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    if (v8FileId)
        {
        Utf8String guidString = stmt->GetValueText(0);
        v8FileId->FromString(guidString.c_str());
        }

    if (v8ModelId)
        *v8ModelId = stmt->GetValueInt(1);

    if (v8ElementId)
        *v8ElementId = stmt->GetValueInt64(2);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnV8ElementProvenance::FindFirst(DgnElementId* elementId, BeSQLite::BeGuidCR v8FileId, int64_t v8ElementId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceElement))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT ElementId FROM " DGN_TABLE_ProvenanceElement " WHERE V8FileId=? AND V8ElementId=?");
    Utf8String guidString = v8FileId.ToString();
    stmt->BindText(1, guidString.c_str(), Statement::MakeCopy::No);
    stmt->BindInt64(2, v8ElementId);

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    if (elementId)
        *elementId = stmt->GetValueId<DgnElementId>(0);

    return SUCCESS;
    }

END_BENTLEY_DGN_NAMESPACE
