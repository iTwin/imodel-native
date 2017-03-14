/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewDefinition.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    HANDLER_DEFINE_MEMBERS(ViewDisplayStyle3d);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

namespace ViewProperties
{
    static constexpr Utf8CP str_ModelSelector() {return "ModelSelector";}
    static constexpr Utf8CP str_CategorySelector() {return "CategorySelector";}
    static constexpr Utf8CP str_DisplayStyle() {return "DisplayStyle";}
    static constexpr Utf8CP str_BackgroundColor(){return "backgroundColor";}
    static constexpr Utf8CP str_MonochromeColor(){return "monochromeColor";}
    static constexpr Utf8CP str_ViewFlags() {return "viewflags";}
    static constexpr Utf8CP str_SubCategory() {return "SubCategory";}
    static constexpr Utf8CP str_SubCategoryOverrides() {return "SubCategoryOvr";}
    static constexpr Utf8CP str_LensAngle() {return "LensAngle";}
    static constexpr Utf8CP str_FocusDistance() {return "FocusDistance";}
    static constexpr Utf8CP str_EyePoint() {return "EyePoint";}
    static constexpr Utf8CP str_BaseModel() {return "BaseModel";}
    static constexpr Utf8CP str_Origin() {return "Origin";}
    static constexpr Utf8CP str_Extents() {return "Extents";}
    static constexpr Utf8CP str_RotationAngle() {return "RotationAngle";}
    static constexpr Utf8CP str_Yaw() {return "Yaw";}
    static constexpr Utf8CP str_Pitch() {return "Pitch";}
    static constexpr Utf8CP str_Roll() {return "Roll";}
    static constexpr Utf8CP str_AspectSkew() {return "AspectSkew";}
    static constexpr Utf8CP str_Width() {return "width";}
    static constexpr Utf8CP str_Height() {return "height";}
    static constexpr Utf8CP str_Format() {return "format";}
    static constexpr Utf8CP str_Jpeg() {return "jpeg";}
    static constexpr Utf8CP str_Png() {return "png";}
    static constexpr Utf8CP str_Clip() {return "clip";}
    static constexpr Utf8CP str_IsCameraOn() {return "IsCameraOn";}
    static constexpr Utf8CP str_IsPrivate() {return "IsPrivate";}
    static constexpr Utf8CP str_Hidden() {return "hidden";}
    static constexpr Utf8CP str_Visible() {return "visible";}
    static constexpr Utf8CP str_OvrColorFlag() {return "ovrColor";}
    static constexpr Utf8CP str_Color() {return "color";}
    static constexpr Utf8CP str_Pattern() {return "pattern";}
    static constexpr Utf8CP str_TransparencyThreshold() {return "transThreshold";}
    static constexpr Utf8CP str_GridOrient() {return "gridOrient";}
    static constexpr Utf8CP str_GridSpaceX() {return "gridSpaceX";}
    static constexpr Utf8CP str_GridSpaceY() {return "gridSpaceY";}
    static constexpr Utf8CP str_GridPerRef() {return "gridPerRef";}
    static constexpr Utf8CP str_Ambient() {return "ambient";}
    static constexpr Utf8CP str_Flash() {return "flash";}
    static constexpr Utf8CP str_PortraitLeft() {return "left";}
    static constexpr Utf8CP str_PortraitRight() {return "right";}
    static constexpr Utf8CP str_Brightness() {return "brightness";}
    static constexpr Utf8CP str_AvgLum() {return "avgLum";}
    static constexpr Utf8CP str_MaxLum() {return "maxLum";}
    static constexpr Utf8CP str_Fstop() {return "fstop";}
    static constexpr Utf8CP str_Sun() {return "sun";}
    static constexpr Utf8CP str_Intensity() {return "intensity";}
    static constexpr Utf8CP str_Direction() {return "dir";}
};

using namespace ViewProperties;

HANDLER_EXTENSION_DEFINE_MEMBERS(ViewControllerOverride)

