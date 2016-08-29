/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnView.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/QueryView.h>

#define PROPNAME_Descr "Descr"
#define PROPNAME_Source "Source"

#define BIS_CLASS_ModelSelector_PROPNAME_ModelIds "ModelIds"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(DrawingViewDef);
    HANDLER_DEFINE_MEMBERS(SheetViewDef);
    HANDLER_DEFINE_MEMBERS(OrthographicViewDef);
    HANDLER_DEFINE_MEMBERS(CameraViewDef);
    HANDLER_DEFINE_MEMBERS(ModelSelectorDef);
    HANDLER_DEFINE_MEMBERS(CategorySelectorDef);
    HANDLER_DEFINE_MEMBERS(DisplayStyleDef);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

HANDLER_EXTENSION_DEFINE_MEMBERS(ViewControllerOverride)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition::CreateParams::CreateParams(DgnDbR db, DgnCode const& code, DgnClassId classId)
    : T_Super(db, DgnModel::DictionaryId(), classId, code)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId ViewDefinition::QueryViewId(DgnCode const& code, DgnDbR db)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return DgnViewId(elemId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(DgnViewId viewId, DgnDbR db, FillModels fill)
    {
    auto view = QueryView(viewId, db);
    return view.IsValid() ? view->LoadViewController(fill) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(FillModels fillModels) const
    {
    return LoadViewController(true, fillModels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(bool allowOverrides, FillModels fillModels) const
    {
    ViewControllerOverride* ovr = allowOverrides ? ViewControllerOverride::Cast(GetElementHandler()) : nullptr;
    ViewControllerPtr controller = ovr ? ovr->_SupplyController(*this) : nullptr;
    if (controller.IsNull())
        controller = _SupplyController();

    if (!controller.IsValid())
        return nullptr;

    controller->LoadFromDefinition();

    if (FillModels::Yes == fillModels)
        controller->_FillModels();

    return controller;
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
ViewControllerPtr CameraViewDefinition::_SupplyController() const
    {
    return new CameraViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr SheetViewDefinition::_SupplyController() const
    {
    return new SheetViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr DrawingViewDefinition::_SupplyController() const
    {
    return new DrawingViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_OnDelete() const
    {
    auto status = T_Super::_OnDelete();
    if (DgnDbStatus::Success != status)
        return status;

    // Foreign key constraint will be enforced for ViewAttachments which reference this view.
    // But we should delete them through element API.
    status = DeleteReferences();
    if (DgnDbStatus::Success != status)
        return status;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::DeleteReferences() const
    {
    CachedECSqlStatementPtr stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE ViewId=?");
    stmt->BindId(1, GetViewId());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto el = GetDgnDb().Elements().GetElement(stmt->GetValueId<DgnElementId>(0));
        if (el.IsValid())
            {
            DgnDbStatus stat = el->Delete();
            if (DgnDbStatus::Success != stat)
                return stat;
            }
        }

    return DgnDbStatus::Success;
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
    auto findViewsStmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA("ViewDefinition2d") " WHERE BaseModel=?");
    if (!findViewsStmt.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }
    findViewsStmt->BindId(1, mid);
    while (BE_SQLITE_ROW == findViewsStmt->Step())
        {
        auto viewId = findViewsStmt->GetValueId<DgnViewId>(0);
        auto view = QueryView(viewId, db);
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
bool ViewDefinition2d::IsBaseModelValid() const
    {
    auto mid = GetBaseModelId();
    if (!mid.IsValid())
        return false;
    auto model = GetDgnDb().Models().GetModel(mid);
    if (!model.IsValid())
        return false;
    return model->Is2dModel();
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
        SetBaseModelId(importer.FindModelId(GetBaseModelId()));
        BeAssert(IsBaseModelValid());

        // NOTE: We're not copying or remapping the settings from the db's properties table...
        }
    }

#ifdef TODO_CODES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_SetCode(DgnCode const& code)
    {
    return IsValidCode(code) ? T_Super::_SetCode(code) : DgnDbStatus::InvalidName;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition::Iterator::Iterator(DgnDbR db, Options const& options)
    {
    static const Utf8CP s_ecsql = "SELECT ECInstanceId,[CodeValue],[" PROPNAME_Source "]," PROPNAME_Descr ",GetECClassId() FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition);

    Utf8CP ecsql = s_ecsql;
    Utf8String customECSql;
    if (!options.IsEmpty())
        {
        customECSql = s_ecsql;
        customECSql.append(options.ToString());

        ecsql = customECSql.c_str();
        }

    Prepare(db, ecsql, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendSourceClause(Utf8StringR str, ViewDefinition::Iterator::Options::Source source)
    {
#ifdef ECSQL_SUPPORTS_BITWISE_OPS // It doesn't. NEEDSWORK.
    Utf8Char buf[0x20];
    BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(m_source));

    str.append("0 != (" PROPNAME_Source " & ");
    str.append(buf);
    str.append(1, ')');
#else
    // ECSql doesn't support bitwise operators...
    typedef ViewDefinition::Iterator::Options::Source Source;
    bool user { 0 != static_cast<uint8_t>(source & Source::User) };
    bool gen  { 0 != static_cast<uint8_t>(source & Source::Generated) };
    bool priv { 0 != static_cast<uint8_t>(source & Source::Private) };

    if (!user && !gen && !priv)
        return;

    // Parens in case we're preceded by an AND...
    str.append(1, '(');

    Utf8Char buf[3];
    if (user)
        {
        BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(Source::User));
        str.append(PROPNAME_Source "=").append(buf);
        }

    if (gen)
        {
        if (user)
            str.append(" OR ");

        BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(Source::Generated));
        str.append(PROPNAME_Source "=").append(buf);
        }

    if (priv)
        {
        if (user || gen)
            str.append(" OR ");

        BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(Source::Private));
        str.append(PROPNAME_Source "=").append(buf);
        }
    
    str.append(1, ')');
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ViewDefinition::Iterator::Options::ToString() const
    {
    Utf8String str;
    if (IsEmpty())
        return str;

    if (!m_customECSql.empty())
        {
        // mutually exclusive with the other options
        if (!m_customECSql.StartsWith(" "))
            str.append(1, ' ');

        str.append(m_customECSql);
        return str;
        }

    // WHERE
    if (Source::All != m_source)
        {
        str.append(str.empty() ? " WHERE " : " AND ");
        appendSourceClause(str, m_source);
        }

    // ORDER BY
    if (Order::Ascending == m_order)
        {
        str.append(" ORDER BY [CodeValue]");
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ViewDefinition::QueryCount(DgnDbR db, Iterator::Options const& opts)
    {
    static const Utf8CP s_ecsql = "SELECT count(*) FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition);
    
    Utf8CP ecsql = s_ecsql;
    Utf8String customECSql;
    if (!opts.IsEmpty())
        {
        customECSql = s_ecsql;
        customECSql.append(opts.ToString());
        ecsql = customECSql.c_str();
        }

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql);
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueInt(0) : 0;
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
bool ViewDefinition::Entry::IsOrthographicView() const { return isEntryOfClass<OrthographicViewDefinition>(*this); }
bool ViewDefinition::Entry::IsCameraView() const { return isEntryOfClass<CameraViewDefinition>(*this); }
bool ViewDefinition::Entry::IsSpatialView() const { return isEntryOfClass<SpatialViewDefinition>(*this); }
bool ViewDefinition::Entry::IsDrawingView() const { return isEntryOfClass<DrawingViewDefinition>(*this); }
bool ViewDefinition::Entry::IsSheetView() const { return isEntryOfClass<SheetViewDefinition>(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::SetCategoryIds(DgnCategoryIdSet const& categories)
    {
    if (!GetElementId().IsValid())
        return DgnDbStatus::MissingId;

    // *** WIP_VIEW_DEFINITION: Delete all existing CategorySelectorRefersToCategories instances with Source = this

    auto statement = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA(BIS_REL_CategorySelectorRefersToCategories) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    for (auto id : categories)
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
DgnCategoryIdSet CategorySelector::GetCategoryIds() const
    {
    if (!GetElementId().IsValid())
        return DgnCategoryIdSet();

    DgnCategoryIdSet categories;

    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_CategorySelectorRefersToCategories) " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnCategoryIdSet();
        }
    if (ECSqlStatus::Success != statement->BindId(1, GetElementId()))
        {
        BeAssert(false);
        return DgnCategoryIdSet();
        }
    while (BE_SQLITE_ROW == statement->Step())
        {
        categories.insert(statement->GetValueId<DgnCategoryId>(0));
        }

    return categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CategorySelector::ContainsCategoryId(DgnCategoryId cid) const
    {
    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_CategorySelectorRefersToCategories) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return false;
        }
    statement->BindId(1, GetElementId());
    statement->BindId(2, cid);
    return (BE_SQLITE_ROW == statement->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CategorySelector::GetSubCategoryOverrides(bmap<DgnSubCategoryId, DgnSubCategory::Override>& overrides) const
    {
    auto jsonStr = GetPropertyValueString("SubCategoryOverrides");
    if (0 == jsonStr.length())
        return;

    Json::Value arr(Json::arrayValue);
    if (!Json::Reader::Parse(jsonStr, arr))
        {
        BeAssert(false && "invalid json");
        return;
        }

    for (Json::ArrayIndex i = 0; i<arr.size(); ++i)
        {
        JsonValueCR val = arr[i];
        DgnSubCategoryId subCategoryId(val["SubCategoryId"].asUInt64());
        if (!subCategoryId.IsValid())
            {
            BeDataAssert(false && "SubCategoryOverride refers to missing SubCategory");
            continue;
            }
        overrides[subCategoryId] = DgnSubCategory::Override(val);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::SetSubCategoryOverrides(bmap<DgnSubCategoryId, DgnSubCategory::Override> const& overrides)
    {
    Json::Value arr(Json::arrayValue);
    for (auto const& ovr: overrides)
        {
        Json::Value entry(Json::objectValue);
        entry["SubCategoryId"] = ovr.first.GetValue();
        ovr.second.ToJson(entry);
        arr.append(entry);
        }
    return SetPropertyValue("SubCategoryOverrides", Json::FastWriter::ToString(arr).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::SetModelId(DgnModelId mid)
    {
    DgnModelIdSet models;
    models.insert(mid);
    return SetModelIds(models);
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
        if (selector->GetModelIds().size() == 1)
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
    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA("ViewDefinition3d") " WHERE ModelSelector=?");
    statement->BindId(1, GetElementId());
    if (BE_SQLITE_ROW == statement->Step())
        {
        auto view = ViewDefinition::QueryView(statement->GetValueId<DgnViewId>(0), GetDgnDb());
        if (view.IsValid())
            view->Delete();
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::SetModelIds(DgnModelIdSet const& Models)
    {
    if (!GetElementId().IsValid())
        return DgnDbStatus::MissingId;

    // *** WIP_VIEW_DEFINITION: Delete all existing ModelSelectorRefersToModels instances with Source = this

    auto statement = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    for (auto id : Models)
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
DgnModelIdSet ModelSelector::GetModelIds() const
    {
    DgnModelIdSet Models;

    if (!GetElementId().IsValid())
        {
        return DgnModelIdSet();
        }

    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnModelIdSet();
        }
    if (ECSqlStatus::Success != statement->BindId(1, GetElementId()))
        {
        BeAssert(false);
        return DgnModelIdSet();
        }
    while (BE_SQLITE_ROW == statement->Step())
        {
        Models.insert(statement->GetValueId<DgnModelId>(0));
        }

    return Models;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ModelSelector::ContainsModelId(DgnModelId mid) const
    {
    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return false;
        }
    statement->BindId(1, GetElementId());
    statement->BindId(2, mid);
    return (BE_SQLITE_ROW == statement->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition3d::_ViewsModel(DgnModelId mid) const 
    {
    auto modSel = GetDgnDb().Elements().Get<ModelSelector>(GetModelSelectorId());
    return modSel.IsValid()? modSel->ContainsModelId(mid): false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::ViewsCategory(DgnCategoryId cid) const
    {
    auto catSel = GetDgnDb().Elements().Get<CategorySelector>(GetCategorySelectorId());
    return catSel.IsValid()? catSel->ContainsCategoryId(cid): false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static YawPitchRollAngles yprFromStandardRotation(StandardView standardView)
    {
    RotMatrix rMatrix;
    if (!bsiRotMatrix_getStandardRotation(&rMatrix, static_cast<int>(standardView)))
        return YawPitchRollAngles();

    YawPitchRollAngles ypr;
    YawPitchRollAngles::TryFromRotMatrix(ypr, rMatrix);
    return ypr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus OrthographicViewDefinition::SetStandardViewDirection(StandardView standardView)
    {
    return SetViewDirection(yprFromStandardRotation(standardView));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraViewDefinition::SetStandardViewDirection(StandardView standardView)
    {
    return SetViewDirection(yprFromStandardRotation(standardView));
    }

