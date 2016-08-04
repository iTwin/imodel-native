/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GeomPart.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PARAM_BBoxLow "BBoxLow"
#define PARAM_BBoxHigh "BBoxHigh"
#define PARAM_GeometryStream "GeometryStream"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_ReadSelectParams(ECSqlStatement& statement, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(statement, params);
    if (DgnDbStatus::Success != status)
        return status;

    int geometryStreamIndex = params.GetSelectIndex(PARAM_GeometryStream);
    if (statement.IsValueNull(geometryStreamIndex))
        return DgnDbStatus::BadElement;

    int blobSize;
    void const* blob = statement.GetValueBinary(geometryStreamIndex, &blobSize);
    status = m_geometry.ReadGeometryStream(GetDgnDb().Elements().GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    if (DgnDbStatus::Success != status)
        return status;

    DPoint3d bboxLow = statement.GetValuePoint3D(params.GetSelectIndex(PARAM_BBoxLow));
    DPoint3d bboxHigh = statement.GetValuePoint3D(params.GetSelectIndex(PARAM_BBoxHigh));
    m_bbox = ElementAlignedBox3d(bboxLow.x, bboxLow.y, bboxLow.z, bboxHigh.x, bboxHigh.y, bboxHigh.z);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::BindParams(ECSqlStatement& statement)
    {
    statement.BindPoint3D(statement.GetParameterIndex(PARAM_BBoxLow), m_bbox.low);
    statement.BindPoint3D(statement.GetParameterIndex(PARAM_BBoxHigh), m_bbox.high);
    return m_geometry.BindGeometryStream(m_multiChunkGeomStream, GetDgnDb().Elements().GetSnappyTo(), statement, PARAM_GeometryStream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_BindInsertParams(ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindInsertParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    return BindParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_BindUpdateParams(ECSqlStatement& statement)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(statement);
    if (DgnDbStatus::Success != status)
        return status;

    return BindParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_InsertInDb()
    {
    DgnDbStatus status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteGeometryStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteGeometryStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::WriteGeometryStream()
    {
    if (!m_multiChunkGeomStream)
        return DgnDbStatus::Success;

    m_multiChunkGeomStream = false;
    DgnDbR db = GetDgnDb();
    return GeometryStream::WriteGeometryStream(db.Elements().GetSnappyTo(), db, GetElementId(), BIS_TABLE(BIS_CLASS_DefinitionElement), "sc01"); // NOTE: takes advantage of knowing how the EC --> SQLite mapping will turn out!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_CopyFrom(DgnElementCR element)
    {
    T_Super::_CopyFrom(element);

    DgnGeometryPartCP otherPart = element.ToGeometryPart();
    if (nullptr != otherPart)
        {
        GetGeometryStreamR() = otherPart->GetGeometryStream();
        SetBoundingBox(otherPart->GetBoundingBox());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DgnGeometryPart::QueryGeometryPartId(DgnCode const& code, DgnDbR db)
    {
    return DgnGeometryPartId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnGeometryPart::InsertElementUsesGeometryParts(DgnDbR db, DgnElementId elementId, DgnGeometryPartId geomPartId)
    {
    if (!elementId.IsValid() || !geomPartId.IsValid())
        return BentleyStatus::ERROR;

    CachedECSqlStatementPtr statementPtr = db.GetPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_ElementUsesGeometryParts) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");

    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    statementPtr->BindId(1, elementId);
    statementPtr->BindId(2, geomPartId);

    if (BE_SQLITE_DONE != statementPtr->Step())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeometryPart::QueryGeometryPartRange(DRange3dR range, DgnDbR db, DgnGeometryPartId geomPartId)
    {
    if (!geomPartId.IsValid())
        return BentleyStatus::ERROR;

    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement("SELECT " PARAM_BBoxLow "," PARAM_BBoxHigh " FROM " BIS_SCHEMA(BIS_CLASS_GeometryPart) " WHERE ECInstanceId=?");
    if (!statement.IsValid())
        return BentleyStatus::ERROR;

    statement->BindId(1, geomPartId);
    if (BE_SQLITE_ROW != statement->Step())
        return BentleyStatus::ERROR;

    DPoint3d bboxLow = statement->GetValuePoint3D(0);
    DPoint3d bboxHigh = statement->GetValuePoint3D(1);
    ElementAlignedBox3d bbox(bboxLow.x, bboxLow.y, bboxLow.z, bboxHigh.x, bboxHigh.y, bboxHigh.z);
    range = bbox;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2016
//---------------------------------------------------------------------------------------
DgnGeometryPartPtr DgnGeometryPart::Create(DgnDbR db, DgnCode code)
    {
    DgnModelId modelId = DgnModel::DictionaryId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::GeometryPart::GetHandler());

    return new DgnGeometryPart(CreateParams(db, modelId, classId, code));
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

    DgnGeometryPartCPtr sourceGeometryPart = GetSourceDb().Elements().Get<DgnGeometryPart>(source);
    if (!sourceGeometryPart.IsValid())
        return DgnGeometryPartId();

    dest = DgnGeometryPart::QueryGeometryPartId(sourceGeometryPart->GetCode(), GetDestinationDb());

    if (!dest.IsValid())
        {
        DgnGeometryPartPtr destGeometryPart = DgnGeometryPart::Create(GetDestinationDb());
        GeometryStreamIO::Import(destGeometryPart->GetGeometryStreamR(), sourceGeometryPart->GetGeometryStream(), *this);

        destGeometryPart->SetBoundingBox(sourceGeometryPart->GetBoundingBox());

        if (!GetDestinationDb().Elements().Insert<DgnGeometryPart>(*destGeometryPart).IsValid())
            {
            BeAssert(false);
            return DgnGeometryPartId();
            }
        dest = destGeometryPart->GetId();
        }

    return m_remap.Add(source, dest);
    }
