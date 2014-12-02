/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SectioningPhysicalViewController.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
SectioningViewController::SectioningViewController (DgnProjectR project, DgnViewId viewId) : PhysicalViewController (project, viewId)
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

    DynamicViewSettings fakeSettings;
    NullContext fakeViewContext;

    ViewContextR context (fakeViewContext);
    DynamicViewSettingsR settings (fakeSettings);

    DPlane3d cutPlane, closestPlane;
    DVec3d closestPlaneXDir, closestPlaneYDir;
    ClipMask clipMask_unused;
    DRange2d clipRange_unused;
    bool forwardFacing_unused;

    // *** NEEDS WORK: I am assuming that plane 0 is a cut plane (and not a crop plane or a dogleg transition plane).
    m_clip->GetCuttingPlane (closestPlane, closestPlaneXDir, closestPlaneYDir, clipMask_unused, clipRange_unused, forwardFacing_unused, 0, context, settings);

    UInt32 closestPlaneIndex = 0;
    m_cutPlaneCount = 1;
    for (size_t i=1; i < nCutPlanes; ++i)
        {
        DVec3d cutPlaneXDir, cutPlaneYDir;
        m_clip->GetCuttingPlane (cutPlane, cutPlaneXDir, cutPlaneYDir, clipMask_unused, clipRange_unused, forwardFacing_unused, (int)i, context, settings);

        if (cutPlane.normal.IsParallelTo (closestPlane.normal))
            { 
            ++m_cutPlaneCount;

            //   FROM closestPlane   TO cutPlane                as measured along closestPlane's normal. This is away from the eye, so negate it.
            if (-closestPlane.Evaluate (cutPlane.origin) > 0)   
                {
                closestPlane = cutPlane;
                closestPlaneXDir = cutPlaneXDir;
                closestPlaneYDir = cutPlaneYDir;
                closestPlaneIndex = (UInt32)i;
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
ClipVectorPtr SectioningViewController::GetClipVectorInternal (ClipVolumePass pass) const
    {
    if (!m_clip.IsValid())
        return NULL;

    DynamicViewSettings fakeSettings;   // WIP_DV
    DRange3d range (_GetProjectExtents());

    ClipVectorPtr insideForward;
    m_clip->GetClipBoundary (insideForward, range, pass, &fakeSettings, /*displayCutGeometry*/true);
    if (!insideForward.IsValid())
        return insideForward;

    Transform   auxTransform;
    if (m_clip->GetAuxTransform (auxTransform, pass, fakeSettings))
        insideForward->TransformInPlace (auxTransform);

    return insideForward;
    }

ClipVectorPtr SectioningViewController::_GetClipVector() const {return ClipVector::Create();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void SectioningViewController::_SaveToSettings (JsonValueR val) const
    {
    T_Super::_SaveToSettings(val);

    if (m_clip.IsValid())
        IViewClipObject::Factory::ToJson (val["dgn_SectioningView_clip"], *m_clip);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void SectioningViewController::_RestoreFromSettings (JsonValueCR val) 
    {
    T_Super::_RestoreFromSettings(val);
    m_clip = IViewClipObject::Factory::FromJson (val["dgn_SectioningView_clip"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModel* fillModel (DgnProjectR project, DgnModelId mid)
    {
    auto model = project.Models().GetModelById (mid);
    if (model == NULL)
        return NULL;

    model->FillModel();
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::DrawViewInternal (ViewContextR context)
    {
    for (auto modelId : m_viewedModels)
        {
        auto model = fillModel (context.GetDgnProject(), modelId);
        if (model != NULL)
            context.VisitDgnModel (model);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::_DrawView (ViewContextR context)
    {
    //  Draw the stuff inside the clip
    ClipVectorPtr insideForward = GetClipVectorInternal (m_pass = ClipVolumePass::InsideForward);
    if (!insideForward.IsValid())
        {
        DrawViewInternal (context);
        return;
        }

    context.PushClip(*insideForward);
    DrawViewInternal (context);
    context.PopTransformClip();

    //  Draw the clip planes themselves
    m_clip->Draw (context);
    
    //  Draw the stuff outside of the clip
    context.PushClip(*GetClipVectorInternal (m_pass = ClipVolumePass::InsideBackward));
    SetOverrideMatSymb (context);
    DrawViewInternal (context);
    context.PopTransformClip();

    context.PushClip(*GetClipVectorInternal (m_pass = ClipVolumePass::Outside));
    SetOverrideMatSymb (context);
    DrawViewInternal (context);
    context.PopTransformClip();

    context.GetOverrideMatSymb()->Clear();
    context.ActivateOverrideMatSymb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::_DrawElement(ViewContextR context, ElementHandleCR elIter)
    {
    if (m_pass == ClipVolumePass::InsideForward)
        T_Super::_DrawElement (context, elIter);
    else
        {
        SetOverrideMatSymb (context);
        elIter.GetDisplayHandler()->Draw (elIter, context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SectioningViewController::SetOverrideMatSymb (ViewContextR context) const
    {
    if (m_pass == ClipVolumePass::InsideForward)
        return;

    //  Everything outside of the inside-forward clip volume is grayed out and transparent.

    UInt32 color = (m_pass == ClipVolumePass::InsideBackward)? 0xcf00ffff: 0xcfffff00;

    OvrMatSymbP overrideMatSymb = context.GetOverrideMatSymb();
    overrideMatSymb->Clear();
    overrideMatSymb->SetLineColorTBGR (color);
    overrideMatSymb->SetFillColorTBGR (color);
    overrideMatSymb->SetFlags (overrideMatSymb->GetFlags() | MATSYMB_OVERRIDE_FillColorTransparency);
    overrideMatSymb->SetWidth (0);
    context.ActivateOverrideMatSymb ();
    }
