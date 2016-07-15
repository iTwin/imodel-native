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
#define PROPNAME_BaseModel "BaseModelId"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define VIEWDEF_HANDLER_DEFINE_MEMBERS(CLASSNAME) \
    HANDLER_DEFINE_MEMBERS(CLASSNAME) \
    void CLASSNAME ::_GetClassParams(ECSqlClassParams& params) \
        { \
        T_Super::_GetClassParams(params); \
        ViewDefinition::AddClassParams(params); \
        }

namespace dgn_ElementHandler
{
    VIEWDEF_HANDLER_DEFINE_MEMBERS(SpatialViewDef);
    VIEWDEF_HANDLER_DEFINE_MEMBERS(DrawingViewDef);
    VIEWDEF_HANDLER_DEFINE_MEMBERS(SheetViewDef);
    HANDLER_DEFINE_MEMBERS(CameraViewDef);
    HANDLER_DEFINE_MEMBERS(RedlineViewDef);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

HANDLER_EXTENSION_DEFINE_MEMBERS(ViewControllerOverride)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::AddClassParams(ECSqlClassParams& params)
    {
    params.Add(PROPNAME_Descr);
    params.Add(PROPNAME_Source);
    params.Add(PROPNAME_BaseModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        Utf8String descr = stmt.GetValueText(params.GetSelectIndex(PROPNAME_Descr));
        auto source = static_cast<DgnViewSource>(stmt.GetValueInt(params.GetSelectIndex(PROPNAME_Source)));
        auto baseModelId = stmt.GetValueId<DgnModelId>(params.GetSelectIndex(PROPNAME_BaseModel));

        m_data.Init(baseModelId, source, descr);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Descr), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROPNAME_Source), static_cast<int32_t>(m_data.m_source))
        || ECSqlStatus::Success != stmt.BindId(stmt.GetParameterIndex(PROPNAME_BaseModel), m_data.m_baseModelId))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<ViewDefinitionCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition::CreateParams::CreateParams(DgnDbR db, DgnCode const& code, DgnClassId classId, Data const& data)
    : T_Super(db, DgnModel::DictionaryId(), classId, code), m_data(data)
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
bool ViewDefinition::IsBaseModelValid() const
    {
    auto mid = GetBaseModelId();
    auto model = mid.IsValid() ? GetDgnDb().Models().GetModel(mid) : nullptr;
    return model.IsValid() && _IsValidBaseModel(*model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_OnInsert()
    {
    auto status = T_Super::_OnInsert();
    if (DgnDbStatus::Success == status && !IsBaseModelValid())
        status = DgnDbStatus::BadModel;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_OnUpdate(DgnElementCR orig)
    {
    auto status = T_Super::_OnUpdate(orig);
    if (DgnDbStatus::Success == status && !IsBaseModelValid())
        status = DgnDbStatus::BadModel;

    return status;
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

    DeleteSettings();
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
void ViewDefinition::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    if (importer.IsBetweenDbs())
        {
        // We're not going to deep-copy the model in. We're expecting the user already copied it.
        m_data.m_baseModelId = importer.FindModelId(m_data.m_baseModelId);
        BeAssert(m_data.m_baseModelId.IsValid());

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
DbResult ViewDefinition::QuerySettings(Utf8StringR settings, DgnViewId viewId, DgnDbR db)
    {
    return db.QueryProperty(settings, DgnViewProperty::Settings(), viewId.GetValue(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::SaveSettings(Utf8StringCR settings, DgnViewId viewId, DgnDbR db)
    {
    return db.SavePropertyString(DgnViewProperty::Settings(), settings, viewId.GetValue(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::DeleteSettings(DgnViewId viewId, DgnDbR db)
    {
    return db.DeleteProperty(DgnViewProperty::Settings(), viewId.GetValue(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition::Iterator::Iterator(DgnDbR db, Options const& options)
    {
    static const Utf8CP s_ecsql = "SELECT ECInstanceId,[CodeValue],[" PROPNAME_Source "]," PROPNAME_BaseModel "," PROPNAME_Descr ",GetECClassId() FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition);

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
    Utf8Char buf[0x20];
    if (m_baseModelId.IsValid())
        {
        BeStringUtilities::FormatUInt64(buf, m_baseModelId.GetValue());
        str.append(" WHERE " PROPNAME_BaseModel " = ");
        str.append(buf);
        }

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


