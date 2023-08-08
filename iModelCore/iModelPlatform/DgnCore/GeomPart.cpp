/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_ToJson(BeJsValue out, BeJsConst opts) const
    {
    T_Super::_ToJson(out, opts);
    BeJsGeomUtils::DRange3dToJson(out[json_bbox()], m_bbox);

    if (!opts["wantGeometry"].asBool())
        return;

    // load geometry
    GeometryCollection collection(m_geometry, GetDgnDb());
    collection.ToJson(out[json_geom()], opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_FromJson(BeJsConst val)
    {
    T_Super::_FromJson(val);

    auto elementGeometryBuilderParams = val[json_elementGeometryBuilderParams()];
    if (!elementGeometryBuilderParams.isNull()) {
        auto napiValue = elementGeometryBuilderParams.AsNapiValueRef();
        if (nullptr == napiValue) {
            throw std::invalid_argument("only supported from JavaScript");
        }
        auto napiObj = napiValue->m_napiVal.As<Napi::Object>();

        if (napiObj.Has("viewIndependent") || napiObj.Has("isWorld")) { // make sure the caller is not confused about what kind of element this is for
            BeNapi::ThrowJsException(m_dgndb.GetJsIModelDb()->Env(), "BuildGeometryStream failed - invalid builder parameter", (int)DgnDbStatus::BadArg);
            return;
        }

        auto is2dPartVal = napiObj.Get("is2dPart");
        auto entryArrayObj = napiObj.Get("entryArray");
        BeAssert(is2dPartVal.IsUndefined() || is2dPartVal.IsBoolean());
        BeAssert(entryArrayObj.IsArray());

        GeometryBuilderParams bparams;
        bparams.is2dPart = is2dPartVal.IsBoolean() && is2dPartVal.As<Napi::Boolean>().Value();

        auto status = GeometryStreamIO::BuildGeometryStream(*this, bparams, entryArrayObj.As<Napi::Array>());
        if (DgnDbStatus::Success != status) {
            // throw std::runtime_error("BuildGeometryStream failed");
            BeNapi::ThrowJsException(m_dgndb.GetJsIModelDb()->Env(), "BuildGeometryStream failed", (int)status);
        }
        return;
    }

    if (val.isMember(json_geom()))
        {
        // NOTE: Bounding box should not be updated from json, the GeometryBuilder computes the correct range from the GeometryStream...
        GeometryBuilder::UpdateFromJson(*this, val[json_geom()]);
        }
    else if (val.isMember(json_geomBinary()))
        {
        val[json_geomBinary()].GetBinary(m_geometry);
        if (val.isMember(json_bbox()))
            BeJsGeomUtils::DRange3dFromJson(m_bbox, val[json_bbox()]); // use the existing bounding box when the GeometryStream is cloned as binary
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_BindWriteParams(ECSqlStatement& statement, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(statement, forInsert);
    statement.BindPoint3d(statement.GetParameterIndex(prop_BBoxLow()), m_bbox.low);
    statement.BindPoint3d(statement.GetParameterIndex(prop_BBoxHigh()), m_bbox.high);
    m_geometry.BindGeometryStream(m_multiChunkGeomStream, GetDgnDb().Elements().GetSnappyTo(), statement, prop_GeometryStream());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_OnInsert()
    {
    return m_geometry.HasGeometry() ? T_Super::_OnInsert() : DgnDbStatus::BadElement; // can't insert a DgnGeometryPart without geometry
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_InsertInDb()
    {
    DgnDbStatus status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteGeometryStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteGeometryStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnGeometryPart::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryPart::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    importer.RemapGeometryStreamIds(m_geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId DgnGeometryPart::QueryGeometryPartId(DgnDbR db, DgnCodeCR code)
    {
    return DgnGeometryPartId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
DgnGeometryPartPtr DgnGeometryPart::Create(DgnDbR db, DgnCodeCR code)
    {
    DgnModelId modelId = DgnModel::DictionaryId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::GeometryPart::GetHandler());
    return new DgnGeometryPart(CreateParams(db, modelId, classId, code));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
* @bsimethod
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
* @bsimethod
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
