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
#define PROP_OriginX        "OriginX"
#define PROP_OriginY        "OriginY"
#define PROP_DeltaX         "DeltaX"
#define PROP_DeltaY         "DeltaY"
#define PROP_Angle          "Angle"
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
    params.Add(PROP_OriginX);
    params.Add(PROP_OriginY);
    params.Add(PROP_DeltaX);
    params.Add(PROP_DeltaY);
    params.Add(PROP_Angle);
    params.Add(PROP_Scale);
    }
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool bindDouble(Utf8CP propName, double value, ECSqlStatement& stmt)
    {
    return ECSqlStatus::Success == stmt.BindDouble(stmt.GetParameterIndex(propName), value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool bindPoint(Utf8CP propX, Utf8CP propY, DPoint2dCR pt, ECSqlStatement& stmt)
    {
    return bindDouble(propX, pt.x, stmt) && bindDouble(propY, pt.y, stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindId(stmt.GetParameterIndex(PROP_ViewId), GetViewId())
        || !bindPoint(PROP_OriginX, PROP_OriginY, GetViewOrigin(), stmt) || !bindPoint(PROP_DeltaX, PROP_DeltaY, GetViewDelta(), stmt)
        || !bindDouble(PROP_Angle, GetViewAngle(), stmt) || !bindDouble(PROP_Scale, GetViewScale(), stmt))
        return DgnDbStatus::BadArg;

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
static void extractDouble(double& value, Utf8CP prop, ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    value = stmt.GetValueDouble(params.GetSelectIndex(prop));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void extractPoint(T& pt, Utf8CP propX, Utf8CP propY, ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    extractDouble(pt.x, propX, stmt, params);
    extractDouble(pt.y, propY, stmt, params);
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
        extractPoint(m_data.m_origin, PROP_OriginX, PROP_OriginY, stmt, params);
        extractPoint(m_data.m_delta, PROP_DeltaX, PROP_DeltaY, stmt, params);
        extractDouble(m_data.m_angle, PROP_Angle, stmt, params);
        extractDouble(m_data.m_scale, PROP_Scale, stmt, params);
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

