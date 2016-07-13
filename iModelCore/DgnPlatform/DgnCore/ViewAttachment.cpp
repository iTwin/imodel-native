/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewAttachment.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ViewAttachment.h>

#define PROP_ViewId         "ViewId"
#define PROP_Scale          "Scale"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(ViewAttachmentHandler);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentHandler::_TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) // *** WIP_AUTO_HANDLED_PROPERTIES
    {
    T_Super::_TEMPORARY_GetPropertyHandlingCustomAttributes(params);
    params.Add(PROP_ViewId);
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
        || ECSqlStatus::Success != stmt.BindDouble(stmt.GetParameterIndex(PROP_Scale), GetViewScale()))
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
DgnDbStatus ViewAttachment::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        m_data.m_viewId = stmt.GetValueId<DgnViewId>(params.GetSelectIndex(PROP_ViewId));
        m_data.m_scale = stmt.GetValueDouble(params.GetSelectIndex(PROP_Scale));

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
    return m_viewId.IsValid() && m_scale > 0.0;
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewAttachmentGeomCollector : IGeometryProcessor
{
private:
    ViewAttachmentR             m_attachment;
    DgnSubCategoryId            m_subCategory;
    GeometryBuilderPtr   m_builder;
    Transform                   m_curTransform;
    ViewContextP                m_viewContext;
    NonVisibleViewport          m_viewport;
    Transform                   m_initialTransform;

    virtual void _AnnounceContext(ViewContextR context) override { m_viewContext = &context; }
    virtual void _OutputGraphics(ViewContextR context) override
        {
        context.Attach(&m_viewport, DrawPurpose::CaptureGeometry);
        context.VisitAllViewElements(false, nullptr);
        context.Detach();
        }

    virtual void _AnnounceTransform(TransformCP tf) override;
    virtual void _AnnounceElemDisplayParams(ElemDisplayParamsCR params) override;
    virtual BentleyStatus _ProcessTextString(TextStringCR) override;
    virtual BentleyStatus _ProcessCurveVector(CurveVectorCR, bool) override;

    void FitView();
public:
    ViewAttachmentGeomCollector(ViewControllerR controller, ViewAttachmentR attach, DgnSubCategoryId subcat=DgnSubCategoryId());

    bool IsValid() const
        {
        return m_builder.IsValid() && m_subCategory.IsValid() && m_attachment.GetCategoryId() == DgnSubCategory::QueryCategoryId(m_subCategory, m_attachment.GetDgnDb());
        }

    DgnDbStatus SaveGeom()
        {
#define DEBUG_ORIGIN
#ifdef DEBUG_ORIGIN
        Placement2dCR placement = m_builder->GetPlacement2d();
        static const double s_lenFactor = 0.01;
        double len = placement.GetElementBox().GetRight() - placement.GetElementBox().GetLeft();
        len *= s_lenFactor;
        m_builder->Append(*ICurvePrimitive::CreateRectangle(0, 0, len, len, 0));
#endif
        return IsValid() && SUCCESS == m_builder->SetGeometryStreamAndPlacement(m_attachment) ? DgnDbStatus::Success : DgnDbStatus::BadElement;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewAttachmentGeomCollector::ViewAttachmentGeomCollector(ViewControllerR controller, ViewAttachmentR attach, DgnSubCategoryId subcat)
    : m_attachment(attach), m_subCategory(subcat), m_curTransform(Transform::FromIdentity()), m_viewContext(nullptr), m_viewport(controller)
    {
    if (!m_subCategory.IsValid())
        m_subCategory = DgnCategory::GetDefaultSubCategoryId(m_attachment.GetCategoryId());

    Placement2dCR placement = m_attachment.GetPlacement();
    m_builder = GeometryBuilder::Create(m_attachment, placement.GetOrigin(), placement.GetAngle());
    BeAssert(IsValid());

    // Fit the view to define the range of our geometry
    FitView();

    // Set up transform relative to element origin
    m_initialTransform = Transform::From(controller.GetRotation());

    auto scale = m_attachment.GetViewScale();
    m_initialTransform.ScaleMatrixColumns(scale, scale, scale);

    auto viewOrigin = controller.GetOrigin();
    DPoint3d translation = DPoint3d::FromXYZ(0,0,0);
    translation.Subtract(viewOrigin);

    Transform translate;
    translate.InitFrom(translation);
    m_initialTransform = Transform::FromProduct(m_initialTransform, translate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentGeomCollector::FitView()
    {
    DRange3d range;
    FitViewParams params;
    params.m_useScanRange = true;
    if (!IsValid() || SUCCESS != m_viewport.ComputeViewRange(range, params))
        return;

    ViewController::MarginPercent margin(0,0,0,0);
    m_viewport.GetViewControllerR().LookAtViewAlignedVolume(range, nullptr, &margin, true);
    m_viewport.SynchWithViewController(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentGeomCollector::_AnnounceTransform(TransformCP tf)
    {
    if (nullptr != tf)
        m_curTransform = Transform::FromProduct(m_initialTransform, *tf);
    else
        m_curTransform = m_initialTransform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentGeomCollector::_AnnounceElemDisplayParams(ElemDisplayParamsCR params)
    {
    if (nullptr != m_viewContext)
        {
        // Resolve symbology according to the view
        ElemDisplayParams resolved(params);
        resolved.Resolve(*m_viewContext);

        // Remap to the attachment's subcategory, retaining resolved values
        ElemDisplayParams remapped;
        remapped.SetCategoryId(m_attachment.GetCategoryId());
        remapped.SetSubCategoryId(m_subCategory);

        remapped.SetLineColor(resolved.GetLineColor());
        remapped.SetFillColor(resolved.GetFillColor());
        remapped.SetFillDisplay(resolved.GetFillDisplay());
        remapped.SetGeometryClass(resolved.GetGeometryClass());
        remapped.SetTransparency(resolved.GetTransparency());
        remapped.SetFillTransparency(resolved.GetFillTransparency());
        remapped.SetDisplayPriority(resolved.GetDisplayPriority());
        remapped.SetWeight(resolved.GetWeight());

        auto cpGradient = resolved.GetGradient();
        if (nullptr != cpGradient)
            {
            auto gradient = GradientSymb::Create();
            gradient->CopyFrom(*cpGradient);
            remapped.SetGradient(gradient.get());
            }

        auto cpPattern = resolved.GetPatternParams();
        if (nullptr != cpPattern)
            remapped.SetPatternParams(PatternParams::CreateFromExisting(*cpPattern).get());

        auto cpStyle = resolved.GetLineStyle();
        if (nullptr != cpStyle)
            remapped.SetLineStyle(LineStyleInfo::Create(cpStyle->GetStyleId(), cpStyle->GetStyleParams()).get());

        // Ignore: Material

        // push the effective symbology
        m_builder->Append(remapped);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewAttachmentGeomCollector::_ProcessTextString(TextStringCR text)
    {
    TextString tfText(text);
    tfText.ApplyTransform(m_curTransform);
    m_builder->Append(tfText);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewAttachmentGeomCollector::_ProcessCurveVector(CurveVectorCR cv, bool isFilled)
    {
    CurveVector tfCv(cv);
    tfCv.TransformInPlace(m_curTransform);
    m_builder->Append(tfCv);
    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::GenerateGeomStream(DgnSubCategoryId subcat)
    {
#ifdef ALLOW_CONTROLLER_OVERRIDES
    auto controller = ViewDefinition::LoadViewController(GetViewId(), GetDgnDb(), ViewDefinition::FillModels::Yes);
#else
    auto view = ViewDefinition::QueryView(GetViewId(), GetDgnDb());
    auto controller = view.IsValid() ? view->LoadViewController(false, ViewDefinition::FillModels::Yes) : nullptr;
#endif

    if (controller.IsNull())
        return DgnDbStatus::ViewNotFound;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    ViewAttachmentGeomCollector proc(*controller, *this, subcat);
    if (!proc.IsValid())
        return DgnDbStatus::BadElement;

    GeometryProcessor::Process(proc, GetDgnDb());

    return proc.SaveGeom();
#endif
    return DgnDbStatus::BadElement;
    }

