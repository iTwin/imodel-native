/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_ReadSelectParams(ECSqlStatement& statement, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(statement, params);
    if (DgnDbStatus::Success != status)
        return status;

    int geometryStreamIndex = params.GetSelectIndex(prop_GeometryStream());
    if (statement.IsValueNull(geometryStreamIndex))
        return DgnDbStatus::BadElement;

    int blobSize;
    void const* blob = statement.GetValueBlob(geometryStreamIndex, &blobSize);
    status = m_geometry.ReadGeometryStream(GetDgnDb().Elements().GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    if (DgnDbStatus::Success != status)
        return status;

    DPoint3d bboxLow = statement.GetValuePoint3d(params.GetSelectIndex(prop_BBoxLow()));
    DPoint3d bboxHigh = statement.GetValuePoint3d(params.GetSelectIndex(prop_BBoxHigh()));
    m_bbox = ElementAlignedBox3d(bboxLow.x, bboxLow.y, bboxLow.z, bboxHigh.x, bboxHigh.y, bboxHigh.z);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_ToJson(JsonValueR out, JsonValueCR opts) const
    {
    T_Super::_ToJson(out, opts);
    JsonUtils::DRange3dToJson(out[json_bbox()], m_bbox);

    if (!opts["wantGeometry"].asBool())
        return;

    // load geometry
    GeometryCollection collection(m_geometry, GetDgnDb());
    out[json_geom()] = collection.ToJson(opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_FromJson(JsonValueR val)
    {
    T_Super::_FromJson(val);

    // NOTE: Bounding box should not be updated from json, the GeometryBuilder computes the correct range from the GeometryStream...
    if (val.isMember(json_geom()))
        GeometryBuilder::UpdateFromJson(*this, val[json_geom()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_BindWriteParams(ECSqlStatement& statement, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(statement, forInsert);
    statement.BindPoint3d(statement.GetParameterIndex(prop_BBoxLow()), m_bbox.low);
    statement.BindPoint3d(statement.GetParameterIndex(prop_BBoxHigh()), m_bbox.high);
    m_geometry.BindGeometryStream(m_multiChunkGeomStream, GetDgnDb().Elements().GetSnappyTo(), statement, prop_GeometryStream());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_OnInsert()
    {
    return m_geometry.HasGeometry() ? T_Super::_OnInsert() : DgnDbStatus::BadElement; // can't insert a DgnGeometryPart without geometry
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
    return GeometryStream::WriteGeometryStream(db.Elements().GetSnappyTo(), db, GetElementId(), BIS_CLASS_GeometryPart, prop_GeometryStream());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_CopyFrom(DgnElementCR element, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(element, opts);

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeometryPart::QueryGeometryPartRange(DRange3dR range, DgnDbR db, DgnGeometryPartId geomPartId)
    {
    if (!geomPartId.IsValid())
        return BentleyStatus::ERROR;

    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement("SELECT BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometryPart) " WHERE ECInstanceId=?");
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartPtr DgnGeometryPart::Create(DefinitionModelR model, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId modelId = model.GetModelId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::GeometryPart::GetHandler());
    DgnCode code = DgnGeometryPart::CreateCode(model, name);
    return new DgnGeometryPart(CreateParams(db, modelId, classId, code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DgnImportContext::_RemapGeometryPartId(DgnGeometryPartId sourceGeometryPartId)
    {
    if (!IsBetweenDbs())
        return sourceGeometryPartId;

    DgnGeometryPartId destGeometryPartId = m_remap.Find(sourceGeometryPartId);
    if (destGeometryPartId.IsValid())
        return destGeometryPartId;

    DgnGeometryPartCPtr sourceGeometryPart = GetSourceDb().Elements().Get<DgnGeometryPart>(sourceGeometryPartId);
    if (!sourceGeometryPart.IsValid())
        return DgnGeometryPartId();

    DgnModelId destModelId = m_remap.Find(sourceGeometryPart->GetModelId());
    if (!destModelId.IsValid())
        return DgnGeometryPartId();

    DefinitionModelPtr destModel = GetDestinationDb().Models().Get<DefinitionModel>(destModelId);
    if (!destModel.IsValid())
        return DgnGeometryPartId();

    DgnGeometryPartPtr destGeometryPart = DgnGeometryPart::Create(*destModel, sourceGeometryPart->GetCode().GetValue().GetUtf8());
    if (!destGeometryPart.IsValid())
        return DgnGeometryPartId();

    GeometryStreamIO::Import(destGeometryPart->GetGeometryStreamR(), sourceGeometryPart->GetGeometryStream(), *this);
    destGeometryPart->SetBoundingBox(sourceGeometryPart->GetBoundingBox());

    if (!GetDestinationDb().Elements().Insert<DgnGeometryPart>(*destGeometryPart).IsValid())
        return DgnGeometryPartId();

    return m_remap.Add(sourceGeometryPartId, destGeometryPart->GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::GeometryPart::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

#define GETBBOXPROP(EXPR) [](ECValueR value, DgnElementCR elIn)      {auto const& el = (DgnGeometryPart const&)elIn; ElementAlignedBox3dCR bbox = el.GetBoundingBox(); EXPR; return DgnDbStatus::Success;}
#define SETBBOXPROP(EXPR) [](DgnElement& elIn, ECN::ECValueCR value) \
    {                                                                \
    if (value.IsNull() || !value.IsPoint3d())                        \
        return DgnDbStatus::BadArg;                                  \
    auto& el = (DgnGeometryPart&)elIn;                               \
    ElementAlignedBox3d bbox = el.GetBoundingBox();                  \
    EXPR;                                                            \
    el.SetBoundingBox(bbox);                                         \
    return DgnDbStatus::Success;                                     \
    }

    params.RegisterPropertyAccessors(layout, DgnGeometryPart::prop_BBoxLow(),
        GETBBOXPROP(value.SetPoint3d(bbox.low)),
        SETBBOXPROP(bbox.low = value.GetPoint3d()));

    params.RegisterPropertyAccessors(layout, DgnGeometryPart::prop_BBoxHigh(),
        GETBBOXPROP(value.SetPoint3d(bbox.high)),
        SETBBOXPROP(bbox.high = value.GetPoint3d()));

#undef GETBBOXPROP
#undef SETBBOXPROP

    params.RegisterPropertyAccessors(layout, DgnGeometryPart::prop_GeometryStream(),
        [](ECValueR, DgnElementCR)
            {
            return DgnDbStatus::BadRequest;//  => Use GeometryCollection interface
            },

        [](DgnElementR, ECValueCR)
            {
            return DgnDbStatus::BadRequest;//  => Use GeometryBuilder
            });

    }
