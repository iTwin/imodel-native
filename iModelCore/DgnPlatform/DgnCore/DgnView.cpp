/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnView.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PROPNAME_Descr "Descr"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace ViewElementHandler
{
    HANDLER_DEFINE_MEMBERS(View);
    HANDLER_DEFINE_MEMBERS(View3d);
    HANDLER_DEFINE_MEMBERS(View2d);
    HANDLER_DEFINE_MEMBERS(DrawingView);
    HANDLER_DEFINE_MEMBERS(SheetView);
    HANDLER_DEFINE_MEMBERS(SpatialView);
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
    static constexpr Utf8CP str_TemplateModel() {return "TemplateModel";}
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
    auto& val = GetDetail(str_AspectSkew());
    return val.isNull() ? 1.0 : val.asDouble();
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
    Utf8String sql("SELECT ECInstanceId,CodeValue,IsPrivate," PROPNAME_Descr ",ECClassId FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition));

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
    stat = stmt.BindDouble(stmt.GetParameterIndex(str_LensAngle()), GetLensAngle());
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
    m_cameraDef.SetLensAngle(stmt.GetValueDouble(params.GetSelectIndex(str_LensAngle())));
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
        BeAssert(false);
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

    if (ColorDef::Black() == GetBackgroundColor())
        RemoveStyle(str_BackgroundColor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DisplayStyle::GetBackgroundColor() const
    {
    JsonValueCR val = GetStyle(str_BackgroundColor());
    return val.isNull() ? ColorDef::Black() : ColorDef(val.asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetBackgroundColor(ColorDef val)
    {
    SetStyle(str_BackgroundColor(), Json::Value(val.GetValue()));
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
            viewDef.SetDisplayStyle(*style->MakeCopy<Dgn::DisplayStyle>());
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

    params.RegisterPropertyAccessors(layout, str_Origin(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT(viewDef.GetOrigin());
            },
        [](DgnElementR el, ECValueCR value)
            {
            SET_POINT(viewDef.SetOrigin(value.GetPoint3d()));
            });

    params.RegisterPropertyAccessors(layout, str_Extents(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT(viewDef.GetExtents());
            },
        [](DgnElementR el, ECValueCR value)
            {
            SET_POINT(viewDef.SetExtents(DVec3d::From(value.GetPoint3d())));
            });

    params.RegisterPropertyAccessors(layout, str_Yaw(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetYaw().Degrees());
            },
        [](DgnElementR el, ECValueCR value)
            {
            SET_DOUBLE(angles.SetYaw(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, str_Pitch(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetPitch().Degrees());
            },
        [](DgnElementR el, ECValueCR value)
            {
            SET_DOUBLE(angles.SetPitch(AngleInDegrees::FromDegrees(value.GetDouble())));
            });

    params.RegisterPropertyAccessors(layout, str_Roll(),
        [](ECValueR value, DgnElementCR el)
            {
            GET_DOUBLE(angles.GetRoll().Degrees());
            },
        [](DgnElementR el, ECValueCR value)
            {
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
            if (!value.IsPoint3d())
                return DgnDbStatus::BadArg;

            ViewDefinition3dR viewDef = (ViewDefinition3dR)el;
            viewDef.SetEyePoint(value.GetPoint3d());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, str_LensAngle(), 
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition3dCR viewDef = (ViewDefinition3dCR)el;
            value.SetLong(viewDef.GetLensAngle());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsDouble())
                return DgnDbStatus::BadArg;

            ViewDefinition3dR viewDef = (ViewDefinition3dR)el;
            viewDef.SetLensAngle(value.GetDouble());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, str_FocusDistance(), 
        [](ECValueR value, DgnElementCR el)
            {
            ViewDefinition3dCR viewDef = (ViewDefinition3dCR)el;
            value.SetDouble(viewDef.GetFocusDistance());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsDouble())
                return DgnDbStatus::BadArg;

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

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void View2d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    #define GET_POINT2d(EXPR) ViewDefinition2d& viewDef = (ViewDefinition2d&)el; value.SetPoint2d(EXPR); return DgnDbStatus::Success;
    #define SET_POINT2d(EXPR) ViewDefinition2d& viewDef = (ViewDefinition2d&)el; EXPR; return DgnDbStatus::Success;

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
            value.SetLong(Angle::FromRadians(viewDef.GetRotAngle()).Degrees());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsDouble())
                return DgnDbStatus::BadArg;

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
            SET_POINT2d(viewDef.SetOrigin2d(value.GetPoint2d()));
            });

    params.RegisterPropertyAccessors(layout, str_Extents(), 
        [](ECValueR value, DgnElementCR el)
            {
            GET_POINT2d(viewDef.GetDelta2d());
            },
        [](DgnElementR el, ECValueCR value)
            {
            SET_POINT2d(viewDef.SetDelta2d(DVec2d::From(value.GetPoint2d())));
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TemplateView3d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, str_TemplateModel(), 
        [](ECValueR value, DgnElementCR el)
            {
            TemplateViewDefinition3dCR viewDef = (TemplateViewDefinition3dCR)el;
            value.SetLong(viewDef.GetTemplateModelId().GetValue());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsLong())
                return DgnDbStatus::BadArg;

            TemplateViewDefinition3d& viewDef = (TemplateViewDefinition3dR)el;
            viewDef.m_templateModelId = DgnModelId((uint64_t) value.GetLong());
            return DgnDbStatus::Success;
            });
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
    }


END_BENTLEY_DGNPLATFORM_NAMESPACE

DrawingViewControllerPtr DrawingViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToDrawingViewP() : nullptr;}
Sheet::ViewControllerPtr SheetViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToSheetViewP() : nullptr;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateViewDefinition3dPtr TemplateViewDefinition3d::Create(GeometricModel3dR templateModel, Utf8StringCR name, CategorySelectorP categorySelectorIn, DisplayStyle3dP displayStyleIn)
    {
    if (!templateModel.IsTemplate())
        return nullptr;

    DgnDbR db = templateModel.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(ViewElementHandler::TemplateView3d::GetHandler());
    if (!classId.IsValid())
        return nullptr;

    CategorySelectorP categorySelector = categorySelectorIn ? categorySelectorIn : new CategorySelector(db, "");
    DisplayStyle3dP displayStyle = displayStyleIn ? displayStyleIn : new DisplayStyle3d(db, "");

    TemplateViewDefinition3dPtr viewDef = new TemplateViewDefinition3d(CreateParams(db, classId, CreateCode(db, name), *categorySelector, *displayStyle));
    viewDef->m_templateModelId = templateModel.GetModelId();

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
DgnDbStatus TemplateViewDefinition3d::_ReadSelectParams(ECSqlStatement& statement, ECSqlClassParamsCR params)
    {
    m_templateModelId = statement.GetValueNavigation<DgnModelId>(params.GetSelectIndex(str_TemplateModel()));
    return T_Super::_ReadSelectParams(statement, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TemplateViewDefinition3d::_BindWriteParams(ECSqlStatement& statement, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(statement, forInsert);
    statement.BindNavigationValue(statement.GetParameterIndex(str_TemplateModel()), m_templateModelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TemplateViewDefinition3d::_CopyFrom(DgnElementCR element)
    {
    T_Super::_CopyFrom(element);
    TemplateViewDefinition3dCR other = (TemplateViewDefinition3dCR) element;
    m_templateModelId = other.m_templateModelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TemplateViewDefinition3d::_EqualState(ViewDefinitionR in)
    {
    TemplateViewDefinition3dCR other = (TemplateViewDefinition3dCR) in;
    if (m_templateModelId != other.m_templateModelId)
        return false;

    return T_Super::_EqualState(in);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr TemplateViewDefinition3d::_SupplyController() const
    {
    return new TemplateViewController3d(*this);
    }