BEGIN_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> T& getThreadSafe(DgnDbR db, DgnElementId id, RefCountedPtr<T>& var)
    {
    if (!var.IsValid()) // first try without mutex
        {
        BeMutexHolder (db.Elements().GetMutex());
        if (!var.IsValid()) // test again after acquiring mutex
            {
            BeAssert(id.IsValid());
            auto el = db.Elements().Get<T>(id);
            if (!el.IsValid())
                {
                BeAssert(false);
                }
            var = el->template MakeCopy<T>();
            }
        }

    return *var;
    }
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::IsValidCode(DgnCodeCR code)
    {
    return !code.GetValue().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId ViewDefinition::QueryViewId(DgnDbR db, DgnCodeCR code)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return DgnViewId(elemId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(DgnViewId viewId, DgnDbR db)
    {
    auto view = Get(db, viewId);
    return view.IsValid() ? view->LoadViewController() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(bool allowOverrides) const
    {
    ViewControllerOverride* ovr = allowOverrides ? ViewControllerOverride::Cast(GetElementHandler()) : nullptr;
    ViewControllerPtr controller = ovr ? ovr->_SupplyController(*this) : nullptr;

    if (controller.IsNull())
        controller = _SupplyController();

    if (!controller.IsValid())
        return nullptr;

    controller->LoadState();
    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::_EqualState(ViewDefinitionR other)
    {
    if (m_isPrivate != other.m_isPrivate)
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_OnInsert()
    {
    if (!GetDisplayStyle().GetElementId().IsValid())
        {
        m_displayStyle->Insert();
        m_displayStyleId = m_displayStyle->GetElementId();
        }

    if (!GetCategorySelector().GetElementId().IsValid())
        {
        m_categorySelector->Insert();
        m_categorySelectorId = m_categorySelector->GetElementId();
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    BeAssert(GetDisplayStyleId().IsValid());
    BeAssert(GetCategorySelectorId().IsValid());
    auto stat = stmt.BindNavigationValue(stmt.GetParameterIndex(str_DisplayStyle()), GetDisplayStyleId());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindNavigationValue(stmt.GetParameterIndex(str_CategorySelector()), GetCategorySelectorId());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindBoolean(stmt.GetParameterIndex(str_IsPrivate()), IsPrivate());
    BeAssert(ECSqlStatus::Success == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ViewDefinition::ToDetailJson()
    {
    _OnSaveJsonProperties();
    return Json::FastWriter::ToString(GetDetails());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::SetViewClip(ClipVectorPtr clip)
    {
    if (clip.IsValid())
        SetDetail(str_Clip(), clip->ToJson());
    else
        RemoveDetail(str_Clip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ViewDefinition::GetViewClip() const
    {
    return ClipVector::FromJson(GetDetail(str_Clip()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::SetGridSettings(GridOrientationType orientation, DPoint2dCR spacing, uint32_t gridsPerRef)
    {
    if (GridOrientationType::WorldXY != orientation)
        SetDetail(str_GridOrient(), Json::Value((uint32_t) orientation));
    else
        RemoveDetail(str_GridOrient());

    if (10 != gridsPerRef)
        SetDetail(str_GridPerRef(), Json::Value(gridsPerRef));
    else
        RemoveDetail(str_GridPerRef());

    if (1.0 != spacing.x)
        SetDetail(str_GridSpaceX(), Json::Value(spacing.x));
    else
        RemoveDetail(str_GridSpaceX());

    if (spacing.y != spacing.x)
        SetDetail(str_GridSpaceY(), Json::Value(spacing.y));
    else
        RemoveDetail(str_GridSpaceY());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::GetGridSettings(GridOrientationType& orientation, DPoint2dR spacing, uint32_t& gridsPerRef) const
    {
    JsonValueCR valO = GetDetail(str_GridOrient());
    orientation = valO.isNull() ? GridOrientationType::WorldXY : (GridOrientationType) valO.asUInt();

    JsonValueCR valR = GetDetail(str_GridPerRef());
    gridsPerRef = valR.isNull() ? 10 : valR.asUInt();

    JsonValueCR valX = GetDetail(str_GridSpaceX());
    spacing.x = valX.isNull() ? 1.0 : valX.asDouble();

    JsonValueCR valY = GetDetail(str_GridSpaceY());
    spacing.y = valY.isNull() ? spacing.x : valY.asDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
CategorySelectorR ViewDefinition::GetCategorySelector()
    {
    BeAssert (!IsPersistent()); // you can only call this on a writeable copy 
    return getThreadSafe<CategorySelector>(m_dgndb, GetCategorySelectorId(), m_categorySelector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleR ViewDefinition::GetDisplayStyle()
    {
    BeAssert (!IsPersistent()); // you can only call this on a writeable copy 
    return getThreadSafe<DisplayStyle>(m_dgndb, GetDisplayStyleId(), m_displayStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSelectorR SpatialViewDefinition::GetModelSelector()
    {
    BeAssert (!IsPersistent()); // you can only call this on a writeable copy 
    return getThreadSafe<ModelSelector>(m_dgndb, GetModelSelectorId(), m_modelSelector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_isPrivate = stmt.GetValueBoolean(params.GetSelectIndex(str_IsPrivate()));
    m_displayStyleId = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(str_DisplayStyle()));
    m_categorySelectorId = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(str_CategorySelector()));

    // NOTE: Const ViewDefinitions should never have their display styles or category selector set! You must get a writeable copy to have them.
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);

    auto& other = static_cast<ViewDefinitionCR>(el);
    m_isPrivate = other.m_isPrivate;
    m_categorySelectorId = other.m_categorySelectorId;
    m_displayStyleId = other.m_displayStyleId;
    m_categorySelector = other.m_categorySelector.IsValid() ? other.m_categorySelector->MakeCopy<CategorySelector>() : nullptr;
    m_displayStyle = other.m_displayStyle.IsValid() ? other.m_displayStyle->MakeCopy<DisplayStyle>() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr SpatialViewDefinition::_SupplyController() const
    {
    return new SpatialViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr OrthographicViewDefinition::_SupplyController() const
    {
    return new OrthographicViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr DrawingViewDefinition::_SupplyController() const
    {
    return new DrawingViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double DrawingViewDefinition::GetAspectRatioSkew() const
    {
    return GetDetail(str_AspectSkew()).asDouble(1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus lockElement(DgnElementCR ele)
    {
    LockRequest lockReq;
    lockReq.Insert(ele, LockLevel::Exclusive);
    return (RepositoryStatus::Success == ele.GetDgnDb().BriefcaseManager().AcquireLocks(lockReq).Result())? DgnDbStatus::Success: DgnDbStatus::LockNotHeld;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition2d::OnModelDelete(DgnDbR db, DgnModelId mid)
    {
    auto findViewsStmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA("ViewDefinition2d") " WHERE BaseModel.Id=?");
    if (!findViewsStmt.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }
    findViewsStmt->BindId(1, mid);
    while (BE_SQLITE_ROW == findViewsStmt->Step())
        {
        auto viewId = findViewsStmt->GetValueId<DgnViewId>(0);
        auto view = Get(db, viewId);
        if (view.IsValid())
            {
            lockElement(*view);
            view->Delete();
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    auto stat = stmt.BindPoint2d(stmt.GetParameterIndex(str_Origin()), m_origin);
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindPoint2d(stmt.GetParameterIndex(str_Extents()), m_delta);
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindDouble(stmt.GetParameterIndex(str_RotationAngle()), Angle::FromRadians(m_rotAngle).Degrees());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindNavigationValue(stmt.GetParameterIndex(str_BaseModel()), m_baseModelId);
    BeAssert(ECSqlStatus::Success == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition2d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    m_origin  = stmt.GetValuePoint2d(params.GetSelectIndex(str_Origin()));
    m_delta   = DVec2d::From(stmt.GetValuePoint2d(params.GetSelectIndex(str_Extents())));
    m_rotAngle = Angle::FromDegrees(stmt.GetValueDouble(params.GetSelectIndex(str_RotationAngle()))).Radians();
    m_baseModelId = stmt.GetValueNavigation<DgnModelId>(params.GetSelectIndex(str_BaseModel()));

    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto const& other = (ViewDefinition2d const&) el;
    m_baseModelId = other.m_baseModelId;
    m_origin = other.m_origin;
    m_delta = other.m_delta;
    m_rotAngle = other.m_rotAngle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition2d::_EqualState(ViewDefinitionR in)
    {
    auto const& other= (ViewDefinition2d const&) in;
    if (m_baseModelId != other.m_baseModelId || !m_origin.IsEqual(other.m_origin) || !m_delta.IsEqual(other.m_delta) || m_rotAngle != other.m_rotAngle)
        return false;

    return T_Super::_EqualState(in);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
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
* @bsimethod                                                    Shaun.Sewall    02/17
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
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Desired> static bool isEntryOfClass(ViewDefinition::Entry const& entry)
    {
    DgnDbP db = entry.GetDgnDb();
    if (nullptr == db)
        return false;

    auto entryClass = db->Schemas().GetECClass(entry.GetClassId());
    auto desiredClass = db->Schemas().GetECClass(T_Desired::QueryClassId(*db));
    return nullptr != entryClass && nullptr != desiredClass && entryClass->Is(desiredClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::Entry::IsOrthographicView() const {return isEntryOfClass<OrthographicViewDefinition>(*this);}
bool ViewDefinition::Entry::IsView3d() const {return isEntryOfClass<ViewDefinition3d>(*this);}
bool ViewDefinition::Entry::IsSpatialView() const {return isEntryOfClass<SpatialViewDefinition>(*this);}
bool ViewDefinition::Entry::IsDrawingView() const {return isEntryOfClass<DrawingViewDefinition>(*this);}
bool ViewDefinition::Entry::IsSheetView() const {return isEntryOfClass<SheetViewDefinition>(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CategorySelector::_CopyFrom(DgnElementCR in)
    {
    T_Super::_CopyFrom(in);
    auto const& other = (CategorySelector const&) in;
    m_categories = other.m_categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_InsertInDb()
    {
    auto status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteCategories();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CategorySelector::EqualState(CategorySelectorCR other) const
    {
    return m_categories == other.m_categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_OnUpdate(DgnElementCR el)
    {
    auto status = T_Super::_OnUpdate(el);
    if (DgnDbStatus::Success != status)
        return status;

    auto delExisting = GetDgnDb().GetNonSelectPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " WHERE SourceECInstanceId=?", GetDgnDb().GetECCrudWriteToken());
    delExisting->BindId(1, GetElementId());
    delExisting->Step();

    return WriteCategories();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
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
* @bsimethod                                                    Sam.Wilson      08/16
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  10/2016
//---------------------------------------------------------------------------------------
DgnElementIdSet CategorySelector::QuerySelectors(DgnDbR db)
    {
    DgnElementIdSet ids;

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_CategorySelector));
    if (stmt.IsValid())
        {
        while (BE_SQLITE_ROW == stmt->Step())
            ids.insert(stmt->GetValueId<DgnElementId>(0));
        }

    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::OnModelDelete(DgnDbR db, DgnModelId mid)
    {
    // Detect all ModelSelectors that include this model
    auto statement = db.GetPreparedECSqlStatement("SELECT SourceECInstanceId FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE TargetECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }
    statement->BindId(1, mid);
    while (BE_SQLITE_ROW == statement->Step())
        {
        auto selId = statement->GetValueId<DgnElementId>(0);
        auto selector = db.Elements().Get<ModelSelector>(selId);
        if (!selector.IsValid())
            continue;

        //  If the this ModelSelector contains *ONLY* this model, it is about to become empty. Delete it.
        if (selector->GetModels().size() == 1)
            {
            lockElement(*selector);
            selector->Delete();
            }
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_OnDelete() const
    {
    // Delete all 3d views that are based on on this selector
    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA("SpatialViewDefinition") " WHERE ModelSelector.Id=?");
    statement->BindId(1, GetElementId());
    if (BE_SQLITE_ROW == statement->Step())
        {
        auto view = ViewDefinition::Get(GetDgnDb(), statement->GetValueId<DgnViewId>(0));
        if (view.IsValid())
            view->Delete();
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_CopyFrom(DgnElementCR rhsElement)
    {
    T_Super::_CopyFrom(rhsElement);

    auto const& rhs = (ModelSelector const&)rhsElement;
    m_models = rhs.m_models;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_InsertInDb()
    {
    auto status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteModels();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_OnUpdate(DgnElementCR el)
    {
    auto status = T_Super::_OnUpdate(el);
    if (DgnDbStatus::Success != status)
        return status;

    auto delExisting = GetDgnDb().GetNonSelectPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE SourceECInstanceId=?", GetDgnDb().GetECCrudWriteToken());
    delExisting->BindId(1, GetElementId());
    delExisting->Step();

    return WriteModels();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
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
* @bsimethod                                                    Sam.Wilson      06/16
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindPoint3d(stmt.GetParameterIndex(str_Origin()), m_origin);
    stmt.BindPoint3d(stmt.GetParameterIndex(str_Extents()), m_extents);

    YawPitchRollAngles angles;
    YawPitchRollAngles::TryFromRotMatrix(angles, m_rotation);

    stmt.BindDouble(stmt.GetParameterIndex(str_Yaw()), angles.GetYaw().Degrees());
    stmt.BindDouble(stmt.GetParameterIndex(str_Pitch()), angles.GetPitch().Degrees());
    stmt.BindDouble(stmt.GetParameterIndex(str_Roll()), angles.GetRoll().Degrees());

    auto stat = stmt.BindPoint3d(stmt.GetParameterIndex(str_EyePoint()), GetEyePoint());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindDouble(stmt.GetParameterIndex(str_LensAngle()), GetLensAngle().Radians());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindDouble(stmt.GetParameterIndex(str_FocusDistance()), GetFocusDistance());
    BeAssert(ECSqlStatus::Success == stat);
    stat = stmt.BindBoolean(stmt.GetParameterIndex(str_IsCameraOn()), IsCameraOn());
    BeAssert(ECSqlStatus::Success == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition3d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    m_origin  = stmt.GetValuePoint3d(params.GetSelectIndex(str_Origin()));
    m_extents = DVec3d::From(stmt.GetValuePoint3d(params.GetSelectIndex(str_Extents())));

    double yaw   = stmt.GetValueDouble(params.GetSelectIndex(str_Yaw())),
           pitch = stmt.GetValueDouble(params.GetSelectIndex(str_Pitch())),
           roll  = stmt.GetValueDouble(params.GetSelectIndex(str_Roll()));

    m_rotation = YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)).ToRotMatrix();

    m_cameraDef.SetEyePoint(stmt.GetValuePoint3d(params.GetSelectIndex(str_EyePoint())));
    m_cameraDef.SetLensAngle(Angle::FromRadians(stmt.GetValueDouble(params.GetSelectIndex(str_LensAngle()))));
    m_cameraDef.SetFocusDistance(stmt.GetValueDouble(params.GetSelectIndex(str_FocusDistance())));
    m_cameraOn = stmt.GetValueBoolean(params.GetSelectIndex(str_IsCameraOn()));

    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);

    auto const& other = (ViewDefinition3d const&) el;
    m_origin = other.m_origin;
    m_extents = other.m_extents;
    m_rotation = other.m_rotation;
    m_cameraDef = other.m_cameraDef;
    m_cameraOn = other.m_cameraOn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::SaveThumbnail(Point2d size, Render::ImageSourceCR source) const
    {
    Json::Value val;
    val[str_Width()] = size.x;
    val[str_Height()] = size.y;
    val[str_Format()] = (source.GetFormat() == ImageSource::Format::Jpeg) ? str_Jpeg() : str_Png();

    DbResult rc = m_dgndb.SaveProperty(DgnViewProperty::ViewThumbnail(), Json::FastWriter().ToString(val), source.GetByteStream().GetData(), source.GetByteStream().GetSize(), GetViewId().GetValue());
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
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

    Json::Value value;
    Json::Reader::Parse(jsonStr, value);
    image.SetFormat(value[str_Format()] == str_Jpeg() ? ImageSource::Format::Jpeg : ImageSource::Format::Png);
    image.GetByteStreamR().Resize(bytes);
    stat = db.QueryProperty(image.GetByteStreamR().GetDataP(), bytes, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    BeAssert(BE_SQLITE_ROW == stat);
    return image;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Point2d ViewDefinition::GetThumbnailSize() const
    {
    Point2d size = {0,0};

    DgnDbR db = GetDgnDb();
    Utf8String jsonStr;
    auto stat = db.QueryProperty(jsonStr, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    if (BE_SQLITE_ROW != stat)
        return size;

    Json::Value value;
    Json::Reader::Parse(jsonStr, value);
    size.x = value[str_Width()].asInt();
    size.y = value[str_Height()].asInt();
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::DeleteThumbnail() const
    {
    GetDgnDb().DeleteProperty(DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialViewDefinition::_OnInsert()
    {
    if (!GetModelSelector().GetElementId().IsValid())
        {
        m_modelSelector->Insert();
        m_modelSelectorId = m_modelSelector->GetElementId();
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto& other = static_cast<SpatialViewDefinitionCR>(el);

    m_modelSelectorId = other.m_modelSelectorId;
    m_modelSelector = other.m_modelSelector.IsValid() ? other.m_modelSelector->MakeCopy<ModelSelector>() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    BeAssert(GetModelSelectorId().IsValid());
    stmt.BindNavigationValue(stmt.GetParameterIndex(str_ModelSelector()), GetModelSelectorId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialViewDefinition::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    m_modelSelectorId = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(str_ModelSelector()));
    return T_Super::_ReadSelectParams(stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
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
* @bsimethod                                    Keith.Bentley                   01/14
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
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::HiddenLineParams::Style::ToJson() const
    {
    Json::Value val;
    val[Json::StaticString(str_OvrColorFlag())] = m_ovrColor;
    val[Json::StaticString(str_Color())] = m_color.GetValue();
    val[Json::StaticString(str_Pattern())] = (Json::UInt32) m_pattern;
    val[Json::StaticString(str_Width())] = (Json::UInt32) m_width;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::HiddenLineParams::Style::FromJson(JsonValueCR val)
    {
    m_ovrColor = val[str_OvrColorFlag()].asBool(m_ovrColor);
    m_color = ColorDef(val[str_Color()].asUInt(m_color.GetValue()));
    m_pattern = (GraphicParams::LinePixels) val[str_Pattern()].asUInt((uint32_t) m_pattern);
    m_width = val[str_Width()].asUInt(m_width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::HiddenLineParams::ToJson() const
    {
    HiddenLineParams defaults;
    Json::Value val;

    if (m_visible != defaults.m_visible) val[Json::StaticString(str_Visible())] = m_visible.ToJson();
    if (m_hidden != defaults.m_hidden) val[Json::StaticString(str_Hidden())] = m_hidden.ToJson();
    if (m_transparencyThreshold != defaults.m_transparencyThreshold) val[Json::StaticString(str_TransparencyThreshold())] = m_transparencyThreshold;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::HiddenLineParams Render::HiddenLineParams::FromJson(JsonValueCR val)
    {
    HiddenLineParams params;

    if (!val.isNull())
        {
        params.m_visible.FromJson(val[str_Visible()]);
        params.m_hidden.FromJson(val[str_Hidden()]);
        params.m_transparencyThreshold = val[str_TransparencyThreshold()].asDouble(params.m_transparencyThreshold);
        }
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::SceneLights::Solar::FromJson(JsonValueCR val)
    {
    m_enabled = false;
    if (val.isNull())
        return;

    m_enabled = true;
    m_intensity = val[str_Intensity()].asDouble();
    JsonUtils::DVec3dFromJson(m_direction, val[str_Direction()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::SceneLights::Solar::ToJson() const
    {
    Json::Value val;
    val[Json::StaticString(str_Intensity())] = m_intensity;
    JsonUtils::DVec3dToJson(val[str_Direction()], m_direction);
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::SceneLights::Brightness::FromJson(JsonValueCR val)
    {
    if (val.isNull())
        return;
    m_avgLum = val[str_AvgLum()].asDouble(0.0);
    m_maxLum = val[str_MaxLum()].asDouble(0.0);
    m_fstop = val[str_Fstop()].asDouble(0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::SceneLights::Brightness::ToJson() const
    {
    Json::Value val;
    if (0.0 != m_avgLum) val[Json::StaticString(str_AvgLum())] = m_avgLum;
    if (0.0 != m_maxLum) val[Json::StaticString(str_MaxLum())] = m_maxLum;
    if (0.0 != m_fstop) val[Json::StaticString(str_Fstop())] = m_fstop;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::SceneLights Render::SceneLights::FromJson(JsonValueCR val)
    {
    SceneLights lights;

    if (!val.isNull())
        {
        lights.m_ambient = val[str_Ambient()].asDouble(0.0);
        lights.m_flash = val[str_Flash()].asDouble(0.0);
        lights.m_portraitLeft = val[str_PortraitLeft()].asDouble(0.0);
        lights.m_portraitRight = val[str_PortraitRight()].asDouble(0.0);
        lights.m_brightness.FromJson(val[str_Brightness()]);
        lights.m_sun.FromJson(val[str_Sun()]);
        }
    return lights;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::SceneLights::ToJson() const
    {
    Json::Value val;
    if (0.0 != m_ambient) val[Json::StaticString(str_Ambient())] = m_ambient;
    if (0.0 != m_flash) val[Json::StaticString(str_Flash())] = m_flash;
    if (0.0 != m_portraitLeft) val[Json::StaticString(str_PortraitLeft())] = m_portraitLeft;
    if (0.0 != m_portraitRight) val[Json::StaticString(str_PortraitRight())] = m_portraitRight;
    if (m_brightness.IsValid()) val[Json::StaticString(str_Brightness())] = m_brightness.ToJson();
    if (m_sun.IsEnabled()) val[Json::StaticString(str_Sun())] = m_sun.ToJson();
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
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
* @bsimethod                                    Keith.Bentley                   12/13
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
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::DropSubCategoryOverride(DgnSubCategoryId id)
    {
    m_subCategoryOverrides.erase(id);
    LoadSubCategory(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);

    auto& other = static_cast<DisplayStyleCR>(el);

    m_viewFlags = other.m_viewFlags;
    m_subCategories = other.m_subCategories;
    m_subCategoryOverrides = other.m_subCategoryOverrides;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_OnLoadedJsonProperties()
    {
    m_viewFlags.FromJson(GetStyle(str_ViewFlags()));

    JsonValueCR overrides = GetStyle(str_SubCategoryOverrides());
    for (Json::ArrayIndex i = 0; i<overrides.size(); ++i)
        {
        JsonValueCR val = overrides[i];
        DgnSubCategoryId subCategoryId(val[str_SubCategory()].asUInt64());
        if (!subCategoryId.IsValid())
            {
            BeDataAssert(false && "SubCategoryOverride refers to missing SubCategory");
            continue;
            }
        OverrideSubCategory(subCategoryId, DgnSubCategory::Override(val));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DisplayStyle::ToJson() const
    {
    const_cast<DisplayStyleR>(*this)._OnSaveJsonProperties();
    return Json::FastWriter::ToString(GetStyles());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayStyle::EqualState(DisplayStyleR other) 
    {
    _OnSaveJsonProperties();
    other._OnSaveJsonProperties();
    return GetStyles()==other.GetStyles();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_OnSaveJsonProperties()
    {
    SetStyle(str_ViewFlags(), m_viewFlags.ToJson());

    if (m_subCategoryOverrides.empty())
        {
        RemoveStyle(str_SubCategoryOverrides());
        }
    else
        {
        Json::Value ovrJson;
        int i=0;
        for (auto const& it : m_subCategoryOverrides)
            {
            ovrJson[i][Json::StaticString(str_SubCategory())] = it.first.GetValue();
            it.second.ToJson(ovrJson[i++]);
            }
        SetStyle(str_SubCategoryOverrides(), ovrJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DisplayStyle::GetBackgroundColor() const
    {
    return ColorDef(GetStyle(str_BackgroundColor()).asUInt(ColorDef::Black().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetBackgroundColor(ColorDef val)
    {
    if (ColorDef::Black() == val)
        RemoveStyle(str_BackgroundColor());    // black is the default
    else
        SetStyle(str_BackgroundColor(), Json::Value(val.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DisplayStyle::GetMonochromeColor() const
    {
    return ColorDef(GetStyle(str_MonochromeColor()).asUInt(ColorDef::White().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetMonochromeColor(ColorDef val)
    {
    if (ColorDef::White() == val)
        RemoveStyle(str_MonochromeColor());    // white is the default
    else
        SetStyle(str_MonochromeColor(), Json::Value(val.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* Search the (8) standard view matrices for one that is close to given matrix.
* @bsimethod                                                    EarlinLutz      05/05
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
* @bsimethod                                                    KeithBentley    11/02
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

    // use center of frustum and view diagonal for origin. Original frustum may not have been orgthogonal
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
* @bsimethod                                    Keith.Bentley                   02/10
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
* @bsimethod                                    Keith.Bentley                   02/10
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
* @bsimethod                                    Keith.Bentley                   09/13
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
        _AdjustAspectRatio(*aspect, true);

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
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Angle ViewDefinition3d::CalcLensAngle() const
    {
    double maxDelta = std::max(m_extents.x, m_extents.y);
    return Angle::FromRadians(2.0 * Angle::Atan2(maxDelta*0.5, m_cameraDef.GetFocusDistance()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
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
* @bsimethod                                    Keith.Bentley                   07/14
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
* @bsimethod                                    Keith.Bentley                   08/13
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
* @bsimethod                                    Keith.Bentley                   07/15
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
* @bsimethod                                    Keith.Bentley                   08/13
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

    frontDist = std::max(frontDist, (.5 *DgnUnits::OneMeter())); 
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
* @bsimethod                                    Keith.Bentley                   09/13
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
* @bsimethod                                    Keith.Bentley                   08/13
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
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::MoveCameraLocal(DVec3dCR distanceLocal)
    {
    DVec3d distWorld = distanceLocal;
    GetRotation().MultiplyTranspose(distWorld);
    return MoveCameraWorld(distWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
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
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus ViewDefinition3d::RotateCameraLocal(double radAngle, DVec3dCR axis, DPoint3dCP aboutPointIn)
    {
    DVec3d axisWorld = axis;
    GetRotation().MultiplyTranspose(axisWorld);
    return RotateCameraWorld(radAngle, axisWorld, aboutPointIn);
    }

/*---------------------------------------------------------------------------------**//**
* See diagram in ViewDefinition.h
* @bsimethod                                    Keith.Bentley                   08/13
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
* @bsimethod                                    Keith.Bentley                   03/17
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
* @bsimethod                                    Keith.Bentley                   03/17
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
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_AdjustAspectRatio(double windowAspect, bool expandView)
    {
    DPoint3dR origin = m_origin;
    DVec3dR   delta = m_extents;
    RotMatrixR rotation = m_rotation; 

    // first, make sure none of the deltas are negative
    delta.x = fabs(delta.x);
    delta.y = fabs(delta.y);
    delta.z = fabs(delta.z);

    double maxAbs = max(delta.x, delta.y);

    // if all deltas are zero, set to 1m (what else can we do?)
    if (0.0 == maxAbs)
        delta.x = delta.y = DgnUnits::OneMeter();

    // if either dimension is zero, set it to the other.
    if (delta.x == 0)
        delta.x = maxAbs;
    if (delta.y == 0)
        delta.y = maxAbs;

    double viewAspect  = delta.x / delta.y;

    if (fabs(1.0 -(viewAspect / windowAspect)) < 1.0e-9)
        return;

    DVec3d oldDelta = delta;

    if (!expandView)
        {
        if (viewAspect > 1.0)
            delta.y = delta.x;
        else
            delta.x = delta.y;
        }

    double maxExtent, minExtent;
    _GetExtentLimits(minExtent, maxExtent);
    if (expandView ? (viewAspect > windowAspect) : (windowAspect > 1.0))
        {
        double rtmp = delta.x / windowAspect;
        if (rtmp < maxExtent)
            delta.y = rtmp;
        else
            {
            delta.y = maxExtent;
            delta.x = maxExtent * windowAspect;
            }
        }
    else
        {
        double rtmp = delta.y * windowAspect;
        if (rtmp < maxExtent)
            delta.x = rtmp;
        else
            {
            delta.x = maxExtent;
            delta.y = maxExtent / windowAspect;
            }
        }

    DPoint3d newOrigin;
    rotation.Multiply(&newOrigin, &origin, 1);
    newOrigin.x +=(oldDelta.x - delta.x) / 2.0;
    newOrigin.y +=(oldDelta.y - delta.y) / 2.0;
    rotation.MultiplyTranspose(origin, newOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_AdjustAspectRatio(double windowAspect, bool expandView)
    {
    // first, make sure none of the deltas are negative
    m_delta.x = fabs(m_delta.x);
    m_delta.y = fabs(m_delta.y);

    double maxAbs = max(m_delta.x, m_delta.y);

    // if all deltas are zero, set to 1m (what else can we do?)
    if (0.0 == maxAbs)
        m_delta.x = m_delta.y = DgnUnits::OneMeter();

    // if either dimension is zero, set it to the other.
    if (m_delta.x == 0)
        m_delta.x = maxAbs;
    if (m_delta.y == 0)
        m_delta.y = maxAbs;

    double viewAspect  = m_delta.x / m_delta.y;
    if (fabs(1.0 -(viewAspect / windowAspect)) < 1.0e-9)
        return;

    DVec2d oldDelta = m_delta;
    if (!expandView)
        {
        if (viewAspect > 1.0)
            m_delta.y = m_delta.x;
        else
            m_delta.x = m_delta.y;
        }

    double maxExtent, minExtent;
    _GetExtentLimits(minExtent, maxExtent);
    if (expandView ? (viewAspect > windowAspect) : (windowAspect > 1.0))
        {
        double rtmp = m_delta.x / windowAspect;
        if (rtmp < maxExtent)
            m_delta.y = rtmp;
        else
            {
            m_delta.y = maxExtent;
            m_delta.x = maxExtent * windowAspect;
            }
        }
    else
        {
        double rtmp = m_delta.y * windowAspect;
        if (rtmp < maxExtent)
            m_delta.x = rtmp;
        else
            {
            m_delta.x = maxExtent;
            m_delta.y = maxExtent / windowAspect;
            }
        }

    DPoint2d origin;
    RotMatrix rMatrix = GetRotation();
    rMatrix.Multiply(&origin, &m_origin, 1);
    origin.x +=(oldDelta.x - m_delta.x) / 2.0;
    origin.y +=(oldDelta.y - m_delta.y) / 2.0;
    rMatrix.Transpose();
    rMatrix.Multiply(&m_origin, &origin, 1);
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace ViewElementHandler
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void View::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, str_IsPrivate(), 
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinitionCR viewDef = (ViewDefinitionCR)el;
            value.SetBoolean(viewDef.IsPrivate());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsBoolean())
                return DgnDbStatus::BadArg;

            ViewDefinitionR viewDef = (ViewDefinitionR)el;
            viewDef.SetIsPrivate(value.GetBoolean());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, str_DisplayStyle(), 
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
                view2d->SetDisplayStyle(*style->MakeCopy<Dgn::DisplayStyle>());
                }

            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, str_CategorySelector(), 
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void View3d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    #define GET_DOUBLE(EXPR) ViewDefinition3d& viewDef = (ViewDefinition3d&)el; YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, viewDef.GetRotation()); value.SetDouble(EXPR); return DgnDbStatus::Success;
    #define GET_POINT(EXPR) ViewDefinition3d& viewDef = (ViewDefinition3d&)el; value.SetPoint3d(EXPR); return DgnDbStatus::Success;
    #define SET_POINT(EXPR) ViewDefinition3d& viewDef = (ViewDefinition3d&)el; EXPR; return DgnDbStatus::Success;
    #define SET_DOUBLE(EXPR) ViewDefinition3d& viewDef = (ViewDefinition3d&)el; YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, viewDef.GetRotation()); EXPR; viewDef.SetRotation(angles.ToRotMatrix()); return DgnDbStatus::Success;

    #define VALIDATE_POINT3d_VALUE(VALUE) if (VALUE.IsNull() || !VALUE.IsPoint3d()) return DgnDbStatus::BadArg;
    #define TO_DOUBLE(VALUE,VALUEIN)                                                    \
            if (VALUEIN.IsNull() || VALUEIN.IsBoolean() || !VALUEIN.IsPrimitive())      \
                return DgnDbStatus::BadArg;                                             \
            ECN::ECValue VALUE(VALUEIN);                                                \
            if (!VALUE.ConvertToPrimitiveType(ECN::PRIMITIVETYPE_Double))               \
                return DgnDbStatus::BadArg;

    params.RegisterPropertyAccessors(layout, str_Origin(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT(viewDef.GetOrigin());
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT3d_VALUE(value);
            SET_POINT(viewDef.SetOrigin(value.GetPoint3d()));
            });

    params.RegisterPropertyAccessors(layout, str_Extents(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT(viewDef.GetExtents());
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT3d_VALUE(value);
            SET_POINT(viewDef.SetExtents(DVec3d::From(value.GetPoint3d())));
            });

    params.RegisterPropertyAccessors(layout, str_Yaw(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetYaw().Degrees());
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            SET_DOUBLE(angles.SetYaw(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, str_Pitch(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetPitch().Degrees());
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            SET_DOUBLE(angles.SetPitch(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, str_Roll(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetRoll().Degrees());
            },
        [](DgnElementR el, ECValueCR valueIn)
            {
            TO_DOUBLE(value, valueIn);
            SET_DOUBLE(angles.SetRoll(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, str_EyePoint(), 
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

    params.RegisterPropertyAccessors(layout, str_LensAngle(), 
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

    params.RegisterPropertyAccessors(layout, str_FocusDistance(), 
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

    params.RegisterPropertyAccessors(layout, str_IsCameraOn(), 
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void View2d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    #define GET_POINT2d(EXPR) ViewDefinition2d& viewDef = (ViewDefinition2d&)el; value.SetPoint2d(EXPR); return DgnDbStatus::Success;
    #define SET_POINT2d(EXPR) ViewDefinition2d& viewDef = (ViewDefinition2d&)el; EXPR; return DgnDbStatus::Success;

    #define VALIDATE_POINT2d_VALUE(VALUE) if (VALUE.IsNull() || !VALUE.IsPoint2d()) return DgnDbStatus::BadArg;
    #define TO_DOUBLE(VALUE,VALUEIN)                                                    \
            if (VALUEIN.IsNull() || VALUEIN.IsBoolean() || !VALUEIN.IsPrimitive())      \
                return DgnDbStatus::BadArg;                                             \
            ECN::ECValue VALUE(VALUEIN);                                                \
            if (!VALUE.ConvertToPrimitiveType(ECN::PRIMITIVETYPE_Double))               \
                return DgnDbStatus::BadArg;

    params.RegisterPropertyAccessors(layout, str_BaseModel(), 
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

    params.RegisterPropertyAccessors(layout, str_RotationAngle(), 
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

    params.RegisterPropertyAccessors(layout, str_Origin(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT2d(viewDef.GetOrigin2d());
            },
        [](DgnElementR el, ECValueCR value)
            {
            VALIDATE_POINT2d_VALUE(value);
            SET_POINT2d(viewDef.SetOrigin2d(value.GetPoint2d()));
            });

    params.RegisterPropertyAccessors(layout, str_Extents(), 
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
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialView::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, str_ModelSelector(), 
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

static DgnHost::Key s_displayMetricsRecorderKey;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2017
//---------------------------------------------------------------------------------------
IDisplayMetricsRecorder* IDisplayMetricsRecorder::GetRecorder()
    {
    // This is normally NULL. It is only used when playing back a DisplayBenchmark
    return static_cast<IDisplayMetricsRecorder*> (T_HOST.GetHostObject (s_displayMetricsRecorderKey));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2017
//---------------------------------------------------------------------------------------
void IDisplayMetricsRecorder::SetRecorder(IDisplayMetricsRecorder*recorder)
    {
    //  This should happen 0 or 1 times.
    BeAssert(GetRecorder() == nullptr);

    T_HOST.SetHostObject (s_displayMetricsRecorderKey, recorder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2017
//---------------------------------------------------------------------------------------
bool IDisplayMetricsRecorder::IsRecorderActive()
    {
    IDisplayMetricsRecorder*recorder = GetRecorder();
    return recorder ? recorder->_IsActive() : false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2017
//---------------------------------------------------------------------------------------
void DisplayMetricsRecorder::RecordQuerySceneComplete(double seconds, ViewController::QueryResults const& queryResults)
    {
    if (!IDisplayMetricsRecorder::IsRecorderActive())
        return;

    IDisplayMetricsRecorder*recorder = IDisplayMetricsRecorder::GetRecorder();
    Json::Value measurement(Json::objectValue);
    measurement["seconds"] = seconds;
    measurement["count"] = queryResults.GetCount();
    if (queryResults.m_incomplete)
        measurement["incomplete"] = 1;
        
    recorder->_RecordMeasurement("QuerySceneFinished", measurement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2017
//---------------------------------------------------------------------------------------
void DisplayMetricsRecorder::RecordCreateSceneComplete(double seconds, ViewController::Scene const & scene, bool aborted, bool complete)
    {
    if (!IDisplayMetricsRecorder::IsRecorderActive())
        return;

    IDisplayMetricsRecorder*recorder = IDisplayMetricsRecorder::GetRecorder();
    Json::Value measurement(Json::objectValue);
    measurement["seconds"] = seconds;
    if (aborted)
        measurement["aborted"] = 1;
    if (!complete)
        measurement["incomplete"] = 1;
        
    recorder->_RecordMeasurement("CreateSceneComplete", measurement);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

DrawingViewControllerPtr DrawingViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToDrawingViewP() : nullptr;}
Sheet::ViewControllerPtr SheetViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToSheetViewP() : nullptr;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateViewDefinition2dPtr TemplateViewDefinition2d::Create(DgnDbR db, Utf8StringCR name, CategorySelectorP categorySelectorIn, DisplayStyleP displayStyleIn)
    {
    DgnClassId classId = db.Domains().GetClassId(ViewElementHandler::TemplateView2d::GetHandler());
    if (!classId.IsValid())
        return nullptr;

    CategorySelectorP categorySelector = categorySelectorIn ? categorySelectorIn : new CategorySelector(db, "");
    DisplayStyleP displayStyle = displayStyleIn ? displayStyleIn : new DisplayStyle(db, "");

    TemplateViewDefinition2dPtr viewDef = new TemplateViewDefinition2d(CreateParams(db, classId, CreateCode(db, name), *categorySelector));
    viewDef->SetDisplayStyle(*displayStyle);

    if (nullptr == categorySelectorIn)
        {
        for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
            viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());
        }

    return viewDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateViewDefinition3dPtr TemplateViewDefinition3d::Create(DgnDbR db, Utf8StringCR name, CategorySelectorP categorySelectorIn, DisplayStyle3dP displayStyleIn)
    {
    DgnClassId classId = db.Domains().GetClassId(ViewElementHandler::TemplateView3d::GetHandler());
    if (!classId.IsValid())
        return nullptr;

    CategorySelectorP categorySelector = categorySelectorIn ? categorySelectorIn : new CategorySelector(db, "");
    DisplayStyle3dP displayStyle = displayStyleIn ? displayStyleIn : new DisplayStyle3d(db, "");

    TemplateViewDefinition3dPtr viewDef = new TemplateViewDefinition3d(CreateParams(db, classId, CreateCode(db, name), *categorySelector, *displayStyle));
    if (nullptr == categorySelectorIn)
        {
        for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
            viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());
        }

    return viewDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr TemplateViewDefinition2d::_SupplyController() const
    {
    return new TemplateViewController2d(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr TemplateViewDefinition3d::_SupplyController() const
    {
    return new TemplateViewController3d(*this);
    }
