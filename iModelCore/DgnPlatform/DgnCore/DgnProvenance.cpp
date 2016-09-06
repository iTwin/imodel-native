/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnProvenance.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8FileProvenance::CreateTable(DgnDbR dgndb)
    {
    dgndb.CreateTable(DGN_TABLE_ProvenanceFile, "V8FileId INTEGER PRIMARY KEY NOT NULL, V8Name CHAR NOT NULL, V8UniqueName CHAR NOT NULL UNIQUE");
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
void DgnV8FileProvenance::Insert(uint32_t v8FileId, Utf8StringCR v8Pathname, Utf8StringCR v8UniqueName, DgnDbR dgndb)
    {
    Utf8String v8Name = ExtractEmbeddedFileName(v8Pathname);

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "INSERT INTO " DGN_TABLE_ProvenanceFile " (V8FileId,V8Name,V8UniqueName) VALUES (?,?,?)");
    stmt->BindInt(1, (int) v8FileId);
    stmt->BindText(2, v8Name.c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, v8UniqueName.c_str(), Statement::MakeCopy::No);
    
    DbResult result = stmt->Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8FileProvenance::Delete(uint32_t v8FileId, DgnDbR dgndb)
    {
    Statement stmt;
    stmt.Prepare(dgndb, "DELETE FROM " DGN_TABLE_ProvenanceFile " WHERE V8FileId=?");
    stmt.BindInt64(1, (int) v8FileId);

    DbResult result = stmt.Step();
    BeAssert(result == BE_SQLITE_DONE);
    UNUSED_VARIABLE(result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnV8FileProvenance::Find(Utf8StringP v8Name, Utf8StringP v8UniqueName, uint32_t v8FileId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceFile))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT V8Name,V8UniqueName FROM " DGN_TABLE_ProvenanceFile " WHERE V8FileId=?");
    stmt->BindInt(1, (int) v8FileId);
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
BentleyStatus DgnV8FileProvenance::FindFirst(uint32_t* v8FileId, Utf8CP v8NameOrUniqueName, bool findByUniqueName, DgnDbCR dgndb)
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
        *v8FileId = (uint32_t) stmt->GetValueInt(0);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ModelProvenance::CreateTable(DgnDbR dgndb)
    {
    dgndb.CreateTable(DGN_TABLE_ProvenanceModel,
                      "Id INTEGER PRIMARY KEY NOT NULL,"
                      "ModelId BIGINT,"
                      "V8FileId INTEGER REFERENCES " DGN_TABLE_ProvenanceFile "(V8FileId) ON DELETE CASCADE,"
                      "V8ModelId INT, V8ModelName CHAR NOT NULL");
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceModel "_ModelId ON "  DGN_TABLE_ProvenanceModel "(ModelId)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ModelProvenance::Insert(DgnModelId modelId, uint32_t v8FileId, int v8ModelId, Utf8StringCR v8ModelName, DgnDbR dgndb)
    {
    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "INSERT INTO " DGN_TABLE_ProvenanceModel " (ModelId,V8FileId,V8ModelId,V8ModelName) VALUES (?,?,?,?)");
    stmt->BindId(1, modelId);
    stmt->BindInt(2, (int) v8FileId);
    stmt->BindInt(3, v8ModelId);
    stmt->BindText(4, v8ModelName.c_str(), Statement::MakeCopy::No);

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
BentleyStatus DgnV8ModelProvenance::FindFirst(uint32_t* v8FileId, int* v8ModelId, Utf8StringP v8ModelName, DgnModelId modelId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceModel))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT V8FileId,V8ModelId,V8ModelName FROM " DGN_TABLE_ProvenanceModel " WHERE ModelId=?");
    stmt->BindId(1, modelId);
    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    if (v8FileId)
        *v8FileId = (uint32_t) stmt->GetValueInt(0);

    if (v8ModelId)
        *v8ModelId = stmt->GetValueInt(1);

    if (v8ModelName)
        *v8ModelName = stmt->GetValueText(2);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ElementProvenance::CreateTable(DgnDbR dgndb)
    {
    dgndb.CreateTable(DGN_TABLE_ProvenanceElement, "Id INTEGER PRIMARY KEY NOT NULL, ElementId BIGINT, V8ElementId BIGINT, V8ModelId INT, V8FileId INT");
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceElement "_ElementId ON "  DGN_TABLE_ProvenanceElement "(ElementId)");
    dgndb.ExecuteSql("CREATE INDEX " DGN_TABLE_ProvenanceElement "_V8ElementId ON "  DGN_TABLE_ProvenanceElement "(V8FileId,V8ElementId)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
void DgnV8ElementProvenance::Insert(DgnElementId elementId, uint32_t v8FileId, int v8ModelId, int64_t v8ElementId, DgnDbR dgndb)
    {
    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "INSERT INTO " DGN_TABLE_ProvenanceElement " (ElementId,V8ElementId,V8ModelId,V8FileId) VALUES (?,?,?,?)");
    stmt->BindId(1, elementId);
    stmt->BindInt64(2, v8ElementId);
    stmt->BindInt(3, v8ModelId);
    stmt->BindInt(4, (int) v8FileId);

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
BentleyStatus DgnV8ElementProvenance::FindFirst(uint32_t* v8FileId, int* v8ModelId, int64_t* v8ElementId, DgnElementId elementId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceElement))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT V8FileId,V8ModelId,V8ElementId FROM " DGN_TABLE_ProvenanceElement " WHERE ElementId=?");
    stmt->BindId(1, elementId);
    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    if (v8FileId)
        *v8FileId = (uint32_t) stmt->GetValueInt(0);

    if (v8ModelId)
        *v8ModelId = stmt->GetValueInt(1);

    if (v8ElementId)
        *v8ElementId = stmt->GetValueInt64(2);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnV8ElementProvenance::FindFirst(DgnElementId* elementId, uint32_t v8FileId, int64_t v8ElementId, DgnDbCR dgndb)
    {
    if (!dgndb.TableExists(DGN_TABLE_ProvenanceElement))
        return ERROR; // Provenance not supported

    CachedStatementPtr stmt;
    dgndb.GetCachedStatement(stmt, "SELECT ElementId FROM " DGN_TABLE_ProvenanceElement " WHERE V8FileId=? AND V8ElementId=?");
    stmt->BindInt(1, (int) v8FileId);
    stmt->BindInt64(2, v8ElementId);

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    if (elementId)
        *elementId = stmt->GetValueId<DgnElementId>(0);

    return SUCCESS;
    }

END_BENTLEY_DGN_NAMESPACE
