/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnView.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/QueryView.h>
#include <ECDb/JsonAdapter.h>

#define PROPNAME_Descr "Descr"
#define PROPNAME_Source "Source"

#define BIS_CLASS_ViewDefinition_PROPNAME_CategorySelector "CategorySelector"
#define BIS_CLASS_ViewDefinition3d_PROPNAME_ModelSelector "ModelSelector"
#define BIS_CLASS_ModelSelector_PROPNAME_ModelIds "ModelIds"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(DrawingViewDef);
    HANDLER_DEFINE_MEMBERS(SheetViewDef);
    HANDLER_DEFINE_MEMBERS(OrthographicViewDef);
    HANDLER_DEFINE_MEMBERS(CameraViewDef);
    HANDLER_DEFINE_MEMBERS(RedlineViewDef);
    HANDLER_DEFINE_MEMBERS(ModelSelectorDef);
    HANDLER_DEFINE_MEMBERS(CategorySelectorDef);
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

    if (!controller.IsValid() || BE_SQLITE_OK != controller->Load())
        return nullptr;

    if (FillModels::Yes == fillModels)
        controller->_FillModels();

    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr SpatialViewDefinition::_SupplyController() const
    {
    return new DgnQueryView(GetDgnDb(), GetViewId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr SheetViewDefinition::_SupplyController() const
    {
    return new SheetViewController(GetDgnDb(), GetViewId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr DrawingViewDefinition::_SupplyController() const
    {
    return new DrawingViewController(GetDgnDb(), GetViewId());
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
    auto stmt = entry.GetStatement();
    DgnDbP db = nullptr != stmt ? const_cast<DgnDbP>(static_cast<DgnDbCP>(stmt->GetECDb())) : nullptr; // ugh constness.
    if (nullptr == db)
        return false;

    auto entryClass = db->Schemas().GetECClass(entry.GetClassId());
    auto desiredClass = db->Schemas().GetECClass(T_Desired::QueryClassId(*db));
    return nullptr != entryClass && nullptr != desiredClass && entryClass->Is(desiredClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::Entry::IsSpatialView() const { return isEntryOfClass<SpatialViewDefinition>(*this); }
bool ViewDefinition::Entry::IsDrawingView() const { return isEntryOfClass<DrawingViewDefinition>(*this); }
bool ViewDefinition::Entry::IsSheetView() const { return isEntryOfClass<SheetViewDefinition>(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::SetModelId(DgnModelId mid)
    {
    bvector<DgnModelId> models;
    models.push_back(mid);
    return SetModelIds(models);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::SetModelIds(bvector<DgnModelId> const& models)
    {
    SetPropertyValue(BIS_CLASS_ModelSelector_PROPNAME_ModelIds, JsonUtils::IdsToJsonString(models).c_str());
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnModelId> ModelSelector::GetModelIds() const
    {
    return JsonUtils::IdsFromJsonString<DgnModelId>(GetPropertyValueString(BIS_CLASS_ModelSelector_PROPNAME_ModelIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_RemapIds(DgnImportContext& context)
    {
    auto ids = GetModelIds();
    for (size_t i=0; i<ids.size(); ++i)
        {
        ids[i] = context.FindModelId(ids[i]);
        }
    SetModelIds(ids);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::SetCategories(DgnCategoryIdSet const& categories)
    {
    // *** WIP_VIEW_DEFINITION: Delete existing CategoryInSelector with Source = this

    auto statement = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA(BIS_REL_CategorySelectorsReferToCategories) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");
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
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryIdSet CategorySelector::GetCategories() const
    {
    DgnCategoryIdSet categories;

    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_CategorySelectorsReferToCategories) " WHERE SourceECInstanceId");
    if (!statement.IsValid())
        return DgnCategoryIdSet();

    if (ECSqlStatus::Success != statement->BindId(1, GetElementId()))
        return DgnCategoryIdSet();

    while (BE_SQLITE_ROW == statement->Step())
        {
        categories.insert(statement->GetValueId<DgnCategoryId>(0));
        }

    return categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSelectorCPtr ViewDefinition3d::GetModelSelector() const
    {
    auto id = GetPropertyValueId<DgnElementId>(BIS_CLASS_ViewDefinition3d_PROPNAME_ModelSelector);
    if (!id.IsValid())
        return nullptr;
    return GetDgnDb().Elements().Get<ModelSelector>(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition3d::SetModelSelector(ModelSelectorCR modSel)
    {
    return SetPropertyValue(BIS_CLASS_ViewDefinition3d_PROPNAME_ModelSelector, modSel.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
CategorySelectorCPtr ViewDefinition::GetCategorySelector() const
    {
    auto id = GetPropertyValueId<DgnElementId>(BIS_CLASS_ViewDefinition_PROPNAME_CategorySelector);
    if (!id.IsValid())
        return nullptr;
    return GetDgnDb().Elements().Get<CategorySelector>(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::SetCategorySelector(CategorySelectorCR modSel)
    {
    return SetPropertyValue(BIS_CLASS_ViewDefinition_PROPNAME_CategorySelector, modSel.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_ToJson(Utf8StringR jsonStr) const
    {
    auto formatter = ECPropertyFormatter::Create();

    auto eclass = GetElementClass();
    for (auto prop : eclass->GetProperties())
        {
        ECN::ECValue value;
        if (DgnDbStatus::Success != GetPropertyValue(value, prop->GetName().c_str()))
            continue;
        Utf8String str;
        if (!formatter->FormattedStringFromECValue(str, value, *prop, false))
            continue;
        jsonStr.append(str);
        }
    }