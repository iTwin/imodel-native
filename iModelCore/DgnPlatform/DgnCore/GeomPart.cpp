/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GeomPart.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  03/15
//=======================================================================================
struct GeomBlobHeader
{
enum
    {
    DB_GeomSignature06 = 0x0600, // Graphite06
    };

uint32_t m_signature;    // write this so we can detect errors on read
uint32_t m_size;
GeomBlobHeader(uint32_t size) {m_signature = DB_GeomSignature06; m_size=size;}
GeomBlobHeader(BeSQLite::SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  03/15
//=======================================================================================
struct DbGeomPartsWriter
{
protected:
    DgnDbR m_dgndb;
    BeSQLite::SnappyToBlob m_snappy;
    BeSQLite::CachedStatementPtr m_stmt;

    void ExecStatement();
    void PrepareInsertStatement();
    void PrepareUpdateStatement();
    StatusInt SaveGeomPartToRow(const void* geometryBlob, int geometryBlobSize, Utf8CP code, DgnGeomPartId geomPartId);

public:
    DbGeomPartsWriter(DgnDbR db) : m_dgndb(db) {}

    DgnGeomPartId InsertGeomPart(const void* geometryBlob, int geometryBlobSize, Utf8CP code);
    BentleyStatus UpdateGeomPart(DgnGeomPartId geomPartId, const void* geometryBlob, int geometryBlobSize, Utf8CP code);

    static int GetColumnIndexForId()            {return 1;}     // Must match columns in PrepareInsertStatement & PrepareUpdateStatement
    static int GetColumnIndexForCode()          {return 2;}     // Must match columns in PrepareInsertStatement & PrepareUpdateStatement
    static int GetColumnIndexForGeom()          {return 3;}     // Must match columns in PrepareInsertStatement & PrepareUpdateStatement
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeomPartsWriter::PrepareInsertStatement()
    {
    Utf8CP insertSql =
            "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_GeomPart) "("
                "Id,"   // 1
                "Code," // 2
                "Geom"  // 3
            ")VALUES(?,?,?)";

    m_dgndb.GetCachedStatement(m_stmt, insertSql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeomPartsWriter::PrepareUpdateStatement()
    {
    Utf8CP updateSql =
            "UPDATE " DGN_TABLE(DGN_CLASSNAME_GeomPart) " SET "
                "Code=?2,"
                "Geom=?3,"
            " WHERE Id=?1";

    m_dgndb.GetCachedStatement(m_stmt, updateSql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeomPartsWriter::ExecStatement()
    {
    DbResult status = m_stmt->Step();
    if (status != BE_SQLITE_DONE)
        {
        BeAssert(false);
        return;
        }
    m_stmt->Reset();
    m_stmt->ClearBindings();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DbGeomPartsWriter::SaveGeomPartToRow(const void* geometryBlob, int geometryBlobSize, Utf8CP code, DgnGeomPartId geomPartId)
    {
    m_stmt->BindId(GetColumnIndexForId(), geomPartId);

    if (code)
        m_stmt->BindText(GetColumnIndexForCode(), code, Statement::MakeCopy::No);
    else
        m_stmt->BindNull(GetColumnIndexForCode());

    if (0 == geometryBlobSize)
        {
        ExecStatement();
        return SUCCESS; // Is this an error?!?
        }

    GeomBlobHeader  header((uint32_t) geometryBlobSize);

    m_snappy.Init();
    m_snappy.Write((ByteCP) &header, sizeof (header));
    m_snappy.Write((ByteCP) geometryBlob, (uint32_t)geometryBlobSize);

    uint32_t zipSize = m_snappy.GetCompressedSize();

    if (1 == m_snappy.GetCurrChunk())
        {
        m_stmt->BindBlob(GetColumnIndexForGeom(), m_snappy.GetChunkData(0), zipSize, Statement::MakeCopy::No);
        ExecStatement();
        return SUCCESS;
        }

    m_stmt->BindZeroBlob(GetColumnIndexForGeom(), zipSize); // more than one chunk in graphics stream
    ExecStatement();

    return m_snappy.SaveToRow(m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeomPart), "Geom", geomPartId.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeomPartId DbGeomPartsWriter::InsertGeomPart(const void* geometryBlob, int geometryBlobSize, Utf8CP code)
    {
    DgnGeomPartId geomPartId = m_dgndb.GeomParts().MakeNewGeomPartId();

    if (!geomPartId.IsValid())
        return DgnGeomPartId();

    PrepareInsertStatement();

    return (SUCCESS == SaveGeomPartToRow(geometryBlob, geometryBlobSize, code, geomPartId) ? geomPartId : DgnGeomPartId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DbGeomPartsWriter::UpdateGeomPart(DgnGeomPartId geomPartId, const void* geometryBlob, int geometryBlobSize, Utf8CP code)
    {
    if (!geomPartId.IsValid())
        return ERROR;

    PrepareUpdateStatement();

    return (SUCCESS == SaveGeomPartToRow(geometryBlob, geometryBlobSize, code, geomPartId) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeomPartId DgnGeomParts::GetHighestGeomPartId()
    {
    if (!m_highestGeomPartId.IsValid())
        {
        BeLuid nextRepo(m_dgndb.GetRepositoryId().GetNextRepositoryId().GetValue(),0);
        Statement stmt;
        
        stmt.Prepare(m_dgndb, "SELECT max(Id) FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " WHERE Id<?");
        stmt.BindInt64(1,nextRepo.GetValue());

        DbResult result = stmt.Step();
        UNUSED_VARIABLE(result);
        BeAssert(result == BE_SQLITE_ROW);

        m_highestGeomPartId.m_id = stmt.GetValueInt64(0);
        }

    return m_highestGeomPartId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeomPartId DgnGeomParts::MakeNewGeomPartId()
    {
    GetHighestGeomPartId();
    m_highestGeomPartId.UseNext();

    return m_highestGeomPartId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::InsertGeomPart(DgnGeomPartR geomPart)
    {
    ElementGeomIO::Writer writer(m_dgndb);

    for (ElementGeometryPtr elemGeom : geomPart.GetGeometry())
        {
        if (!elemGeom.IsValid())
            continue;

        writer.Append(*elemGeom);
        }

    if (0 == writer.m_buffer.size())
        return BentleyStatus::ERROR;

    DgnGeomPartId geomPartId = InsertGeomPart(&writer.m_buffer.front(), (uint32_t) writer.m_buffer.size(), geomPart.GetCode());
    if (!geomPartId.IsValid())
        return BentleyStatus::ERROR;

    geomPart.SetId(geomPartId);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
DgnGeomPartId DgnGeomParts::InsertGeomPart(const void* geometryBlob, int geometryBlobSize, Utf8CP code)
    {
    if (0 == geometryBlobSize)
        return DgnGeomPartId();

    DbGeomPartsWriter writer(GetDgnDb());
    return writer.InsertGeomPart(geometryBlob, geometryBlobSize, code);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::UpdateGeomPart(DgnGeomPartR geomPart)
    {
    ElementGeomIO::Writer writer(m_dgndb);

    for (ElementGeometryPtr elemGeom : geomPart.GetGeometry())
        {
        if (!elemGeom.IsValid())
            continue;

        writer.Append(*elemGeom);
        }

    if (0 == writer.m_buffer.size())
        return BentleyStatus::ERROR;

    return UpdateGeomPart(geomPart.GetId(), &writer.m_buffer.front(), (uint32_t) writer.m_buffer.size(), geomPart.GetCode());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::UpdateGeomPart(DgnGeomPartId geomPartId, const void* geometryBlob, int geometryBlobSize, Utf8CP code)
    {
    DbGeomPartsWriter writer(GetDgnDb());
    return writer.UpdateGeomPart(geomPartId, geometryBlob, geometryBlobSize, code);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::InsertElementGeomUsesParts(DgnElementId elementId, DgnGeomPartId geomPartId)
    {
    if (!elementId.IsValid() || !geomPartId.IsValid())
        return BentleyStatus::ERROR;

    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement(
        "INSERT INTO " DGN_SCHEMA(DGN_RELNAME_ElementGeomUsesParts) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");

    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    statementPtr->BindId(1, elementId);
    statementPtr->BindId(2, geomPartId);

    if (ECSqlStepStatus::Done != statementPtr->Step())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
DgnGeomPartPtr DgnGeomParts::LoadGeomPart(DgnGeomPartId geomPartId)
    {
    if (!geomPartId.IsValid())
        return nullptr;

    HighPriorityOperationBlock hpo;
   
    auto& elements = m_dgndb.Elements();

    CachedStatementPtr stmt=elements.GetStatement("SELECT Code FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " WHERE Id=?");
    stmt->BindId(1, geomPartId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    SnappyFromBlob& snappy = elements.GetSnappyFrom();
    
    if (ZIP_SUCCESS != snappy.Init(m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeomPart), "Geom", geomPartId.GetValue()))
        return nullptr;

    GeomBlobHeader header(snappy);
    if ((GeomBlobHeader::DB_GeomSignature06 != header.m_signature) || 0 == header.m_size)
        {
        BeAssert(false);
        return nullptr;
        }

    ScopedArray<Byte, 8192> geometryBlob(header.m_size);
    uint32_t actuallyRead;
    snappy.ReadAndFinish(geometryBlob.GetData(), header.m_size, actuallyRead);

    if (actuallyRead != header.m_size)
        return nullptr;

    DgnGeomPartPtr geomPartPtr = new DgnGeomPart(stmt->GetValueText(0));

    ElementGeometryCollection collection(m_dgndb, geometryBlob.GetDataCP(), header.m_size);

    for (ElementGeometryPtr geom : collection)
        geomPartPtr->GetGeometryR().push_back(geom);

    geomPartPtr->SetId(geomPartId);
    return geomPartPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    05/2015
//---------------------------------------------------------------------------------------
DgnGeomPartId DgnGeomParts::QueryGeomPartId(Utf8CP code)
    {
    if (!code || !*code)
        return DgnGeomPartId();

    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " WHERE Code=?");
    stmt->BindText(1, code, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnGeomPartId() : stmt->GetValueId<DgnGeomPartId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings              05/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::Draw(DgnGeomPartId geomPartId, ViewContextR context, DgnCategoryId categoryId, ViewFlagsCR flags)
    {
    if (!geomPartId.IsValid())
        return ERROR;

    if (context.GetCurrentDisplayParams()->GetCategoryId() != categoryId)
        return ERROR;

    HighPriorityOperationBlock hpo;
   
    auto& elements = context.GetDgnDb().Elements();

    CachedStatementPtr stmt=elements.GetStatement("SELECT Code FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " WHERE Id=?");
    stmt->BindId(1, geomPartId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return ERROR;

    SnappyFromBlob& snappy = elements.GetSnappyFrom();
    
    if (ZIP_SUCCESS != snappy.Init(context.GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_GeomPart), "Geom", geomPartId.GetValue()))
        return ERROR;

    GeomBlobHeader header(snappy);
    if ((GeomBlobHeader::DB_GeomSignature06 != header.m_signature) || 0 == header.m_size)
        {
        BeAssert(false);
        return ERROR;
        }

    ScopedArray<Byte, 8192> geometryBlob(header.m_size);
    uint32_t actuallyRead;
    snappy.ReadAndFinish(geometryBlob.GetData(), header.m_size, actuallyRead);

    if (actuallyRead != header.m_size)
        return ERROR;

    ElementGeomIO::Collection collection(geometryBlob.GetDataCP(), header.m_size);

    context.SetDgnGeomPartId(geomPartId); // Announce geom part id for picking, etc.
    collection.Draw(context, categoryId, flags);
    context.SetDgnGeomPartId(DgnGeomPartId());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::DeleteGeomPart(DgnGeomPartId geomPartId)
    {
    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement("DELETE FROM ONLY " DGN_SCHEMA(DGN_CLASSNAME_GeomPart) " WHERE ECInstanceId=?");
    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    if (ECSqlStatus::Success != statementPtr->BindId(1, geomPartId))
        return BentleyStatus::ERROR;

    if (ECSqlStepStatus::Done != statementPtr->Step())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnGeomPartPtr DgnGeomPart::Create(Utf8CP code)
    {
    return new DgnGeomPart(code);
    }
