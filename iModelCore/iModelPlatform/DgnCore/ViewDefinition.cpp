/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PROPNAME_Description "Description"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace ViewElementHandler
{
    HANDLER_DEFINE_MEMBERS(View);
    HANDLER_DEFINE_MEMBERS(View3d);
    HANDLER_DEFINE_MEMBERS(View2d);
    HANDLER_DEFINE_MEMBERS(DrawingView);
    HANDLER_DEFINE_MEMBERS(SheetView);
    HANDLER_DEFINE_MEMBERS(SpatialView);
    HANDLER_DEFINE_MEMBERS(TemplateView2d);
    HANDLER_DEFINE_MEMBERS(TemplateView3d);
    HANDLER_DEFINE_MEMBERS(OrthographicView);
    HANDLER_DEFINE_MEMBERS(ViewModels);
    HANDLER_DEFINE_MEMBERS(ViewCategories);
    HANDLER_DEFINE_MEMBERS(ViewDisplayStyle);
    HANDLER_DEFINE_MEMBERS(ViewDisplayStyle2d);
    HANDLER_DEFINE_MEMBERS(ViewDisplayStyle3d);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

namespace ViewProperties
{
    BE_PROP_NAME(ModelSelector)
    BE_PROP_NAME(CategorySelector)
    BE_PROP_NAME(DisplayStyle)
    BE_PROP_NAME(LensAngle)
    BE_PROP_NAME(FocusDistance)
    BE_PROP_NAME(EyePoint)
    BE_PROP_NAME(BaseModel)
    BE_PROP_NAME(Origin)
    BE_PROP_NAME(Extents)
    BE_PROP_NAME(RotationAngle)
    BE_PROP_NAME(Yaw)
    BE_PROP_NAME(Pitch)
    BE_PROP_NAME(Roll)
    BE_PROP_NAME(IsCameraOn)
    BE_PROP_NAME(jpeg)
    BE_PROP_NAME(png)
};

using namespace ViewProperties;


BEGIN_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> void getThreadSafe(DgnDbR db, DgnElementId id, RefCountedPtr<T>& var)
    {
    if (!var.IsValid()) // first try without mutex
        {
        BeMutexHolder (db.Elements().GetMutex());
        if (!var.IsValid()) // test again after acquiring mutex
            {
            auto el = db.Elements().Get<T>(id);
            if (el.IsValid())
                var = el->template MakeCopy<T>();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void removeScheduleScriptElementIds(BeJsValue script)
    {
    script.ForEachArrayMemberValue([](BeJsValue::ArrayIndex, BeJsValue modelTimeline)
        {
        modelTimeline["elementTimelines"].ForEachArrayMemberValue([](BeJsValue::ArrayIndex, BeJsValue elementTimeline)
            {
            if (elementTimeline.isMember("elementIds"))
                elementTimeline["elementIds"] = "";

            return false;
            });

        return false;
        });
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::IsValidCode(DgnCodeCR code)
    {
    return !code.GetValue().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId ViewDefinition::QueryViewId(DgnDbR db, DgnCodeCR code)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return DgnViewId(elemId.GetValueUnchecked());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::_EqualState(ViewDefinitionR other)
    {
    if (IsPrivate()!= other.IsPrivate())
        return false;

    if (m_categorySelectorId != other.m_categorySelectorId)
        return false;

    if (m_displayStyleId != other.m_displayStyleId)
        return false;

    if (!GetCategorySelector().EqualState(other.GetCategorySelector()))
        return false;

    if (!GetDisplayStyle().EqualState(other.GetDisplayStyle()))
        return false;

    return GetDetails() == other.GetDetails();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_OnInsert()
    {
    if (!GetDisplayStyle().GetElementId().IsValid())
        {
        DgnDbStatus stat;
        m_displayStyle->Insert(&stat);
        if (DgnDbStatus::Success != stat)
            return stat;
        m_displayStyleId = m_displayStyle->GetElementId();
        }

    if (!GetCategorySelector().GetElementId().IsValid())
        {
        DgnDbStatus stat;
        m_categorySelector->Insert(&stat);
        if (DgnDbStatus::Success != stat)
            return stat;
        m_categorySelectorId = m_categorySelector->GetElementId();
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    BeAssert(GetDisplayStyleId().IsValid());
    BeAssert(GetCategorySelectorId().IsValid());
    auto stat = stmt.BindNavigationValue(stmt.GetParameterIndex(prop_DisplayStyle()), GetDisplayStyleId());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindNavigationValue(stmt.GetParameterIndex(prop_CategorySelector()), GetCategorySelectorId());
    BeAssert(ECSqlStatus::Success == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ViewDefinition::ToDetailJson()
    {
    _OnSaveJsonProperties();
    return GetDetails().Stringify();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::SetViewClip(ClipVectorPtr clip)
    {
    if (clip.IsValid())
        clip->ToJson(GetDetailR(json_clip()));
    else
        RemoveDetail(json_clip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ViewDefinition::GetViewClip() const
    {
    return ClipVector::FromJson(GetDetail(json_clip()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::SetGridSettings(GridOrientationType orientation, DPoint2dCR spacing, uint32_t gridsPerRef)
    {
    switch (orientation)
        {
        case GridOrientationType::WorldYZ:
        case GridOrientationType::WorldXZ:
            {
            if (!IsView3d())
                return;
            break;
            }

        case GridOrientationType::GeoCoord:
            {
            if (!IsSpatialView())
                return;
            break;
            }
        }

    auto details = GetDetailsR();
    details.SetOrRemoveUInt(json_gridOrient(), (uint32_t) orientation, (uint32_t)GridOrientationType::WorldXY);
    details.SetOrRemoveUInt(json_gridPerRef(), gridsPerRef, 10);
    details.SetOrRemoveDouble(json_gridSpaceX(), spacing.x, 1.0);
    details.SetOrRemoveDouble(json_gridSpaceY(), spacing.y, spacing.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::GetGridSettings(GridOrientationType& orientation, DPoint2dR spacing, uint32_t& gridsPerRef) const
    {
    orientation = (GridOrientationType) GetDetail(json_gridOrient()).asUInt((uint32_t) GridOrientationType::WorldXY);
    gridsPerRef = GetDetail(json_gridPerRef()).asUInt(10);
    spacing.x = GetDetail(json_gridSpaceX()).asDouble(1.0);
    spacing.y = GetDetail(json_gridSpaceY()).asDouble(spacing.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ViewDefinition::GetAuxiliaryCoordinateSystemId() const
    {
    return DgnElementId(GetDetail(json_acs()).asUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::SetAuxiliaryCoordinateSystem(DgnElementId acsId)
    {
    if (acsId.IsValid())
        GetDetailR(json_acs()) = acsId;
    else
        RemoveDetail(json_acs());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CategorySelectorR ViewDefinition::GetCategorySelector() {
    getThreadSafe<CategorySelector>(m_dgndb, GetCategorySelectorId(), m_categorySelector);
    if (!m_categorySelector.IsValid())
        m_dgndb.ThrowException("invalid categorySelector", (int) DgnDbStatus::BadElement);

    return *m_categorySelector;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleR ViewDefinition::GetDisplayStyle() {
    getThreadSafe<DisplayStyle>(m_dgndb, GetDisplayStyleId(), m_displayStyle);
    if (!m_displayStyle.IsValid())
        m_dgndb.ThrowException("invalid displayStyle", (int) DgnDbStatus::BadElement);

    return *m_displayStyle;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSelectorR SpatialViewDefinition::GetModelSelector() {
    getThreadSafe<ModelSelector>(m_dgndb, GetModelSelectorId(), m_modelSelector);
    if (!m_modelSelector.IsValid())
        m_dgndb.ThrowException("invalid modelSelector", (int) DgnDbStatus::BadElement);

    return *m_modelSelector;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_displayStyleId = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(prop_DisplayStyle()));
    m_categorySelectorId = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(prop_CategorySelector()));

    // NOTE: Const ViewDefinitions should never have their display styles or category selector set! You must get a writeable copy to have them.
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void limitWindowSize(ViewportStatus& error, double& value, double minDelta, double maxDelta)
    {
    if (value < minDelta)
        {
        value = minDelta;
        error = ViewportStatus::MinWindow;
        }
    else if (value > maxDelta)
        {
        value = maxDelta;
        error = ViewportStatus::MaxWindow;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition::ValidateViewDelta(DPoint3dR delta, bool messageNeeded)
    {
    ViewportStatus  error=ViewportStatus::Success, ignore;

    double maxExtent, minExtent;
    _GetExtentLimits(minExtent, maxExtent);

    limitWindowSize(error,  delta.x, minExtent, maxExtent);
    limitWindowSize(error,  delta.y, minExtent, maxExtent);
    limitWindowSize(ignore, delta.z, minExtent, maxExtent);

    return error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpatialViewDefinition::_ValidateState()
    {
    GetModelSelector();
    return (T_Super::_ValidateState() == SUCCESS && m_modelSelector.IsValid()) ? SUCCESS : ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewDefinition::_ValidateState()
    {
    GetDisplayStyle();
    GetCategorySelector();
    return (m_displayStyle.IsValid() && m_categorySelector.IsValid()) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    val[json_categorySelectorId()] = m_categorySelectorId;
    val[json_displayStyleId()] = m_displayStyleId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_FromJson(BeJsConst val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_categorySelectorId()))
        m_categorySelectorId.FromJson(val[json_categorySelectorId()]);

    if (val.isMember(json_displayStyleId()))
        m_displayStyleId.FromJson(val[json_displayStyleId()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);

    auto& other = static_cast<ViewDefinitionCR>(el);
    m_categorySelectorId = other.m_categorySelectorId;
    m_displayStyleId = other.m_displayStyleId;
    m_categorySelector = other.m_categorySelector.IsValid() ? other.m_categorySelector->MakeCopy<CategorySelector>() : nullptr;
    m_displayStyle = other.m_displayStyle.IsValid() ? other.m_displayStyle->MakeCopy<DisplayStyle>() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_RemapIds(DgnImportContext& importContext)
    {
    if (importContext.IsBetweenDbs())
        {
        m_categorySelectorId = importContext.FindElementId(m_categorySelectorId);
        m_displayStyleId = importContext.FindElementId(m_displayStyleId);
        }

    T_Super::_RemapIds(importContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    if (importer.IsBetweenDbs())
        {
        // We're not going to deep-copy the model in. We're expecting the user already copied it.
        m_baseModelId  = importer.FindModelId(GetBaseModelId());

        // NOTE: We're not copying or remapping the settings from the db's properties table...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    auto stat = stmt.BindPoint2d(stmt.GetParameterIndex(prop_Origin()), m_origin);
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindPoint2d(stmt.GetParameterIndex(prop_Extents()), m_delta);
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindDouble(stmt.GetParameterIndex(prop_RotationAngle()), Angle::FromRadians(m_rotAngle).Degrees());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindNavigationValue(stmt.GetParameterIndex(prop_BaseModel()), m_baseModelId);
    BeAssert(ECSqlStatus::Success == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition2d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    m_origin  = stmt.GetValuePoint2d(params.GetSelectIndex(prop_Origin()));
    m_delta   = DVec2d::From(stmt.GetValuePoint2d(params.GetSelectIndex(prop_Extents())));
    m_rotAngle = Angle::FromDegrees(stmt.GetValueDouble(params.GetSelectIndex(prop_RotationAngle()))).Radians();
    m_baseModelId = stmt.GetValueNavigation<DgnModelId>(params.GetSelectIndex(prop_BaseModel()));

    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    BeJsGeomUtils::DPoint2dToJson(val[json_origin()], m_origin);
    BeJsGeomUtils::DPoint2dToJson(val[json_delta()], m_delta);
    BeJsGeomUtils::AngleInDegreesToJson(val[json_angle()],AngleInDegrees::FromRadians(m_rotAngle));
    val[json_baseModelId()] = m_baseModelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_FromJson(BeJsConst val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_origin()))
        BeJsGeomUtils::DPoint2dFromJson(m_origin, val[json_origin()]);

    if (val.isMember(json_delta()))
        BeJsGeomUtils::DPoint2dFromJson(m_delta, val[json_delta()]);

    if (val.isMember(json_angle()))
        m_rotAngle = BeJsGeomUtils::AngleInDegreesFromJson(val[json_angle()]).Radians();

    if (val.isMember(json_baseModelId()))
        m_baseModelId.FromJson(val[json_baseModelId()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto const& other = (ViewDefinition2d const&) el;
    m_baseModelId = other.m_baseModelId;
    m_origin = other.m_origin;
    m_delta = other.m_delta;
    m_rotAngle = other.m_rotAngle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition2d::_EqualState(ViewDefinitionR in)
    {
    auto const& other= (ViewDefinition2d const&) in;
    if (m_baseModelId != other.m_baseModelId || !m_origin.IsEqual(other.m_origin) || !m_delta.IsEqual(other.m_delta) || m_rotAngle != other.m_rotAngle)
        return false;

    return T_Super::_EqualState(in);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition::Iterator::Iterator(DgnDbR db, Utf8CP whereClause, Utf8CP orderByClause)
    {
    Utf8String sql("SELECT ECInstanceId,CodeValue,IsPrivate," PROPNAME_Description ",ECClassId FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition));

    if (whereClause)
        {
        sql.append(" ");
        sql.append(whereClause);
        }

    if (orderByClause)
        {
        sql.append(" ");
        sql.append(orderByClause);
        }

    Prepare(db, sql.c_str(), 0 /* Index of ECInstanceId */);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ViewDefinition::QueryCount(DgnDbR db, Utf8CP whereClause)
    {
    Utf8String sql("SELECT COUNT(*) FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition));

    if (whereClause)
        {
        sql.append(" ");
        sql.append(whereClause);
        }

    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(sql.c_str());
    return statement.IsValid() && BE_SQLITE_ROW == statement->Step() ? statement->GetValueInt(0) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnViewId queryFirstPublicView(DgnDbR db, Utf8CP className)
    {
    Utf8PrintfString sql("SELECT ECInstanceId FROM %s WHERE IsPrivate=FALSE LIMIT 1", className);
    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(sql.c_str());
    BeAssert(statement.IsValid());
    return (statement.IsValid() && (BE_SQLITE_ROW == statement->Step())) ? statement->GetValueId<DgnViewId>(0) : DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId ViewDefinition::QueryDefaultViewId(DgnDbR db)
    {
    DgnViewId viewId;
    if (BeSQLite::BE_SQLITE_ROW == db.QueryProperty(&viewId, sizeof(viewId), DgnViewProperty::DefaultView()) && ViewDefinition::Get(db, viewId).IsValid())
        return viewId;

    viewId = queryFirstPublicView(db, BIS_SCHEMA(BIS_CLASS_SpatialViewDefinition));
    if (viewId.IsValid())
        return viewId;

    viewId = queryFirstPublicView(db, BIS_SCHEMA(BIS_CLASS_SheetViewDefinition));
    if (viewId.IsValid())
        return viewId;

    return queryFirstPublicView(db, BIS_SCHEMA(BIS_CLASS_DrawingViewDefinition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Desired> static bool isEntryOfClass(ViewDefinition::Entry const& entry)
    {
    DgnDbP db = entry.GetDgnDb();
    if (nullptr == db)
        return false;

    auto entryClass = db->Schemas().GetClass(entry.GetClassId());
    auto desiredClass = db->Schemas().GetClass(T_Desired::QueryClassId(*db));
    return nullptr != entryClass && nullptr != desiredClass && entryClass->Is(desiredClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::Entry::IsOrthographicView() const {return isEntryOfClass<OrthographicViewDefinition>(*this);}
bool ViewDefinition::Entry::IsView3d() const {return isEntryOfClass<ViewDefinition3d>(*this);}
bool ViewDefinition::Entry::IsSpatialView() const {return isEntryOfClass<SpatialViewDefinition>(*this);}
bool ViewDefinition::Entry::IsDrawingView() const {return isEntryOfClass<DrawingViewDefinition>(*this);}
bool ViewDefinition::Entry::IsSheetView() const {return isEntryOfClass<SheetViewDefinition>(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CategorySelector::_CopyFrom(DgnElementCR in, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(in, opts);
    auto const& other = (CategorySelector const&) in;
    m_categories = other.m_categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CategorySelector::_RemapIds(DgnImportContext& importContext)
    {
    if (importContext.IsBetweenDbs())
        {
        DgnCategoryIdSet remappedCategoryIds;
        for (auto const& categoryId: m_categories)
            {
            DgnCategoryId remappedCategoryId = importContext.FindCategory(categoryId);
            if (remappedCategoryId.IsValid())
                remappedCategoryIds.insert(remappedCategoryId);
            }

        m_categories = remappedCategoryIds;
        }

    T_Super::_RemapIds(importContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_InsertInDb()
    {
    auto status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteCategories();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CategorySelector::_ToJson(BeJsValue out, BeJsConst opts) const
    {
    T_Super::_ToJson(out, opts);
    auto categories = out[json_categories()];
    categories.SetEmptyArray();
    for (auto const& category: m_categories)
        {
        if (category.IsValid())
            categories.appendValue() = category;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CategorySelector::_FromJson(BeJsConst val)
    {
    T_Super::_FromJson(val);
    m_categories.clear();

    auto const& categories = val[json_categories()];
    int size = categories.size();
    for (int i=0; i<size; ++i)
        m_categories.insert(DgnCategoryId(categories[i].asUInt64()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CategorySelector::EqualState(CategorySelectorCR other) const
    {
    return m_categories == other.m_categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_OnUpdate(DgnElementCR el)
    {
    auto status = T_Super::_OnUpdate(el);
    if (DgnDbStatus::Success != status)
        return status;

    CategorySelectorCP original = dynamic_cast<CategorySelectorCP>(&el);
    if (nullptr == original)
        {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
        }

    if (!EqualState(*original))
        {
        auto delExisting = GetDgnDb().GetNonSelectPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " WHERE SourceECInstanceId=?", GetDgnDb().GetECCrudWriteToken());
        delExisting->BindId(1, GetElementId());
        delExisting->Step();
        return WriteCategories();
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::WriteCategories()
    {
    if (!GetElementId().IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::MissingId;
        }

    auto statement = GetDgnDb().GetNonSelectPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA("CategorySelectorRefersToCategories") " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)", GetDgnDb().GetECCrudWriteToken());
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    for (auto id : m_categories)
        {
        statement->Reset();
        statement->ClearBindings();
        statement->BindId(1, GetElementId());
        statement->BindId(2, id);
        if (BE_SQLITE_DONE != statement->Step())
            return DgnDbStatus::WriteError;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_LoadFromDb()
    {
    auto stat = T_Super::_LoadFromDb();
    if (stat  != DgnDbStatus::Success)
        return stat;

    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::BadSchema;

    statement->BindId(1, GetElementId());

    while (BE_SQLITE_ROW == statement->Step())
        m_categories.insert(statement->GetValueId<DgnCategoryId>(0));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_CopyFrom(DgnElementCR rhsElement, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(rhsElement, opts);

    auto const& rhs = (ModelSelector const&)rhsElement;
    m_models = rhs.m_models;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_RemapIds(DgnImportContext& importContext)
    {
    if (importContext.IsBetweenDbs())
        {
        DgnModelIdSet remappedModelIds;
        for (auto const& modelId: m_models)
            {
            DgnModelId remappedModelId = importContext.FindModelId(modelId);
            if (remappedModelId.IsValid())
                remappedModelIds.insert(remappedModelId);
            }

        m_models = remappedModelIds;
        }

    T_Super::_RemapIds(importContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_InsertInDb()
    {
    DgnDbStatus status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteModels();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_OnUpdate(DgnElementCR el)
    {
    DgnDbStatus status = T_Super::_OnUpdate(el);
    if (DgnDbStatus::Success != status)
        return status;

    ModelSelectorCP original = dynamic_cast<ModelSelectorCP>(&el);
    if (nullptr == original)
        {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
        }

    if (m_models != original->m_models)
        {
        GetDgnDb().DeleteLinkTableRelationships(BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels), GetElementId(), DgnElementId() /* all targets */);
        return WriteModels();
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_OnDeleted() const
    {
    T_Super::_OnDeleted();
    // provide clean up behavior previously handled by foreign keys (the bis_ModelSelectorRefersToModels table uses "logical" foreign keys now)
    GetDgnDb().DeleteLinkTableRelationships(BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels), GetElementId(), DgnElementId() /* all targets */);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::WriteModels()
    {
    if (!GetElementId().IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::MissingId;
        }

    auto statement = GetDgnDb().GetNonSelectPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)", GetDgnDb().GetECCrudWriteToken());
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    for (auto id : m_models)
        {
        statement->Reset();
        statement->ClearBindings();
        statement->BindId(1, GetElementId());
        statement->BindId(2, id);
        if (BE_SQLITE_DONE != statement->Step())
            return DgnDbStatus::WriteError;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_LoadFromDb()
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadSchema;
        }

    statement->BindId(1, GetElementId());
    while (BE_SQLITE_ROW == statement->Step())
        {
        auto id = statement->GetValueId<DgnModelId>(0);
        m_models.insert(id);
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_ToJson(BeJsValue out, BeJsConst opts) const {
    T_Super::_ToJson(out, opts);
    auto models = out[json_models()];
    models.SetEmptyArray();
    for (auto const& model : m_models)
        models.appendValue() = model;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_FromJson(BeJsConst val) {
    T_Super::_FromJson(val);
    m_models.clear();
    auto models = val[json_models()];
    uint32_t size = models.size();
    for (uint32_t i = 0; i < size; ++i) {
        auto id = DgnModelId(models[i].asUInt64());
        if (id.IsValid())
            m_models.insert(id);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindPoint3d(stmt.GetParameterIndex(prop_Origin()), m_origin);
    stmt.BindPoint3d(stmt.GetParameterIndex(prop_Extents()), m_extents);

    YawPitchRollAngles angles;
    YawPitchRollAngles::TryFromRotMatrix(angles, m_rotation);

    stmt.BindDouble(stmt.GetParameterIndex(prop_Yaw()), angles.GetYaw().Degrees());
    stmt.BindDouble(stmt.GetParameterIndex(prop_Pitch()), angles.GetPitch().Degrees());
    stmt.BindDouble(stmt.GetParameterIndex(prop_Roll()), angles.GetRoll().Degrees());

    auto stat = stmt.BindPoint3d(stmt.GetParameterIndex(prop_EyePoint()), GetEyePoint());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindDouble(stmt.GetParameterIndex(prop_LensAngle()), GetLensAngle().Radians());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindDouble(stmt.GetParameterIndex(prop_FocusDistance()), GetFocusDistance());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindBoolean(stmt.GetParameterIndex(prop_IsCameraOn()), IsCameraOn());
    BeAssert(ECSqlStatus::Success == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition3d::_EqualState(ViewDefinitionR in)
    {
    auto& other = (ViewDefinition3dR) in;
    if (!m_origin.IsEqual(other.m_origin) || !m_extents.IsEqual(other.m_extents) || !m_rotation.IsEqual(other.m_rotation))
        return false;

    if (IsCameraOn() != other.IsCameraOn())
        return false;

    // if camera is off, don't compare cameras
    if (IsCameraOn() && !m_cameraDef.IsEqual(other.m_cameraDef))
        return false;

    return T_Super::_EqualState(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition3d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    m_origin  = stmt.GetValuePoint3d(params.GetSelectIndex(prop_Origin()));
    m_extents = DVec3d::From(stmt.GetValuePoint3d(params.GetSelectIndex(prop_Extents())));

    double yaw   = stmt.GetValueDouble(params.GetSelectIndex(prop_Yaw())),
           pitch = stmt.GetValueDouble(params.GetSelectIndex(prop_Pitch())),
           roll  = stmt.GetValueDouble(params.GetSelectIndex(prop_Roll()));

    m_rotation = YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)).ToRotMatrix();

    m_cameraDef.SetEyePoint(stmt.GetValuePoint3d(params.GetSelectIndex(prop_EyePoint())));
    m_cameraDef.SetLensAngle(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(prop_LensAngle()))));
    m_cameraDef.SetFocusDistance(stmt.GetValueDouble(params.GetSelectIndex(prop_FocusDistance())));
    m_cameraOn = stmt.GetValueBoolean(params.GetSelectIndex(prop_IsCameraOn()));

    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::Camera::ToJson(BeJsValue val) const
    {
    val.SetEmptyObject();
    BeJsGeomUtils::FromAngle(val[json_lens()], m_lensAngle);
    val[json_focusDist()] = m_focusDistance;
    BeJsGeomUtils::DPoint3dToJson(val[json_eye()], m_eyePoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition3d::Camera ViewDefinition3d::Camera::FromJson(BeJsConst val)
    {
    Camera camera;
    camera.SetLensAngle(BeJsGeomUtils::ToAngle(val[json_lens()]));
    camera.SetFocusDistance(val[json_focusDist()].asDouble());
    BeJsGeomUtils::DPoint3dFromJson(camera.m_eyePoint, val[json_eye()]);
    return camera;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    val[json_cameraOn()] = m_cameraOn;
    BeJsGeomUtils::DPoint3dToJson(val[json_origin()], m_origin);
    BeJsGeomUtils::DPoint3dToJson(val[json_extents()], m_extents);

    YawPitchRollAngles angles;
    YawPitchRollAngles::TryFromRotMatrix(angles, m_rotation);
    BeJsGeomUtils::YawPitchRollToJson(val[json_angles()], angles);
    m_cameraDef.ToJson(val[json_camera()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_FromJson(BeJsConst val)
    {
    T_Super::_FromJson(val);

    if (val.isMember(json_cameraOn()))
        m_cameraOn = val[json_cameraOn()].asBool();

    if (val.isMember(json_origin()))
        m_origin = BeJsGeomUtils::ToDPoint3d(val[json_origin()]);

    if (val.isMember(json_extents()))
        m_extents = BeJsGeomUtils::ToDVec3d(val[json_extents()]);

    if (val.isMember(json_angles()))
        m_rotation = BeJsGeomUtils::YawPitchRollFromJson(val[json_angles()]).ToRotMatrix();

    if (val.isMember(json_camera()))
        m_cameraDef = Camera::FromJson(val[json_camera()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);

    auto const& other = (ViewDefinition3d const&) el;
    m_origin = other.m_origin;
    m_extents = other.m_extents;
    m_rotation = other.m_rotation;
    m_cameraDef = other.m_cameraDef;
    m_cameraOn = other.m_cameraOn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::SaveThumbnail(Point2d size, Render::ImageSourceCR source) const
    {
    BeJsDocument val;
    val[json_width()] = size.x;
    val[json_height()] = size.y;
    val[json_format()] = (source.GetFormat() == ImageSource::Format::Jpeg) ? prop_jpeg() : prop_png();

    DbResult rc = m_dgndb.SaveProperty(DgnViewProperty::ViewThumbnail(), val.Stringify(), source.GetByteStream().GetData(), source.GetByteStream().GetSize(), GetViewId().GetValue());
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource ViewDefinition::ReadThumbnail() const
    {
    ImageSource image;

    DgnDbR db = GetDgnDb();
    uint32_t bytes;
    auto stat = db.QueryPropertySize(bytes, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    if (BE_SQLITE_ROW != stat)
        return image;

    Utf8String jsonStr;
    stat = db.QueryProperty(jsonStr, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    if (BE_SQLITE_ROW != stat)
        return image;

    BeJsDocument value(jsonStr);
    image.SetFormat(value[json_format()].asString() == prop_jpeg() ? ImageSource::Format::Jpeg : ImageSource::Format::Png);
    image.GetByteStreamR().Resize(bytes);
    stat = db.QueryProperty(image.GetByteStreamR().GetDataP(), bytes, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    BeAssert(BE_SQLITE_ROW == stat);
    return image;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Point2d ViewDefinition::GetThumbnailSize() const
    {
    Point2d size = {0,0};

    DgnDbR db = GetDgnDb();
    Utf8String jsonStr;
    auto stat = db.QueryProperty(jsonStr, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    if (BE_SQLITE_ROW != stat)
        return size;

    BeJsDocument value(jsonStr);
    size.x = value[json_width()].asInt();
    size.y = value[json_height()].asInt();
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ViewDefinition::HasThumbnail() const
    {
    DgnDbR db = GetDgnDb();
    Utf8String jsonStr;
    auto stat = db.QueryProperty(jsonStr, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    return (BE_SQLITE_ROW == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::DeleteThumbnail() const
    {
    GetDgnDb().DeleteProperty(DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialViewDefinition::_OnInsert()
    {
    if (!GetModelSelector().GetElementId().IsValid())
        {
        DgnDbStatus stat;
        m_modelSelector->Insert(&stat);
        if (DgnDbStatus::Success != stat)
            return stat;
        m_modelSelectorId = m_modelSelector->GetElementId();
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto& other = static_cast<SpatialViewDefinitionCR>(el);

    m_modelSelectorId = other.m_modelSelectorId;
    m_modelSelector = other.m_modelSelector.IsValid() ? other.m_modelSelector->MakeCopy<ModelSelector>() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_RemapIds(DgnImportContext& importContext)
    {
    if (importContext.IsBetweenDbs())
        m_modelSelectorId = importContext.FindElementId(m_modelSelectorId);

    T_Super::_RemapIds(importContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialViewDefinition::_EqualState(ViewDefinitionR in)
    {
    if (!T_Super::_EqualState(in))
        return false;

    auto& other = (SpatialViewDefinition&) in;
    if (m_modelSelectorId != other.m_modelSelectorId)
        return false;

    return GetModelSelector().EqualState(other.GetModelSelector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    BeAssert(GetModelSelectorId().IsValid());
    stmt.BindNavigationValue(stmt.GetParameterIndex(prop_ModelSelector()), GetModelSelectorId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialViewDefinition::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    m_modelSelectorId = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(prop_ModelSelector()));
    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    val[json_modelSelectorId()] = m_modelSelectorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_FromJson(BeJsConst val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_modelSelectorId()))
        m_modelSelectorId.FromJson(val[json_modelSelectorId()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewDefinition::SetStandardViewRotation(StandardView standardView)
    {
    RotMatrix rMatrix;
    if (!bsiRotMatrix_getStandardRotation(&rMatrix, static_cast<int>(standardView)))
        return  ERROR;

    SetRotation(rMatrix);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* load a subcategory appearance into its unmodified state
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::Appearance DisplayStyle::LoadSubCategory(DgnSubCategoryId id) const
    {
    auto unmodified = DgnSubCategory::Get(GetDgnDb(), id);
    BeAssert(unmodified.IsValid());
    if (!unmodified.IsValid())
        return DgnSubCategory::Appearance();

    BeMutexHolder _v(m_mutex);
    auto const& result = m_subCategories.Insert(id, unmodified->GetAppearance());
    if (!result.second)
        result.first->second = unmodified->GetAppearance(); // we already had this SubCategory; change it to unmodified state

    return unmodified->GetAppearance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::OverrideSubCategory(DgnSubCategoryId id, DgnSubCategory::Override const& ovr)
    {
    if (!id.IsValid())
        return;

    auto result = m_subCategoryOverrides.Insert(id, ovr);
    if (!result.second)
        result.first->second = ovr; // we already had this override; change it.

    LoadSubCategory(id); // To ensure none of the previous overrides are still active, we reload the original SubCategory

    // now apply this override to the unmodified SubCategory appearance
    auto it = m_subCategories.find(id);
    if (it != m_subCategories.end())
        ovr.ApplyTo(it->second);
    else
        {
        BeAssert(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::Override DisplayStyle::GetSubCategoryOverride(DgnSubCategoryId id) const
    {
    auto iter = m_subCategoryOverrides.find(id);
    return m_subCategoryOverrides.end() != iter ? iter->second : DgnSubCategory::Override();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::Appearance DisplayStyle::GetSubCategoryAppearance(DgnSubCategoryId subCategoryId) const
    {
    if (true)
        {
        BeMutexHolder _v(m_mutex);
        auto const entry = m_subCategories.find(subCategoryId);
        if (entry != m_subCategories.end())
            return entry->second;
        }

    return LoadSubCategory(subCategoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::DropSubCategoryOverride(DgnSubCategoryId id)
    {
    m_subCategoryOverrides.erase(id);
    LoadSubCategory(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::OverrideModelAppearance(DgnModelId modelId, ModelAppearanceOverrides const& overrides)
    {
    this->DropModelAppearanceOverrides(modelId);     // Remove existing.

    if (!overrides.IsAnyOverridden())
        return;

    auto modelOverrides = GetStyleR(json_modelOvr());
    modelOverrides.toArray();
    overrides.ToJson(modelOverrides.appendValue(), modelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::DropModelAppearanceOverrides(DgnModelId modelId) {
    auto modelOverrides = GetStyleR(json_modelOvr());
    auto key = modelId.ToHexStr();
    if (modelOverrides.isArray()) {
        for (unsigned int index = 0; index < modelOverrides.size(); ) {
            if (modelOverrides[index].asString() == key)
                modelOverrides.removeIndex(index);
            else
                ++index;
        }
    }
}

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ModelAppearanceOverrides ModelAppearanceOverrides::FromJson(BeJsConst value)
    {
    ModelAppearanceOverrides overrides;
    if (value[json_transparency()].isNumeric())
       overrides.SetTransparency(value[json_transparency()].asDouble());

    if (value[json_nonLocatable()].isBool())
        overrides.SetNonLocatable(value[json_nonLocatable()].asBool());

    if (value[json_emphasized()].isBool())
         overrides.SetEmphasized(value[json_emphasized()].asBool());

    if (value[json_ignoresMaterial()].isBool())
        overrides.SetIgnoresMaterial(value[json_ignoresMaterial()].asBool());

    if (value[json_weight()].isNumeric())
        overrides.SetWeight(value[json_weight()].asUInt());

    if (value[json_linePixels()].isNumeric())
        overrides.SetLinePixels((Render::LinePixels) value[json_linePixels()].asUInt());

    auto rgb = value[json_rgb()];
    if (rgb.isObject() && rgb.isMember("r") && rgb.isMember("g") && rgb.isMember("b"))
        {
        ColorDef    color = ColorDef::White();
        color.SetRed((Byte) rgb["r"].asUInt());
        color.SetGreen((Byte) rgb["g"].asUInt());
        color.SetBlue((Byte) rgb["b"].asUInt());
        overrides.SetColor(color);
        }
    return overrides;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelAppearanceOverrides::ToJson(BeJsValue output, DgnModelId modelId) const
    {
    output.SetEmptyObject();
    output[json_modelId()] = modelId;

    double  transparency;
    if (this->GetTransparency(transparency))
        output[json_transparency()] = transparency;

    bool nonLocatable;
    if (this->GetNonLocatable(nonLocatable) && nonLocatable)
       output[json_nonLocatable()] = nonLocatable;

    bool emphasized;
    if (this->GetEmphasized(emphasized) && emphasized)
        output[json_emphasized()] = emphasized;

    bool ignoresMaterial;
    if (this->GetIgnoresMaterial(ignoresMaterial) && ignoresMaterial)
        output[json_ignoresMaterial()] = ignoresMaterial;

    Render::LinePixels  linePixels;
    if (this->GetLinePixels(linePixels))
        output[json_linePixels()] = (uint32_t) linePixels;

    uint32_t    weight;
    if (this->GetWeight(weight))
        output[json_weight()] = weight;

    ColorDef    color;
    if (this->GetColor(color))
        {
        auto rgbValue = output[json_rgb()];
        rgbValue["r"] = color.GetRed();
        rgbValue["g"] = color.GetGreen();
        rgbValue["b"] = color.GetBlue();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ModelAppearanceOverrides::operator == (ModelAppearanceOverrides const& other) const
    {
    if (this->m_flags.m_int32 != other.m_flags.m_int32)
        return false;

    if (this->m_flags.m_transparency && this->m_transparency != other.m_transparency)
        return false;

    if (this->m_flags.m_nonLocatable && this->m_nonLocatable != other.m_nonLocatable)
        return false;

    if (this->m_flags.m_emphasized && this->m_emphasized != other.m_emphasized)
        return false;

    if (this->m_flags.m_ignoresMaterial && this->m_ignoresMaterial != other.m_ignoresMaterial)
        return false;

    if (this->m_flags.m_rgb && this->m_rgb != other.m_rgb)
        return false;

    if (this->m_flags.m_weight && this->m_weight != other.m_weight)
        return false;

    if (this->m_flags.m_linePixels && this->m_linePixels != other.m_linePixels)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayStyle::GetModelAppearanceOverrides(ModelAppearanceOverrides& overrides, DgnModelId modelId)
    {
    auto modelOverrides = this->GetStyle(json_modelOvr());

    if (modelOverrides.isNull())
        return false;

    auto modelIdString = modelId.ToHexStr();

    return modelOverrides.ForEachArrayMember([&] (BeJsValue::ArrayIndex, BeJsConst modelOverride) {
        if (modelOverride.isObject() && modelOverride[json_modelId()].asString() == modelIdString)
            {
            overrides = ModelAppearanceOverrides::FromJson(modelOverride);
            return true;
            }
        return false;
     });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);

    auto& other = static_cast<DisplayStyleCR>(el);

    m_viewFlags = other.m_viewFlags;
    m_subCategories = other.m_subCategories;
    m_subCategoryOverrides = other.m_subCategoryOverrides;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DisplayStyle::_OnDelete() const
    {
    // can only be deleted through a purge operation
    return GetDgnDb().IsPurgeOperationActive() ? T_Super::_OnDelete() : DgnDbStatus::DeletionProhibited;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_OnLoadedJsonProperties()
    {
    auto styles = GetStyles();
    m_viewFlags.FromJson(styles[json_viewflags()]);

    auto overrides = styles[json_subCategoryOvr()];
    auto size = overrides.size();
    for (Json::ArrayIndex i = 0; i<size; ++i)
        {
        auto val = overrides[i];
        auto subCatIdJson = val[json_subCategory()];
        DgnSubCategoryId subCategoryId(subCatIdJson.asUInt64());
        if (!subCategoryId.IsValid())
            {
            BeDataAssert(false && "SubCategoryOverride refers to missing SubCategory");
            continue;
            }

        // Retroactively fix IDs that were previously stored as 64-bit integers rather than as ID strings -
        // otherwise the front-end cannot decipher them.
        if (!subCatIdJson.isString())
            (BeJsValue&) subCatIdJson = subCategoryId;

        OverrideSubCategory(subCategoryId, DgnSubCategory::Override(val));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);

    auto props = val[json_jsonProperties()];
    if (!props.isMember(json_styles()))
        return;

    auto styles = props[json_styles()];

    // Customize aspects of the jsonProperties.
    auto styleOpts = opts[json_displayStyle()];

    if (styleOpts[json_omitScheduleScriptElementIds()].asBool() && styles.isMember(json_scheduleScript()))
        removeScheduleScriptElementIds(styles[json_scheduleScript()]);

    if (!styles.isMember(json_excludedElements()))
        return;

    auto excludedElems = styles[json_excludedElements()];
    bool isArray = excludedElems.isArray();
    bool isString = !isArray && excludedElems.isString();
    if (!isArray && !isString)
        {
        // null or some garbage.
        styles.removeMember(json_excludedElements());
        return;
        }

    if (isArray && styleOpts[json_compressExcludedElementIds()].asBool())
        {
        // The Ids are not compressed; caller wants them compressed.
        auto ids = GetExcludedElements();
        excludedElems = ids.GetBeIdSet().ToCompactString();
        }
    else if (isString && !styleOpts[json_compressExcludedElementIds()].asBool())
        {
        // The Ids are compressed; callers wants them uncompressed.
        auto ids = GetExcludedElements();
        excludedElems.SetEmptyArray();
        Json::ArrayIndex i = 0;
        for (auto const& id : ids)
            excludedElems[i++] = id;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DisplayStyle::ToJson() const
    {
    const_cast<DisplayStyleR>(*this)._OnSaveJsonProperties();
    return GetStyles().Stringify();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayStyle::EqualState(DisplayStyleR other)
    {
    _OnSaveJsonProperties();
    other._OnSaveJsonProperties();
    return GetStyles()==other.GetStyles();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_OnSaveJsonProperties()
    {
    BeJsDocument viewFlags;
    m_viewFlags.ToJson(viewFlags);
    SetStyle(json_viewflags(), viewFlags);

    if (m_subCategoryOverrides.empty())
        {
        RemoveStyle(json_subCategoryOvr());
        }
    else
        {
        BeJsDocument ovrJson;
        int i=0;
        for (auto const& it : m_subCategoryOverrides)
            {
            ovrJson[i][json_subCategory()] = it.first.ToHexStr();
            it.second.ToJson(ovrJson[i++]);
            }
        SetStyle(json_subCategoryOvr(), ovrJson);
        }

    auto excludedJson = GetStyles()[json_excludedElements()];
    if (excludedJson.isArray())
        {
        // Convert from array to compact string
        SetExcludedElements(GetExcludedElements());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DisplayStyle::GetRenderTimelineId() const
    {
    return GetStyle(json_renderTimeline()).GetId64<DgnElementId>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetRenderTimelineId(DgnElementId id)
    {
    if (id.IsValid())
        {
        GetStylesR()[json_renderTimeline()] = id;
        }
    else
        {
        RemoveStyle(json_renderTimeline());
        }
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DisplayStyle::GetExcludedElements() const
    {
    DgnElementIdSet excludedElements;

    auto excludedElementsJson = GetStyles()[json_excludedElements()];
    if (excludedElementsJson.isArray())
        {
        auto size = excludedElementsJson.size();
        for (Json::ArrayIndex i = 0; i < size; ++i)
            excludedElements.insert(excludedElementsJson[i].GetId64<DgnElementId>());
        }
    else if (excludedElementsJson.isString())
        {
        excludedElements.FromString(excludedElementsJson.asString());
        }

    return excludedElements;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetExcludedElements(DgnElementIdSet const& excludedElements)
    {
    if (excludedElements.empty())
        RemoveStyle(json_excludedElements());
    else
        GetStyleR(json_excludedElements()) = excludedElements.GetBeIdSet().ToCompactString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DisplayStyle::GetBackgroundColor() const
    {
    return ColorDef(GetStyle(json_backgroundColor()).asUInt(ColorDef::Black().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetBackgroundColor(ColorDef val)
    {
    GetStylesR().SetOrRemoveUInt(json_backgroundColor(), val.GetValue(), ColorDef::Black().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DisplayStyle::GetMonochromeColor() const
    {
    return ColorDef(GetStyle(json_monochromeColor()).asUInt(ColorDef::White().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetMonochromeColor(ColorDef val)
    {
    GetStylesR().SetOrRemoveInt(json_monochromeColor(), val.GetValue(), ColorDef::White().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::SetSceneLight(Lighting::Parameters const& params)
    {
    if (!params.IsValid())
        return;

    auto sceneLights = GetStylesR()[json_sceneLights()];
    switch (params.GetType())
        {
        case Lighting::LightType::Ambient:
            sceneLights[json_ambient()].From(params);
            break;

        case Lighting::LightType::Flash:
            sceneLights[json_flash()].From(params);
            break;

        case Lighting::LightType::Portrait:
            sceneLights[json_portrait()].From(params);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::SetSolarLight(Lighting::Parameters const& params, DVec3dCR direction)
    {
    auto sceneLights = GetStylesR()[json_sceneLights()];
    if (params.GetType() != Lighting::LightType::Solar || !params.IsValid())
        {
        sceneLights.removeMember(json_sunDir());
        return;
        }

    sceneLights[json_sun()].From(params);
    BeJsGeomUtils::DVec3dToJson(sceneLights[json_sunDir()], direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::PlanProjectionSettings::ToJson(BeJsValue json) const
    {
    json.SetNull();
    if (MatchesDefaults())
        return;

    if (m_elevation)
        json[json_elevation()] = *m_elevation;

    if (m_transparency)
        {
        auto transp = *m_transparency;
        json[json_transparency()] = transp > 1 ? 1 : (transp < 0 ? 0 : transp);
        }

    if (m_drawAsOverlay)
        json[json_overlay()] = true;

    if (m_enforceDisplayPriority)
        json[json_enforceDisplayPriority()] = true;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyle3d::PlanProjectionSettings DisplayStyle3d::PlanProjectionSettings::FromJson(BeJsConst json)
    {
    PlanProjectionSettings settings;
    if (json.isMember(json_elevation()))
        settings.m_elevation = json[json_elevation()].asDouble();

    if (json.isMember(json_transparency()))
        {
        auto transp = json[json_transparency()].asDouble();
        settings.m_transparency = transp > 1 ? 1 : (transp < 0 ? 0 : transp);
        }

    settings.m_drawAsOverlay = json[json_overlay()].asBool();
    settings.m_enforceDisplayPriority = json[json_enforceDisplayPriority()].asBool();

    return settings;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyle3d::PlanProjectionSettingsMap DisplayStyle3d::GetPlanProjectionSettings() const
    {
    PlanProjectionSettingsMap map;
    auto json = GetStyle(json_planProjections());
    if (json.isNull() || !json.isObject())
        return map;

    json.ForEachProperty([&](Utf8CP name, BeJsConst member){
        auto id64 = BeInt64Id::FromString(name);
        if (!id64.IsValid())
            return false;

        auto settings = PlanProjectionSettings::FromJson(member);
        if (!settings.MatchesDefaults())
            map.Insert(DgnModelId(id64.GetValue()), settings);
        return false;
    });

    return map;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::SetPlanProjectionSettings(PlanProjectionSettingsMap const* settings)
    {
    if (nullptr == settings || 0 == settings->size())
        {
        GetStyleR(json_planProjections()).SetNull();
        return;
        }

    auto json = GetStyleR(json_planProjections());
    json.SetEmptyObject();
    for (auto const& entry : *settings)
        {
        if (!entry.second.MatchesDefaults())
            entry.second.ToJson(json[entry.first.ToHexStr()]);
        }

    }

/*---------------------------------------------------------------------------------**//**
* Search the (8) standard view matrices for one that is close to given matrix.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool findNearbyStandardViewMatrix(RotMatrixR rMatrix)
    {
    static double const s_viewMatrixTolerance = 1.0e-7;
    RotMatrix   test;

    // Standard views are numbered from 1 ....
    for (int i = 1; bsiRotMatrix_getStandardRotation(&test, i); ++i)
        {
        double a = test.MaxDiff(rMatrix);
        if (a < s_viewMatrixTolerance)
            {
            rMatrix = test;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition::_SetupFromFrustum(Frustum const& frustum)
    {
    DPoint3dCP frustPts = frustum.GetPts();
    DPoint3d viewOrg = frustPts[NPC_LeftBottomRear];

    // frustumX, frustumY, frustumZ are vectors along edges of the frustum. They are NOT unit vectors.
    // X and Y should be perpendicular, and Z should be right handed.
    DVec3d frustumX, frustumY, frustumZ;
    frustumX.DifferenceOf(frustPts[NPC_RightBottomRear], viewOrg);
    frustumY.DifferenceOf(frustPts[NPC_LeftTopRear], viewOrg);
    frustumZ.DifferenceOf(frustPts[NPC_LeftBottomFront], viewOrg);

    RotMatrix   frustMatrix;
    frustMatrix.InitFromColumnVectors(frustumX, frustumY, frustumZ);
    if (!frustMatrix.SquareAndNormalizeColumns(frustMatrix, 0, 1))
        return ViewportStatus::InvalidWindow;

    findNearbyStandardViewMatrix(frustMatrix);

    DVec3d xDir, yDir, zDir;
    frustMatrix.GetColumns(xDir, yDir, zDir);

    // set up view Rotation matrix as rows of frustum matrix.
    RotMatrix viewRot;
    viewRot.InverseOf(frustMatrix);

    // Left handed frustum?
    double zSize = zDir.DotProduct(frustumZ);
    if (zSize < 0.0)
        return ViewportStatus::InvalidWindow;

    DPoint3d viewDiagRoot;
    viewDiagRoot.SumOf(xDir, xDir.DotProduct(frustumX), yDir, yDir.DotProduct(frustumY));  // vectors on the back plane
    viewDiagRoot.SumOf(viewDiagRoot, zDir, zSize);       // add in z vector perpendicular to x,y

    // use center of frustum and view diagonal for origin. Original frustum may not have been orthogonal
    viewOrg.SumOf(frustum.GetCenter(), viewDiagRoot, -0.5);

    // delta is in view coordinates
    DVec3d viewDelta;
    viewRot.Multiply(viewDelta, viewDiagRoot);

    ViewportStatus validSize = ValidateViewDelta(viewDelta, false);
    if (validSize != ViewportStatus::Success)
        return validSize;

    SetOrigin(viewOrg);
    SetExtents(viewDelta);
    SetRotation(viewRot);
    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::_SetupFromFrustum(Frustum const& frustum)
    {
    auto stat = T_Super::_SetupFromFrustum(frustum);
    if (ViewportStatus::Success != stat)
        return stat;

    TurnCameraOff();
    DPoint3dCP frustPts = frustum.GetPts();

    // use comparison of back, front plane X sizes to indicate camera or flat view ...
    double xBack  = frustPts[NPC_LeftBottomRear].Distance(frustPts[NPC_RightBottomRear]);
    double xFront = frustPts[NPC_LeftBottomFront].Distance(frustPts[NPC_RightBottomFront]);

    static double const s_flatViewFractionTolerance = 1.0e-6;
    if (xFront > xBack *(1.0 + s_flatViewFractionTolerance))
        return ViewportStatus::InvalidWindow;

    // see if the frustum is tapered, and if so, set up camera eyepoint and adjust viewOrg and delta.
    double compression = xFront / xBack;
    if (compression >= (1.0 - s_flatViewFractionTolerance))
        return ViewportStatus::Success;

    DPoint3d viewOrg = frustPts[NPC_LeftBottomRear];
    DVec3d viewDelta = GetExtents();
    DVec3d zDir = GetZVector();
    DVec3d frustumZ = DVec3d::FromStartEnd(viewOrg, frustPts[NPC_LeftBottomFront]);
    DVec3d frustOrgToEye = DVec3d::FromScale(frustumZ, 1.0 /(1.0 - compression));
    DPoint3d eyePoint = DPoint3d::FromSumOf(viewOrg, frustOrgToEye);

    double backDistance  = frustOrgToEye.DotProduct(zDir);         // distance from eye to back plane of frustum
    double focusDistance = backDistance -(viewDelta.z / 2.0);
    double focalFraction = focusDistance / backDistance;           // ratio of focus plane distance to back plane distance

    viewOrg.SumOf(eyePoint, frustOrgToEye, -focalFraction);        // project point along org-to-eye vector onto focus plane
    viewOrg.SumOf(viewOrg, zDir, focusDistance - backDistance);    // now project that point onto back plane

    viewDelta.x *= focalFraction;                                  // adjust view delta for x and y so they are also at focus plane
    viewDelta.y *= focalFraction;

    SetEyePoint(eyePoint);
    SetFocusDistance(focusDistance);
    SetOrigin(viewOrg);
    SetExtents(viewDelta);
    SetLensAngle(CalcLensAngle());
    _EnableCamera();
    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::LookAtVolume(DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
    {
    DPoint3d rangebox[8];
    volume.Get8Corners(rangebox);
    GetRotation().Multiply(rangebox, rangebox, 8);

    DRange3d viewAlignedVolume;
    viewAlignedVolume.InitFrom(rangebox, 8);

    return LookAtViewAlignedVolume(viewAlignedVolume, aspect, margin, expandClippingPlanes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::LookAtViewAlignedVolume(DRange3dCR volume, double const* aspect, MarginPercent const* margin, bool expandClippingPlanes)
    {
    DPoint3d    oldDelta = GetExtents();
    DPoint3d    oldOrg   = GetOrigin();
    RotMatrix   viewRot  = GetRotation();

    DPoint3d  newOrigin = volume.low;
    DVec3d    newDelta;
    newDelta.DifferenceOf(volume.high, volume.low);

    double minimumDepth = DgnUnits::OneMillimeter();
    if (newDelta.z < minimumDepth)
        {
        newOrigin.z -=(minimumDepth - newDelta.z)/2.0;
        newDelta.z = minimumDepth;
        }

    DPoint3d origNewDelta = newDelta;

    auto cameraView = ToView3dP();
    bool isCameraOn = cameraView && cameraView->IsCameraOn();
    if (isCameraOn)
        {
        // If the camera is on, the only way to guarantee we can see the entire volume is to set delta at the front plane, not focus plane.
        // That generally causes the view to be too large (objects in it are too small), since we can't tell whether the objects are at
        // the front or back of the view. For this reason, don't attempt to add any "margin" to camera views.
        }
    else if (nullptr != margin)
        {
        // compute how much space we'll need for both of X and Y margins in root coordinates
        double wPercent = margin->Left() + margin->Right();
        double hPercent = margin->Top()  + margin->Bottom();

        double marginHoriz = wPercent/(1-wPercent) * newDelta.x;
        double marginVert  = hPercent/(1-hPercent) * newDelta.y;

        // compute left and bottom margins in root coordinates
        double marginLeft   = margin->Left()/(1-wPercent) *   newDelta.x;
        double marginBottom = margin->Bottom()/(1-hPercent) * newDelta.y;

        // add the margins to the range
        newOrigin.x -= marginLeft;
        newOrigin.y -= marginBottom;
        newDelta.x  += marginHoriz;
        newDelta.y  += marginVert;

        // don't fix the origin due to changes in delta here
        origNewDelta = newDelta;
        }
    else
        {
        newDelta.Scale(1.04); // default "dilation"
        }

    if (isCameraOn)
        {
        // make sure that the zDelta is large enough so that entire model will be visible from any rotation
        double diag = newDelta.MagnitudeXY();
        if (diag > newDelta.z)
            newDelta.z = diag;
        }

    ValidateViewDelta(newDelta, true);

    SetExtents(newDelta);
    if (aspect)
        _AdjustAspectRatio(*aspect);

    newDelta = GetExtents();

    newOrigin.x -=(newDelta.x - origNewDelta.x) / 2.0;
    newOrigin.y -=(newDelta.y - origNewDelta.y) / 2.0;
    newOrigin.z -=(newDelta.z - origNewDelta.z) / 2.0;

    // if they don't want the clipping planes to change, set them back to where they were
    if (nullptr != cameraView && !expandClippingPlanes)
        {
        viewRot.Multiply(oldOrg);
        newOrigin.z = oldOrg.z;

        DVec3d delta = GetExtents();
        delta.z = oldDelta.z;
        SetExtents(delta);
        }

    DPoint3d newOrgView;
    viewRot.MultiplyTranspose(&newOrgView, &newOrigin, 1);
    SetOrigin(newOrgView);

    if (nullptr == cameraView)
        return;

    auto& cameraDef = cameraView->GetCameraR();
    cameraDef.ValidateLens();
    // move the camera back so the entire x,y range is visible at front plane
    double frontDist = std::max(newDelta.x, newDelta.y) / (2.0*tan(cameraDef.GetLensAngle().Radians()/2.0));
    double backDist = frontDist + newDelta.z;

    cameraDef.SetFocusDistance(frontDist); // do this even if the camera isn't currently on.
    cameraView->CenterEyePoint(&backDist); // do this even if the camera isn't currently on.
    cameraView->VerifyFocusPlane(); // changes delta/origin
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Angle ViewDefinition3d::CalcLensAngle() const
    {
    double maxDelta = std::max(m_extents.x, m_extents.y);
    return Angle::FromRadians(2.0 * Angle::Atan2(maxDelta*0.5, m_cameraDef.GetFocusDistance()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::CenterEyePoint(double const* backDistanceIn)
    {
    DVec3d delta = GetExtents();
    DPoint3d eyePoint;
    eyePoint.Scale(delta, 0.5);
    eyePoint.z = backDistanceIn ? *backDistanceIn : GetBackDistance();

    GetRotation().MultiplyTranspose(eyePoint);
    eyePoint.Add(GetOrigin());

    m_cameraDef.SetEyePoint(eyePoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::CenterFocusDistance()
    {
    double backDist  = GetBackDistance();
    double frontDist = GetFrontDistance();
    DPoint3d eye     = GetEyePoint();
    DPoint3d target  = DPoint3d::FromSumOf(eye, GetZVector(), frontDist-backDist);
    LookAtUsingLensAngle(eye, target, GetYVector(), GetLensAngle(), &frontDist, &backDist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewDefinition3d::CalculateMaxDepth(DVec3dCR delta, DVec3dCR zVec)
    {
    // We are going to limit maximum depth to a value that will avoid subtractive cancellation
    // errors on the inverse frustum matrix. - These values will occur when the Z'th row values
    // are very small in comparison to the X-Y values.  If the X-Y values are exactly zero then
    // no error is possible and we'll arbitrarily limit to 1.0E8.
    // This change made to resolve TR# 271876.   RayBentley   04/28/2009.

    static double s_depthRatioLimit       = 1.0E8;          // Limit for depth Ratio.
    static double s_maxTransformRowRatio  = 1.0E5;

    double minXYComponent = std::min(fabs(zVec.x), fabs(zVec.y));
    double maxDepthRatio =(0.0 == minXYComponent) ? s_depthRatioLimit : std::min((s_maxTransformRowRatio / minXYComponent), s_depthRatioLimit);

    return  std::max(delta.x, delta.y) * maxDepthRatio;
    }

/*---------------------------------------------------------------------------------**//**
* Ensure the focus plane lies between the front and back planes. If not, center it.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::VerifyFocusPlane()
    {
    if (!IsCameraOn())
        return;

    DVec3d eyeOrg = DVec3d::FromStartEnd(m_origin, m_cameraDef.GetEyePoint());
    m_rotation.Multiply(eyeOrg);

    double backDist = eyeOrg.z;
    double frontDist = backDist - m_extents.z;

    if (backDist<=0.0 || frontDist<=0.0)
        {
        // the camera location is invalid. Set it based on the view range.
        double tanangle = tan(m_cameraDef.GetLensAngle().Radians()/2.0);
        backDist = m_extents.z / tanangle;
        m_cameraDef.SetFocusDistance(backDist/2);
        CenterEyePoint(&backDist);
        return;
        }

    double focusDist = m_cameraDef.GetFocusDistance();
    if (focusDist>frontDist && focusDist<backDist)
        return;

    // put it halfway between front and back planes
    m_cameraDef.SetFocusDistance((m_extents.z / 2.0) + frontDist);

    // moving the focus plane means we have to adjust the origin and delta too (they're on the focus plane, see diagram in ViewDefinition.h)
    double ratio = m_cameraDef.GetFocusDistance() / focusDist;
    m_extents.x *= ratio;
    m_extents.y *= ratio;

    DVec3d xVec, yVec, zVec;
    m_rotation.GetRows(xVec, yVec, zVec);
    m_origin.SumOf(m_cameraDef.GetEyePoint(), zVec, -backDist, xVec, -0.5*m_extents.x, yVec, -0.5*m_extents.y); // this centers the camera too
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in ViewDefinition.h
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::LookAt(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
                                        DVec2dCP extentsIn, double const* frontDistIn, double const* backDistIn)
    {
    DVec3d yVec = upVec;
    if (yVec.Normalize() <= mgds_fc_epsilon) // up vector zero length?
        return ViewportStatus::InvalidUpVector;

    DVec3d zVec; // z defined by direction from eye to target
    zVec.DifferenceOf(eyePoint, targetPoint);

    double focusDist = zVec.Normalize(); // set focus at target point
    if (focusDist <= MinimumFrontDistance())      // eye and target are too close together
        return ViewportStatus::InvalidTargetPoint;

    DVec3d xVec; // x is the normal to the Up-Z plane
    if (xVec.NormalizedCrossProduct(yVec, zVec) <= mgds_fc_epsilon)
        return ViewportStatus::InvalidUpVector;    // up is parallel to z

    if (yVec.NormalizedCrossProduct(zVec, xVec) <= mgds_fc_epsilon) // make sure up vector is perpendicular to z vector
        return ViewportStatus::InvalidUpVector;

    // we now have rows of the rotation matrix
    RotMatrix rotation = RotMatrix::FromRowVectors(xVec, yVec, zVec);

    double backDist  = backDistIn  ? *backDistIn  : GetBackDistance();
    double frontDist = frontDistIn ? *frontDistIn : GetFrontDistance();
    DVec3d delta     = extentsIn   ? DVec3d::From(fabs(extentsIn->x),fabs(extentsIn->y),GetExtents().z) : GetExtents();

    frontDist = std::max(frontDist, MinimumFrontDistance());
    backDist  = std::max(backDist, focusDist+(.5*DgnUnits::OneMeter()));

    if (backDist < focusDist) // make sure focus distance is in front of back distance.
        backDist = focusDist + DgnUnits::OneMillimeter();

    if (frontDist > focusDist)
        frontDist = focusDist - MinimumFrontDistance();

    if (frontDist < MinimumFrontDistance())
        frontDist = MinimumFrontDistance();

    BeAssert(backDist > frontDist);
    delta.z =(backDist - frontDist);

    DVec3d frontDelta = DVec3d::FromScale(delta, frontDist/focusDist);
    ViewportStatus stat = ValidateViewDelta(frontDelta, false); // validate window size on front (smallest) plane
    if (ViewportStatus::Success != stat)
        return  stat;

    if (delta.z > CalculateMaxDepth(delta, zVec)) // make sure we're not zoomed out too far
        return ViewportStatus::MaxDisplayDepth;

    // The origin is defined as the lower left of the view rectangle on the focus plane, projected to the back plane.
    // Start at eye point, and move to center of back plane, then move left half of width. and down half of height
    DPoint3d origin = DPoint3d::FromSumOf(eyePoint, zVec, -backDist, xVec, -0.5*delta.x, yVec, -0.5*delta.y);

    SetEyePoint(eyePoint);
    SetRotation(rotation);
    SetFocusDistance(focusDist);
    SetOrigin(origin);
    SetExtents(delta);
    SetLensAngle(CalcLensAngle());
    _EnableCamera();

    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::LookAtUsingLensAngle(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVec,
                                                  Angle lens, double const* frontDist, double const* backDist)
    {
    DVec3d zVec; // z defined by direction from eye to target
    zVec.DifferenceOf(eyePoint, targetPoint);

    double focusDist = zVec.Normalize();  // set focus at target point
    if (focusDist <= DgnUnits::OneMillimeter())       // eye and target are too close together
        return ViewportStatus::InvalidTargetPoint;

    if (lens.Radians() < .0001 || lens > Angle::AnglePi())
        return ViewportStatus::InvalidLens;

    double extent = 2.0 * tan(lens.Radians()/2.0) * focusDist;

    DVec2d delta  = DVec2d::From(GetExtents().x, GetExtents().y);
    double longAxis = std::max(delta.x, delta.y);
    delta.Scale(extent/longAxis);

    return LookAt(eyePoint, targetPoint, upVec, &delta, frontDist, backDist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::MoveCameraWorld(DVec3dCR distance)
    {
    if (!IsCameraOn())
        {
        m_origin.SumOf(m_origin, distance);
        return ViewportStatus::Success;
        }

    DPoint3d newTarget, newEyePt;
    newTarget.SumOf(GetTargetPoint(), distance);
    newEyePt.SumOf(GetEyePoint(), distance);
    return LookAt(newEyePt, newTarget, GetYVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::MoveCameraLocal(DVec3dCR distanceLocal)
    {
    DVec3d distWorld = distanceLocal;
    GetRotation().MultiplyTranspose(distWorld);
    return MoveCameraWorld(distWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::RotateCameraWorld(double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
    {
    DPoint3d about = aboutPointIn ? *aboutPointIn : GetEyePoint();
    RotMatrix rotation = RotMatrix::FromVectorAndRotationAngle(axis, radAngle);
    Transform trans    = Transform::FromMatrixAndFixedPoint(rotation, about);

    DPoint3d newTarget = GetTargetPoint();
    trans.Multiply(newTarget);
    DVec3d upVec = GetYVector();
    rotation.Multiply(upVec);

    return LookAt(GetEyePoint(), newTarget, upVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::RotateCameraLocal(double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
    {
    DVec3d axisWorld = axis;
    GetRotation().MultiplyTranspose(axisWorld);
    return RotateCameraWorld(radAngle, axisWorld, aboutPointIn);
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in ViewDefinition.h
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewDefinition3d::GetBackDistance() const
    {
    // backDist is the z component of the vector from the eyePoint to the origin.
    DPoint3d eyeOrg;
    eyeOrg.DifferenceOf(GetEyePoint(), GetOrigin());
    GetRotation().Multiply(eyeOrg); // orient to view
    return eyeOrg.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ViewDefinition::GetCenter() const
    {
    DPoint3d delta;
    GetRotation().MultiplyTranspose(delta, GetExtents());

    DPoint3d center;
    center.SumOf(GetOrigin(), delta, 0.5);
    return  center;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d ViewDefinition3d::_GetTargetPoint() const
    {
    if (!IsCameraOn())
        return T_Super::_GetTargetPoint();

    DVec3d viewZ;
    GetRotation().GetRow(viewZ, 2);
    DPoint3d target;
    target.SumOf(GetEyePoint(), viewZ, -1.0 * GetFocusDistance());
    return  target;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_AdjustAspectRatio(double windowAspect)
    {
    DVec3d extents = GetExtents();
    double viewAspect = extents.x / extents.y;

    windowAspect *= GetAspectRatioSkew();

    if (fabs(1.0 - (viewAspect / windowAspect)) < 1.0e-9)
        return;

    DVec3d oldDelta = extents;
    if (viewAspect > windowAspect)
        extents.y = extents.x / windowAspect;
    else
        extents.x = extents.y * windowAspect;

    DPoint3d origin = GetOrigin();
    DPoint3d newOrigin;
    GetRotation().Multiply(&newOrigin, &origin, 1);
    newOrigin.x += ((oldDelta.x - extents.x) / 2.0);
    newOrigin.y += ((oldDelta.y - extents.y) / 2.0);
    GetRotation().MultiplyTranspose(origin, newOrigin);
    SetOrigin(origin);
    SetExtents(extents);
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace ViewElementHandler
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void View::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, prop_DisplayStyle(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinitionCR viewDef = (ViewDefinitionCR)el;
            value.SetNavigationInfo(viewDef.GetDisplayStyleId());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsNavigation())
                return DgnDbStatus::BadArg;

            DgnElementId id = value.GetNavigationInfo().GetId<DgnElementId>();
            auto style = el.GetDgnDb().Elements().Get<Dgn::DisplayStyle>(id);
            if (!style.IsValid())
                return DgnDbStatus::BadArg;

            ViewDefinitionR viewDef = (ViewDefinitionR)el;
            auto view3d = viewDef.ToView3dP();
            if (view3d)
                {
                auto style3d = style->ToDisplayStyle3d();
                if (nullptr == style3d)
                    return DgnDbStatus::BadArg;
                view3d->SetDisplayStyle3d(*style3d->MakeCopy<Dgn::DisplayStyle3d>());
                }
            else
                {
                auto view2d = viewDef.ToView2dP();
                auto style2d = style->ToDisplayStyle2d();
                if (nullptr == style2d)
                    return DgnDbStatus::BadArg;
                view2d->SetDisplayStyle2d(*style2d->MakeCopy<Dgn::DisplayStyle2d>());
                }

            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, prop_CategorySelector(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinitionCR viewDef = (ViewDefinitionCR)el;
            value.SetNavigationInfo(viewDef.GetCategorySelectorId());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsNavigation())
                return DgnDbStatus::BadArg;

            DgnElementId id = value.GetNavigationInfo().GetId<DgnElementId>();
            auto cats = el.GetDgnDb().Elements().Get<Dgn::CategorySelector>(id);
            if (!cats.IsValid())
                return DgnDbStatus::BadArg;

            ViewDefinitionR viewDef = (ViewDefinitionR)el;
            viewDef.SetCategorySelector(*cats->MakeCopy<Dgn::CategorySelector>());
            return DgnDbStatus::Success;
            });

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void View3d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    #define GET_DOUBLE(EXPR) ViewDefinition3d const& viewDef = (ViewDefinition3d const&)el; YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, viewDef.GetRotation()); value.SetDouble(EXPR); return DgnDbStatus::Success;
    #define GET_POINT(EXPR) ViewDefinition3d const& viewDef = (ViewDefinition3d const&)el; value.SetPoint3d(EXPR); return DgnDbStatus::Success;
    #define SET_POINT(EXPR) ViewDefinition3d& viewDef = (ViewDefinition3d&)el; EXPR; return DgnDbStatus::Success;
    #define SET_DOUBLE(EXPR) ViewDefinition3d& viewDef = (ViewDefinition3d&)el; YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, viewDef.GetRotation()); EXPR; viewDef.SetRotation(angles.ToRotMatrix()); return DgnDbStatus::Success;

    #define VALIDATE_POINT3d_VALUE(VALUE) if (VALUE.IsNull() || !VALUE.IsPoint3d()) return DgnDbStatus::BadArg;
    #define TO_DOUBLE(VALUE,VALUEIN)                                                    \
            if (VALUEIN.IsNull() || VALUEIN.IsBoolean() || !VALUEIN.IsPrimitive())      \
                return DgnDbStatus::BadArg;                                             \
            ECN::ECValue VALUE(VALUEIN);                                                \
            if (!VALUE.ConvertToPrimitiveType(ECN::PRIMITIVETYPE_Double))               \
                return DgnDbStatus::BadArg;

    params.RegisterPropertyAccessors(layout, prop_Origin(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT(viewDef.GetOrigin());
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT3d_VALUE(value);
            SET_POINT(viewDef.SetOrigin(value.GetPoint3d()));
            });

    params.RegisterPropertyAccessors(layout, prop_Extents(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT(viewDef.GetExtents());
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT3d_VALUE(value);
            SET_POINT(viewDef.SetExtents(DVec3d::From(value.GetPoint3d())));
            });

    params.RegisterPropertyAccessors(layout, prop_Yaw(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetYaw().Degrees());
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            SET_DOUBLE(angles.SetYaw(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, prop_Pitch(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetPitch().Degrees());
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            SET_DOUBLE(angles.SetPitch(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, prop_Roll(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetRoll().Degrees());
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            SET_DOUBLE(angles.SetRoll(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, prop_EyePoint(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition3dCR viewDef = (ViewDefinition3dCR)el;
            value.SetPoint3d(viewDef.GetEyePoint());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT3d_VALUE(value);
            ViewDefinition3dR viewDef = (ViewDefinition3dR)el;
            viewDef.SetEyePoint(value.GetPoint3d());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, prop_LensAngle(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition3dCR viewDef = (ViewDefinition3dCR)el;
            value.SetDouble(viewDef.GetLensAngle().Radians());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            ViewDefinition3dR viewDef = (ViewDefinition3dR)el;
            viewDef.SetLensAngle(Angle::FromRadians(value.GetDouble()));
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, prop_FocusDistance(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition3dCR viewDef = (ViewDefinition3dCR)el;
            value.SetDouble(viewDef.GetFocusDistance());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            ViewDefinition3dR viewDef = (ViewDefinition3dR)el;
            viewDef.SetFocusDistance(value.GetDouble());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, prop_IsCameraOn(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition3dCR viewDef = (ViewDefinition3dCR)el;
            value.SetBoolean(viewDef.IsCameraOn());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            return DgnDbStatus::ReadOnly;
            });

#undef TO_DOUBLE
#undef VALIDATE_POINT3d_VALUE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void View2d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    #define GET_POINT2d(EXPR) ViewDefinition2d const& viewDef = (ViewDefinition2d const&)el; value.SetPoint2d(EXPR); return DgnDbStatus::Success;
    #define SET_POINT2d(EXPR) ViewDefinition2d& viewDef = (ViewDefinition2d&)el; EXPR; return DgnDbStatus::Success;

    #define VALIDATE_POINT2d_VALUE(VALUE) if (VALUE.IsNull() || !VALUE.IsPoint2d()) return DgnDbStatus::BadArg;
    #define TO_DOUBLE(VALUE,VALUEIN)                                                    \
            if (VALUEIN.IsNull() || VALUEIN.IsBoolean() || !VALUEIN.IsPrimitive())      \
                return DgnDbStatus::BadArg;                                             \
            ECN::ECValue VALUE(VALUEIN);                                                \
            if (!VALUE.ConvertToPrimitiveType(ECN::PRIMITIVETYPE_Double))               \
                return DgnDbStatus::BadArg;

    params.RegisterPropertyAccessors(layout, prop_BaseModel(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition2dCR viewDef = (ViewDefinition2dCR)el;
            value.SetLong(viewDef.GetBaseModelId().GetValue());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsLong())
                return DgnDbStatus::BadArg;

            ViewDefinition2d& viewDef = (ViewDefinition2d&)el;
            viewDef.m_baseModelId = DgnModelId((uint64_t) value.GetLong());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, prop_RotationAngle(),
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition2dCR viewDef = (ViewDefinition2dCR)el;
            value.SetDouble(Angle::FromRadians(viewDef.GetRotAngle()).Degrees());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            ViewDefinition2d& viewDef = (ViewDefinition2d&)el;
            viewDef.SetRotAngle(Angle::FromDegrees(value.GetDouble()).Radians());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, prop_Origin(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT2d(viewDef.GetOrigin2d());
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT2d_VALUE(value);
            SET_POINT2d(viewDef.SetOrigin2d(value.GetPoint2d()));
            });

    params.RegisterPropertyAccessors(layout, prop_Extents(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT2d(viewDef.GetDelta2d());
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT2d_VALUE(value);
            SET_POINT2d(viewDef.SetDelta2d(DVec2d::From(value.GetPoint2d())));
            });

#undef TO_DOUBLE
#undef VALIDATE_POINT2d_VALUE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialView::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, prop_ModelSelector(),
        [](ECValueR value, DgnElementCR el)
            {
            SpatialViewDefinitionCR viewDef = (SpatialViewDefinitionCR)el;
            value.SetNavigationInfo(viewDef.GetModelSelectorId());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsNavigation())
                return DgnDbStatus::BadArg;
            DgnElementId id = value.GetNavigationInfo().GetId<DgnElementId>();
            auto modelSel = el.GetDgnDb().Elements().Get<ModelSelector>(id);
            if (!modelSel.IsValid())
                return DgnDbStatus::BadArg;

            SpatialViewDefinitionR viewDef = (SpatialViewDefinitionR)el;
            viewDef.SetModelSelector(*modelSel->MakeCopy<ModelSelector>());
            return DgnDbStatus::Success;
            });
    }

}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateViewDefinition2dPtr TemplateViewDefinition2d::Create(DefinitionModelR definitionModel, Utf8StringCR name, CategorySelectorP categorySelectorIn, DisplayStyle2dP displayStyleIn)
    {
    DgnDbR db = definitionModel.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(ViewElementHandler::TemplateView2d::GetHandler());
    if (!classId.IsValid())
        return nullptr;

    CategorySelectorP categorySelector = categorySelectorIn ? categorySelectorIn : new CategorySelector(definitionModel, "");
    DisplayStyle2dP displayStyle = displayStyleIn ? displayStyleIn : new DisplayStyle2d(definitionModel, "");

    TemplateViewDefinition2dPtr viewDef = new TemplateViewDefinition2d(CreateParams(db, definitionModel.GetModelId(), classId, CreateCode(definitionModel, name), *categorySelector));
    viewDef->SetDisplayStyle2d(*displayStyle);

    if (nullptr == categorySelectorIn)
        {
        for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
            viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());
        }

    return viewDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateViewDefinition3dPtr TemplateViewDefinition3d::Create(DefinitionModelR definitionModel, Utf8StringCR name, CategorySelectorP categorySelectorIn, DisplayStyle3dP displayStyleIn)
    {
    DgnDbR db = definitionModel.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(ViewElementHandler::TemplateView3d::GetHandler());
    if (!classId.IsValid())
        return nullptr;

    CategorySelectorP categorySelector = categorySelectorIn ? categorySelectorIn : new CategorySelector(definitionModel, "");
    DisplayStyle3dP displayStyle = displayStyleIn ? displayStyleIn : new DisplayStyle3d(definitionModel, "");

    TemplateViewDefinition3dPtr viewDef = new TemplateViewDefinition3d(CreateParams(db, definitionModel.GetModelId(), classId, CreateCode(definitionModel, name), *categorySelector, *displayStyle));
    if (nullptr == categorySelectorIn)
        {
        for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
            viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());
        }

    return viewDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double  ViewDefinition3d::MinimumFrontDistance(double nearScaleLimit) const
    {
    return GetDgnDb().GeoLocation().GetProjectExtents().DiagonalDistance() * nearScaleLimit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TerrainProps TerrainProps::FromJson(BeJsConst json)
    {
    TerrainProps props;
    if (!json.isObject())
        return props;

    TerrainProps defs;
    props.m_providerName = json[json_providerName()].asString(defs.m_providerName.c_str());
    props.m_exaggeration = json[json_exaggeration()].asDouble(defs.m_exaggeration);
    props.m_heightOrigin = json[json_heightOrigin()].asDouble(defs.m_heightOrigin);
    props.m_heightOriginMode = static_cast<TerrainHeightOriginMode>(json[json_heightOriginMode()].asUInt(static_cast<uint32_t>(defs.m_heightOriginMode)));
    props.m_applyLighting = json[json_applyLighting()].asBool(defs.m_applyLighting);

    return props;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TerrainProps::ToJson(BeJsValue json) const
    {
    json.SetNull();
    TerrainProps defs;

    if (!m_providerName.Equals(defs.m_providerName))
        json[json_providerName()] = m_providerName;

    if (m_exaggeration != defs.m_exaggeration)
        json[json_exaggeration()] = m_exaggeration;

    if (m_heightOrigin != defs.m_heightOrigin)
        json[json_heightOrigin()] = m_heightOrigin;

    if (m_heightOriginMode != defs.m_heightOriginMode)
        json[json_heightOriginMode()] = static_cast<uint32_t>(m_heightOriginMode);

    if (m_applyLighting != defs.m_applyLighting)
        json[json_applyLighting()] = m_applyLighting;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BackgroundMapProps BackgroundMapProps::FromJson(BeJsConst json)
    {
    BackgroundMapProps props;
    if (!json.isObject())
        return props;

    BackgroundMapProps defs;
    props.m_providerName = json[json_providerName()].asString(defs.m_providerName.c_str());
    props.m_groundBias = json[json_groundBias()].asDouble(defs.m_groundBias);
    props.m_useDepthBuffer = json[json_useDepthBuffer()].asBool(defs.m_useDepthBuffer);
    props.m_applyTerrain = json[json_applyTerrain()].asBool(defs.m_applyTerrain);
    props.m_mapType = static_cast<BackgroundMapType>(json[json_providerData()][json_mapType()].asUInt(static_cast<uint32_t>(defs.m_mapType)));
    props.m_globeMode = static_cast<GlobeMode>(json[json_globeMode()].asUInt(static_cast<uint32_t>(defs.m_globeMode)));

    auto transp = json[json_transparency()];
    if (transp.isNumeric())
        props.m_transparency = transp.asDouble(0.0);

    props.m_terrain = TerrainProps::FromJson(json[json_terrainSettings()]);

    return props;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BackgroundMapProps::ToJson(BeJsValue json) const
    {
    json.SetNull();
    BackgroundMapProps defs;

    json[json_providerName()] = m_providerName;
    if (m_groundBias != defs.m_groundBias)
        json[json_groundBias()] = m_groundBias;

    if (m_transparency)
        json[json_transparency()] = *m_transparency;

    if (m_useDepthBuffer != defs.m_useDepthBuffer)
        json[json_useDepthBuffer()] = m_useDepthBuffer;

    if (m_applyTerrain != defs.m_applyTerrain)
        json[json_applyTerrain()] = m_applyTerrain;

    if (m_globeMode != defs.m_globeMode)
        json[json_globeMode()] = static_cast<uint32_t>(m_globeMode);

    if (m_mapType != defs.m_mapType)
        json[json_providerData()][json_mapType()] = static_cast<uint32_t>(m_mapType);

    m_terrain.ToJson(json[json_terrainSettings()]);
    if (json[json_terrainSettings()].isNull())
        json.removeMember(json_terrainSettings());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RenderTimeline::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    if (opts[json_renderTimeline()][json_omitScriptElementIds()].asBool())
        {
        BeJsDocument script(val["script"].asString());
        removeScheduleScriptElementIds(script);
        val["script"] = script.Stringify();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RenderTimeline::ScriptHost RenderTimeline::GetScriptHost(DgnElementId elemId, DgnDbR db)
    {
    // Check for a RenderTimeline element first. This class was introduced in BisCore 01.00.13.
    auto timeline = db.Elements().Get<RenderTimeline>(elemId);
    if (timeline.IsValid())
        return *timeline;

    // Prior to introduction of RenderTimeline element the script was stored in DisplayStyle JSON properties.
    auto style = db.Elements().Get<DisplayStyle>(elemId);
    if (style.IsValid())
        return *style;

    return ScriptHost();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RenderTimeline::ScriptHost::ScriptHost(DisplayStyleCR style) : ScriptHost(style, false) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsConst RenderTimeline::ScriptHost::GetScript() const
    {
    if (!IsValid())
        return BeJsDocument::Null();
    else if (m_isTimeline)
        return static_cast<RenderTimelineCR>(*m_elem).GetScript();
    else
        return static_cast<DisplayStyleCR>(*m_elem).GetStyle("scheduleScript");
    }
