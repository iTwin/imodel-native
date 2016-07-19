/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GeomPart.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define GEOMPART_BBox "BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z"
#define GEOMPART_Code "Code_AuthorityId,Code_Namespace,Code_Value"
#define GEOMPART_GeometryStream "GeometryStream"

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  03/15
//=======================================================================================
struct DbGeometryPartsWriter
{
protected:
    DgnDbR m_dgndb;
    BeSQLite::SnappyToBlob m_snappy;
    BeSQLite::CachedStatementPtr m_stmt;

    void ExecStatement();
    void PrepareInsertStatement();
    void PrepareUpdateStatement();
    StatusInt SaveGeometryPartToRow(GeomStreamCR, DgnCodeCR code, DgnGeometryPartId geomPartId, ElementAlignedBox3dCR bbox);
    void BindBoundingBox(ElementAlignedBox3dCR box);
    void BindCode(DgnCodeCR code);

public:
    DbGeometryPartsWriter(DgnDbR db) : m_dgndb(db) {}

    DgnGeometryPartId InsertGeometryPart(GeomStreamCR, DgnCodeCR code, ElementAlignedBox3dCR bbox);
    BentleyStatus UpdateGeometryPart(DgnGeometryPartId geomPartId, GeomStreamCR, DgnCodeCR code, ElementAlignedBox3dCR bbox);

