/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GeomPart.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    void const* blob = statement.GetValueBlob(geometryStreamIndex, &blobSize);
    status = m_geometry.ReadGeometryStream(GetDgnDb().Elements().GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    if (DgnDbStatus::Success != status)
        return status;

    DPoint3d bboxLow = statement.GetValuePoint3d(params.GetSelectIndex(PARAM_BBoxLow));
    DPoint3d bboxHigh = statement.GetValuePoint3d(params.GetSelectIndex(PARAM_BBoxHigh));
    m_bbox = ElementAlignedBox3d(bboxLow.x, bboxLow.y, bboxLow.z, bboxHigh.x, bboxHigh.y, bboxHigh.z);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_BindWriteParams(ECSqlStatement& statement, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(statement, forInsert);
    statement.BindPoint3d(statement.GetParameterIndex(PARAM_BBoxLow), m_bbox.low);
    statement.BindPoint3d(statement.GetParameterIndex(PARAM_BBoxHigh), m_bbox.high);
    m_geometry.BindGeometryStream(m_multiChunkGeomStream, GetDgnDb().Elements().GetSnappyTo(), statement, PARAM_GeometryStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnGeometryPart::_SetPropertyValue(ElementECPropertyAccessor& accessor, ECN::ECValueCR value, PropertyArrayIndex const& arrayIdx)
    {
    // *** WIP_PROPERTIES - DON'T OVERRIDE _GET/SETPROPERTYVALUE - handler should register property accessors instead
    auto name = accessor.GetAccessString();
    if (0 == strcmp(PARAM_BBoxLow, name))
        {
        m_bbox.low = value.GetPoint3d();
        return DgnDbStatus::Success;
        }
    else if (0 == strcmp(PARAM_BBoxHigh, name))
        {
        m_bbox.high = value.GetPoint3d();
        return DgnDbStatus::Success;
        }

    return T_Super::_SetPropertyValue(accessor, value, arrayIdx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnGeometryPart::_GetPropertyValue(ECN::ECValueR value, ElementECPropertyAccessor& accessor, PropertyArrayIndex const& arrayIdx) const
    {
    // *** WIP_PROPERTIES - DON'T OVERRIDE _GET/SETPROPERTYVALUE - handler should register property accessors instead
    auto name = accessor.GetAccessString();
    if (0 == strcmp(PARAM_BBoxLow, name))
        {
        value.SetPoint3d(m_bbox.low);
        return DgnDbStatus::Success;
        }
    else if (0 == strcmp(PARAM_BBoxHigh, name))
        {
        value.SetPoint3d(m_bbox.high);
        return DgnDbStatus::Success;
        }

    return T_Super::_GetPropertyValue(value, accessor, arrayIdx);
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
    return GeometryStream::WriteGeometryStream(db.Elements().GetSnappyTo(), db, GetElementId(), BIS_CLASS_GeometryPart, PARAM_GeometryStream);
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
DgnGeometryPartId DgnGeometryPart::QueryGeometryPartId(DgnDbR db, DgnCodeCR code)
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

    CachedECSqlStatementPtr statementPtr = db.GetNonSelectPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_ElementUsesGeometryParts) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)", db.GetECCrudWriteToken());

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

    DPoint3d bboxLow = statement->GetValuePoint3d(0);
    DPoint3d bboxHigh = statement->GetValuePoint3d(1);
    ElementAlignedBox3d bbox(bboxLow.x, bboxLow.y, bboxLow.z, bboxHigh.x, bboxHigh.y, bboxHigh.z);
    range = bbox;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    04/2016
//---------------------------------------------------------------------------------------
DgnGeometryPartPtr DgnGeometryPart::Create(DgnDbR db, DgnCodeCR code)
    {
    DgnModelId modelId = DgnModel::DictionaryId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::GeometryPart::GetHandler());

    return new DgnGeometryPart(CreateParams(db, modelId, classId, code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DgnImportContext::_RemapGeometryPartId(DgnGeometryPartId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnGeometryPartId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    DgnGeometryPartCPtr sourceGeometryPart = GetSourceDb().Elements().Get<DgnGeometryPart>(source);
    if (!sourceGeometryPart.IsValid())
        return DgnGeometryPartId();

    DgnCode destCode = sourceGeometryPart->GetCode();
    dest = DgnGeometryPart::QueryGeometryPartId(GetDestinationDb(), destCode);

    if (!dest.IsValid())
        {
        DgnGeometryPartPtr destGeometryPart = DgnGeometryPart::Create(GetDestinationDb(), destCode);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::GeometryPart::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

#define GETBBOXPROP(EXPR) [](ECValueR value, DgnElementCR elIn)      {auto& el = (DgnGeometryPart&)elIn; ElementAlignedBox3dCR bbox = el.GetBoundingBox(); EXPR; return DgnDbStatus::Success;}
#define SETBBOXPROP(EXPR) [](DgnElement& elIn, ECN::ECValueCR value) {auto& el = (DgnGeometryPart&)elIn; ElementAlignedBox3d   bbox = el.GetBoundingBox(); EXPR; el.SetBoundingBox(bbox); return DgnDbStatus::Success;}

    params.RegisterPropertyAccessors(layout, PARAM_BBoxLow, 
        GETBBOXPROP(value.SetPoint3d(bbox.low)),
        SETBBOXPROP(bbox.low = value.GetPoint3d()));

    params.RegisterPropertyAccessors(layout, PARAM_BBoxHigh, 
        GETBBOXPROP(value.SetPoint3d(bbox.high)),
        SETBBOXPROP(bbox.high = value.GetPoint3d()));

#undef GETBBOXPROP
#undef SETBBOXPROP

    params.RegisterPropertyAccessors(layout, PARAM_GeometryStream, 
        [](ECValueR, DgnElementCR)
            {
            return DgnDbStatus::BadRequest;//  => Use GeometryCollection interface
            },

        [](DgnElementR, ECValueCR)
            {
            return DgnDbStatus::BadRequest;//  => Use GeometryBuilder
            });

    }