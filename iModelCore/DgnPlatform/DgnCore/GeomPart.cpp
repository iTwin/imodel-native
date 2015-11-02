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
struct DbGeomPartsWriter
{
protected:
    DgnDbR m_dgndb;
    BeSQLite::SnappyToBlob m_snappy;
    BeSQLite::CachedStatementPtr m_stmt;

    void ExecStatement();
    void PrepareInsertStatement();
    void PrepareUpdateStatement();
    StatusInt SaveGeomPartToRow(GeomStreamCR, Utf8CP code, DgnGeomPartId geomPartId);

public:
    DbGeomPartsWriter(DgnDbR db) : m_dgndb(db) {}

    DgnGeomPartId InsertGeomPart(GeomStreamCR, Utf8CP code);
    BentleyStatus UpdateGeomPart(DgnGeomPartId geomPartId, GeomStreamCR, Utf8CP code);

    static int GetColumnIndexForId()   {return 1;} // Must match columns in PrepareInsertStatement & PrepareUpdateStatement
    static int GetColumnIndexForCode() {return 2;} // Must match columns in PrepareInsertStatement & PrepareUpdateStatement
    static int GetColumnIndexForGeom() {return 3;} // Must match columns in PrepareInsertStatement & PrepareUpdateStatement
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
                "Geom=?3"
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
StatusInt DbGeomPartsWriter::SaveGeomPartToRow(GeomStreamCR geom, Utf8CP code, DgnGeomPartId geomPartId)
    {
    m_stmt->BindId(GetColumnIndexForId(), geomPartId);

    if (code)
        m_stmt->BindText(GetColumnIndexForCode(), code, Statement::MakeCopy::No);
    else
        m_stmt->BindNull(GetColumnIndexForCode());

    if (0 == geom.GetSize())
        {
        ExecStatement();
        return SUCCESS; // Is this an error?!?
        }

    return (DgnDbStatus::Success == geom.WriteGeomStreamAndStep(m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeomPart), "Geom", geomPartId.GetValue(), *m_stmt, GetColumnIndexForGeom()))? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeomPartId DbGeomPartsWriter::InsertGeomPart(GeomStreamCR geom, Utf8CP code)
    {
    DgnGeomPartId geomPartId = m_dgndb.GeomParts().MakeNewGeomPartId();

    if (!geomPartId.IsValid())
        return DgnGeomPartId();

    PrepareInsertStatement();

    return (SUCCESS == SaveGeomPartToRow(geom, code, geomPartId) ? geomPartId : DgnGeomPartId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DbGeomPartsWriter::UpdateGeomPart(DgnGeomPartId geomPartId, GeomStreamCR geom, Utf8CP code)
    {
    if (!geomPartId.IsValid())
        return ERROR;

    PrepareUpdateStatement();

    return (SUCCESS == SaveGeomPartToRow(geom, code, geomPartId) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeomPartId DgnGeomParts::MakeNewGeomPartId()
    {
    if (!m_highestGeomPartId.IsValid())
        {
        m_highestGeomPartId = DgnGeomPartId(m_dgndb, DGN_TABLE(DGN_CLASSNAME_GeomPart), "Id");
        }
    else
        {
        m_highestGeomPartId.UseNext(m_dgndb);
        }

    return m_highestGeomPartId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::InsertGeomPart(DgnGeomPartR geomPart)
    {
    DbGeomPartsWriter writer(GetDgnDb());
    DgnGeomPartId geomPartId = writer.InsertGeomPart(geomPart.GetGeomStream(), geomPart.GetCode());

    if (!geomPartId.IsValid())
        return BentleyStatus::ERROR;

    geomPart.SetId(geomPartId);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  02/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::UpdateGeomPart(DgnGeomPartR geomPart)
    {
    DbGeomPartsWriter writer(GetDgnDb());
    return writer.UpdateGeomPart(geomPart.GetId(), geomPart.GetGeomStream(), geomPart.GetCode());
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

    if (BE_SQLITE_DONE != statementPtr->Step())
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

    wt_OperationForGraphics hpo;
   
    auto& elements = m_dgndb.Elements();

    CachedStatementPtr stmt=elements.GetStatement("SELECT Code FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " WHERE Id=?");
    stmt->BindId(1, geomPartId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    DgnGeomPartPtr geomPartPtr = new DgnGeomPart(stmt->GetValueText(0));
    GeomStreamR    geom = geomPartPtr->GetGeomStreamR();

    if (DgnDbStatus::Success != geom.ReadGeomStream(GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_GeomPart), "Geom", geomPartId.GetValue()))
        return nullptr;

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
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeomParts::DeleteGeomPart(DgnGeomPartId geomPartId)
    {
    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement("DELETE FROM ONLY " DGN_SCHEMA(DGN_CLASSNAME_GeomPart) " WHERE ECInstanceId=?");
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
DgnGeomPartPtr DgnGeomPart::Create(Utf8CP code)
    {
    return new DgnGeomPart(code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeomPartId DgnImportContext::RemapGeomPartId(DgnGeomPartId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnGeomPartId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    DgnGeomPartPtr sourceGeomPart = GetSourceDb().GeomParts().LoadGeomPart(source);
    if (!sourceGeomPart.IsValid())
        return DgnGeomPartId();

#ifdef WIP_GEOM_PART_COPYING // *** We can't rely on a GeomPart's code as an identifier. First, it's optional. Second, there's no unique constraint on it.
    dest = GetDestinationDb().GeomParts().QueryGeomPartId(sourceGeomPart->GetCode());
#endif
    if (!dest.IsValid())
        {
        DgnGeomPartPtr destGeomPart = DgnGeomPart::Create();
        ElementGeomIO::Import(destGeomPart->GetGeomStreamR(), sourceGeomPart->GetGeomStream(), *this);

        if (BSISUCCESS != GetDestinationDb().GeomParts().InsertGeomPart(*destGeomPart))
            {
            BeAssert(false);
            return DgnGeomPartId();
            }
        dest = destGeomPart->GetId();
        }

    return m_remap.Add(source, dest);
    }
