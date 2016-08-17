/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SectioningPhysicalViewController.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
SectioningViewController::SectioningViewController(SectionViewDe) : SpatialViewController(project, viewId)
    {
    m_hasAnalyzedCutPlanes = false;
    m_foremostCutPlaneIndex = 0;
    m_foremostCutPlane.Zero();
    m_cutPlaneCount = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::AnalyzeCutPlanes() const
    {
    if (m_hasAnalyzedCutPlanes)
        return;

    m_hasAnalyzedCutPlanes = true;

    size_t nCutPlanes = m_clip->GetPrimaryCutPlaneCount();
    if (nCutPlanes == 0)
        {
/*<=*/  return;
        }

    // Note on "forward" and viewing direction. When you define a section cut using a section marker, you indicate the direction in which
    //  you will look at the section cut. 
    //  As you stand and look at the section graphics, you may see some graphics beyond the section cut. They are call "forward graphics". 
    //  The forward graphics are largely obscured by the cut graphics.
    //  The graphics that are effectively behind your back are called "backward graphics". Backward graphics are typically turned off.
    //  Since you face the cut plane and the forward region beyond it, "closer to the eye" is the direction from cut plane toward the backward region.
    //  The clip plane normals point toward the forward section, that is, away from the eye.

    NullContext fakeViewContext;

    ViewContextR context(fakeViewContext);

    DPlane3d cutPlane, closestPlane;
    DVec3d closestPlaneXDir, closestPlaneYDir;
    ClipMask clipMask_unused;
    DRange2d clipRange_unused;
    bool forwardFacing_unused;

    // *** NEEDS WORK: I am assuming that plane 0 is a cut plane (and not a crop plane or a dogleg transition plane).
    m_clip->GetCuttingPlane(closestPlane, closestPlaneXDir, closestPlaneYDir, clipMask_unused, clipRange_unused, forwardFacing_unused, 0, context);

    uint32_t closestPlaneIndex = 0;
    m_cutPlaneCount = 1;
    for (size_t i=1; i < nCutPlanes; ++i)
        {
        DVec3d cutPlaneXDir, cutPlaneYDir;
        m_clip->GetCuttingPlane(cutPlane, cutPlaneXDir, cutPlaneYDir, clipMask_unused, clipRange_unused, forwardFacing_unused, (int)i, context);

        if (cutPlane.normal.IsParallelTo(closestPlane.normal))
            { 
            ++m_cutPlaneCount;

            //   FROM closestPlane   TO cutPlane                as measured along closestPlane's normal. This is away from the eye, so negate it.
            if (-closestPlane.Evaluate(cutPlane.origin) > 0)   
                {
                closestPlane = cutPlane;
                closestPlaneXDir = cutPlaneXDir;
                closestPlaneYDir = cutPlaneYDir;
                closestPlaneIndex = (uint32_t)i;
                }
            }
        }

    m_foremostCutPlaneIndex = closestPlaneIndex;
    m_foremostCutPlane = closestPlane;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool SectioningViewController::HasDogLeg() const
    {
    AnalyzeCutPlanes();
    return m_cutPlaneCount>1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d SectioningViewController::GetForemostCutPlane() const
    {
    AnalyzeCutPlanes();
    return m_foremostCutPlane;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr SectioningViewController::GetClipVectorInternal(ClipVolumePass pass) const
    {
    if (!m_clip.IsValid())
        return NULL;

#if defined (NEEDS_WORK_GROUND_PLANE)
    DRange3d range(_GetViewedExtents());

    ClipVectorPtr insideForward;
    m_clip->GetClipBoundary(insideForward, range, pass, /*displayCutGeometry*/true);
    if (!insideForward.IsValid())
        return insideForward;

    Transform   auxTransform;
    if (m_clip->GetAuxTransform(auxTransform, pass))
        insideForward->TransformInPlace(auxTransform);

    return insideForward;
#endif
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void SectioningViewController::_SaveToSettings() const
    {
    T_Super::_SaveToSettings();

#ifdef WIP_VIEW_DEFINITION
    if (m_clip.IsValid())
        IViewClipObject::Factory::ToJson(m_settings["dgn_SectioningView_clip"], *m_clip);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void SectioningViewController::_RestoreFromSettings() 
    {
    T_Super::_RestoreFromSettings();
#ifdef WIP_VIEW_DEFINITION
        m_clip = IViewClipObject::Factory::FromJson(m_settings["dgn_SectioningView_clip"]);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelPtr fillModel(DgnDbR project, DgnModelId mid)
    {
    DgnModelPtr model = project.Models().GetModel(mid);
    if (model == NULL)
        return NULL;

    model->FillModel();
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::DrawViewInternal(ViewContextR context)
    {
    for (auto modelId : m_viewedModels)
        {
        auto model = fillModel(context.GetDgnDb(), modelId);
        if (model != NULL)
            context.VisitDgnModel(model.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::_DrawView(ViewContextR context)
    {
    //  Draw the stuff inside the clip
    ClipVectorPtr insideForward = GetClipVectorInternal(m_pass = ClipVolumePass::InsideForward);
    if (!insideForward.IsValid())
        {
        DrawViewInternal(context);
        return;
        }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    context.PushClip(*insideForward);
    DrawViewInternal(context);
    context.PopTransformClip();
    context.PopClip();

    //  Draw the clip planes themselves
    m_clip->Draw(context);
    
    //  Draw the stuff outside of the clip
    context.PushClip(*GetClipVectorInternal(m_pass = ClipVolumePass::InsideBackward));
    SetOverrideGraphicParams(context);
    DrawViewInternal(context);
    context.PopTransformClip();
    context.PopClip();

    context.PushClip(*GetClipVectorInternal(m_pass = ClipVolumePass::Outside));
    SetOverrideGraphicParams(context);
    DrawViewInternal(context);
    context.PopTransformClip();
    context.PopClip();
    context.GetOverrideGraphicParams()->Clear();
    context.GetCurrentGraphicR().ActivateOverrideGraphicParams(context.GetOverrideGraphicParams());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr SectioningViewController::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    if (m_pass == ClipVolumePass::InsideForward)
        return T_Super::_StrokeGeometry(context, source, pixelSize);

    SetOverrideGraphicParams(context);
    return source.Stroke(context, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::SetOverrideGraphicParams(ViewContextR context) const
    {
    if (m_pass == ClipVolumePass::InsideForward)
        return;

    //  Everything outside of the inside-forward clip volume is grayed out and transparent.

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    ColorDef color = (m_pass == ClipVolumePass::InsideBackward)? ColorDef(0xcf00ffff) : ColorDef(0xcfffff00);

    OvrGraphicParamsP overrideMatSymb = context.GetOverrideGraphicParams();
    overrideMatSymb->Clear();
    overrideMatSymb->SetLineColor(color);
    overrideMatSymb->SetFillColor(color);
    overrideMatSymb->SetFlags(overrideMatSymb->GetFlags() | OvrGraphicParams::FLAGS_FillColorTransparency);
    overrideMatSymb->SetWidth(0);
    context.GetCurrentGraphicR().ActivateOverrideGraphicParams(overrideMatSymb);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr SectionDrawingViewController::GetProjectClipVector() const
    {
    auto sectionView = GetSectioningViewController();
    if (!sectionView.IsValid())
        return new ClipVector();
    return sectionView->GetInsideForwardClipVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SectionDrawingViewController::GetSectionHasDogLeg() const
    {
    auto sectionView = GetSectioningViewController();
    if (!sectionView.IsValid())
        return false;

    return sectionView->HasDogLeg();
    }
