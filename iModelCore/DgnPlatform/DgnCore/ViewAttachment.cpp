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
static void fixScanCriteriaHACKHACKHACK(ViewContextR context)
    {
    struct FixMePlease : NullContext
    {
        void FixMe() { m_setupScan = true; }
    };

    auto fixMe = reinterpret_cast<FixMePlease*>(&context);
    fixMe->FixMe();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewAttachmentGeomCollector : IElementGraphicsProcessor
{
private:
    ViewAttachmentR             m_attachment;
    ViewControllerR             m_controller;
    DgnSubCategoryId            m_subCategory;
    ElementGeometryBuilderPtr   m_builder;
    Transform                   m_transform;
    ViewContextP                m_viewContext;
    NonVisibleViewport          m_viewport;

    virtual void _AnnounceContext(ViewContextR context) override { m_viewContext = &context; }
    virtual void _OutputGraphics(ViewContextR context) override
        {
        fixScanCriteriaHACKHACKHACK(context);
        context.Attach(&m_viewport, DrawPurpose::CaptureGeometry);
        context.SetScanReturn(); // ###TODO wtf there is no documentation regarding attaching a viewport, everything breaks, why.
        m_controller.DrawView(context);
        context.Detach();
        }

    virtual void _AnnounceTransform(TransformCP tf) override;
    virtual void _AnnounceElemDisplayParams(ElemDisplayParamsCR params) override;
    virtual BentleyStatus _ProcessTextString(TextStringCR) override;
    virtual BentleyStatus _ProcessCurveVector(CurveVectorCR, bool) override;
public:
    ViewAttachmentGeomCollector(ViewControllerR controller, ViewAttachmentR attach, DgnSubCategoryId subcat=DgnSubCategoryId())
        : m_attachment(attach), m_controller(controller), m_subCategory(subcat), m_transform(Transform::FromIdentity()), m_viewContext(nullptr), m_viewport(controller)
        {
        if (!m_subCategory.IsValid())
            m_subCategory = DgnCategory::GetDefaultSubCategoryId(m_attachment.GetCategoryId());

        m_builder = ElementGeometryBuilder::Create(m_attachment);
        BeAssert(IsValid());

        controller.SetOrigin(m_attachment.GetViewOrigin());
        controller.SetDelta(m_attachment.GetViewDelta());

        // ###TODO: Scaling transform...
        }

    bool IsValid() const { return m_builder.IsValid() && m_subCategory.IsValid(); }

    DgnDbStatus SaveGeom()
        {
        return IsValid() && SUCCESS == m_builder->SetGeomStreamAndPlacement(m_attachment) ? DgnDbStatus::Success : DgnDbStatus::BadElement;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentGeomCollector::_AnnounceTransform(TransformCP tf)
    {
    if (nullptr != tf)
        m_transform = *tf;
    else
        m_transform.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentGeomCollector::_AnnounceElemDisplayParams(ElemDisplayParamsCR params)
    {
    if (nullptr != m_viewContext)
        {
        ElemDisplayParams resolved(params);
        resolved.Resolve(*m_viewContext);
        resolved.SetCategoryId(m_attachment.GetCategoryId());
        resolved.SetSubCategoryId(m_subCategory);
        m_builder->Append(resolved);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewAttachmentGeomCollector::_ProcessTextString(TextStringCR text)
    {
    TextString tfText(text);
    tfText.ApplyTransform(m_transform);
    m_builder->Append(tfText);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewAttachmentGeomCollector::_ProcessCurveVector(CurveVectorCR cv, bool isFilled)
    {
    CurveVector tfCv(cv);
    tfCv.TransformInPlace(m_transform);
    m_builder->Append(tfCv);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::GenerateGeomStream()
    {
    auto controller = ViewDefinition::LoadViewController(GetViewId(), GetDgnDb(), ViewDefinition::FillModels::No);
    if (controller.IsNull())
        return DgnDbStatus::ViewNotFound;

    controller->Load();
    ViewAttachmentGeomCollector proc(*controller, *this);
    if (!proc.IsValid())
        return DgnDbStatus::BadElement;

    ElementGraphicsOutput::Process(proc, GetDgnDb());

    return proc.SaveGeom();
    }

