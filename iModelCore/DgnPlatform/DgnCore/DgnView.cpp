/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnView.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    VIEWDEF_HANDLER_DEFINE_MEMBERS(CameraViewDef);
    VIEWDEF_HANDLER_DEFINE_MEMBERS(DrawingViewDef);
    VIEWDEF_HANDLER_DEFINE_MEMBERS(SheetViewDef);
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
DgnDbStatus ViewDefinition::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
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
ViewDefinition::CreateParams::CreateParams(DgnDbR db, Code const& code, DgnClassId classId, Data const& data)
    : T_Super(db, classId, code), m_data(data)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
WIPViewId ViewDefinition::QueryViewId(Code const& code, DgnDbR db)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return WIPViewId(elemId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(WIPViewId viewId, DgnDbR db, FillModels fill)
    {
    auto view = QueryView(viewId, db);
    return view.IsValid() ? view->LoadViewController(fill) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(FillModels fillModels) const
    {
    ViewControllerOverride* ovr = ViewControllerOverride::Cast(GetElementHandler());
    ViewControllerPtr controller = ovr ? ovr->_SupplyController(*this) : nullptr;
    if (controller.IsNull())
        controller = _SupplyController();

    if (controller.IsValid())
        {
        controller->Load();
        if (FillModels::Yes == fillModels)
            controller->_FillModels();
        }

    return controller;
    }

// ###TODO: Remove old DgnViewId, rename WIPViewId
#define WIP_TO_VIEW_ID(WIPID) DgnViewId(WIPID . GetValueUnchecked())

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr CameraViewDefinition::_SupplyController() const
    {
    return new QueryViewController(GetDgnDb(), WIP_TO_VIEW_ID(GetViewId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr SheetViewDefinition::_SupplyController() const
    {
    return new SheetViewController(GetDgnDb(), WIP_TO_VIEW_ID(GetViewId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr DrawingViewDefinition::_SupplyController() const
    {
    return new DrawingViewController(GetDgnDb(), WIP_TO_VIEW_ID(GetViewId()));
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
    if (DgnDbStatus::Success == status)
        DeleteSettings();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);   // ###TODO anything?
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::QuerySettings(Utf8StringR settings, WIPViewId viewId, DgnDbR db)
    {
    return db.QueryProperty(settings, DgnViewProperty::Settings(), viewId.GetValue(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::SaveSettings(Utf8StringCR settings, WIPViewId viewId, DgnDbR db)
    {
    return db.SavePropertyString(DgnViewProperty::Settings(), settings, viewId.GetValue(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::DeleteSettings(WIPViewId viewId, DgnDbR db)
    {
    return db.DeleteProperty(DgnViewProperty::Settings(), viewId.GetValue(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition::Iterator ViewDefinition::MakeIterator(DgnDbR db)
    {
    static const Utf8CP s_ecsql = "SELECT ECInstanceId,Code," PROPNAME_Source "," PROPNAME_Descr "," PROPNAME_BaseModel ",GetECClassId() FROM " DGN_SCHEMA(DGN_CLASSNAME_ViewDefinition);

    Iterator iter;
    iter.Prepare(db, s_ecsql, 0);
    return iter;
    }

