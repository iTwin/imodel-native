/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewAttachment.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ViewAttachment.h>

#define PROP_ViewId         "ViewId"
#define PROP_Origin         "Origin"
#define PROP_Delta          "Delta"
#define PROP_Scale          "Scale"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(ViewAttachmentHandler);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentHandler::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(PROP_ViewId);
    params.Add(PROP_Origin);
    params.Add(PROP_Delta);
    params.Add(PROP_Scale);
    }
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindId(stmt.GetParameterIndex(PROP_ViewId), GetViewId())
        || ECSqlStatus::Success != stmt.BindDouble(stmt.GetParameterIndex(PROP_Scale), GetViewScale())
        || ECSqlStatus::Success != stmt.BindPoint3D(stmt.GetParameterIndex(PROP_Origin), GetViewOrigin())
        || ECSqlStatus::Success != stmt.BindPoint3D(stmt.GetParameterIndex(PROP_Delta), GetViewDelta()))
        return DgnDbStatus::BadArg;
    else
        return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        m_data.m_viewId = stmt.GetValueId<DgnViewId>(params.GetSelectIndex(PROP_ViewId));
        m_data.m_scale = stmt.GetValueDouble(params.GetSelectIndex(PROP_Scale));
        m_data.m_origin = stmt.GetValuePoint3D(params.GetSelectIndex(PROP_Origin));
        m_data.m_delta = DVec3d::From(stmt.GetValuePoint3D(params.GetSelectIndex(PROP_Delta)));

        BeAssert(m_data.IsValid());
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachment::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<ViewAttachmentCP>(&el);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_OnInsert()
    {
    return m_data.IsValid() ? T_Super::_OnInsert() : DgnDbStatus::BadElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::_OnUpdate(DgnElementCR el)
    {
    return m_data.IsValid() ? T_Super::_OnUpdate(el) : DgnDbStatus::BadElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewAttachment::Data::IsValid() const
    {
    return m_viewId.IsValid() && m_delta.MagnitudeSquared() > 0.0 && m_scale > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachment::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    if (importer.IsBetweenDbs())
        {
        m_data.m_viewId = DgnViewId(importer.FindElementId(m_data.m_viewId).GetValueUnchecked());
        BeAssert(m_data.m_viewId.IsValid() && "Cannot copy a ViewAttachment without first copying the view...");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::GenerateGeomStream()
    {
    // ###TODO: This is kinda the important part...
    return DgnDbStatus::BadElement;
    }