    enum Column : int // Must match columns in PrepareInsertStatement & PrepareUpdateStatement
    {
        Id = 1,
        Authority = 2,
        Namespace = 3,
        Name = 4,
        GeometryStream = 5,
        LowX = 6,
        LowY = 7,
        LowZ = 8,
        HighX = 9,
        HighY = 10,
        HighZ = 11
    };
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeometryPartsWriter::PrepareInsertStatement()
    {
    Utf8CP insertSql =
            "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_GeometryPart) "("
                "Id,"                       // 1
                GEOMPART_Code ","           // 2,3,4
                GEOMPART_GeometryStream "," // 5
                GEOMPART_BBox               // 6..11
            ")VALUES(?,?,?,?,?,?,?,?,?,?,?)";

    m_dgndb.GetCachedStatement(m_stmt, insertSql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeometryPartsWriter::PrepareUpdateStatement()
    {
    Utf8CP updateSql =
            "UPDATE " DGN_TABLE(DGN_CLASSNAME_GeometryPart) " SET "
                "Code_AuthorityId=?2,"
                "Code_Namespace=?3,"
                "Code_Value=?4,"
                GEOMPART_GeometryStream "=?5,"
                "BBoxLow_X=?6,BBoxLow_Y=?7,BBoxLow_Z=?8,"
                "BBoxHigh_X=?9,BBoxHigh_Y=?10,BBoxHigh_Z=?11"
            " WHERE Id=?1";

    m_dgndb.GetCachedStatement(m_stmt, updateSql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeometryPartsWriter::ExecStatement()
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
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeometryPartsWriter::BindBoundingBox(ElementAlignedBox3dCR box)
    {
    m_stmt->BindDouble(Column::LowX, box.GetLeft());
    m_stmt->BindDouble(Column::LowY, box.GetFront());
    m_stmt->BindDouble(Column::LowZ, box.GetBottom());
    m_stmt->BindDouble(Column::HighX, box.GetRight());
    m_stmt->BindDouble(Column::HighY, box.GetBack());
    m_stmt->BindDouble(Column::HighZ, box.GetTop());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DbGeometryPartsWriter::BindCode(DgnCodeCR code)
    {
    m_stmt->BindId(Column::Authority, code.GetAuthority());
    m_stmt->BindText(Column::Namespace, code.GetNamespace(), Statement::MakeCopy::No);
    if (code.IsEmpty())
        m_stmt->BindNull(Column::Name);
    else
        m_stmt->BindText(Column::Name, code.GetValue(), Statement::MakeCopy::No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DbGeometryPartsWriter::SaveGeometryPartToRow(GeomStreamCR geom, DgnCodeCR code, DgnGeometryPartId geomPartId, ElementAlignedBox3dCR box)
    {
    m_stmt->BindId(Column::Id, geomPartId);

    BindCode(code);
    BindBoundingBox(box);

    if (0 == geom.GetSize())
        {
        ExecStatement();
        return SUCCESS; // Is this an error?!?
        }

    return (DgnDbStatus::Success == geom.WriteGeomStreamAndStep(m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeometryPart), GEOMPART_GeometryStream, geomPartId.GetValue(), *m_stmt, Column::GeometryStream))? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DbGeometryPartsWriter::InsertGeometryPart(GeomStreamCR geom, DgnCodeCR code, ElementAlignedBox3dCR bbox)
    {
    DgnGeometryPartId geomPartId = m_dgndb.GeometryParts().MakeNewGeometryPartId();

    if (!geomPartId.IsValid())
        return DgnGeometryPartId();

    PrepareInsertStatement();

    return (SUCCESS == SaveGeometryPartToRow(geom, code, geomPartId, bbox) ? geomPartId : DgnGeometryPartId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DbGeometryPartsWriter::UpdateGeometryPart(DgnGeometryPartId geomPartId, GeomStreamCR geom, DgnCodeCR code, ElementAlignedBox3dCR bbox)
    {
    if (!geomPartId.IsValid())
        return ERROR;

    PrepareUpdateStatement();

    return (SUCCESS == SaveGeometryPartToRow(geom, code, geomPartId, bbox) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DgnGeometryParts::MakeNewGeometryPartId()
    {
    if (!m_highestGeometryPartId.IsValid())
        {
        m_highestGeometryPartId = DgnGeometryPartId(m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeometryPart), "Id");
        }
    else
        {
        m_highestGeometryPartId.UseNext(m_dgndb);
        }

    return m_highestGeometryPartId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeometryParts::InsertGeometryPart(DgnGeometryPartR geomPart)
    {
    if (!geomPart.GetCode().IsValid())
        geomPart.SetCode(geomPart.GenerateDefaultCode());

    if (CodeStatus::Success != GetDgnDb().Codes().ReserveCode(geomPart.GetCode()))
        return BentleyStatus::ERROR;    // NEEDSWORK return codes...

    DbGeometryPartsWriter writer(GetDgnDb());
    DgnGeometryPartId geomPartId = writer.InsertGeometryPart(geomPart.GetGeomStream(), geomPart.GetCode(), geomPart.GetBoundingBox());

    if (!geomPartId.IsValid())
        return BentleyStatus::ERROR;

    geomPart.SetId(geomPartId);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeometryParts::UpdateGeometryPart(DgnGeometryPartR geomPart)
    {
    if (CodeStatus::Success != GetDgnDb().Codes().ReserveCode(geomPart.GetCode()))
        return BentleyStatus::ERROR;    // NEEDSWORK return codes...

    DbGeometryPartsWriter writer(GetDgnDb());
    return writer.UpdateGeometryPart(geomPart.GetId(), geomPart.GetGeomStream(), geomPart.GetCode(), geomPart.GetBoundingBox());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeometryParts::InsertElementGeomUsesParts(DgnElementId elementId, DgnGeometryPartId geomPartId)
    {
    if (!elementId.IsValid() || !geomPartId.IsValid())
        return BentleyStatus::ERROR;

    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement(
        "INSERT INTO " DGN_SCHEMA(DGN_RELNAME_ElementUsesGeometryParts) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");

    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    statementPtr->BindId(1, elementId);
    statementPtr->BindId(2, geomPartId);

    if (BE_SQLITE_DONE != statementPtr->Step())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
DgnGeometryPartPtr DgnGeometryParts::LoadGeometryPart(DgnGeometryPartId geomPartId)
    {
    if (!geomPartId.IsValid())
        return nullptr;

    wt_OperationForGraphics hpo;
   
    auto& elements = m_dgndb.Elements();

    CachedStatementPtr stmt=elements.GetStatement("SELECT " GEOMPART_Code "," GEOMPART_BBox "," GEOMPART_GeometryStream " FROM " DGN_TABLE(DGN_CLASSNAME_GeometryPart) " WHERE Id=?");
    stmt->BindId(1, geomPartId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    DgnCode code(stmt->GetValueId<DgnAuthorityId>(0), stmt->GetValueText(2), stmt->GetValueText(1));
    DgnGeometryPartPtr geomPartPtr = new DgnGeometryPart(GetDgnDb(), code);

    ElementAlignedBox3d bbox(stmt->GetValueDouble(3),stmt->GetValueDouble(4),stmt->GetValueDouble(5),stmt->GetValueDouble(6),stmt->GetValueDouble(7),stmt->GetValueDouble(8));
    geomPartPtr->SetBoundingBox(bbox);

    GeomStreamR    geom = geomPartPtr->GetGeomStreamR();
    DgnDbStatus status = stmt->IsColumnNull(9) ? DgnDbStatus::Success : geom.ReadGeomStream(GetDgnDb(), stmt->GetValueBlob(9), stmt->GetColumnBytes(9));
    if (DgnDbStatus::Success != status)
        return nullptr;

    geomPartPtr->SetId(geomPartId);
    return geomPartPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    05/2015
//---------------------------------------------------------------------------------------
DgnGeometryPartId DgnGeometryParts::QueryGeometryPartId(DgnCodeCR code)
    {
    // empty codes are not unique...
    if (!code.IsValid() || code.IsEmpty())
        return DgnGeometryPartId();

    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_GeometryPart) " WHERE Code_AuthorityId=? AND Code_Namespace=? AND Code_Value=?");
    stmt->BindId(1, code.GetAuthority());
    stmt->BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
    stmt->BindText(3, code.GetValue(), Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnGeometryPartId() : stmt->GetValueId<DgnGeometryPartId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeometryParts::QueryGeometryPartRange(DRange3dR range, DgnGeometryPartId geomPartId)
    {
    if (!geomPartId.IsValid())
        return ERROR;

    CachedStatementPtr stmt = GetDgnDb().Elements().GetStatement("SELECT " GEOMPART_BBox " FROM " DGN_TABLE(DGN_CLASSNAME_GeometryPart) " WHERE Id=?");
    stmt->BindId(1, geomPartId);

    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    ElementAlignedBox3d bbox(stmt->GetValueDouble(0), stmt->GetValueDouble(1), stmt->GetValueDouble(2), stmt->GetValueDouble(3), stmt->GetValueDouble(4), stmt->GetValueDouble(5));
    range = bbox;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeometryParts::DeleteGeometryPart(DgnGeometryPartId geomPartId)
    {
    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement("DELETE FROM ONLY " DGN_SCHEMA(DGN_CLASSNAME_GeometryPart) " WHERE ECInstanceId=?");
    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    if (ECSqlStatus::Success != statementPtr->BindId(1, geomPartId))
        return BentleyStatus::ERROR;

    if (BE_SQLITE_DONE != statementPtr->Step())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnGeometryPartPtr DgnGeometryPart::Create(DgnDbR db, DgnCode code)
    {
    return new DgnGeometryPart(db, code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DgnImportContext::RemapGeometryPartId(DgnGeometryPartId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnGeometryPartId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    DgnGeometryPartPtr sourceGeometryPart = GetSourceDb().GeometryParts().LoadGeometryPart(source);
    if (!sourceGeometryPart.IsValid())
        return DgnGeometryPartId();

    dest = GetDestinationDb().GeometryParts().QueryGeometryPartId(sourceGeometryPart->GetCode());

    if (!dest.IsValid())
        {
        DgnGeometryPartPtr destGeometryPart = DgnGeometryPart::Create(GetDestinationDb());
        ElementGeomIO::Import(destGeometryPart->GetGeomStreamR(), sourceGeometryPart->GetGeomStream(), *this);

        destGeometryPart->SetBoundingBox(sourceGeometryPart->GetBoundingBox());

        if (BSISUCCESS != GetDestinationDb().GeometryParts().InsertGeometryPart(*destGeometryPart))
            {
            BeAssert(false);
            return DgnGeometryPartId();
            }
        dest = destGeometryPart->GetId();
        }

    return m_remap.Add(source, dest);
    }

