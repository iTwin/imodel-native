/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PhysicalGeometry.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/11
//=======================================================================================
struct DbException
    {
    DgnFileStatus m_status;
    int           m_result;
    DbException (DgnFileStatus s, int r){m_status = s; m_result=r;}
    };

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
GeomBlobHeader(BeSQLite::SnappyReader& in) {uint32_t actuallyRead; in._Read ((Byte*) this, sizeof(*this), actuallyRead);}
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
    StatusInt SaveGeomPartToRow (const void* geometryBlob, int geometryBlobSize, Utf8CP code, DgnGeomPartId geomPartId);

public:
    DbGeomPartsWriter (DgnDbR db) : m_dgndb(db) {}

    DgnGeomPartId InsertGeomPart (const void* geometryBlob, int geometryBlobSize, Utf8CP code);
    BentleyStatus UpdateGeomPart (DgnGeomPartId geomPartId, const void* geometryBlob, int geometryBlobSize, Utf8CP code);

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

    DbResult status = m_dgndb.GetCachedStatement (m_stmt, insertSql);
    if (BE_SQLITE_OK != status)
        throw DbException(DGNDB_ERROR_SQLSchemaError, status);
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

    DbResult status = m_dgndb.GetCachedStatement (m_stmt, updateSql);
    if (BE_SQLITE_OK != status)
        throw DbException(DGNDB_ERROR_SQLSchemaError, status);
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
        throw DbException(DGNOPEN_STATUS_CorruptFile, status);
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

    StatusInt status = m_snappy.SaveToRow (m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeomPart), "Geom", geomPartId.GetValue());
    if (SUCCESS != status)
        {
        BeAssert(false);
        throw DbException(DGNOPEN_STATUS_CorruptFile, status);
        }

    return status;
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
BentleyStatus DbGeomPartsWriter::UpdateGeomPart (DgnGeomPartId geomPartId, const void* geometryBlob, int geometryBlobSize, Utf8CP code)
    {
    if (!geomPartId.IsValid())
        return ERROR;

    PrepareUpdateStatement();

    return (SUCCESS == SaveGeomPartToRow(geometryBlob, geometryBlobSize, code, geomPartId) ? SUCCESS : ERROR);
    }

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  03/15
//=======================================================================================
struct DbGeomPartsReader
{
protected:
    BeSQLite::SnappyFromBlob m_snappy;
    DgnDbCR m_dgndb;
    BeSQLite::CachedStatementPtr m_selectStmt;

    Utf8CP GetSelectStatement();

public:
    DbGeomPartsReader(DgnDbCR dgndb) : m_dgndb(dgndb) {}
    virtual ~DbGeomPartsReader() {}
    BentleyStatus ReadGeomPart (bvector<Byte>& geometryBlob, Utf8StringR code, DgnGeomPartId geomPartId);

    static int GetColumnIndexForCode()          {return 0;}     // Must match columns in GetSelectStatement
    static int GetColumnIndexForGeom()          {return 1;}     // Must match columns in GetSelectStatement
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DbGeomPartsReader::GetSelectStatement()
    {
    // WIP: need to query [MaterialId] also
    return "SELECT "
                "Code," // 0
                "Geom"  // 1
           " FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " ";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DbGeomPartsReader::ReadGeomPart(bvector<Byte>& geometryBlob, Utf8StringR code, DgnGeomPartId geomPartId)
    {
    Utf8String sql (GetSelectStatement());
    sql.append ("WHERE Id=?");

    HighPriorityOperationBlock hpo;

    m_dgndb.GetCachedStatement(m_selectStmt, sql.c_str());
    m_selectStmt->BindId(1, geomPartId);

    DbResult result = m_selectStmt->Step();
    if (BE_SQLITE_ROW != result)
        return ERROR;

    code.AssignOrClear(m_selectStmt->GetValueText(GetColumnIndexForCode()));

    if (ZIP_SUCCESS == m_snappy.Init (m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeomPart), "Geom", geomPartId.GetValue()))
        {
        GeomBlobHeader header (m_snappy);
        if ((GeomBlobHeader::DB_GeomSignature06 != header.m_signature) || 0 == header.m_size)
            {
            BeAssert (false);
            throw DbException(DGNOPEN_STATUS_CorruptFile, 0);
            }

        uint32_t actuallyRead;

        geometryBlob.resize(header.m_size);
        m_snappy._Read (&geometryBlob[0], header.m_size, actuallyRead);

        if (actuallyRead != header.m_size)
            {
            BeAssert(false);
            throw DbException(DGNOPEN_STATUS_CorruptFile, 0);
            return ERROR;
            }
        }

    return SUCCESS;
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
        
        stmt.Prepare (m_dgndb, "SELECT max(Id) FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " WHERE Id<?");
        stmt.BindInt64 (1,nextRepo.GetValue());

        DbResult result = stmt.Step();
        UNUSED_VARIABLE(result);
        BeAssert (result == BE_SQLITE_ROW);

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
    ElementGeomIO::Writer writer;

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
    return writer.InsertGeomPart (geometryBlob, geometryBlobSize, code);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::UpdateGeomPart(DgnGeomPartR geomPart)
    {
    ElementGeomIO::Writer writer;

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
    return writer.UpdateGeomPart (geomPartId, geometryBlob, geometryBlobSize, code);
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
    bvector<Byte> geometryBlob;
    Utf8String code;

    if (BentleyStatus::SUCCESS != QueryGeomPart(geometryBlob, code, geomPartId))
        return nullptr;

    DgnGeomPartPtr geomPartPtr = new DgnGeomPart(code.c_str());

    ElementGeometryCollection collection(&geometryBlob.front(), geometryBlob.size());

    for (ElementGeometryPtr geom : collection)
        geomPartPtr->GetGeometryR().push_back (geom);

    geomPartPtr->SetId(geomPartId);
    return geomPartPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::QueryGeomPart(bvector<Byte>& geometryBlob, Utf8StringR code, DgnGeomPartId geomPartId)
    {
    if (!geomPartId.IsValid())
        return BentleyStatus::ERROR;

    DbGeomPartsReader reader (GetDgnDb());

    return reader.ReadGeomPart(geometryBlob, code, geomPartId);
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
PhysicalGeometryPtr PhysicalGeometry::Create()
    {
    return new PhysicalGeometry();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnGeomPartPtr DgnGeomPart::Create(Utf8CP code)
    {
    return new DgnGeomPart(code);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
PlacedGeomPart::PlacedGeomPart(DgnDbR db, DgnGeomPartId geomPartId, DPoint3dCR origin, YawPitchRollAnglesCR angles)
    : m_origin(origin), m_angles(angles)
    {
    m_partPtr = db.GeomParts().LoadGeomPart(geomPartId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2014
//---------------------------------------------------------------------------------------
ElementAlignedBox3d PhysicalGeometry::CalculateBoundingBox() const
    {
    ElementAlignedBox3d overallBoundingBox;
    ElementAlignedBox3d partBoundingBox;

    for (PlacedGeomPart part : *this)
        {
        Transform   partToAspect = part.GetGeomPartToGeomAspectTransform();

        bvector<ElementGeometryPtr> const& partGeometry = part.GetPartPtr()->GetGeometry();

        for (ElementGeometryPtr elemGeom : partGeometry)
            {
            if (elemGeom->GetRange(partBoundingBox, &partToAspect))
                overallBoundingBox.Extend(partBoundingBox);
            }
        }

    return overallBoundingBox;
    }

